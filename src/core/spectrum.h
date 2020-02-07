#ifndef AYA_SPECTRUM_H
#define AYA_SPECTRUM_H

#include "config.h"
#include "../math/vector3.h"

#include <algorithm>
#include <vector>

namespace Aya {
	
	// Spectrum Utility Declarations
	static const float sampled_lambda_start = 400;
	static const float sampled_lambda_end = 700;
	static const int n_spectral_samples = 60;

	extern bool SpectrumSamplesSorted(const float *lambda, const float *vals, int n);
	extern void SortSpectrumSamples(float *lambda, float *vals, int n);
	extern float InterpolateSpectrumSamples(const float *lambda, const float *vals, int n, float l);
	extern float AverageSpectrumSamples(const float *lambda, const float *vals,
		int n, float lambda_start, float lambda_end);

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

	enum class SpectrumType { Reflectance, Illuminant };

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
	protected:
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
	protected:
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
		CoefficientSpectrum clamp(float low = 0, float high = INFINITY) const {
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
	};

	class RGBSpectrum;
	class SampledSpectrum;

	class SampledSpectrum : public CoefficientSpectrum<n_spectral_samples> {
	private:
		// SampledSpectrum Private Data
		static SampledSpectrum X, Y, Z;
		static SampledSpectrum rgb_refl_2spect_white, rgb_illum_2spect_white;
		static SampledSpectrum rgb_refl_2spect_magenta, rgb_illum_2spect_magenta;
		static SampledSpectrum rgb_refl_2spect_red, rgb_illum_2spect_red;
		static SampledSpectrum rgb_refl_2spect_blue, rgb_illum_2spect_blue;
		static SampledSpectrum rgb_refl_2spect_cyan, rgb_illum_2spect_cyan;
		static SampledSpectrum rgb_refl_2spect_yellow, rgb_illum_2spect_yellow;
		static SampledSpectrum rgb_refl_2spect_green, rgb_illum_2spect_green;

	public:
		SampledSpectrum() : CoefficientSpectrum() {}
		SampledSpectrum(const CoefficientSpectrum<n_spectral_samples> &s) :
			CoefficientSpectrum<n_spectral_samples>(s) {}

