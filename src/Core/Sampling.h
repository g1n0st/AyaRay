#ifndef AYA_CORE_SAMPLING_H
#define AYA_CORE_SAMPLING_H

#include "../Math/Vector2.h"
#include "../Math/Vector3.h"
#include "../Core/Memory.h"

#include <algorithm>
#include <vector>

namespace Aya {
	class Distribution1D {
	private:
		std::vector<float> m_pdf;
		std::vector<float> m_cdf;
		int m_count = -1;
		float m_integral_value;
		friend class Distribution2D;

	public:
		Distribution1D() = default;
		Distribution1D(const float *func, int size) {
			setFunction(func, size);
		}

		void setFunction(const float *func, int size) {
			assert(func);
			assert(size > 0);
			
			if (size != m_count) {
				m_count = size;
				m_pdf.resize(size);
				m_cdf.resize(size + 1);
			}

			for (auto i = 0; i < size; i++) {
				m_pdf[i] = func[i];
			}

			float inv_size = 1.f / float(size);
			m_cdf[0] = 0.f;
			for (auto i = 1; i < m_cdf.size(); i++) {
				m_cdf[i] = m_cdf[i - 1] + m_pdf[i - 1] * inv_size;
			}

			m_integral_value = m_cdf[size];
			if (m_integral_value > 0.f) {
				float inv_value = 1.f / m_integral_value;
				for (auto i = 1; i < m_cdf.size(); i++) {
					m_cdf[i] *= inv_value;
				}
			}
		}

		float sampleContinuous(float u, float *pdf, int *p_offset = nullptr) const {
			const int idx = int(std::lower_bound(m_cdf.begin(), m_cdf.end(), u) - m_cdf.begin());
			int offset = Clamp(idx - 1, 0, m_count - 1);
			if (pdf)
				*pdf = m_pdf[offset] / m_integral_value;
			if (p_offset)
				*p_offset = offset;

			float du = (u - m_cdf[offset]) / (m_cdf[offset + 1] - m_cdf[offset] + float(AYA_EPSILON));
			return (offset + du) / float(m_count);
		}
		int sampleDiscrete(float u, float *pdf) const {
			const int idx = int(std::lower_bound(m_cdf.begin(), m_cdf.end(), u) - m_cdf.begin());
			int offset = Clamp(idx - 1, 0, m_count - 1);
			if (pdf)
				*pdf = m_pdf[offset] / (m_integral_value * m_count);

			return offset;
		}
		float getInterval() const {
			return m_integral_value;
		}
	};

	class Distribution2D {
	private:
		std::vector<UniquePtr<Distribution1D>> m_conditional;
		UniquePtr<Distribution1D> mp_marginal;

	public:
		Distribution2D(const float *func, int count_x, int count_y) {
			assert(func);
			assert(count_x > 0);
			assert(count_y > 0);

			m_conditional.resize(count_y);
			for (auto i = 0; i < count_y; i++) {
				m_conditional[i] = MakeUnique<Distribution1D>(&func[i * count_x], count_x);
			}

			float *marginal = new float[count_y];
			for (auto i = 0; i < count_y; i++) {
				marginal[i] = m_conditional[i]->getInterval();
			}
			mp_marginal = MakeUnique<Distribution1D>(marginal, count_y);
			SafeDeleteArray(marginal);
		}

		void sampleContinuous(float u, float v, float *sample_u, float *sample_v, float *pdf) const {
			float pdfs[2];
			int iv;
			*sample_v = mp_marginal->sampleContinuous(v, &pdfs[1], &iv);
			*sample_u = m_conditional[iv]->sampleContinuous(u, &pdfs[0]);
			*pdf = pdfs[0] * pdfs[1];
			assert(!std::isnan(*pdf));
		}
		float pdf(float u, float v) const {
			int iu = Clamp(int(u * m_conditional[0]->m_count), 0, m_conditional[0]->m_count - 1);
			int iv = Clamp(int(v * mp_marginal->m_count), 0, mp_marginal->m_count - 1);
			if (m_conditional[iv]->getInterval() * mp_marginal->getInterval() == 0.f)
				return 0.f;

			return (m_conditional[iv]->m_pdf[iu] * mp_marginal->m_pdf[iv]) /
				(m_conditional[iv]->getInterval() * mp_marginal->getInterval());
		}
	};

