#include "Light.h"

#include "../Core/Scene.h"

namespace Aya {
	bool VisibilityTester::unoccluded(const Scene *scene) const {
		return !scene->occluded(ray);
	}
	Spectrum VisibilityTester::tr(const Scene *scene, Sampler *sampler) const {
		if (ray.mp_medium)
			return ray.mp_medium->tr(ray, sampler);
		else
			return Spectrum(1.f);
	}
}