#ifndef AYA_SPECTRUM_H
#define AYA_SPECTRUM_H

#include "config.h"
#include "../math/vector3.h"

namespace Aya {
	
	// Spectrum Utility Declarations
	static const int sampled_lambda_start = 400;
	static const int sampled_lambda_end = 700;
	static const int n_spectral_samples = 60;

	inline void XYZToRGB(const float xyz[3], float rgb[3]) {
		rgb[0] = 3.240479f * xyz[0] - 1.537150f * xyz[1] - 0.498535f * xyz[2];
		rgb[1] = -0.969256f * xyz[0] + 1.875991f * xyz[1] + 0.041556f * xyz[2];
		rgb[2] = 0.055648f * xyz[0] - 0.204043f * xyz[1] + 1.057311f * xyz[2];
	}

	inline void RGBToXYZ(const float rgb[3], float xyz[3]) {
		xyz[0] = 0.412453f * rgb[0] + 0.357580f * rgb[1] + 0.180423f * rgb[2];
		xyz[1] = 0.212671f * rgb[0] + 0.715160f * rgb[1] + 0.072169f * rgb[2];
		xyz[2] = 0.019334f * rgb[0] + 0.119193f * rgb[1] + 0.950227f * rgb[2];
	}

	// Spectral Data Declarations
	static const int n_CIE_samples = 471;
	extern const float CIE_X[n_CIE_samples];
	extern const float CIE_Y[n_CIE_samples];
	extern const float CIE_Z[n_CIE_samples];
	extern const float CIE_lambda[n_CIE_samples];
	static const float CIE_Y_integral = 106.856895f;

	static const int n_RGB_2spect_samples = 32;
	extern const float RGB_2spect_lambda[n_RGB_2spect_samples];

	extern const float RGB_refl_2spect_white[n_RGB_2spect_samples];
	extern const float RGB_refl_2spect_cyan[n_RGB_2spect_samples];
	extern const float RGB_refl_2spect_magenta[n_RGB_2spect_samples];
	extern const float RGB_refl_2spect_yellow[n_RGB_2spect_samples];
	extern const float RGB_refl_2spect_red[n_RGB_2spect_samples];
	extern const float RGB_refl_2spect_green[n_RGB_2spect_samples];
	extern const float RGB_refl_2spect_blue[n_RGB_2spect_samples];

	extern const float RGB_illum_2spect_white[n_RGB_2spect_samples];
	extern const float RGB_illum_2spect_cyan[n_RGB_2spect_samples];
	extern const float RGB_illum_2spect_magenta[n_RGB_2spect_samples];
	extern const float RGB_illum_2spect_yellow[n_RGB_2spect_samples];
	extern const float RGB_illum_2spect_red[n_RGB_2spect_samples];
	extern const float RGB_illum_2spect_green[n_RGB_2spect_samples];
	extern const float RGB_illum_2spect_blue[n_RGB_2spect_samples];

	template<int nSamples>
	class CoefficientSpectrum {
#if defined(AYA_USE_SIMD)
	public:
		const static int nChips = (nSamples + 3) / 4;
		union Chip {
			float m_val[4];
			__m128 m_val128;
		} c[nChips];

		inline __m128 get128(const int &p) const {
			return c[p].m_val128;
		}
		inline void set128(const __m128 &v128, const int &p) {
			c[p].m_val128 = v128;
		}
#else
	public:
		float c[nSamples];
#endif

	public:
		CoefficientSpectrum() {
#if defined(AYA_USE_SIMD)
			for (int i = 0; i < nChips; i++) {
				__m128 &cc = c[i].m_val128;
				cc = _mm_xor_ps(cc, cc);
			}
#else
			for (int i = 0; i < nSamples; i++) {
				c[i] = 0;
			}
#endif
		}

		inline float & operator[](const int &i) {
			assert(i >= 0 && i < nSamples);
#if defined(AYA_USE_SIMD)
			return c[i >> 2].m_val[i & 3];
#else
			return c[i];
#endif
		}
		inline float operator[](const int &i) const {
			assert(i >= 0 && i < nSamples);
#if defined(AYA_USE_SIMD)
			return c[i >> 2].m_val[i & 3];
#else
			return c[i];
#endif
		}

