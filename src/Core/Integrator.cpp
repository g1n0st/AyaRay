#include "integrator.h"

namespace Aya {
	void TiledIntegrator::render(const Scene *scene, const Camera *camera, Sampler *sampler, Film *film) const {
		for (uint32_t spp = 0; spp < m_spp; spp++) {
			int tiles_count = m_task.getTilesCount();
			int height = m_task.getX();
			int width = m_task.getY();
			printf("Rendering %d spp(s)\n", spp);

			concurrency::parallel_for(0, tiles_count, [&](int i) {
			//for (int i = 0; i < tiles_count; i++) {
				const RenderTile& tile = m_task.getTile(i);

				UniquePtr<Sampler> tile_sampler(sampler->clone(spp * tiles_count + i));

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
						memory.freeAll();
					}
				}
			//}
			});

			sampler->advanceSampleIndex();

			film->addSampleCount();
			film->updateDisplay();

			if (m_task.aborted())
				break;
		}
	}

	Spectrum Integrator::estimateDirectLighting(const Scatter &scatter, const Vector3 &out, const Light *light,
		const Scene *scene, Sampler *sampler, ScatterType scatter_type) {
		const Point3& pos = scatter.p;
		const Normal3& norm = scatter.n;

		Spectrum L;

		bool require_MIS = false;
		if (scatter.isSurfaceScatter()) { // handle surface scattering
			const SurfaceIntersection &intersection = static_cast<const SurfaceIntersection&>(scatter);
			const BSDF *bsdf = intersection.bsdf;

			require_MIS = (bsdf->getScatterType() & BSDF_GLOSSY);
		}

		// sample light sources
		{
			Vector3 light_dir;
			VisibilityTester visibility;
			float light_pdf, shading_pdf;
			Spectrum transmittance;
			const Spectrum Li = light->illuminate(scatter, sampler->getSample(), &light_dir, &visibility, &light_pdf);

			if (light_pdf > 0.f && !Li.isBlack()) {
				Spectrum f;
				if (scatter.isSurfaceScatter()) { // handle surface scattering
					const SurfaceIntersection &intersection = static_cast<const SurfaceIntersection&>(scatter);
					const BSDF *bsdf = intersection.bsdf;

					f = bsdf->f(out, light_dir, intersection);
					f *= Abs(light_dir.dot(norm));
					shading_pdf = bsdf->pdf(out, light_dir, intersection);
				}
				else {
					const MediumIntersection &intersection = static_cast<const MediumIntersection&>(scatter);
					const PhaseFunctionHG *func = intersection.mp_func;

					const float phase = func->f(out, light_dir);
					f = Spectrum(phase);
					shading_pdf = phase;
				}

				if (!f.isBlack() && visibility.unoccluded(scene)) {
					Spectrum transmittance = visibility.tr(scene, sampler);
					if (light->isDelta() || !require_MIS) {
						L += f * Li * transmittance / light_pdf;
						return L;
					}

					const float mis_weight = PowerHeuristic(1, light_pdf, 1, shading_pdf);
					L += f * Li * transmittance * mis_weight / light_pdf;
				}
			}
		}

		// sample BSDF or medium
		{
			if (!light->isDelta() && require_MIS) {
				Vector3 light_dir;
				Spectrum f;
				float shading_pdf;
				if (scatter.isSurfaceScatter()) {
					const SurfaceIntersection &intersection = static_cast<const SurfaceIntersection&>(scatter);
					const BSDF *bsdf = intersection.bsdf;

					ScatterType types;
					f = bsdf->sample_f(out, sampler->getSample(), intersection, &light_dir, &shading_pdf, scatter_type, &types);
					f *= Abs(light_dir.dot(norm));
				}
				else {
					const MediumIntersection &intersection = static_cast<const MediumIntersection&>(scatter);
					const PhaseFunctionHG *func = intersection.mp_func;

					const float phase = func->sample_f(out, &light_dir, sampler->get2D());
					f = Spectrum(phase);
					shading_pdf = phase;
				}

				if (shading_pdf > 0.f && !f.isBlack()) {
					float light_pdf = light->pdf(pos, light_dir);
					if (light_pdf > 0.f) {
						float mis_weight = PowerHeuristic(1, shading_pdf, 1, light_pdf);

						Point3 center;
						float radius;
						scene->worldBound().boundingSphere(&center, &radius);

						SurfaceIntersection intersection;
						Ray ray_light = Ray(pos, light_dir, scatter.m_medium_interface.getMedium(light_dir, norm), 0, 2.f * radius);
						Spectrum Li;

						if (scene->intersect(ray_light, &intersection)) {
							scene->postIntersect(ray_light, &intersection);
							//if (light == (Light*)intersection.arealight)
							//	Li = intersection.emit(-light_dir);
						}
						else if ((Light*)scene->getEnviromentLight() == light) {
							Li = light->emit(-light_dir);
						}

						if (!Li.isBlack()) {
							Spectrum transmittance = Spectrum(1.f);
							if (ray_light.mp_medium)
								transmittance = ray_light.mp_medium->tr(ray_light, sampler);

							L += f * Li * transmittance * mis_weight / shading_pdf;
						}
					}
				}
			}
		}

		return L;
	}

	Spectrum Integrator::specularReflect(const TiledIntegrator *integrator, const Scene *scene, Sampler *sampler, const RayDifferential &ray,
		const SurfaceIntersection &intersection, RNG &rng, MemoryPool &memory) {
		const Point3 &pos = intersection.p;
		const Normal3 &norm = intersection.n;
		const BSDF *bsdf = intersection.bsdf;
		Vector3 out = -ray.m_dir, in;
		float pdf;

		Spectrum f = bsdf->sample_f(out, Sample(), intersection, &in, &pdf, ScatterType(BSDF_REFLECTION | BSDF_SPECULAR));

		Spectrum color;
		if (pdf > 0.f && !f.isBlack() && Abs(in.dot(norm)) != 0.f) {
			RayDifferential rd(pos, in, intersection.m_medium_interface.getMedium(in, norm), 0.f, float(INFINITY), ray.m_depth + 1);
			if (ray.m_has_differentials) {
				rd.m_has_differentials = true;
				rd.m_rx_ori = pos + intersection.dpdx;
				rd.m_ry_ori = pos + intersection.dpdy;

				Vector3 dndx = intersection.dndu * intersection.dudx +
					intersection.dndv * intersection.dvdx;
				Vector3 dndy = intersection.dndu * intersection.dudy +
					intersection.dndv * intersection.dvdy;

				Vector3 doutdx = -ray.m_rx_dir - out;
				Vector3 doutdy = -ray.m_ry_dir - out;

				float dcosdx = doutdx.dot(norm) + out.dot(dndx);
				float dcosdy = doutdy.dot(norm) + out.dot(dndy);

				rd.m_rx_dir = in - doutdx + 2.0f * (out.dot(norm) * dndx + dcosdx * norm);
				rd.m_ry_dir = in - doutdy + 2.0f * (out.dot(norm) * dndy + dcosdy * norm);
			}

			Spectrum L = integrator->li(rd, scene, sampler, rng, memory);
			color = f * L * Abs(in.dot(norm)) / pdf;
		}

		return color;
	}

	Spectrum Integrator::specularTransmit(const TiledIntegrator *integrator, const Scene *scene, Sampler *sampler, const RayDifferential &ray,
		const SurfaceIntersection &intersection, RNG &rng, MemoryPool &memory) {
		const Point3 &pos = intersection.p;
		const Normal3 &norm = intersection.n;
		const BSDF *bsdf = intersection.bsdf;
		Vector3 out = -ray.m_dir, in;
		float pdf;

		Spectrum f = bsdf->sample_f(out, Sample(), intersection, &in, &pdf, ScatterType(BSDF_TRANSMISSION | BSDF_SPECULAR));

		Spectrum color;
		if (pdf > 0.f && !f.isBlack() && Abs(in.dot(norm)) != 0.f) {
			RayDifferential rd(pos, in, intersection.m_medium_interface.getMedium(in, norm), 0.f, float(INFINITY), ray.m_depth + 1);
			if (ray.m_has_differentials) {
				rd.m_has_differentials = true;
				rd.m_rx_ori = pos + intersection.dpdx;
				rd.m_ry_ori = pos + intersection.dpdy;

				float eta = 1.5f;
				Vector3 d = -out;
				if (out.dot(norm) < 0.0f)
					eta = 1.0f / eta;

				Vector3 dndx = intersection.dndu * intersection.dudx +
					intersection.dndv * intersection.dvdx;
				Vector3 dndy = intersection.dndu * intersection.dudy +
					intersection.dndv * intersection.dvdy;

				Vector3 doutdx = -ray.m_rx_dir - out;
				Vector3 doutdy = -ray.m_ry_dir - out;

				float dcosdx = doutdx.dot(norm) + out.dot(dndx);
				float dcosdy = doutdy.dot(norm) + out.dot(dndy);

				float mu = eta * d.dot(norm) - in.dot(norm);
				float dmudx = (eta - (eta * eta * d.dot(norm)) / in.dot(norm)) * dcosdx;
				float dmudy = (eta - (eta * eta * d.dot(norm)) / in.dot(norm)) * dcosdy;

				rd.m_rx_dir = in + eta * doutdx - (mu * dndx + dmudx * norm);
				rd.m_ry_dir = in + eta * doutdy - (mu * dndy + dmudy * norm);
			}

			Spectrum L = integrator->li(rd, scene, sampler, rng, memory);
			color = f * L * Abs(in.dot(norm)) / pdf;
		}

		return color;
	}
}