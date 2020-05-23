#include <Integrators/DirectLighting.h>

namespace Aya {
	Spectrum DirectLightingIntegrator::li(const RayDifferential &ray, const Scene *scene, Sampler *sampler, RNG &rng, MemoryPool &memory) const {
		SurfaceIntersection intersection;
		Spectrum L;
		if (scene->intersect(ray, &intersection)) {
			scene->postIntersect(ray, &intersection);

			for (int i = 0; i < (int)scene->getLights().size(); i++) {
				auto light = scene->getLights()[i].get();
				L += estimateDirectLighting(intersection, -ray.m_dir, light, scene, sampler);
			}

			if (ray.m_depth < m_maxDepth) {
				L += specularReflect(this, scene, sampler, ray, intersection, rng, memory);
				L += specularTransmit(this, scene, sampler, ray, intersection, rng, memory);
			}
		}
		else {
			if (auto env_map = scene->getEnviromentLight())
				L += env_map->emit(-ray.m_dir);
		}

		return L;
	}
}