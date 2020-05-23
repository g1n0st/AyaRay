#ifndef AYA_FILTERS_GAUSSIANFILTER_H
#define AYA_FILTERS_GAUSSIANFILTER_H

#include <Core/Filter.h>

namespace Aya {
	class GaussianFilter : public Filter {
	private:
		float m_std, m_alpha, m_expR;

	public:
		GaussianFilter(const float std = .5f) : Filter(4 * std), m_std(std) {
			m_alpha = -1.f / (2.f * m_std * m_std);
			m_expR = std::exp(m_alpha * m_radius * m_radius);
		}

		const float evaluate(const float dx, const float dy) const {
			auto Gaussian = [this](const float d) {
				return Max(0.f, std::exp(m_alpha * d * d) - m_expR);
			};
			return Gaussian(dx) * Gaussian(dy);
		}
	};
}

#endif