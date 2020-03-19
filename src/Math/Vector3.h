#ifndef AYA_MATH_VECTOR3_H
#define AYA_MATH_VECTOR3_H

#include "MathUtility.h"

#if defined(AYA_USE_SIMD)
/* expands to the following value */
//#ifndef _MM_SHUFFLE
#define __MM_SHUFFLE(x, y, z, w) (((w) << 6 | (z) << 4 | (y) << 2 | (x)) & 0xff)
//#endif

#define _mm_pshufd_ps(_a, _mask) _mm_shuffle_ps((_a), (_a), (_mask))
#define _mm_splat_ps(_a, _i) _mm_pshufd_ps((_a), __MM_SHUFFLE(_i, _i, _i, _i))
#define _mm_splat3_ps(_a, _i) _mm_pshufd_ps((_a), __MM_SHUFFLE(_i, _i, _i, 3))

#define vFFFFMask (_mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF))
#define vFFFFfMask _mm_castsi128_ps(vFFFFMask)
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
#define vxyzwMaskf vFFFFfMask
#define vAbsfMask _mm_castsi128_ps(vAbsMask)
#define vMzeroMask (_mm_set_ps(-0.0f, -0.0f, -0.0f, -0.0f))

#define vMPPP (_mm_set_ps(+0.0f, +0.0f, +0.0f, -0.0f))
#define v1000 (_mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f))
#define v0100 (_mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f))
#define v0010 (_mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f))
#define v0001 (_mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f))

#define _mm_swizzle_mask(_a, _mask) _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(_a), _mask))
#define _mm_swizzle(_a, x, y, z, w) _mm_swizzle_mask(_a, __MM_SHUFFLE(x, y, z, w))
#define _mm_swizzle1(_a, x) _mm_swizzle_mask(_a, __MM_SHUFFLE(x, x, x, x))
#define _mm_shuffle2(_a, _b, x, y, z, w)    _mm_shuffle_ps(_a, _b, __MM_SHUFFLE(x, y, z, w))
#endif

namespace Aya {
#if defined(AYA_USE_SIMD)
	__declspec(align(16))
#endif
		class QuadWord {
#if defined(AYA_USE_SIMD)
		public:
			union {
				float m_val[4];
				__m128 m_val128;
			};

			AYA_FORCE_INLINE __m128 get128() const {
				return m_val128;
			}
			AYA_FORCE_INLINE void set128(const __m128 &v128) {
				m_val128 = v128;
			}
#else
		public:

			float m_val[4];

			AYA_FORCE_INLINE const __m128& get128() const {
				return *((const __m128*)&m_val[0]);
			}
#endif

		public:
			QuadWord() {}
			AYA_FORCE_INLINE QuadWord(const float &x, const float &y, const float &z, const float &w) {
				m_val[0] = x;
				m_val[1] = y;
				m_val[2] = z;
				m_val[3] = w;
			}

#if defined(AYA_USE_SIMD)
			AYA_FORCE_INLINE void  *operator new(size_t i) {
				return _mm_malloc(i, 16);
			}

			AYA_FORCE_INLINE void operator delete(void *p) {
				_mm_free(p);
			}
#endif

#if defined(AYA_USE_SIMD)
			AYA_FORCE_INLINE QuadWord(const __m128 &v128) {
				m_val128 = v128;
			}
			AYA_FORCE_INLINE QuadWord(const QuadWord &rhs) {
				m_val128 = rhs.m_val128;
			}
			AYA_FORCE_INLINE QuadWord& operator = (const QuadWord &rhs) {
				m_val128 = rhs.m_val128;
				return *this;
			}
#endif

			AYA_FORCE_INLINE void setX(const float &x) { m_val[0] = x; }
			AYA_FORCE_INLINE void setY(const float &y) { m_val[1] = y; }
			AYA_FORCE_INLINE void setZ(const float &z) { m_val[2] = z; }
			AYA_FORCE_INLINE void setW(const float &w) { m_val[3] = w; }
			AYA_FORCE_INLINE const float& getX() const { return m_val[0]; }
			AYA_FORCE_INLINE const float& getY() const { return m_val[1]; }
			AYA_FORCE_INLINE const float& getZ() const { return m_val[2]; }
			AYA_FORCE_INLINE const float& x() const { return m_val[0]; }
			AYA_FORCE_INLINE const float& y() const { return m_val[1]; }
			AYA_FORCE_INLINE const float& z() const { return m_val[2]; }
			AYA_FORCE_INLINE const float& w() const { return m_val[3]; }

			AYA_FORCE_INLINE float operator [](const int &p) const {
				assert(p >= 0 && p <= 3);
				return m_val[p];
			}
			AYA_FORCE_INLINE float &operator [](const int &p) {
				assert(p >= 0 && p <= 3);
				return m_val[p];
			}

