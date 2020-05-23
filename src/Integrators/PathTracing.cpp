#include <Integrators/PathTracing.h>

namespace Aya {
	Spectrum PathTracingIntegrator::li(const RayDifferential &ray, const Scene *scene, Sampler *sampler, RNG &rng, MemoryPool &memory) const {
		Spectrum L(0.f);
		Spectrum tp = Spectrum(1.f);

		bool spec_bounce = true;
		RayDifferential path_ray = ray;
		for (uint32_t bounce = 0; ; bounce++) {
			SurfaceIntersection intersection;
			bool intersected = scene->intersect(path_ray, &intersection);

			MediumIntersection medium;
			if (path_ray.mp_medium)
				tp *= path_ray.mp_medium->sample(path_ray, sampler, &medium);

			if (tp.isBlack())
				break;

			// Surface
			if (!medium.isValid()) {
				if (intersected)
					scene->postIntersect(path_ray, &intersection);

				if (spec_bounce) {
					if (intersected)
						L += tp * intersection.emit(-path_ray.m_dir);
					else if (scene->getEnviromentLight())
						L += tp * scene->getEnviromentLight()->emit(-path_ray.m_dir);
				}

				if (!intersected || bounce >= m_max_depth)
					break;

				auto bsdf = intersection.bsdf;
				if (!bsdf->isSpecular()) {
					float sample1d = sampler->get1D();
					int light_idx = Min(int(sample1d * scene->getLightCount()), int(scene->getLightCount() - 1));
					L += tp *
						estimateDirectLighting(intersection, -path_ray.m_dir, scene->getLight(light_idx), scene, sampler) *
						float(scene->getLightCount());
				}

				const Point3 &pos = intersection.p;
				const Normal3 &normal = intersection.n;
				Vector3 out = -path_ray.m_dir;
				Vector3 in;
				float pdf;
				ScatterType sample_types;
				Sample scatter_sample = sampler->getSample();
				Spectrum f = bsdf->sample_f(out, scatter_sample, intersection, &in, &pdf, BSDF_ALL, &sample_types);
				if (f.isBlack() || pdf == 0.f)
					break;
				tp *= f * Abs(in.dot(normal)) / pdf;

				bool sample_subsurface = false;
				if (!sample_subsurface) {
					spec_bounce = (sample_types & BSDF_SPECULAR) != 0;
					path_ray = Ray(pos, in, intersection.m_mediumInterface.getMedium(in, normal));
				}
				else {
					// There will be a BSSRDF integrator ...
				}
			}
			// Medium
			else {
				float sample1d = sampler->get1D();
				int light_idx = Min(int(sample1d * scene->getLightCount()), int(scene->getLightCount() - 1));
				L += tp *
					estimateDirectLighting(medium, -path_ray.m_dir, scene->getLight(light_idx), scene, sampler) *
					float(scene->getLightCount());

				if (bounce >= m_max_depth)
					break;

				auto func = medium.mp_func;

				Vector3 out = -path_ray.m_dir;
				Vector3 in;
				func->sample_f(out, &in, sampler->get2D());

				spec_bounce = false;
				path_ray = Ray(medium.p, in, path_ray.mp_medium);
			}

			// Russian Roulette
			if (bounce > 3) {
				float RR = Min(1.f, tp.luminance());
				if (rng.drand48() > RR)
					break;

				tp /= RR;
			}
		}

		return L;
	}
}