#ifndef AYA_INTEGRATOR_H
#define AYA_INTEGRATOR_H

#include "config.h"
#include "camera.h"
#include "primitive.h"

namespace Aya {
	class Integrator {
	public:
		virtual Spectrum li(const Ray &ray, Accelerator *acclerator, int depth) const = 0;
	};

	class SampleIntegrator : Integrator {
	public:
		virtual Spectrum li(const Ray &ray, Accelerator *acclerator, int depth) const {
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
				return ((1.f - t) * Spectrum(.5f, .5f, .5f) + t * Spectrum(.5f, .7f, 1.f)).clamp(0.0f, 1.0f);
			}
		}
	};
}
#endif