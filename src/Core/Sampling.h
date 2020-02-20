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


}

#endif