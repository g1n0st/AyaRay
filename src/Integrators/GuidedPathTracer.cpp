#include <Integrators/GuidedPathTracer.h>
#include <Filters/GaussianFilter.h>

namespace Aya {
	void GuidedPathTracerIntegrator::resetSDTree() {
		m_sdTree->refine((size_t)(std::sqrtf(std::powf(2, m_iter) * m_sppPerPass / 4) * m_sTreeThreshold), m_sdTreeMaxMemory);
		m_sdTree->forEachDTreeWrapperParallel([this](DTreeWrapper *dTree) { dTree->reset(20, m_dTreeThreshold); });
	}

	void GuidedPathTracerIntegrator::buildSDTree() {
		// Build distributions
		m_sdTree->forEachDTreeWrapperParallel([](DTreeWrapper* dTree) { dTree->build(); });

		// Gather statistics
		// no info system, ignored temporarily

		m_isBuilt = true;
	}

	bool GuidedPathTracerIntegrator::doNeeWithSpp(int spp) {
		switch (m_nee) {
		case Never:
			return false;
		case Kickstart:
			return spp < 128;
		default:
			return true;
		}
	}

	void GuidedPathTracerIntegrator::render(const Scene *scene, const Camera *camera, Sampler *sampler, Film *film) {
		m_sdTree = std::unique_ptr<STree>(new STree(scene->worldBound()));
		m_iter = 0;
		m_isFinalIter = false;

		m_varianceBuffer = std::shared_ptr<Film>(new Film(m_task.getX(), m_task.getY(), new GaussianFilter()));
		m_image = std::shared_ptr<Film>(new Film(m_task.getX(), m_task.getY(), new GaussianFilter()));
		m_squaredImage = std::shared_ptr<Film>(new Film(m_task.getX(), m_task.getY(), new GaussianFilter()));

		m_images.clear();
		m_variances.clear();

		{
			m_passesRendered = 0;

			size_t sample_count = static_cast<size_t>(m_spp);

			int n_passes = (int)std::ceilf(sample_count / (float)m_sppPerPass);
			sample_count = n_passes * m_sppPerPass;

			float currentVarAtEnd = std::numeric_limits<float>::infinity();

			while (!m_task.aborted() && m_passesRendered < n_passes) {
				const int spp_rendered = m_passesRendered * m_sppPerPass;
				m_doNee = doNeeWithSpp(spp_rendered);

				int remaining_passes = n_passes - m_passesRendered;
				int current_passes = Min(remaining_passes, 1 << m_iter);

				// If the next iteration does not manage to double the number of passes once more
			   // then it would be unwise to throw away the current iteration. Instead, extend
			   // the current iteration to the end.
			   // This condition can also be interpreted as: the last iteration must always use
			   // at _least_ half the total sample budget.
				if (remaining_passes - current_passes < 2 * current_passes) {
					current_passes = remaining_passes;
				}

				m_isFinalIter = current_passes >= remaining_passes;

				film->clear();
				resetSDTree();

				float variance;
				if (!renderPasses(variance, current_passes, scene, camera, sampler, film)) {
					break;
				}

				const float lastVarAtEnd = currentVarAtEnd;
				currentVarAtEnd = current_passes * variance / remaining_passes;

				remaining_passes -= current_passes;
				if (m_sampleCombination == SampleCombination::DiscardWithAutomaticBudget && remaining_passes > 0 && (
					// if there is any time remaining we want to keep going if
					// either will have less time next iter
					remaining_passes < current_passes ||
					// or, according to the convergence behavior, we're better off if we keep going
					// (we only trust the variance if we drew enough samples for it to be a reliable estimate,
					// captured by an arbitraty threshold).
					(spp_rendered > 256 && currentVarAtEnd > lastVarAtEnd)
					)) {
					m_isFinalIter = true;
					if (!renderPasses(variance, remaining_passes, scene, camera, sampler, film)) {
						break;
					}
				}
				buildSDTree();

				++m_iter;
				m_passesRenderedThisIter = 0;
			}
		}

		if (m_task.aborted()) {
			return;
		}

		if (m_sampleCombination == SampleCombination::InverseVariance) {
			// Combine the last 4 images according to their inverse variance
			film->clear();
			size_t begin = m_images.size() - Min(m_images.size(), (size_t)4);

			float total_weight = 0.f;
			for (size_t i = begin; i < m_variances.size(); ++i) {
				total_weight += 1.f / m_variances[i];
			}

			for (size_t i = begin; i < m_variances.size(); ++i) {
				film->addFilm(&(*m_images[i]), 1.f / m_variances[i] / total_weight);
			}

			film->updateDisplay();
		}
	}

