#ifndef AYA_MEDIUMS_HOMOGENEOUS_H
#define AYA_MEDIUMS_HOMOGENEOUS_H

#include <Core/Medium.h>

namespace Aya {
	class HomogeneousMedium : public Medium {
	private:
		const Spectrum m_sigmaA, m_sigmaS, m_sigmaT;
	public:
		HomogeneousMedium(const Spectrum &sigma_a, const Spectrum &sigma_s, const float g)
			: m_sigmaA(sigma_a),
			m_sigmaS(sigma_s),
			m_sigmaT(sigma_s + sigma_a),
			Medium(g) {}

		Spectrum tr(const Ray &ray, Sampler *sampler) const override;
		Spectrum sample(const Ray &ray, Sampler *sampler, MediumIntersection *mi) const override;
	};
}

#endif