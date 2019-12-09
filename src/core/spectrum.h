#ifndef AYA_SPECTRUM_H
#define AYA_SPECTRUM_H

#include "config.h"
#include "../math/vector3.h"

__declspec(align(16))
class Spectrum {
#if defined(AYA_USE_SIMD)
public:
	union {
		float m_val[4];
		__m128 m_val128;
	};

	inline __m128 get128() const {
		return m_val128;
	}
	inline void set128(const __m128 &v128) {
		m_val128 = v128;
	}
#else
public:

	float m_val[4];

	inline const __m128& get128() const {
		return *((const __m128*)&m_val[0]);
	}
#endif

#if defined(AYA_DEBUG)
private:
	inline void numericValid(int x) {
		assert(!isnan(m_val[0]) && !isnan(m_val[1]) && !isnan(m_val[2]));
	}
#else
#define numericValid
#endif

public:
	Spectrum() {}
	inline Spectrum(const float &r, const float &g, const float &b) {
		m_val[0] = r;
		m_val[1] = g;
		m_val[2] = b;
		m_val[3] = .0f;
		numericValid(1);
	}
#if defined(AYA_USE_SIMD)
	inline void  *operator new(size_t i) {
		return _mm_malloc(i, 16);
	}

	inline void operator delete(void *p) {
		_mm_free(p);
	}
#endif

#if defined(AYA_USE_SIMD)
	inline Spectrum(const __m128 &v128) {
		m_val128 = v128;
	}
	inline Spectrum(const Spectrum &rhs) {
		m_val128 = rhs.m_val128;
	}
	inline Spectrum& operator =(const Spectrum &rhs) {
		m_val128 = rhs.m_val128;
		return *this;
	}
#endif
	inline void setValue(const float &r, const float &g, const float &b) {
		m_val[0] = r;
		m_val[1] = g;
		m_val[2] = b;
		m_val[3] = .0f;
		numericValid(1);
	}
	inline void setR(const float &r) { m_val[0] = r; numericValid(1); }
	inline void setG(const float &g) { m_val[1] = g; numericValid(1); }
	inline void setB(const float &b) { m_val[2] = b; numericValid(1); }
	inline const float& getR() const { return m_val[0]; }
	inline const float& getG() const { return m_val[1]; }
	inline const float& getB() const { return m_val[2]; }
	inline const float& r() const { return m_val[0]; }
	inline const float& g() const { return m_val[1]; }
	inline const float& b() const { return m_val[2]; }

	inline bool operator == (const Spectrum &s) const {
#if defined(AYA_USE_SIMD)
		return (0xf == _mm_movemask_ps((__m128)_mm_cmpeq_ps(m_val128, s.m_val128)));
#else
		return ((m_val[0] == s.m_val[0]) &&
			(m_val[1] == s.m_val[1]) &&
			(m_val[2] == s.m_val[2]) &&
			(m_val[3] == s.m_val[3]));
#endif
	}
	inline bool operator != (const Spectrum &s) const {
		return !((*this) == s);
	}

	inline void setMax(const Spectrum &s) {
#if defined(AYA_USE_SIMD)
		m_val128 = _mm_max_ps(m_val128, s.m_val128);
#else
		SetMax(m_val[0], s.m_val[0]);
		SetMax(m_val[1], s.m_val[1]);
		SetMax(m_val[2], s.m_val[2]);
#endif
	}
	inline void setMin(const Spectrum &s) {
#if defined(AYA_USE_SIMD)
		m_val128 = _mm_min_ps(m_val128, s.m_val128);
#else
		SetMin(m_val[0], s.m_val[0]);
		SetMin(m_val[1], s.m_val[1]);
		SetMin(m_val[2], s.m_val[2]);
#endif
	}
	inline void setZero() {
#if defined(AYA_USE_SIMD)
		m_val128 = _mm_xor_ps(m_val128, m_val128);
#else
		m_val[0] = 0.f;
		m_val[1] = 0.f;
		m_val[2] = 0.f;
		m_val[3] = 0.f;
#endif
	}

	inline bool isZero() const {
		return (m_val[0] == 0.f && m_val[1] == 0.f && m_val[2] == 0.f);
	}