	bool GuidedPathTracerIntegrator::renderPasses(float &variance, int num_passes, const Scene *scene, const Camera *camera, Sampler *sampler, Film *film) {
		m_image->clear();
		m_squaredImage->clear();

		int passesRenderedLocal = { 0 };
		int tiles_count = m_task.getTilesCount();

		for (int pass = 0; pass < num_passes; ++pass) {
			++m_passesRendered;
			++m_passesRenderedThisIter;
			++passesRenderedLocal;

			if (m_task.aborted()) {
				return false;
			}

			for (int spp = 0; spp < m_sppPerPass; ++spp) {
				concurrency::parallel_for(0, tiles_count, [&](int i) {
					//for (int i = 0; i < tiles_count; i++) {
					const RenderTile& tile = m_task.getTile(i);

					std::unique_ptr<Sampler> tile_sampler(sampler->clone(((m_passesRendered - 1) * m_sppPerPass + spp) * tiles_count + i));

					RNG rng;
					MemoryPool memory;

					for (int y = tile.min_y; y < tile.max_y; ++y) {
						for (int x = tile.min_x; x < tile.max_x; ++x) {
							if (m_task.aborted())
								return;

							tile_sampler->startPixel(x, y);
							CameraSample cam_sample;
							tile_sampler->generateSamples(x, y, &cam_sample, rng);
							cam_sample.image_x += x;
							cam_sample.image_y += y;

							RayDifferential ray;
							Spectrum L(0.f);
							if (camera->generateRayDifferential(cam_sample, &ray)) {
								L = li(ray, scene, tile_sampler.get(), rng, memory);
							}

							film->addSample(cam_sample.image_x, cam_sample.image_y, L);
							m_image->addSample(cam_sample.image_x, cam_sample.image_y, L);
							m_squaredImage->addSample(cam_sample.image_x, cam_sample.image_y, L * L);
							memory.freeAll();
						}
					}
					//}
				});

				if (m_task.aborted()) {
					return false;
				}

				film->addSampleCount();
				film->updateDisplay();
			}
		}

		variance = 0.f;
		std::shared_ptr<Film> image(new Film(m_task.getX(), m_task.getY(), nullptr));
		image->addFilm(&(*m_image));

		if (m_sampleCombination == SampleCombination::InverseVariance) {
			// Record all previously rendered iterations such that later on all iterations can be
			// combined by weighting them by their estimated inverse pixel variance.
			m_images.push_back(image);
		}

		m_varianceBuffer->clear();

		int N = passesRenderedLocal * m_sppPerPass;

		Vector2i size = m_squaredImage->getSize();
		for (int x = 0; x < size.x; ++x)
			for (int y = 0; y < size.y; ++y) {
				Spectrum pixel = m_image->getPixel(x, y);
				Spectrum local_var = m_squaredImage->getPixel(x, y) - pixel * pixel / (float)N;
				m_varianceBuffer->setPixel(x, y, local_var);
				// The local variance is clamped such that fireflies don't cause crazily unstable estimates.
				variance += Min(local_var.luminance(), 10000.0f);
			}

		variance /= (float)size.x * size.y * (N - 1);

		if (m_sampleCombination == SampleCombination::InverseVariance) {
			// Record estimated mean pixel variance for later use in weighting of all images.
			m_variances.push_back(variance);
		}

		return true;
	}

	Spectrum GuidedPathTracerIntegrator::li(const RayDifferential &ray, const Scene *scene, Sampler *sampler, RNG &rng, MemoryPool &memory) const {
		struct Vertex {
			DTreeWrapper *dTree;
			Vector3 voxel_size;
			Ray ray;

			Spectrum throughput;
			Spectrum bsdf_val;

			Spectrum radiance;

			float wo_pdf, bsdf_pdf, dTree_pdf;
			bool is_delta;

			void record(const Spectrum &r) {
				radiance += r;
			}

			void commit(STree &sdTree, float statistical_weight,
				SpatialFilter spatial_filter, DirectionalFilter directional_filter,
				BsdfSamplingFractionLoss sampling_loss, Sampler *sampler) {
				if (!(wo_pdf > 0.f) || !radiance.isValid() || !bsdf_val.isValid()) {
					return;
				}

				Spectrum local_radiance(0.f);
				for (int i = 0; i < Spectrum::nSamples; ++i) {
					if (throughput[i] * wo_pdf > 1e-4f) {
						local_radiance[i] = radiance[i] / throughput[i];
					}
				}
				Spectrum product = local_radiance * bsdf_val;

				float radiance_average = 0.f, product_average = 0.f;
				for (int i = 0; i < Spectrum::nSamples; ++i) {
					radiance_average += local_radiance[i];
					product_average += product[i];
				}
				radiance_average /= (float)Spectrum::nSamples;
				product_average /= (float)Spectrum::nSamples;
				DTreeRecord rec{ ray.m_dir, radiance_average, product_average, wo_pdf, bsdf_pdf, dTree_pdf, statistical_weight, is_delta };
				switch (spatial_filter) {
				case SpatialFilter::Nearest:
					dTree->record(rec, directional_filter, sampling_loss);
					break;
				case SpatialFilter::StochasticBox:
					{
						DTreeWrapper *splat_dTree = dTree;

						// Jitter the actual position within the
						// filter box to perform stochastic filtering.
						Vector3 offset = voxel_size;
						offset.x *= sampler->get1D() - .5f;
						offset.y *= sampler->get1D() - .5f;
						offset.z *= sampler->get1D() - .5f;

						Point3 origin = sdTree.aabb().clip(ray.m_ori + offset);
						splat_dTree = sdTree.dTreeWrapper(origin);
						if (splat_dTree) {
							splat_dTree->record(rec, directional_filter, sampling_loss);
						}
						break;
					}
				case SpatialFilter::Box:
					sdTree.record(ray.m_ori, voxel_size, rec, directional_filter, sampling_loss);
					break;
				}
			}
		};

		static const int MAX_NUM_VERTICES = 32;
		std::array<Vertex, MAX_NUM_VERTICES> vertices;

		SurfaceIntersection local_isect;
		MediumIntersection medium_isect;
		Spectrum Li(0.f);
		float eta = 1.f;

		/* Perform the first ray intersection (or ignore if the
		intersection has already been provided). */
		scene->intersect(ray, &local_isect);
		
		Spectrum throughput(1.f);
		bool scattered = false;

		int nVertices = 0;

		auto recordRadiance = [&](Spectrum radiance) {
			Li += radiance;
			for (int i = 0; i < nVertices; ++i) {
				vertices[i].record(radiance);
			}
		};

		for (int depth = 0; depth <= m_maxDepth || m_maxDepth < 0; ++depth) {
			// ...
		}

		return Li;
	}
}