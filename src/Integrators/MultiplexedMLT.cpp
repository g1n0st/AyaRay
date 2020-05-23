#include "MultiplexedMLT.h"
#include "BidirectionalPathTracing.h"

namespace Aya {
	float MetropolisSampler::get1D() {
		const int idx = getNextIndex();
		mutate(idx);

		return m_samples[idx].value;
	}
	Vector2f MetropolisSampler::get2D() {
		return Vector2f(get1D(), get1D());
	}
	Sample MetropolisSampler::getSample() {
		Sample ret;
		ret.u = get1D();
		ret.v = get1D();
		ret.w = get1D();

		return ret;
	}
	void MetropolisSampler::generateSamples(const int pixel_x, const int pixel_y, CameraSample *samples, RNG &rng) {
		samples->lens_u = get1D();
		samples->lens_v = get1D();
		samples->time   = get1D();
	}

	UniquePtr<Sampler> MetropolisSampler::clone(const int seed) const {
		return MakeUnique<MetropolisSampler>(m_sigma, m_large_step_prob, seed);
	}
	UniquePtr<Sampler> MetropolisSampler::deepClone() const {
		return MakeUnique<MetropolisSampler>(m_sigma, m_large_step_prob,
			m_sample_idx, m_stream_idx,
			m_large_step_time, m_time, m_large_step, m_rng, 
			m_samples);
	}

	void MetropolisSampler::startIteration() {
		m_large_step = m_rng.drand48() < m_large_step_prob;
		m_time++;
	}

	void MetropolisSampler::accept() {
		if (m_large_step)
			m_large_step_time = m_time;
	}
	void MetropolisSampler::reject() {
		for (auto &it : m_samples) {
			if (it.modify == m_time)
				it.restore();
		}
		m_time--;
	}

	void MetropolisSampler::mutate(const int idx) {
		if (idx >= m_samples.size())
			m_samples.resize(idx + 1);
		
		PrimarySample &sample = m_samples[idx];
		if (sample.modify < m_large_step_time) {
			sample.modify = m_large_step_time;
			sample.value = m_rng.drand48();
		}

		sample.backup();			// save state
		if (m_large_step) {		// large step
			sample.value = m_rng.drand48();
		}
		else {					// small step
			int num_small_steps = m_time - sample.modify;

			// Sample the standard normal distribution
			const float sqrt2 = 1.41421356237309504880f;
			float normal_sample = sqrt2 * ErfInv(2.f * m_rng.drand48() - 1.f);

			// The product of multiple mutations can be combined into an exponentially addition form
			float eff_sigma = m_sigma * Sqrt(float(num_small_steps));
			sample.value += normal_sample * eff_sigma;
			sample.value -= FloorToInt(sample.value);
		}

		sample.modify = m_time;
	}