	inline Spectrum operator + (const Spectrum &s) const {
#if defined(AYA_USE_SIMD)
		return Spectrum(_mm_add_ps(m_val128, s.m_val128));
#else
		return Spectrum(m_val[0] + s.m_val[0],
			m_val[1] + s.m_val[1],
			m_val[2] + s.m_val[2]);
#endif
	}
	inline Spectrum & operator += (const Spectrum &s) {
#if defined(AYA_USE_SIMD)
		m_val128 = _mm_add_ps(m_val128, s.m_val128);
#else
		m_val[0] += s.m_val[0];
		m_val[1] += s.m_val[1];
		m_val[2] += s.m_val[2];
#endif
		numericValid(1);
		return *this;
	}
	inline Spectrum operator - (const Spectrum &s) const {
#if defined(AYA_USE_SIMD)
		return Spectrum(_mm_sub_ps(m_val128, s.m_val128));
#else
		return Spectrum(m_val[0] - s.m_val[0],
			m_val[1] - s.m_val[1],
			m_val[2] - s.m_val[2]);
#endif
	}
	inline Spectrum & operator -= (const Spectrum &s) {
#if defined(AYA_USE_SIMD)
		m_val128 = _mm_sub_ps(m_val128, s.m_val128);
#else
		m_val[0] -= s.m_val[0];
		m_val[1] -= s.m_val[1];
		m_val[2] -= s.m_val[2];
#endif
		numericValid(1);
		return *this;
	}
	inline Spectrum operator- () const {
#if defined(AYA_USE_SIMD)
		__m128 r = _mm_xor_ps(m_val128, vMzeroMask);
		return Spectrum(_mm_and_ps(r, vFFF0fMask));
#else
		return Spectrum(-m_val[0], -m_val[1], -m_val[2]);
#endif
	}
	inline Spectrum operator * (const Spectrum &s) const {
#if defined(AYA_USE_SIMD)
		return Spectrum(_mm_mul_ps(m_val128, s.m_val128));
#else
		return Spectrum(m_val[0] * s.m_val[0],
			m_val[1] * s.m_val[1],
			m_val[2] * s.m_val[2]);
#endif
	}
	inline Spectrum & operator *= (const Spectrum &s) {
#if defined(AYA_USE_SIMD)
		m_val128 = _mm_mul_ps(m_val128, s.m_val128);
#else
		m_val[0] *= s.m_val[0];
		m_val[1] *= s.m_val[1];
		m_val[2] *= s.m_val[2];
#endif
		numericValid(1);
		return *this;
	}
	inline Spectrum operator * (const float &s) const {
#if defined(AYA_USE_SIMD)
		__m128 vs = _mm_load_ss(&s);
		vs = _mm_pshufd_ps(vs, 0x80);
		return Spectrum(_mm_mul_ps(m_val128, vs));
#else
		return Spectrum(m_val[0] * s,
			m_val[1] * s,
			m_val[2] * s);
#endif
	}
	inline friend Spectrum operator * (const float &v, const Spectrum &s) {
		return s * v;
	}
	inline Spectrum & operator *= (const float &s) {
#if defined(AYA_USE_SIMD)
		__m128 vs = _mm_load_ss(&s);
		vs = _mm_pshufd_ps(vs, 0x80);
		m_val128 = _mm_mul_ps(m_val128, vs);
#else
		m_val[0] *= s;
		m_val[1] *= s;
		m_val[2] *= s;
#endif
		numericValid(1);
		return *this;
	}
	inline Spectrum operator / (const float &s) const {
		assert(s != 0.f);
		Spectrum ret;
		ret = (*this) * (1.f / s);
		return ret;
	}
	inline Spectrum & operator /= (const float &s) {
		assert(s != 0.f);
		return *this *= (1.f / s);
	}

	float operator [](const int &p) const {
		assert(p >= 0 && p <= 2);
		return m_val[p];
	}
	float &operator [](const int &p) {
		assert(p >= 0 && p <= 2);
		return m_val[p];
	}

	inline Spectrum & sqrt () {
		m_val[0] = Sqrt(m_val[0]);
		m_val[1] = Sqrt(m_val[1]);
		m_val[2] = Sqrt(m_val[2]);
		return *this;
	}

	friend inline std::ostream &operator<<(std::ostream &os, const Spectrum &s) {
		os << "[ " << AYA_SCALAR_OUTPUT(s.m_val[0])
			<< ", " << AYA_SCALAR_OUTPUT(s.m_val[1])
			<< ", " << AYA_SCALAR_OUTPUT(s.m_val[2])
			<< " ]";
		return os;
	}
};

#endif