		CoefficientSpectrum operator + (const CoefficientSpectrum &s) const {
			CoefficientSpectrum ret = *this;
#if defined(AYA_USE_SIMD)
			for (int i = 0; i < nChips; i++) {
				ret.c[i].m_val128 = _mm_add_ps(ret.c[i].m_val128, s.c[i].m_val128);
			}
#else
			for (int i = 0; i < nSamples; i++) {
				ret.c[i] += s.c[i];
			}
#endif
			return ret;
		}
		CoefficientSpectrum &operator += (const CoefficientSpectrum &s) {
			CoefficientSpectrum ret = *this;
#if defined(AYA_USE_SIMD)
			for (int i = 0; i < nChips; i++) {
				c[i].m_val128 = _mm_add_ps(c[i].m_val128, s.c[i].m_val128);
			}
#else
			for (int i = 0; i < nSamples; i++) {
				c[i] += s.c[i];
			}
#endif
			return *this;
		}
		CoefficientSpectrum operator - (const CoefficientSpectrum &s) const {
			CoefficientSpectrum ret = *this;
#if defined(AYA_USE_SIMD)
			for (int i = 0; i < nChips; i++) {
				ret.c[i].m_val128 = _mm_sub_ps(ret.c[i].m_val128, s.c[i].m_val128);
			}
#else
			for (int i = 0; i < nSamples; i++) {
				ret.c[i] -= s.c[i];
			}
#endif
			return ret;
		}
		CoefficientSpectrum &operator -= (const CoefficientSpectrum &s) {
			CoefficientSpectrum ret = *this;
#if defined(AYA_USE_SIMD)
			for (int i = 0; i < nChips; i++) {
				c[i].m_val128 = _mm_sub_ps(c[i].m_val128, s.c[i].m_val128);
			}
#else
			for (int i = 0; i < nSamples; i++) {
				c[i] -= s.c[i];
			}
#endif
			return *this;
		}
		inline CoefficientSpectrum operator- () const {
			CoefficientSpectrum ret;
#if defined(AYA_USE_SIMD)
			for (int i = 0; i < nChips; i++) {
				__m128 r = _mm_xor_ps(c[i].m_val128, vMzeroMask);
				ret.c[i].m_val128 = _mm_and_ps(r, vFFFFfMask);
			}
#else
			for (int i = 0; i < nSamples; i++) {
				ret.c[i] = -c[i];
			}
#endif
			return ret;
		}
		CoefficientSpectrum operator * (const CoefficientSpectrum &s) const {
			CoefficientSpectrum ret = *this;
#if defined(AYA_USE_SIMD)
			for (int i = 0; i < nChips; i++) {
				ret.c[i].m_val128 = _mm_mul_ps(ret.c[i].m_val128, s.c[i].m_val128);
			}
#else
			for (int i = 0; i < nSamples; i++) {
				ret.c[i] *= s.c[i];
			}
#endif
			return ret;
		}
		CoefficientSpectrum &operator *= (const CoefficientSpectrum &s) {
			CoefficientSpectrum ret = *this;
#if defined(AYA_USE_SIMD)
			for (int i = 0; i < nChips; i++) {
				c[i].m_val128 = _mm_mul_ps(c[i].m_val128, s.c[i].m_val128);
			}
#else
			for (int i = 0; i < nSamples; i++) {
				c[i] *= s.c[i];
			}
#endif
			return *this;
		}
		CoefficientSpectrum operator / (const CoefficientSpectrum &s) const {
			CoefficientSpectrum ret = *this;
#if defined(AYA_USE_SIMD)
			for (int i = 0; i < nChips; i++) {
				ret.c[i].m_val128 = _mm_div_ps(ret.c[i].m_val128, s.c[i].m_val128);
			}
#else
			for (int i = 0; i < nSamples; i++) {
				ret.c[i] /= s.c[i];
			}
#endif
			return ret;
		}
		CoefficientSpectrum &operator /= (const CoefficientSpectrum &s) {
			CoefficientSpectrum ret = *this;
#if defined(AYA_USE_SIMD)
			for (int i = 0; i < nChips; i++) {
				c[i].m_val128 = _mm_div_ps(c[i].m_val128, s.c[i].m_val128);
			}
#else
			for (int i = 0; i < nSamples; i++) {
				c[i] /= s.c[i];
			}
#endif
			return *this;
		}
		CoefficientSpectrum operator * (const float &s) const {
			CoefficientSpectrum ret = *this;
#if defined(AYA_USE_SIMD)
			__m128 vs = _mm_load1_ps(&s);

			for (int i = 0; i < nChips; i++) {
				ret.c[i].m_val128 = _mm_mul_ps(ret.c[i].m_val128, vs);
			}
#else
			for (int i = 0; i < nSamples; i++) {
				ret.c[i] *= s;
			}
#endif
			return ret;
		}
		CoefficientSpectrum &operator *= (const float &s) {
			CoefficientSpectrum ret = *this;
#if defined(AYA_USE_SIMD)
			__m128 vs = _mm_load1_ps(&s);

			for (int i = 0; i < nChips; i++) {
				c[i].m_val128 = _mm_mul_ps(c[i].m_val128, vs);
			}
#else
			for (int i = 0; i < nSamples; i++) {
				c[i] *= s;
			}
#endif
			return *this;
		}
		friend inline CoefficientSpectrum operator * (const float &a, const CoefficientSpectrum &s) {
			return s * a;
		}
		CoefficientSpectrum operator / (const float &s) const {
			assert(s != 0.f);
			return (*this) * (1.f / s);
		}
		CoefficientSpectrum &operator /= (const float &s) {
			assert(s != 0.f);
			return (*this) *= (1.f / s);
		}

