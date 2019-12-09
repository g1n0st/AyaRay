#ifndef AYA_INTEGRATOR_H
#define AYA_INTEGRATOR_H

#include "config.h"
#include "camera.h"
#include "material.h"

class Integrator {
public:
	virtual Spectrum li(const Ray &ray, Shape **shapes, int shape_size, int depth) const = 0;
};

class SampleIntegrator : Integrator {
public:
	virtual Spectrum li(const Ray &ray, Shape **shapes, int shape_size, int depth) const {
		SurfaceInteraction si;
		float hit;
		bool hit_object = false;

		for (int i = 0; i < shape_size; i++) {
			if (shapes[i]->intersect(ray, &hit, &si)) {
				hit_object = true;
				ray.m_maxt = hit;
			}
		}
		if (hit_object) {
			Ray scatter;
			Spectrum attenuation;

			Spectrum emitted = si.mat->emitted(si.u, si.v, si.p);
			if (depth < 50 && si.mat->scatter(ray, si, attenuation, scatter)) {
				return emitted + attenuation * li(scatter, shapes, shape_size, depth + 1);
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
			return (1.f - t) * Spectrum(1.f, 1.f, 1.f) + t * Spectrum(.5f, .7f, 1.f);
		}
	}
};
#endif