			friend inline std::ostream &operator<<(std::ostream &os, const QuadWord &v) {
				os << "[ " << AYA_SCALAR_OUTPUT(v.m_val[0])
					<< ", " << AYA_SCALAR_OUTPUT(v.m_val[1])
					<< ", " << AYA_SCALAR_OUTPUT(v.m_val[2])
					<< ", " << AYA_SCALAR_OUTPUT(v.m_val[3])
					<< " ]";
				return os;
			}
	};

#if defined(AYA_USE_SIMD)
	__declspec(align(16))
#endif
		class BaseVector3 : public QuadWord {
#if defined(AYA_DEBUG)
		private:
			AYA_FORCE_INLINE void numericValid(int x) {
				assert(!isnan(m_val[0]) && !isnan(m_val[1]) && !isnan(m_val[2]));
			}
#else
#define numericValid
#endif

		public:
			BaseVector3() {}
			AYA_FORCE_INLINE BaseVector3(const float &x, const float &y, const float &z) {
				m_val[0] = x;
				m_val[1] = y;
				m_val[2] = z;
				m_val[3] = .0f;
				numericValid(1);
			}

#if defined(AYA_USE_SIMD)
			AYA_FORCE_INLINE BaseVector3(const __m128 &v128) {
				m_val128 = v128;
			}
			AYA_FORCE_INLINE BaseVector3(const BaseVector3 &rhs) {
				m_val128 = rhs.m_val128;
			}
			AYA_FORCE_INLINE BaseVector3& operator = (const BaseVector3 &rhs) {
				m_val128 = rhs.m_val128;
				return *this;
			}
#endif
			AYA_FORCE_INLINE void setValue(const float &x, const float &y, const float &z) {
				m_val[0] = x;
				m_val[1] = y;
				m_val[2] = z;
				m_val[3] = .0f;
				numericValid(1);
			}

			AYA_FORCE_INLINE bool operator == (const BaseVector3 &v) const {
#if defined(AYA_USE_SIMD)
				return (0xf == _mm_movemask_ps((__m128)_mm_cmpeq_ps(m_val128, v.m_val128)));
#else
				return ((m_val[0] == v.m_val[0]) &&
					(m_val[1] == v.m_val[1]) &&
					(m_val[2] == v.m_val[2]) &&
					(m_val[3] == v.m_val[3]));
#endif
			}
			AYA_FORCE_INLINE bool operator != (const BaseVector3 &v) const {
				return !((*this) == v);
			}

			AYA_FORCE_INLINE void setMax(const BaseVector3 &v) {
#if defined(AYA_USE_SIMD)
				m_val128 = _mm_max_ps(m_val128, v.m_val128);
#else
				SetMax(m_val[0], v.m_val[0]);
				SetMax(m_val[1], v.m_val[1]);
				SetMax(m_val[2], v.m_val[2]);
#endif
			}
			AYA_FORCE_INLINE void setMin(const BaseVector3 &v) {
#if defined(AYA_USE_SIMD)
				m_val128 = _mm_min_ps(m_val128, v.m_val128);
#else
				SetMin(m_val[0], v.m_val[0]);
				SetMin(m_val[1], v.m_val[1]);
				SetMin(m_val[2], v.m_val[2]);
#endif
			}
			AYA_FORCE_INLINE void setZero() {
#if defined(AYA_USE_SIMD)
				m_val128 = _mm_xor_ps(m_val128, m_val128);
#else
				m_val[0] = 0.f;
				m_val[1] = 0.f;
				m_val[2] = 0.f;
				m_val[3] = 0.f;
#endif
			}

			AYA_FORCE_INLINE bool isZero() const {
				return (m_val[0] == 0.f && m_val[1] == 0.f && m_val[2] == 0.f);
			}
			AYA_FORCE_INLINE bool fuzzyZero() const {
				return length2() < AYA_EPSILON * AYA_EPSILON;
			}

