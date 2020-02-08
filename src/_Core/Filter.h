#ifndef AYA_CORE_FILTER_H
#define AYA_CORE_FILTER_H

#include "Config.h"
#include "../Math/MathUtility.h"

namespace Aya {
	class Filter {
	protected:
		float m_radius;

	public:
		Filter(const float &rad) : m_radius(rad) {}
		virtual ~Filter() {}

		const float getRadius() const {
			return m_radius;
		}
		virtual const float evaluate(const float dx, const float dy) const = 0;
	};

	class BoxFilter : public Filter {
	public:
		BoxFilter() : Filter(0.25f) {}
		const float evaluate(const float dx, const float dy) const {
			return 1.0f;
		}
	};

	class TriangleFilter : public Filter {
		TriangleFilter() : Filter(0.25f) {}
		const float evaluate(const float dx, const float dy) const {
			return Max(0.f, m_radius - dx) * Max(0.f, m_radius - dy);
		}
	};

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

	class MitchellNetravaliFilter : public Filter {
	private:
		float m_B, m_C;

	public:
		MitchellNetravaliFilter(const float rad = 2.f, 
			const float B = 1.f / 3.f,
			const float C = 1.f / 3.f) : Filter(rad), m_B(B), m_C(C) {}
		const float evaluate(const float dx, const float dy) const {
			auto MitchellNetravali = [this](const float d) {
				float dd = std::abs(d);
				float sqr = d * d, cc = sqr * dd;

				if (dd < 1) {
					return 1.f / 6.f * ((12.f - 9.f * m_B - 6.f * m_C) * cc
						+ (-18 + 12 * m_B + 6 * m_C) * sqr + (6 - 2 * m_B));
				}
				else if (dd < 2) {
					return 1.0f / 6.0f * ((-m_B - 6 * m_C) * cc + (6.f * m_B + 30.f * m_C) * sqr
						+ (-12 * m_B - 48 * m_C) * dd + (8.f * m_B + 24.f * m_C));
				}
				else return 0.f;
			};

			return MitchellNetravali(dx) * MitchellNetravali(dy);
		}
	};
}
#endif