	void MultiplexMLTIntegrator::render(const Scene *scene, const Camera *camera, Sampler *sampler, Film *film) const {
		// Generate bootstrap samples and compute normalization constant b
		int num_bootstrap_samples = m_num_bootstrap * (int(m_max_depth) + 1);
		std::vector<float> bootstrap_weights(num_bootstrap_samples, 0.f);

		concurrency::parallel_for(0, m_num_bootstrap, [&](int i) {
		//for (int i = 0; i < m_num_bootstrap; i++) {
			RNG rng(i);
			MemoryPool memory;

			for (int depth = 0; depth <= int(m_max_depth); ++depth) {
				if (m_task.aborted())
					return;

				int seed = depth + i * (m_max_depth + 1);
				MetropolisSampler sampler(m_sigma, m_large_step_prob, seed);

				Vector2f raster_pos;
				bootstrap_weights[seed] = evalSample(scene, &sampler, depth, &raster_pos, rng, memory).y();

				memory.freeAll();
			}
		//}
		});

		if (m_task.aborted())
			return;

		Distribution1D bootstrap_distribution(bootstrap_weights.data(), int(bootstrap_weights.size()));
		float b = bootstrap_distribution.getIntegral() * (m_max_depth + 1);

		// Mutations per chain roughly equals to samples per pixel
		float mutations_per_pixel = m_spp;
		uint64_t total_mutations = m_spp * mp_film->getPixelCount();
		uint64_t total_samples	 = 0u;

		concurrency::parallel_for(0, m_num_chains, [&](int i) {
		//for (int i = 0; i < m_num_chains; i++) {
			if (m_task.aborted())
				return;

			uint64_t chain_mutations =
				Min((i + 1) * total_mutations / m_num_chains, total_mutations) -
				i * total_mutations / m_num_chains;

			RNG rng(i);
			MemoryPool memory;

			int bootstrap_idx = bootstrap_distribution.sampleDiscrete(rng.drand48(), nullptr);
			int depth = bootstrap_idx % (m_max_depth + 1);	// Target the bootstrap corresponding depth

			// Initialize local variables for selected state
			MetropolisSampler sampler(m_sigma, m_large_step_prob, bootstrap_idx); // bootstrap_idx equals to the seed

			Vector2f current_raster;
			Spectrum current_Li = evalSample(scene, &sampler, depth, &current_raster, rng, memory);

			// Run the Markov chain for numChainMutations steps
			for (uint64_t j = 0; j < chain_mutations; j++) {
				if (m_task.aborted())
					return;

				sampler.startIteration();

				Vector2f proposed_raster;
				Spectrum proposed_Li = evalSample(scene, &sampler, depth, &proposed_raster, rng, memory);

				// Compute acceptance probability for proposed sample
				float accept_prob = Min(1.f, proposed_Li.y() / (current_Li.y() + 1e-4f));
				if (accept_prob > 0.f)
					mp_film->splat(proposed_raster.x, proposed_raster.y,
						proposed_Li * accept_prob / (proposed_Li.y() + 1e-4f));

				mp_film->splat(current_raster.x, current_raster.y, 
					current_Li * (1.f - accept_prob) / (current_Li.y() + 1e-4f));

				// Accept or reject the proposal
				if (rng.drand48() < accept_prob) {
					current_raster = proposed_raster;
					current_Li = proposed_Li;
					sampler.accept();
				}
				else {
					sampler.reject();
				}

				memory.freeAll();

				// Progressive display
				{
					std::lock_guard<std::mutex> lck(m_mutex);

					total_samples += 1;

					if (total_samples % mp_film->getPixelCount() == 0) {
						mp_film->addSampleCount();

						float current_mutations_per_pixel = (total_samples / float(mp_film->getPixelCount()));
						mp_film->updateDisplay(current_mutations_per_pixel / b);
					}
				}
			}
		//}
		});

		mp_film->updateDisplay(mutations_per_pixel / b);
	}

