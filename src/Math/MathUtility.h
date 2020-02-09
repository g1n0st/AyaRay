#ifndef AYA_MATH_MATHUTILITY_H
#define AYA_MATH_MATHUTILITY_H

#include "..\Core\Config.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>

#if defined(AYA_DEBUG)
#include <assert.h>
#else
#define assert
#endif

#ifdef _WIN32
#include <intrin.h>
#endif

#define AYA_EPSILON FLT_EPSILON

#if defined(AYA_SCALAR_OUTPUT_APPROXIMATION)
#define AYA_SCALAR_OUTPUT(x) (abs(x) < AYA_EPSILON ? 0 : (x))
#else
#define AYA_SCALAR_OUTPUT(x) (x)
#endif

#define v1_0 (_mm_set_ps(1.0f, 1.0f, 1.0f, 1.0f))
#define v1_5 (_mm_set_ps(1.5f, 1.5f, 1.5f, 1.5f))
#define v_0_5 (_mm_set_ps(-0.5f, -0.5f, -0.5f, -0.5f))
#define v0_5 (_mm_set_ps(0.5f, 0.5f, 0.5f, 0.5f))

namespace Aya {
	template<class T>
	__forceinline T Min(const T &a, const T &b) {
		return a < b ? a : b;
	}
	template<class T>
	__forceinline T Max(const T &a, const T &b) {
		return a > b ? a : b;
	}
	template<class T>
	__forceinline void SetMax(T &a, const T &b) {
		if (b > a) a = b;
	}
	template<class T>
	__forceinline void SetMin(T &a, const T &b) {
		if (b < a) a = b;
	}

	__forceinline float Radian(const float &deg) {
		return (float)(M_PI / 180.f) * deg;
	}
	__forceinline float Degree(const float &rad) {
		return (float)(180.f / M_PI) * rad;
	}

	template<class T>
	__forceinline T Lerp(const T &t, const T &a, const T &b) {
		return a + t * (b - a);
	}

	template<class T>
	__forceinline T Clamp(const T &t, const T  &low, const T &high) {
		if (t < low) return low;
		if (t > high) return high;
		return t;
	}

	template <typename T>
	int FindInterval(int size, const T &pred) {
		int first = 0, len = size;
		while (len > 0) {
			int half = len >> 1, middle = first + half;
			// Bisect range based on value of _pred_ at _middle_
			if (pred(middle)) {
				first = middle + 1;
				len -= half + 1;
			}
			else
				len = half;
		}
		return Clamp(first - 1, 0, size - 2);
	}

	__forceinline float RSqrt(const float &x)
	{
#if defined(AYA_USE_SIMD) && defined(AYA_USE_SQRT_APPROXIMATION)
		const __m128 a = _mm_set_ss(x);
		const __m128 r = _mm_rsqrt_ps(a);
		const __m128 c = _mm_add_ps(_mm_mul_ps(v1_5, r),
			_mm_mul_ps(_mm_mul_ps(_mm_mul_ps(a, v_0_5), r), _mm_mul_ps(r, r)));
		return _mm_cvtss_f32(c);
#else
		return 1.f / sqrtf(x);
#endif
	}
	__forceinline float Sqrt(const float &x) {
#if defined(AYA_USE_SIMD) && defined(AYA_USE_SQRT_APPROXIMATION)
		return 1.f / RSqrt(x);
#else
		return sqrtf(x);
#endif
	}

	__forceinline uint32_t roundUpToPowerOfTwo(uint32_t val) {
		--val;
		val |= val >> 1;
		val |= val >> 2;
		val |= val >> 4;
		val |= val >> 8;
		val |= val >> 16;
		return val + 1;
	}
	__forceinline uint32_t countLeadingZeros(uint32_t value) {
		unsigned long log2;
		if (_BitScanReverse(&log2, value)) return 31 - log2;
		return 32;
	}
	__forceinline uint32_t countTrailingZeros(uint32_t value) {
		if (value == 0) {
			return 32;
		}
		uint32_t bitidx; // 0-based, where the LSB is 0 and MSB is 31
		_BitScanForward((unsigned long *)&bitidx, value);
		return bitidx;
	}
	__forceinline uint32_t floorLog2(uint32_t value) {
		unsigned long log2;
		if (_BitScanReverse(&log2, value)) return log2;
		return 0;
	}
	__forceinline uint32_t ceilLog2(uint32_t value) {
		int bitmask = ((int)(countLeadingZeros(value) << 26)) >> 31;
		return (32 - countLeadingZeros(value - 1)) & (~bitmask);
	}
}

#endif