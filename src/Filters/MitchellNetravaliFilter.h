#ifndef AYA_FILTERS_MITCHELLNETRAVALIFILTER_H
#define AYA_FILTERS_MITCHELLNETRAVALIFILTER_H

#include <Core/Filter.h>

namespace Aya {
	class MitchellNetravaliFilter : public Filter {
	private:
		float m_B, m_C;

	public:
		MitchellNetravaliFilter(const float rad = 2.f,
			const float B = 1.f / 3.f,
			const float C = 1.f / 3.f) : Filter(rad), m_B(B), m_C(C) {}
		const float evaluate(const float dx, const float dy) const {
			auto MitchellNetravali = [this](const float d) {
				float dd = Abs(d);
				float sqr = d * d, cc = sqr * dd;

				if (dd < 1.f) {
					return 1.f / 6.f * ((12.f - 9.f * m_B - 6.f * m_C) * cc
						+ (-18.f + 12.f * m_B + 6.f * m_C) * sqr + (6.f - 2.f * m_B));
				}
				else if (dd < 2.f) {
					return 1.f / 6.f * ((-m_B - 6.f * m_C) * cc + (6.f * m_B + 30.f * m_C) * sqr
						+ (-12.f * m_B - 48.f * m_C) * dd + (8.f * m_B + 24.f * m_C));
				}
				else return 0.f;
			};

			return MitchellNetravali(dx) * MitchellNetravali(dy);
		}
	};
}

#endif