	inline void ConcentricSampleDisk(float u1, float u2, float *dx, float *dy) {
		float r1 = 2.f * u1 - 1.f;
		float r2 = 2.f * u2 - 1.f;

		/* Modified concencric map code with less branching (by Dave Cline), see
		http://psgraphics.blogspot.ch/2011/01/improved-code-for-concentric-map.html */

		float phi, r;
		if (r1 == 0.f && r2 == 0.f)
			r = phi = 0.f;
		else {
			if (r1 * r1 > r2 * r2) {
				r = r1;
				phi = float(M_PI_4) * (r2 / r1);
			}
			else {
				r = r2;
				phi = float(M_PI_2) - (r1 / r2) * float(M_PI_4);
			}
		}
		*dx = r * std::cosf(phi);
		*dy = r * std::sinf(phi);
	}
	inline void UniformSampleTriangle(float u1, float u2, float *u, float *v) {
		float su1 = Sqrt(u1);
		*u = 1.f - su1;
		*v = u2 * su1;
	}
	inline Vector3 CosineSampleHemisphere(float u1, float u2) {
		Vector3 ret;
		ConcentricSampleDisk(u1, u2, &ret[0], &ret[1]);
		ret[2] = Sqrt(Max(0.f, 1.f - ret.x() * ret.x() - ret.y() * ret.y()));
		return ret;
	}
	inline Vector3 UniformSampleSphere(float u1, float u2) {
		float z = 1.f - 2.f * u1;
		float r = Sqrt(Max(0.f, 1.f - z * z));
		float phi = 2.f * float(M_PI) * u2;
		float x = r * std::cosf(phi);
		float y = r * std::sinf(phi);
		return Vector3(x, y, z);
	}
	inline Vector3 UniformSampleCone(float u1, float u2, float costhetamax,
		const Vector3 &x, const Vector3 &y, const Vector3 &z) {
		float costheta = Lerp(u1, costhetamax, 1.f);
		float sintheta = Sqrt(1.f - costheta * costheta);
		float phi = 2.f * float(M_PI) * u2;
		return	std::cosf(phi) * sintheta * x + 
				std::sinf(phi) * sintheta * y +
				costheta * z;
	}

	inline float UniformSpherePDF() {
		return float(M_1_PI * .25f);
	}
	inline float UniformHemispherePDF() {
		return float(M_1_PI * .5f);
	}
	inline float ConcentricDiscPdf() {
		return float(M_1_PI);
	}
	inline float CosineHemispherePDF(float cos) {
		return Max(0.f, cos) * float(M_1_PI);
	}
	inline float CosineHemispherePDF(const Vector3 &norm, const Vector3 &vec) {
		return Max(0.f, norm.dot(vec)) * float(M_1_PI);
	}
	inline float UniformConePDF(float cos_theta_max) {
		if (cos_theta_max == 1.f)
			return 1.f;

		return 1.f / (2.f * float(M_PI) * (1.f - cos_theta_max));
	}
	inline bool DirectionInCone(const Vector3 &dir, const Vector3 &cone_dir, const float cos_theta_max) {
		return dir.dot(cone_dir) > cos_theta_max;
	}

	inline float BalanceHeuristic(int nf, float f_pdf, int ng, float g_pdf) {
		return (nf * f_pdf) / (nf * f_pdf + ng * g_pdf);
	}
	inline float PowerHeuristic(int nf, float f_pdf, int ng, float g_pdf) {
		float f = nf * f_pdf, g = ng * g_pdf;
		return (f * f) / (f * f + g * g);
	}
}

#endif