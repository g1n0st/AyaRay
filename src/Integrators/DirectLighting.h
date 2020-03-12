#ifndef AYA_INTEGRATORS_DIRECTLIGHTING_H
#define AYA_INTEGRATORS_DIRECTLIGHTING_H

#include "../Core/Integrator.h"

namespace Aya {
	class DirectLightingIntegrator : public TiledIntegrator {
	private:
		uint32_t m_max_depth;

	public:
		DirectLightingIntegrator(const TaskSynchronizer &task, const uint32_t &spp, uint32_t max_depth) :
			TiledIntegrator(task, spp), m_max_depth(max_depth) {
		}
		~DirectLightingIntegrator() {
		}

		Spectrum li(const RayDifferential &ray, const Scene *scene, Sampler *sampler, RNG &rng, MemoryPool &memory) const override;
	};
}

#endif