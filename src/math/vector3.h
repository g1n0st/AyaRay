#ifndef AYA_VECTOR3_H
#define AYA_VECTOR3_H

#include "MathUtility.h"

#if defined(AYA_USE_SIMD)
/* expands to the following value */
//#ifndef _MM_SHUFFLE
#define __MM_SHUFFLE(x, y, z, w) (((w) << 6 | (z) << 4 | (y) << 2 | (x)) & 0xff)
//#endif

#define _mm_pshufd_ps(_a, _mask) _mm_shuffle_ps((_a), (_a), (_mask))
#define _mm_splat_ps(_a, _i) _mm_pshufd_ps((_a), __MM_SHUFFLE(_i, _i, _i, _i))
#define _mm_splat3_ps(_a, _i) _mm_pshufd_ps((_a), __MM_SHUFFLE(_i, _i, _i, 3))

#define vFFF0Mask (_mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF))
#define vFFF0fMask _mm_castsi128_ps(vFFF0Mask)
#define vFF0FMask (_mm_set_epi32(0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF))
#define vFF0FfMask _mm_castsi128_ps(vFF0FMask)
#define vF0FFMask (_mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF))
#define vF0FFfMask _mm_castsi128_ps(vF0FFMask)
#define v0FFFMask (_mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000))
#define v0FFFfMask _mm_castsi128_ps(v0FFFMask)

#define v3AbsiMask (_mm_set_epi32(0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF))
#define vAbsMask (_mm_set_epi32(0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF))

#define vxyzMaskf vFFF0fMask
#define vAbsfMask _mm_castsi128_ps(vAbsMask)
#define vMzeroMask (_mm_set_ps(-0.0f, -0.0f, -0.0f, -0.0f))

#endif

