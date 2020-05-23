#ifndef AYA_MEDIUMS_HOMOGENEOUS_H
#define AYA_MEDIUMS_HOMOGENEOUS_H

#include <Core/Medium.h>

namespace Aya {
	class HomogeneousMedium : public Medium {
	private:
		const Spectrum m_sigma_a, m_sigma_s, m_sigma_t;
	public:
		HomogeneousMedium(const Spectrum &sigma_a, const Spectrum &sigma_s, const float g)
			: m_sigma_a(sigma_a),
			m_sigma_s(sigma_s),
			m_sigma_t(sigma_s + sigma_a),
			Medium(g) {}

		Spectrum tr(const Ray &ray, Sampler *sampler) const override;
		Spectrum sample(const Ray &ray, Sampler *sampler, MediumIntersection *mi) const override;
	};
}

#endif