		static SampledSpectrum fromSampled(const float *lambda, const float *v, int n) {
			if (!SpectrumSamplesSorted(lambda, v, n)) {
				std::vector<float> sl(&lambda[0], &lambda[n]);
				std::vector<float> sv(&v[0], &v[n]);
				SortSpectrumSamples(&sl[0], &sv[0], n);
				return fromSampled(&sl[0], &sv[0], n);
			}

			SampledSpectrum ret;
			for (int i = 0; i < n_spectral_samples; i++) {
				float l0 = Lerp(float(i) / float(n_spectral_samples),
					sampled_lambda_start, sampled_lambda_end);
				float l1 = Lerp(float(i + 1) / float(n_spectral_samples),
					sampled_lambda_start, sampled_lambda_end);
				ret[i] = AverageSpectrumSamples(lambda, v, n, l0, l1);
			}
		}
		static void init() {
			// Compute XYZ
			for (int i = 0; i < n_spectral_samples; i++) {
				float wl0 = Lerp(float(i) / float(n_spectral_samples),
					sampled_lambda_start, sampled_lambda_end);
				float wl1 = Lerp(float(i + 1) / float(n_spectral_samples),
					sampled_lambda_start, sampled_lambda_end);

				X[i] = AverageSpectrumSamples(CIE_lambda, CIE_X, n_CIE_samples, wl0, wl1);
				Y[i] = AverageSpectrumSamples(CIE_lambda, CIE_Y, n_CIE_samples, wl0, wl1);
				Z[i] = AverageSpectrumSamples(CIE_lambda, CIE_Z, n_CIE_samples, wl0, wl1);
			}
			// Compute RGB
			for (int i = 0; i < n_spectral_samples; i++) {
				float wl0 = Lerp(float(i) / float(n_spectral_samples),
					sampled_lambda_start, sampled_lambda_end);
				float wl1 = Lerp(float(i + 1) / float(n_spectral_samples),
					sampled_lambda_start, sampled_lambda_end);

				rgb_refl_2spect_white[i] = 
					AverageSpectrumSamples(RGB_2spect_lambda, RGB_refl_2spect_white, n_RGB_2spect_samples, wl0, wl1);
				rgb_refl_2spect_cyan[i] =
					AverageSpectrumSamples(RGB_2spect_lambda, RGB_refl_2spect_cyan, n_RGB_2spect_samples, wl0, wl1);
				rgb_refl_2spect_magenta[i] =
					AverageSpectrumSamples(RGB_2spect_lambda, RGB_refl_2spect_magenta, n_RGB_2spect_samples, wl0, wl1);
				rgb_refl_2spect_yellow[i] =
					AverageSpectrumSamples(RGB_2spect_lambda, RGB_refl_2spect_yellow, n_RGB_2spect_samples, wl0, wl1);
				rgb_refl_2spect_red[i] =
					AverageSpectrumSamples(RGB_2spect_lambda, RGB_refl_2spect_red, n_RGB_2spect_samples, wl0, wl1);
				rgb_refl_2spect_green[i] =
					AverageSpectrumSamples(RGB_2spect_lambda, RGB_refl_2spect_green, n_RGB_2spect_samples, wl0, wl1);
				rgb_refl_2spect_blue[i] =
					AverageSpectrumSamples(RGB_2spect_lambda, RGB_refl_2spect_blue, n_RGB_2spect_samples, wl0, wl1);
				rgb_illum_2spect_white[i] =
					AverageSpectrumSamples(RGB_2spect_lambda, RGB_illum_2spect_white, n_RGB_2spect_samples, wl0, wl1);
				rgb_illum_2spect_cyan[i] =
					AverageSpectrumSamples(RGB_2spect_lambda, RGB_illum_2spect_cyan, n_RGB_2spect_samples, wl0, wl1);
				rgb_illum_2spect_magenta[i] =
					AverageSpectrumSamples(RGB_2spect_lambda, RGB_illum_2spect_magenta, n_RGB_2spect_samples, wl0, wl1);
				rgb_illum_2spect_yellow[i] =
					AverageSpectrumSamples(RGB_2spect_lambda, RGB_illum_2spect_yellow, n_RGB_2spect_samples, wl0, wl1);
				rgb_illum_2spect_red[i] =
					AverageSpectrumSamples(RGB_2spect_lambda, RGB_illum_2spect_red, n_RGB_2spect_samples, wl0, wl1);
				rgb_illum_2spect_green[i] =
					AverageSpectrumSamples(RGB_2spect_lambda, RGB_illum_2spect_green, n_RGB_2spect_samples, wl0, wl1);
				rgb_illum_2spect_blue[i] =
					AverageSpectrumSamples(RGB_2spect_lambda, RGB_illum_2spect_blue, n_RGB_2spect_samples, wl0, wl1);
			}
		}

		void toXYZ(float xyz[3]) const {
			xyz[0] = xyz[1] = xyz[2] = 0.f;
#if defined(AYA_USE_SIMD)
			Chip cx, cy, cz;
			cx.m_val128 = cy.m_val128 = cz.m_val128 = _mm_set_ps(0.f, 0.f, 0.f, 0.f);
			for (int i = 0; i < (n_spectral_samples >> 2); i++) {
				cx.m_val128 = _mm_add_ps(cx.m_val128, _mm_mul_ps(X.c[i].m_val128, c[i].m_val128));
				cy.m_val128 = _mm_add_ps(cy.m_val128, _mm_mul_ps(Y.c[i].m_val128, c[i].m_val128));
				cz.m_val128 = _mm_add_ps(cz.m_val128, _mm_mul_ps(Z.c[i].m_val128, c[i].m_val128));
			}
			xyz[0] = cx.m_val[0] + cx.m_val[1] + cx.m_val[2] + cx.m_val[3];
			xyz[1] = cy.m_val[0] + cy.m_val[1] + cy.m_val[2] + cy.m_val[3];
			xyz[2] = cz.m_val[0] + cz.m_val[1] + cz.m_val[2] + cz.m_val[3];
#else
			for (int i = 0; i < n_spectral_samples; i++) {
				xyz[0] += X[i] * c[i];
				xyz[1] += Y[i] * c[i];
				xyz[2] += Z[i] * c[i];
			}
#endif
			float scale = float(sampled_lambda_end - sampled_lambda_start) /
				float(CIE_Y_integral * n_spectral_samples);
			xyz[0] *= scale;
			xyz[1] *= scale;
			xyz[2] *= scale;
		}
		float y() const {
			float scale = float(sampled_lambda_end - sampled_lambda_start) /
				float(CIE_Y_integral * n_spectral_samples);

#if defined(AYA_USE_SIMD)
			Chip cy;
			cy.m_val128 = _mm_set_ps(0.f, 0.f, 0.f, 0.f);
			for (int i = 0; i < (n_spectral_samples >> 2); i++) {
				cy.m_val128 = _mm_add_ps(cy.m_val128, _mm_mul_ps(Y.c[i].m_val128, c[i].m_val128));
			}
			return (cy.m_val[0] + cy.m_val[1] + cy.m_val[2] + cy.m_val[3]) * scale;
#else
			float yy = 0.f;
			for (int i = 0; i < n_spectral_samples; i++) {
				yy += Y[i] * c[i];
			}
			return yy * scale;
#endif
		}
		void toRGB(float rgb[3]) const {
			float xyz[3];
			toXYZ(xyz);
			XYZToRGB(xyz, rgb);
		}

