#include "Light.h"

namespace Aya {
	bool VisibilityTester::unoccluded(const Scene_ * scene) const {
		return !scene->occluded(ray);
	}
	Spectrum VisibilityTester::transmittance(const Scene_ * scene, Sampler * sampler) const {
		if (ray.mp_medium)
			return ray.mp_medium->tr(ray, sampler);
		else
			return Spectrum::fromRGB(1.f, 1.f, 1.f);
	}
}