	Spectrum MultiplexMLTIntegrator::evalSample(const Scene *scene, MetropolisSampler *sampler,
		const int connect_depth, Vector2f *raster_pos,
		RNG &rng, MemoryPool &memory) const {
		sampler->startStream(0);

		int light_length, eye_length, num_strategies;
		if (connect_depth == 0) {
			num_strategies = 1;
			light_length = 0;
			eye_length = 2;
		}
		else {
			num_strategies = connect_depth + 2;
			light_length = Min(int(sampler->get1D() * float(num_strategies)), num_strategies - 1);
			eye_length = num_strategies - light_length;
		}

		// Generate light sub-path
		BidirectionalPathTracingIntegrator::PathVertex *light_path =
			memory.alloc<BidirectionalPathTracingIntegrator::PathVertex>(light_length);
		int num_light_vertex;
		if (BidirectionalPathTracingIntegrator::generateLightPath(scene, sampler, rng,
			mp_camera, mp_film, light_length, light_path, &num_light_vertex, false, -1) != light_length)
			return Spectrum(0.f);

		// Generate primary ray
		sampler->startStream(1);
		CameraSample cam_sample;

		*raster_pos = sampler->get2D();
		raster_pos->x *= m_task.getX();
		raster_pos->y *= m_task.getY();
		sampler->generateSamples(raster_pos->x, raster_pos->y, &cam_sample, rng);
		cam_sample.image_x = raster_pos->x;
		cam_sample.image_y = raster_pos->y;

		RayDifferential ray;
		Spectrum L(0.f);
		if (!mp_camera->generateRayDifferential(cam_sample, &ray))
			return Spectrum(0.f);

		// Initialize the camera PathState
		BidirectionalPathTracingIntegrator::PathState cam_path;
		SurfaceIntersection cam_isect;
		const Light *env_light = nullptr;
		BidirectionalPathTracingIntegrator::sampleCamera(scene, mp_camera, mp_film, ray, cam_path);

		// Trace camera sub-path
		bool last_hitting = false;
		while (eye_length > 1) {
			RayDifferential path_ray(cam_path.ori, cam_path.dir);

			if (!scene->intersect(path_ray, &cam_isect)) {
				last_hitting = false;
				env_light = scene->getEnviromentLight();
				if (env_light)
					++cam_path.path_len;

				break;
			}

			scene->postIntersect(path_ray, &cam_isect);
			last_hitting = true;

			// Update MIS quantities from iteration (34) (35)
			// Divide by g_i-> factor, Forward pdf conversion factor from solid angle measure to area measure (4) (8)
			float cos_in = Abs(cam_isect.n.dot(-path_ray.m_dir));
			cam_path.dvcm *= BidirectionalPathTracingIntegrator::MIS(cam_isect.dist * cam_isect.dist);
			cam_path.dvcm /= BidirectionalPathTracingIntegrator::MIS(cos_in);
			cam_path.dvc /= BidirectionalPathTracingIntegrator::MIS(cos_in);

			if (++cam_path.path_len >= uint32_t(eye_length))
				break;

			if (!BidirectionalPathTracingIntegrator::sampleScattering(scene, rng, path_ray, cam_isect, sampler->getSample(), cam_path))
				break;
		}

		Spectrum ret(0.f);

		// Connect eye sub-path and light sub-path
		if (cam_path.path_len == eye_length) {
			sampler->startStream(2);

			if (light_length == 0) {
				auto light = last_hitting ? (Light*)cam_isect.arealight : env_light;

				if (light) {
					ret = cam_path.throughput *
						BidirectionalPathTracingIntegrator::hittingLightSource(scene, rng,
							Ray(cam_path.ori, cam_path.dir),
							cam_isect, light, cam_path);
				}
			}
			else if (eye_length == 1) {
				if (num_light_vertex > 0) {
					const auto &light_vertex = light_path[num_light_vertex - 1];

					if (light_vertex.path_len == light_length) {
						Point3 raster_p;
						ret = BidirectionalPathTracingIntegrator::connectToCamera(scene, sampler, rng,
							mp_camera, mp_film, light_vertex.isect, light_vertex, &raster_p);

						*raster_pos = Vector2f(raster_p.x, raster_p.y);
					}
				}
			}
			else if (light_length == 1) {
				if (last_hitting && !cam_isect.bsdf->isSpecular()) {
					// Connect to light source
					ret = cam_path.throughput *
						BidirectionalPathTracingIntegrator::connectToLight(scene, sampler, rng,
							Ray(cam_path.ori, cam_path.dir), cam_isect, cam_path);
				}
			}
			else {
				if (num_light_vertex > 0 && last_hitting && !cam_isect.bsdf->isSpecular()) {
					const auto &light_vertex = light_path[num_light_vertex - 1];

					ret = light_vertex.throughput * cam_path.throughput *
						BidirectionalPathTracingIntegrator::connectVertex(scene, rng, cam_isect, light_vertex, cam_path);
				}
			}

			ret *= num_strategies;
		}

		return ret;
	}
}