		bool operator == (const CoefficientSpectrum &s) const {
#if defined(AYA_USE_SIMD)
			for (int i = 0; i < nChips; i++) {
				if (0xf != _mm_movemask_ps((__m128)_mm_cmpeq_ps(c[i].m_val128, s.c[i].m_val128))) return false;
			}
			return true;
#else
			for (int i = 0; i < nSamples; i++) {
				if (c[i] != s.c[i]) return false;
			}
			return true;
#endif
		}
		bool operator != (const CoefficientSpectrum &s) const {
			return !(*this == s);
		}
		bool isBlack() const {
#if defined(AYA_USE_SIMD)
			__m128 v0 = _mm_set_ps(0.f, 0.f, 0.f, 0.f);
			for (int i = 0; i < nChips; i++) {
				if (0xf != _mm_movemask_ps((__m128)_mm_cmpeq_ps(c[i].m_val128, v0))) return false;
			}
#else
			for (int i = 0; i < nSamples; i++) {
				if (c[i] != 0.f) return false;
			}
#endif
			return true;
		}
		CoefficientSpectrum sqrt() const{
			CoefficientSpectrum ret;

#if defined(AYA_USE_SIMD)
			for (int i = 0; i < nChips; i++) {
				const __m128 a = c[i].m_val128;
				const __m128 r = _mm_rsqrt_ps(a);
				const __m128 v = _mm_add_ps(_mm_mul_ps(v1_5, r),
					_mm_mul_ps(_mm_mul_ps(_mm_mul_ps(a, v_0_5), r), _mm_mul_ps(r, r)));
				ret.c[i].m_val128 = _mm_div_ps(v1_0, v);
			}
#else
			for (int i = 0; i < nSamples; i++) {
				ret.c[i] = std::sqrt(c[i]);
			}
#endif
			return ret;
		}
		CoefficientSpectrum exp() const {
			CoefficientSpectrum ret;
			for (int i = 0; i < nSamples; i++) {
				ret[i] = std::exp((*this)[i]);
			}
			return ret;
		}
		CoefficientSpectrum clamp(const float &low, const float &high) const {
			CoefficientSpectrum ret;
			for (int i = 0; i < nSamples; i++) {
				ret[i] = Clamp((*this)[i], low, high);
			}
			return ret;
		}
		float maxValue() const {
#if defined(AYA_USE_SIMD)
			Chip mx = c[0];
			for (int i = 1; i < nChips; i++) {
				mx.m_val128 = _mm_max_ps(mx.m_val128, c[i].m_val128);
			}
			return Max(
				Max(mx.m_val[0], mx.m_val[1]),
				Max(mx.m_val[2], mx.m_val[3])
			);
#else
			float mx = c[0];
			for (int i = 1; i < nSamples; i++) {
				SetMax(mx, c[i]);
			}
			return mx;
#endif
		}
		float minValue() const {
#if defined(AYA_USE_SIMD)
			Chip mn = c[0];
			for (int i = 1; i < nChips; i++) {
				mn.m_val128 = _mm_min_ps(mn.m_val128, c[i].m_val128);
			}
			return Min(
				Min(mn.m_val[0], mn.m_val[1]),
				Min(mn.m_val[2], mn.m_val[3])
			);
#else
			float mn = c[0];
			for (int i = 1; i < nSamples; i++) {
				SetMin(mn, c[i]);
			}
			return mn;
#endif
		}

		template<int n>
		friend inline std::ostream &operator<<(std::ostream &os, const CoefficientSpectrum<n> &s) {
			os << "(" << n << ")[ ";
			for (int i = 0; i < n; i++) {
				os << AYA_SCALAR_OUTPUT(s[i]) << (i < n - 1 ? ", " : " ]");
			}
			return os;
		}
	};
}

#endif