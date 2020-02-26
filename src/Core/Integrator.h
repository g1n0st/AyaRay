#ifndef AYA_CORE_INTEGRATOR_H
#define AYA_CORE_INTEGRATOR_H

#include "Config.h"
#include "Camera.h"
#include "Primitive_.h"

namespace Aya {
	class Integrator {
	public:
		virtual Spectrum li(const Ray &ray, SharedPtr<Accelerator> acclerator, int depth) const = 0;
	};

	class SampleIntegrator : public Integrator {
	public:
		virtual Spectrum li(const Ray &ray, SharedPtr<Accelerator> acclerator, int depth) const {
			SurfaceInteraction si;
			bool hit_object = acclerator->intersect(ray, &si);
			if (hit_object) {
				Ray scatter;
				Spectrum attenuation;

				Spectrum emitted = si.prim->m_material->emitted(si.u, si.v, si.p);
				if (depth < 50 && si.prim->m_material->scatter(ray, si, attenuation, scatter)) {
					return (emitted + attenuation * li(scatter, acclerator, depth + 1)).clamp(0.f, 1.f);
				}
				else {
					return emitted;
				}
			}
			else {
				// sky box
				Vector3 ud = ray.m_dir;
				ud.normalize();

				float t = .5f * (ud.y() + 1.f);
				float rgb1[3] = { .85f, .85f, .85f };
				float rgb2[3] = { .7f, .7f, .7f };
				return (std::pow(1.f - t, 10) * Spectrum::fromRGB(rgb1) + 
					t * Spectrum::fromRGB(rgb2));
			}
		}
	};
}
#endif