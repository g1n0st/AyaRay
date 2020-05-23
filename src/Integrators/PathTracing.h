#ifndef AYA_INTEGRATORS_PATHTRACING_H
#define AYA_INTEGRATORS_PATHTRACING_H

#include <Core/Integrator.h>

namespace Aya {
	class PathTracingIntegrator : public TiledIntegrator {
	private:
		uint32_t m_maxDepth;

	public:
		PathTracingIntegrator(const TaskSynchronizer &task, const uint32_t &spp, uint32_t max_depth) :
			TiledIntegrator(task, spp), m_maxDepth(max_depth) {
		}
		~PathTracingIntegrator() {
		}

		Spectrum li(const RayDifferential &ray, const Scene *scene, Sampler *sampler, RNG &rng, MemoryPool &memory) const override;
	};
}

#endif