			AYA_FORCE_INLINE BaseVector3 operator + (const BaseVector3 &v) const {
#if defined(AYA_USE_SIMD)
				return BaseVector3(_mm_add_ps(m_val128, v.m_val128));
#else
				return BaseVector3(m_val[0] + v.m_val[0],
					m_val[1] + v.m_val[1],
					m_val[2] + v.m_val[2]);
#endif
			}
			AYA_FORCE_INLINE BaseVector3 & operator += (const BaseVector3 &v) {
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
			AYA_FORCE_INLINE BaseVector3 operator - (const BaseVector3 &v) const {
#if defined(AYA_USE_SIMD)
				return BaseVector3(_mm_sub_ps(m_val128, v.m_val128));
#else
				return BaseVector3(m_val[0] - v.m_val[0],
					m_val[1] - v.m_val[1],
					m_val[2] - v.m_val[2]);
#endif
			}
			AYA_FORCE_INLINE BaseVector3 & operator -= (const BaseVector3 &v) {
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
			AYA_FORCE_INLINE BaseVector3 operator- () const {
#if defined(AYA_USE_SIMD)
				__m128 r = _mm_xor_ps(m_val128, vMzeroMask);
				return BaseVector3(_mm_and_ps(r, vFFF0fMask));
#else
				return BaseVector3(-m_val[0], -m_val[1], -m_val[2]);
#endif
			}
			AYA_FORCE_INLINE BaseVector3 operator * (const float &s) const {
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
			AYA_FORCE_INLINE friend BaseVector3 operator * (const float &s, const BaseVector3 &v) {
				return v * s;
			}
			AYA_FORCE_INLINE BaseVector3 & operator *= (const float &s) {
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
			AYA_FORCE_INLINE BaseVector3 operator / (const float &s) const {
				assert(s != 0.f);
				BaseVector3 ret;
				ret = (*this) * (1.f / s);
				return ret;
			}
			AYA_FORCE_INLINE BaseVector3 & operator /= (const float &s) {
				assert(s != 0.f);
				return *this *= (1.f / s);
			}

			AYA_FORCE_INLINE float dot(const BaseVector3 &v) const {
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

			AYA_FORCE_INLINE float length2() const {
				return dot(*this);
			}
			AYA_FORCE_INLINE float length() const {
				return Sqrt(length2());
			}
			AYA_FORCE_INLINE float safeLength() const {
				float d = length2();
				if (d > AYA_EPSILON) return Sqrt(d);
				return 0.f;
			}
			AYA_FORCE_INLINE float distance2(const BaseVector3 &p) const {
				return (p - (*this)).length2();
			}
			AYA_FORCE_INLINE float distance(const BaseVector3 &p) const {
				return (p - (*this)).length();
			}

			AYA_FORCE_INLINE BaseVector3 normalize() const {
				BaseVector3 ret;
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
				ret.m_val128 = _mm_mul_ps(m_val128, y);

				return ret;
#else
				return *this / length();
#endif
			}
			AYA_FORCE_INLINE void normalized() {
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
#else
				*this /= length();
#endif
			}

			AYA_FORCE_INLINE BaseVector3 cross(const BaseVector3 &v) const {
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

			AYA_FORCE_INLINE BaseVector3 dot3(const BaseVector3 &v0, const BaseVector3 &v1, const BaseVector3 &v2) const {
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

			static AYA_FORCE_INLINE BaseVector3 sphericalDirection(float sin_theta, float cos_theta, float phi) {
				return BaseVector3(sin_theta * std::cosf(phi),
					cos_theta,
					sin_theta * std::sinf(phi));
			}
			static AYA_FORCE_INLINE BaseVector3 sphericalDirection(float sin_theta, float cos_theta,
				float phi, const BaseVector3& vX,
				const BaseVector3& vY, const BaseVector3& vZ)
			{
				return sin_theta * std::cosf(phi) * vX +
					cos_theta * vY + 
					sin_theta * std::sinf(phi) * vZ;
			}
			static AYA_FORCE_INLINE float sphericalTheta(const BaseVector3& v) {
				return std::acosf(Clamp(v.y(), -1.f, 1.f));
			}
			static AYA_FORCE_INLINE float sphericalPhi(const BaseVector3& v) {
				float p = std::atan2f(v.z(), v.x());
				return (p < 0.f) ? p + float(M_PI) * 2.f : p;
			}
			static AYA_FORCE_INLINE void coordinateSystem(const BaseVector3 &x, BaseVector3 *y, BaseVector3 *z) {
				if (Abs(x.x()) > Abs(x.y())) {
					float inv = 1.f / Sqrt(x.x() * x.x() + x.z() * x.z());
					*y = BaseVector3(-x.z() * inv, 0.f, x.x() * inv);
				}
				else {
					float inv = 1.f / Sqrt(x.y() * x.y() + x.z() * x.z());
					*y = BaseVector3(0.f, x.z() * inv, -x.y() * inv);
				}
				*z = x.cross(*y);
			}

			friend inline std::ostream &operator<<(std::ostream &os, const BaseVector3 &v) {
				os << "[ " << AYA_SCALAR_OUTPUT(v.m_val[0])
					<< ", " << AYA_SCALAR_OUTPUT(v.m_val[1])
					<< ", " << AYA_SCALAR_OUTPUT(v.m_val[2])
					<< " ]";
				return os;
			}
	};

#if defined(AYA_USE_SIMD)
	__declspec(align(16))
#endif
		class Vector3 : public BaseVector3 {
		public:
			Vector3() {}
			AYA_FORCE_INLINE Vector3(const float &x, const float &y, const float &z) {
				m_val[0] = x;
				m_val[1] = y;
				m_val[2] = z;
				m_val[3] = .0f;
			}
#if defined(AYA_USE_SIMD)
			AYA_FORCE_INLINE void  *operator new(size_t i) {
				return _mm_malloc(i, 16);
			}

			AYA_FORCE_INLINE void operator delete(void *p) {
				_mm_free(p);
			}
#endif

#if defined(AYA_USE_SIMD)
			AYA_FORCE_INLINE Vector3(const __m128 &v128) {
				m_val128 = v128;
			}
			AYA_FORCE_INLINE Vector3(const BaseVector3 &rhs) {
				m_val128 = rhs.m_val128;
			}
			AYA_FORCE_INLINE Vector3& operator =(const BaseVector3 &rhs) {
				m_val128 = rhs.m_val128;
				return *this;
			}
#else
			AYA_FORCE_INLINE Vector3(const BaseVector3 &rhs) {
				m_val[0] = rhs.m_val[0];
				m_val[1] = rhs.m_val[1];
				m_val[2] = rhs.m_val[2];
			}
			AYA_FORCE_INLINE Vector3& operator =(const BaseVector3 &rhs) {
				m_val[0] = rhs.m_val[0];
				m_val[1] = rhs.m_val[1];
				m_val[2] = rhs.m_val[2];
				return *this;
			}
#endif
	};

#if defined(AYA_USE_SIMD)
	__declspec(align(16))
#endif
		class Point3 : public BaseVector3 {
		public:
			Point3() {}
			AYA_FORCE_INLINE Point3(const float &x, const float &y, const float &z) {
				m_val[0] = x;
				m_val[1] = y;
				m_val[2] = z;
				m_val[3] = .0f;
			}
#if defined(AYA_USE_SIMD)
			AYA_FORCE_INLINE void  *operator new(size_t i) {
				return _mm_malloc(i, 16);
			}

			AYA_FORCE_INLINE void operator delete(void *p) {
				_mm_free(p);
			}
#endif

#if defined(AYA_USE_SIMD)
			AYA_FORCE_INLINE Point3(const __m128 &v128) {
				m_val128 = v128;
			}
			AYA_FORCE_INLINE Point3(const BaseVector3 &rhs) {
				m_val128 = rhs.m_val128;
			}
			AYA_FORCE_INLINE Point3& operator =(const BaseVector3 &rhs) {
				m_val128 = rhs.m_val128;
				return *this;
			}
#else
			AYA_FORCE_INLINE Point3(const BaseVector3 &rhs) {
				m_val[0] = rhs.m_val[0];
				m_val[1] = rhs.m_val[1];
				m_val[2] = rhs.m_val[2];
			}
			AYA_FORCE_INLINE Point3& operator =(const BaseVector3 &rhs) {
				m_val[0] = rhs.m_val[0];
				m_val[1] = rhs.m_val[1];
				m_val[2] = rhs.m_val[2];
				return *this;
			}
#endif
	};

#if defined(AYA_USE_SIMD)
	__declspec(align(16))
#endif
		class Normal3 : public BaseVector3 {
		public:
			Normal3() {}
			AYA_FORCE_INLINE Normal3(const float &x, const float &y, const float &z) {
				m_val[0] = x;
				m_val[1] = y;
				m_val[2] = z;
				m_val[3] = .0f;
			}
#if defined(AYA_USE_SIMD)
			AYA_FORCE_INLINE void  *operator new(size_t i) {
				return _mm_malloc(i, 16);
			}

			AYA_FORCE_INLINE void operator delete(void *p) {
				_mm_free(p);
			}
#endif

#if defined(AYA_USE_SIMD)
			AYA_FORCE_INLINE Normal3(const __m128 &v128) {
				m_val128 = v128;
			}
			AYA_FORCE_INLINE Normal3(const BaseVector3 &rhs) {
				m_val128 = rhs.m_val128;
			}
			AYA_FORCE_INLINE Normal3& operator =(const BaseVector3 &rhs) {
				m_val128 = rhs.m_val128;
				return *this;
			}
#else
			AYA_FORCE_INLINE Normal3(const BaseVector3 &rhs) {
				m_val[0] = rhs.m_val[0];
				m_val[1] = rhs.m_val[1];
				m_val[2] = rhs.m_val[2];
			}
			AYA_FORCE_INLINE Normal3& operator =(const BaseVector3 &rhs) {
				m_val[0] = rhs.m_val[0];
				m_val[1] = rhs.m_val[1];
				m_val[2] = rhs.m_val[2];
				return *this;
			}
#endif
	};
}

#endif