__declspec(align(16)) 
class BaseVector3 {
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
	BaseVector3() {}
	inline BaseVector3(const float &x, const float &y, const float &z) {
		m_val[0] = x;
		m_val[1] = y;
		m_val[2] = z;
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
	inline BaseVector3(const __m128 &v128) {
		m_val128 = v128;
	}
	inline BaseVector3(const BaseVector3 &rhs) {
		m_val128 = rhs.m_val128;
	}
	inline BaseVector3& operator =(const BaseVector3 &rhs) {
		m_val128 = rhs.m_val128;
		return *this;
	}
#endif
	inline void setValue(const float &x, const float &y, const float &z) {
		m_val[0] = x;
		m_val[1] = y;
		m_val[2] = z;
		m_val[3] = .0f;
		numericValid(1);
	}
	inline void setX(const float &x) { m_val[0] = x; numericValid(1); }
	inline void setY(const float &y) { m_val[1] = y; numericValid(1); }
	inline void setZ(const float &z) { m_val[2] = z; numericValid(1); }
	inline void setW(const float &w) { m_val[3] = w; numericValid(1); }
	inline const float& getX() const { return m_val[0]; }
	inline const float& getY() const { return m_val[1]; }
	inline const float& getZ() const { return m_val[2]; }
	inline const float& x() const { return m_val[0]; }
	inline const float& y() const { return m_val[1]; }
	inline const float& z() const { return m_val[2]; }
	inline const float& w() const { return m_val[3]; }

	inline bool operator == (const BaseVector3 &v) const {
#if defined(AYA_USE_SIMD)
		return (0xf == _mm_movemask_ps((__m128)_mm_cmpeq_ps(m_val128, v.m_val128)));
#else
		return ((m_val[0] == v.m_val[0]) &&
			(m_val[1] == v.m_val[1]) &&
			(m_val[2] == v.m_val[2]) &&
			(m_val[3] == v.m_val[3]));
#endif
	}
	inline bool operator != (const BaseVector3 &v) const {
		return !((*this) == v);
	}

	inline void setMax(const BaseVector3 &v) {
#if defined(AYA_USE_SIMD)
		m_val128 = _mm_max_ps(m_val128, v.m_val128);
#else
		SetMax(m_val[0], v.m_val[0]);
		SetMax(m_val[1], v.m_val[1]);
		SetMax(m_val[2], v.m_val[2]);
#endif
	}
	inline void setMin(const BaseVector3 &v) {
#if defined(AYA_USE_SIMD)
		m_val128 = _mm_min_ps(m_val128, v.m_val128);
#else
		SetMin(m_val[0], v.m_val[0]);
		SetMin(m_val[1], v.m_val[1]);
		SetMin(m_val[2], v.m_val[2]);
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
	inline bool fuzzyZero() const {
		return length2() < SIMD_EPSILON * SIMD_EPSILON;
	}

	inline BaseVector3 operator + (const BaseVector3 &v) const {
#if defined(AYA_USE_SIMD)
		return BaseVector3(_mm_add_ps(m_val128, v.m_val128));
#else
		return BaseVector3(m_val[0] + v.m_val[0],
			m_val[1] + v.m_val[1],
			m_val[2] + v.m_val[2]);
#endif
	}
	inline BaseVector3 & operator += (const BaseVector3 &v) {
#if defined(AYA_USE_SIMD)
		m_val128 = _mm_add_ps(m_val128, v.m_val128);
#else
		m_val[0] += v.m_val[0];
		m_val[1] += v.m_val[1];
		m_val[2] += v.m_val[2];
#endif
		numericValid(1);
		return *this;
	}
	inline BaseVector3 operator - (const BaseVector3 &v) const {
#if defined(AYA_USE_SIMD)
		return BaseVector3(_mm_sub_ps(m_val128, v.m_val128));
#else
		return BaseVector3(m_val[0] - v.m_val[0],
			m_val[1] - v.m_val[1],
			m_val[2] - v.m_val[2]);
#endif
	}
	inline BaseVector3 & operator -= (const BaseVector3 &v) {
#if defined(AYA_USE_SIMD)
		m_val128 = _mm_sub_ps(m_val128, v.m_val128);
#else
		m_val[0] -= v.m_val[0];
		m_val[1] -= v.m_val[1];
		m_val[2] -= v.m_val[2];
#endif
		numericValid(1);
		return *this;
	}
	inline BaseVector3 operator- () const {
#if defined(AYA_USE_SIMD)
		__m128 r = _mm_xor_ps(m_val128, vMzeroMask);
		return BaseVector3(_mm_and_ps(r, vFFF0fMask));
#else
		return BaseVector3(-m_val[0], -m_val[1], -m_val[2]);
#endif
	}
	inline BaseVector3 operator * (const float &s) const {
#if defined(AYA_USE_SIMD)
		__m128 vs = _mm_load_ss(&s);
		vs = _mm_pshufd_ps(vs, 0x80);
		return BaseVector3(_mm_mul_ps(m_val128, vs));
#else
		return BaseVector3(m_val[0] * s,
			m_val[1] * s,
			m_val[2] * s);
#endif

	}
	inline friend BaseVector3 operator * (const float &s, const BaseVector3 &v) {
		return v * s;
	}
	inline BaseVector3 & operator *= (const float &s) {
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
	inline BaseVector3 operator / (const float &s) const {
		assert(s != 0.f);
		BaseVector3 ret;
		ret = (*this) * (1.f / s);
		return ret;
	}
	inline BaseVector3 & operator /= (const float &s) {
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

	inline float dot(const BaseVector3 &v) const {
#if defined(AYA_USE_SIMD)
		__m128 vd = _mm_mul_ps(m_val128, v.m_val128);
		__m128 z = _mm_movehl_ps(vd, vd);
		__m128 y = _mm_pshufd_ps(vd, 0x55);
		vd = _mm_add_ss(vd, y);
		vd = _mm_add_ss(vd, z);
		return _mm_cvtss_f32(vd);

#else
		return	m_val[0] * v.m_val[0] +
			m_val[1] * v.m_val[1] +
			m_val[2] * v.m_val[2];
#endif
	}

	inline float length2() const {
		return dot(*this);
	}
	inline float length() const {
		return Sqrt(length2());
	}
	inline float safeLength() const {
		float d = length2();
		if (d > SIMD_EPSILON) return Sqrt(d);
		return 0.f;
	}
	inline float distance2(const BaseVector3 &p) const {
		return (p - (*this)).length2();
	}
	inline float distance(const BaseVector3 &p) const {
		return (p - (*this)).length();
	}

	inline BaseVector3& normalize() {
#if defined(AYA_USE_SIMD)
		__m128 vd = _mm_mul_ps(m_val128, m_val128);
		__m128 z = _mm_movehl_ps(vd, vd);
		__m128 y = _mm_pshufd_ps(vd, 0x55);
		vd = _mm_add_ss(vd, y);
		vd = _mm_add_ss(vd, z);

		y = _mm_rsqrt_ss(vd);
		z = v1_5;
		vd = _mm_mul_ss(vd, v0_5);
		vd = _mm_mul_ss(vd, y);
		vd = _mm_mul_ss(vd, y);
		z = _mm_sub_ss(z, vd);
		y = _mm_mul_ss(y, z);

		y = _mm_splat_ps(y, 0x80);
		m_val128 = _mm_mul_ps(m_val128, y);

		return *this;
#else
		return *this /= length();
#endif
	}
	inline BaseVector3& safeNormalize() {
		float l2 = safeLength();
		if (l2 >= SIMD_EPSILON) {
			return *this /= l2;
		}
		else {
			setValue(1, 0, 0);
		}
		return *this;
	}

	inline BaseVector3 cross(const BaseVector3 &v) const {
#if defined(AYA_USE_SIMD)
		__m128 T, V;

		T = _mm_pshufd_ps(m_val128, __MM_SHUFFLE(1, 2, 0, 3)); //			(Y Z X 0)
		V = _mm_pshufd_ps(v.m_val128, __MM_SHUFFLE(1, 2, 0, 3)); //		(Y Z X 0)

		V = _mm_mul_ps(V, m_val128);
		T = _mm_mul_ps(T, v.m_val128);
		V = _mm_sub_ps(V, T);

		V = _mm_pshufd_ps(V, __MM_SHUFFLE(1, 2, 0, 3));
		return BaseVector3(V);
#else
		return BaseVector3(
			m_val[1] * v.m_val[2] - m_val[2] * v.m_val[1],
			m_val[2] * v.m_val[0] - m_val[0] * v.m_val[2],
			m_val[0] * v.m_val[1] - m_val[1] * v.m_val[0]);
#endif
	}

	inline float angle(const BaseVector3 &v) const {
		float s = Sqrt(length2() * v.length2());
		assert(s != 0);
		return acosf(dot(v) / s);
	}
	inline BaseVector3 rotate(const BaseVector3 &axis, const float &angle) const {
#if defined(AYA_USE_SIMD)
		__m128 O = _mm_mul_ps(axis.m_val128, m_val128);
		float ssin = sinf(angle);
		__m128 C = axis.cross(m_val128).m_val128;
		O = _mm_and_ps(O, vFFF0fMask);
		float scos = cosf(angle);

		__m128 vsin = _mm_load_ss(&ssin); // (S 0 0 0)
		__m128 vcos = _mm_load_ss(&scos); // (S 0 0 0)

		__m128 Y = _mm_pshufd_ps(O, 0xC9);    // (Y Z X 0)
		__m128 Z = _mm_pshufd_ps(O, 0xD2);    // (Z X Y 0)
		O = _mm_add_ps(O, Y);
		vsin = _mm_pshufd_ps(vsin, 0x80);     // (S S S 0)
		O = _mm_add_ps(O, Z);
		vcos = _mm_pshufd_ps(vcos, 0x80);     // (S S S 0)

		vsin = _mm_mul_ps(vsin, C);
		O = _mm_mul_ps(O, axis.m_val128);
		__m128 X = _mm_sub_ps(m_val128, O);

		O = _mm_add_ps(O, vsin);
		vcos = _mm_mul_ps(vcos, X);
		O = _mm_add_ps(O, vcos);

		return BaseVector3(O);
#else
		BaseVector3 o = axis * axis.dot(*this);
		BaseVector3 X = *this - o;
		BaseVector3 Y = axis.cross(*this);

		return (o + X * cosf(angle) + Y * sinf(angle));
#endif
	}
	inline BaseVector3 dot3(const BaseVector3 &v0, const BaseVector3 &v1, const BaseVector3 &v2) const {
#if defined(AYA_USE_SIMD)
		__m128 a0 = _mm_mul_ps(v0.m_val128, m_val128);
		__m128 a1 = _mm_mul_ps(v1.m_val128, m_val128);
		__m128 a2 = _mm_mul_ps(v2.m_val128, m_val128);
		__m128 b0 = _mm_unpacklo_ps(a0, a1);
		__m128 b1 = _mm_unpackhi_ps(a0, a1);
		__m128 b2 = _mm_unpacklo_ps(a2, _mm_setzero_ps());
		__m128 r = _mm_add_ps(_mm_movelh_ps(b0, b2), _mm_movehl_ps(b2, b0));
		a2 = _mm_and_ps(a2, vxyzMaskf);
		r = _mm_add_ps(r, _mm_castpd_ps(_mm_move_sd(_mm_castps_pd(a2), _mm_castps_pd(b1))));
		return BaseVector3(r);
#else
		return BaseVector3(dot(v0), dot(v1), dot(v2));
#endif
	}

	friend inline std::ostream &operator<<(std::ostream &os, const BaseVector3 &v) {
		os << "[ " << AYA_SCALAR_OUTPUT(v.m_val[0])
			<< ", " << AYA_SCALAR_OUTPUT(v.m_val[1])
			<< ", " << AYA_SCALAR_OUTPUT(v.m_val[2])
			<< " ]";
		return os;
	}
};

__declspec(align(16))
class Vector3 : public BaseVector3 {
public:
	Vector3() {}
	inline Vector3(const float &x, const float &y, const float &z) {
		m_val[0] = x;
		m_val[1] = y;
		m_val[2] = z;
		m_val[3] = .0f;
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
	inline Vector3(const __m128 &v128) {
		m_val128 = v128;
	}
	inline Vector3(const BaseVector3 &rhs) {
		m_val128 = rhs.m_val128;
	}
	inline Vector3& operator =(const BaseVector3 &rhs) {
		m_val128 = rhs.m_val128;
		return *this;
	}
#else
	inline Vector3(const BaseVector3 &rhs) {
		m_val[0] = rhs.m_val[0];
		m_val[1] = rhs.m_val[1];
		m_val[2] = rhs.m_val[2];
	}
	inline Vector3& operator =(const BaseVector3 &rhs) {
		m_val[0] = rhs.m_val[0];
		m_val[1] = rhs.m_val[1];
		m_val[2] = rhs.m_val[2];
		return *this;
	}
#endif
};

__declspec(align(16))
class Point3 : public BaseVector3 {
public:
	Point3() {}
	inline Point3(const float &x, const float &y, const float &z) {
		m_val[0] = x;
		m_val[1] = y;
		m_val[2] = z;
		m_val[3] = .0f;
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
	inline Point3(const __m128 &v128) {
		m_val128 = v128;
	}
	inline Point3(const BaseVector3 &rhs) {
		m_val128 = rhs.m_val128;
	}
	inline Point3& operator =(const BaseVector3 &rhs) {
		m_val128 = rhs.m_val128;
		return *this;
	}
#else
	inline Point3(const BaseVector3 &rhs) {
		m_val[0] = rhs.m_val[0];
		m_val[1] = rhs.m_val[1];
		m_val[2] = rhs.m_val[2];
	}
	inline Point3& operator =(const BaseVector3 &rhs) {
		m_val[0] = rhs.m_val[0];
		m_val[1] = rhs.m_val[1];
		m_val[2] = rhs.m_val[2];
		return *this;
	}
#endif
};

__declspec(align(16))
class Normal3 : public BaseVector3 {
public:
	Normal3() {}
	inline Normal3(const float &x, const float &y, const float &z) {
		m_val[0] = x;
		m_val[1] = y;
		m_val[2] = z;
		m_val[3] = .0f;
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
	inline Normal3(const __m128 &v128) {
		m_val128 = v128;
	}
	inline Normal3(const BaseVector3 &rhs) {
		m_val128 = rhs.m_val128;
	}
	inline Normal3& operator =(const BaseVector3 &rhs) {
		m_val128 = rhs.m_val128;
		return *this;
	}
#else
	inline Normal3(const BaseVector3 &rhs) {
		m_val[0] = rhs.m_val[0];
		m_val[1] = rhs.m_val[1];
		m_val[2] = rhs.m_val[2];
	}
	inline Normal3& operator =(const BaseVector3 &rhs) {
		m_val[0] = rhs.m_val[0];
		m_val[1] = rhs.m_val[1];
		m_val[2] = rhs.m_val[2];
		return *this;
	}
#endif
};

#endif