		RGBSpectrum toRGBSpectrum() const;
		static SampledSpectrum fromRGB(const float rgb[3], SpectrumType type = SpectrumType::Illuminant);
		static SampledSpectrum fromXYZ(const float xyz[3], SpectrumType type = SpectrumType::Reflectance) {
			float rgb[3];
			XYZToRGB(xyz, rgb);
			return fromRGB(rgb, type);
		}
		SampledSpectrum(const RGBSpectrum &r, SpectrumType type = SpectrumType::Reflectance);

		friend inline std::ostream &operator << (std::ostream &os, const SampledSpectrum &s) {
			os << "(" << n_spectral_samples << ")[ ";
			for (int i = 0; i < n_spectral_samples; i++) {
				os << AYA_SCALAR_OUTPUT(s[i]) << (i < n_spectral_samples - 1 ? ", " : " ]");
			}
			return os;
		}
	};

	class RGBSpectrum : public CoefficientSpectrum<3> {
	public:
		RGBSpectrum() : CoefficientSpectrum<3>() {}
		RGBSpectrum(const CoefficientSpectrum<3> &s) : CoefficientSpectrum<3>(s) {}
		RGBSpectrum(const RGBSpectrum &s, SpectrumType type = SpectrumType::Reflectance) {
			*this = s;
		}

		static RGBSpectrum fromRGB(const float rgb[3], SpectrumType type = SpectrumType::Reflectance) {
			RGBSpectrum s;
			s[0] = rgb[0];
			s[1] = rgb[1];
			s[2] = rgb[2];
			return s;
		}
		static RGBSpectrum fromXYZ(const float xyz[3], SpectrumType type = SpectrumType::Reflectance) {
			float rgb[3];
			XYZToRGB(xyz, rgb);
			return fromRGB(rgb, type);
		}
		static RGBSpectrum fromSampled(const float *lambda, const float *v, int n) {
			if (!SpectrumSamplesSorted(lambda, v, n)) {
				std::vector<float> sl(&lambda[0], &lambda[n]);
				std::vector<float> sv(&v[0], &v[n]);
				SortSpectrumSamples(&sl[0], &sv[0], n);
				return fromSampled(&sl[0], &sv[0], n);
			}

			float xyz[3] = { 0, 0, 0 };
			for (int i = 0; i < n_CIE_samples; i++) {
				float val = InterpolateSpectrumSamples(lambda, v, n, CIE_lambda[i]);
				xyz[0] += val * CIE_X[i];
				xyz[1] += val * CIE_Y[i];
				xyz[2] += val * CIE_Z[i];
			}
			float scale = float(CIE_lambda[n_CIE_samples - 1] - CIE_lambda[0]) /
				float(CIE_Y_integral * n_CIE_samples);
			xyz[0] *= scale;
			xyz[1] *= scale;
			xyz[2] *= scale;
			return fromXYZ(xyz);
		}

		void toRGB(float *rgb) const {
			rgb[0] = (*this)[0];
			rgb[1] = (*this)[1];
			rgb[2] = (*this)[2];
		}
		void toXYZ(float xyz[3]) const {
			float rgb[3];
			toRGB(rgb);
			RGBToXYZ(rgb, xyz);
		}
		const RGBSpectrum & toRGBSpectrum() const {
			return *this;
		}
		float y() {
			const float yy[3] = { 0.212671f, 0.715160f, 0.072169f };
			return yy[0] * (*this)[0] + yy[1] * (*this)[1] + yy[2] * (*this)[2];
		}

		friend inline std::ostream &operator << (std::ostream &os, const RGBSpectrum &s) {
			os << "(" << 3 << ")[ " << s[0] << ", " << s[1] << ", " << s[2] << " ]";
			return os;
		}
	};

#ifdef AYA_SAMPLED_SPECTRUM
	typedef SampledSpectrum Spectrum;
#else
	typedef RGBSpectrum Spectrum;
#endif
}

#endif