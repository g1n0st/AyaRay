#include <Integrators/GuidedPathTracer.h>

namespace Aya {
	void GuidedPathTracerIntegrator::resetSDTree() {
		m_sdTree->refine((size_t)(std::sqrtf(std::powf(2, m_iter) * m_sppPerPass / 4) * m_sTreeThreshold), m_sdTreeMaxMemory);
		m_sdTree->forEachDTreeWrapperParallel([this](DTreeWrapper *dTree) { dTree->reset(20, m_dTreeThreshold); });
	}

	void GuidedPathTracerIntegrator::buildSDTree() {
		// Build distributions
		m_sdTree->forEachDTreeWrapperParallel([](DTreeWrapper* dTree) { dTree->build(); });

		// Gather statistics
		// ...

		m_isBuilt = true;
	}

	bool GuidedPathTracerIntegrator::doNeeWithSpp(int spp) {
		switch (m_nee) {
		case Never:
			return false;
		case Kickstart:
			return spp < 128;
		default:
			return true;
		}
	}
}