#ifndef AYA_MATH_MATRIX3X3_H
#define AYA_MATH_MATRIX3X3_H

#include "Vector3.h"

#if defined(AYA_USE_SIMD)
#define vMPPP (_mm_set_ps(+0.0f, +0.0f, +0.0f, -0.0f))
#define v1000 (_mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f))
#define v0100 (_mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f))
#define v0010 (_mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f))
#endif

namespace Aya {
#if defined(AYA_USE_SIMD)
	__declspec(align(16))
#endif
		class Matrix3x3 {
		public:
			BaseVector3 m_el[3];

			Matrix3x3() {}
			Matrix3x3(const float &xx, const float &xy, const float &xz,
				const float &yx, const float &yy, const float &yz,
				const float &zx, const float &zy, const float &zz) {
				setValue(xx, xy, xz,
					yx, yy, yz,
					zx, zy, zz);
			}

#if defined(AYA_USE_SIMD)
			AYA_FORCE_INLINE Matrix3x3(const __m128  &v0, const __m128  &v1, const __m128  &v2) {
				m_el[0].m_val128 = v0;
				m_el[1].m_val128 = v1;
				m_el[2].m_val128 = v2;
			}
			AYA_FORCE_INLINE Matrix3x3(const BaseVector3 &v0, const BaseVector3 &v1, const BaseVector3 &v2) {
				m_el[0] = v0;
				m_el[1] = v1;
				m_el[2] = v2;
			}
			AYA_FORCE_INLINE Matrix3x3(const Matrix3x3 &rhs) {
				m_el[0].m_val128 = rhs.m_el[0].m_val128;
				m_el[1].m_val128 = rhs.m_el[1].m_val128;
				m_el[2].m_val128 = rhs.m_el[2].m_val128;
			}
			AYA_FORCE_INLINE Matrix3x3& operator = (const Matrix3x3 &rhs) {
				m_el[0].m_val128 = rhs.m_el[0].m_val128;
				m_el[1].m_val128 = rhs.m_el[1].m_val128;
				m_el[2].m_val128 = rhs.m_el[2].m_val128;

				return *this;
			}
#if defined(AYA_USE_SIMD)
			AYA_FORCE_INLINE void  *operator new(size_t i) {
				return _mm_malloc(i, 16);
			}

			AYA_FORCE_INLINE void operator delete(void *p) {
				_mm_free(p);
			}
#endif

#else
			AYA_FORCE_INLINE Matrix3x3(const BaseVector3 &v0, const BaseVector3 &v1, const BaseVector3 &v2) {
				m_el[0] = v0;
				m_el[1] = v1;
				m_el[2] = v2;
			}
			AYA_FORCE_INLINE Matrix3x3(const Matrix3x3 &rhs) {
				m_el[0] = rhs.m_el[0];
				m_el[1] = rhs.m_el[1];
				m_el[2] = rhs.m_el[2];
			}
			AYA_FORCE_INLINE Matrix3x3& operator = (const Matrix3x3 &rhs) {
				m_el[0] = rhs.m_el[0];
				m_el[1] = rhs.m_el[1];
				m_el[2] = rhs.m_el[2];

				return *this;
			}
#endif
			AYA_FORCE_INLINE BaseVector3 getColumn(int x) const {
				assert(x >= 0 && x < 3);
				return BaseVector3(m_el[0][x], m_el[1][x], m_el[2][x]);
			}
			AYA_FORCE_INLINE BaseVector3 getRow(int x) const {
				assert(x >= 0 && x < 3);
				return m_el[x];
			}
			AYA_FORCE_INLINE BaseVector3& operator [](int x) {
				assert(x >= 0 && x < 3);
				return m_el[x];
			}
			AYA_FORCE_INLINE const BaseVector3& operator [](int x) const {
				assert(x >= 0 && x < 3);
				return m_el[x];
			}

			void setValue(const float &xx, const float &xy, const float &xz,
				const float &yx, const float &yy, const float &yz,
				const float &zx, const float &zy, const float &zz) {
				m_el[0].setValue(xx, xy, xz);
				m_el[1].setValue(yx, yy, yz);
				m_el[2].setValue(zx, zy, zz);
			}

			void setIdentity() {
#if defined(AYA_USE_SIMD)
				m_el[0] = v1000;
				m_el[1] = v0100;
				m_el[2] = v0010;
#else
				setValue(1.f, 0.f, 0.f,
					0.f, 1.f, 0.f,
					0.f, 0.f, 1.f);
#endif
			}

			Matrix3x3 getIdentity() {
#if defined(AYA_USE_SIMD)
				return Matrix3x3(v1000, v0100, v0010);
#else
				return Matrix3x3(1, 0, 0,
					0, 1, 0,
					0, 0, 1);
#endif
			}

			AYA_FORCE_INLINE Matrix3x3 operator + (const Matrix3x3& m1) const {
#if defined(AYA_USE_SIMD)
				return Matrix3x3(_mm_add_ps(m_el[0].m_val128, m1.m_el[0].m_val128),
					_mm_add_ps(m_el[1].m_val128, m1.m_el[1].m_val128),
					_mm_add_ps(m_el[2].m_val128, m1.m_el[2].m_val128));
#else
				return Matrix3x3(
					m_el[0][0] + m1[0][0],
					m_el[0][1] + m1[0][1],
					m_el[0][2] + m1[0][2],

					m_el[1][0] + m1[1][0],
					m_el[1][1] + m1[1][1],
					m_el[1][2] + m1[1][2],

					m_el[2][0] + m1[2][0],
					m_el[2][1] + m1[2][1],
					m_el[2][2] + m1[2][2]
				);
#endif
			}
			AYA_FORCE_INLINE Matrix3x3& operator += (const Matrix3x3& m) {
#if defined(AYA_USE_SIMD)
				m_el[0].m_val128 = _mm_add_ps(m_el[0].m_val128, m.m_el[0].m_val128);
				m_el[1].m_val128 = _mm_add_ps(m_el[1].m_val128, m.m_el[1].m_val128);
				m_el[2].m_val128 = _mm_add_ps(m_el[2].m_val128, m.m_el[2].m_val128);
#else
				m_el[0][0] += m[0][0];
				m_el[0][1] += m[0][1];
				m_el[0][2] += m[0][2];

				m_el[1][0] += m[1][0];
				m_el[1][1] += m[1][1];
				m_el[1][2] += m[1][2];

				m_el[2][0] += m[2][0];
				m_el[2][1] += m[2][1];
				m_el[2][2] += m[2][2];
#endif
				return *this;
			}
			AYA_FORCE_INLINE Matrix3x3 operator - (const Matrix3x3& m1) const {
#if defined(AYA_USE_SIMD)
				return Matrix3x3(_mm_sub_ps(m_el[0].m_val128, m1.m_el[0].m_val128),
					_mm_sub_ps(m_el[1].m_val128, m1.m_el[1].m_val128),
					_mm_sub_ps(m_el[2].m_val128, m1.m_el[2].m_val128));
#else
				return Matrix3x3(
					m_el[0][0] - m1[0][0],
					m_el[0][1] - m1[0][1],
					m_el[0][2] - m1[0][2],

					m_el[1][0] - m1[1][0],
					m_el[1][1] - m1[1][1],
					m_el[1][2] - m1[1][2],

					m_el[2][0] - m1[2][0],
					m_el[2][1] - m1[2][1],
					m_el[2][2] - m1[2][2]
				);
#endif
			}
			AYA_FORCE_INLINE Matrix3x3& operator -= (const Matrix3x3& m) {
#if defined(AYA_USE_SIMD)
				m_el[0].m_val128 = _mm_sub_ps(m_el[0].m_val128, m.m_el[0].m_val128);
				m_el[1].m_val128 = _mm_sub_ps(m_el[1].m_val128, m.m_el[1].m_val128);
				m_el[2].m_val128 = _mm_sub_ps(m_el[2].m_val128, m.m_el[2].m_val128);
#else
				m_el[0][0] -= m[0][0];
				m_el[0][1] -= m[0][1];
				m_el[0][2] -= m[0][2];

				m_el[1][0] -= m[1][0];
				m_el[1][1] -= m[1][1];
				m_el[1][2] -= m[1][2];

				m_el[2][0] -= m[2][0];
				m_el[2][1] -= m[2][1];
				m_el[2][2] -= m[2][2];
#endif
				return *this;
			}
			AYA_FORCE_INLINE Matrix3x3 operator * (const float &s) const {
#if defined(AYA_USE_SIMD)
				__m128 vk = _mm_splat_ps(_mm_load_ss((float*)&s), 0x80);
				return Matrix3x3(_mm_mul_ps(m_el[0].m_val128, vk),
					_mm_mul_ps(m_el[1].m_val128, vk),
					_mm_mul_ps(m_el[2].m_val128, vk));
#else
				return Matrix3x3(
					m_el[0][0] * s,
					m_el[0][1] * s,
					m_el[0][2] * s,

					m_el[1][0] * s,
					m_el[1][1] * s,
					m_el[1][2] * s,

					m_el[2][0] * s,
					m_el[2][1] * s,
					m_el[2][2] * s
				);
#endif
			}
			AYA_FORCE_INLINE Matrix3x3& operator *= (const float &s) {
#if defined(AYA_USE_SIMD)
				__m128 vk = _mm_splat_ps(_mm_load_ss((float*)&s), 0x80);
				m_el[0].m_val128 = _mm_mul_ps(m_el[0].m_val128, vk);
				m_el[1].m_val128 = _mm_mul_ps(m_el[1].m_val128, vk);
				m_el[2].m_val128 = _mm_mul_ps(m_el[2].m_val128, vk);
#else
				m_el[0][0] *= s;
				m_el[0][1] *= s;
				m_el[0][2] *= s;

				m_el[1][0] *= s;
				m_el[1][1] *= s;
				m_el[1][2] *= s;

				m_el[2][0] *= s;
				m_el[2][1] *= s;
				m_el[2][2] *= s;
#endif
				return *this;
			}
			AYA_FORCE_INLINE friend Matrix3x3 operator * (const float &s, const Matrix3x3 &v) {
				return v * s;
			}
			AYA_FORCE_INLINE Matrix3x3 operator / (const float &s) const {
				assert(s != 0);
				return (*this) * (1.f / s);
			}
			AYA_FORCE_INLINE Matrix3x3& operator /= (const float &s) {
				assert(s != 0);
				return (*this) *= (1.f / s);
			}
			AYA_FORCE_INLINE Matrix3x3 operator * (const Matrix3x3 &m) const {
#if defined(AYA_USE_SIMD)
				__m128 m10 = m_el[0].m_val128;
				__m128 m11 = m_el[1].m_val128;
				__m128 m12 = m_el[2].m_val128;

				__m128 m2v = _mm_and_ps(m[0].m_val128, vFFF0fMask);

				__m128 c0 = _mm_splat_ps(m10, 0);
				__m128 c1 = _mm_splat_ps(m11, 0);
				__m128 c2 = _mm_splat_ps(m12, 0);

				c0 = _mm_mul_ps(c0, m2v);
				c1 = _mm_mul_ps(c1, m2v);
				c2 = _mm_mul_ps(c2, m2v);

				m2v = _mm_and_ps(m[1].m_val128, vFFF0fMask);

				__m128 c0_1 = _mm_splat_ps(m10, 1);
				__m128 c1_1 = _mm_splat_ps(m11, 1);
				__m128 c2_1 = _mm_splat_ps(m12, 1);

				c0_1 = _mm_mul_ps(c0_1, m2v);
				c1_1 = _mm_mul_ps(c1_1, m2v);
				c2_1 = _mm_mul_ps(c2_1, m2v);

				m2v = _mm_and_ps(m[2].m_val128, vFFF0fMask);

				c0 = _mm_add_ps(c0, c0_1);
				c1 = _mm_add_ps(c1, c1_1);
				c2 = _mm_add_ps(c2, c2_1);

				m10 = _mm_splat_ps(m10, 2);
				m11 = _mm_splat_ps(m11, 2);
				m12 = _mm_splat_ps(m12, 2);

				m10 = _mm_mul_ps(m10, m2v);
				m11 = _mm_mul_ps(m11, m2v);
				m12 = _mm_mul_ps(m12, m2v);

				c0 = _mm_add_ps(c0, m10);
				c1 = _mm_add_ps(c1, m11);
				c2 = _mm_add_ps(c2, m12);

				return Matrix3x3(c0, c1, c2);
#else
				return Matrix3x3(
					m.tdotx(m_el[0]), m.tdoty(m_el[0]), m.tdotz(m_el[0]),
					m.tdotx(m_el[1]), m.tdoty(m_el[1]), m.tdotz(m_el[1]),
					m.tdotx(m_el[2]), m.tdoty(m_el[2]), m.tdotz(m_el[2]));
#endif
			}
			AYA_FORCE_INLINE Matrix3x3& operator *= (const Matrix3x3 &m) {
#if defined(AYA_USE_SIMD)
				__m128 rv00, rv01, rv02;
				__m128 rv10, rv11, rv12;
				__m128 rv20, rv21, rv22;
				__m128 mv0, mv1, mv2;

				rv02 = m_el[0].m_val128;
				rv12 = m_el[1].m_val128;
				rv22 = m_el[2].m_val128;

				mv0 = _mm_and_ps(m[0].m_val128, vFFF0fMask);
				mv1 = _mm_and_ps(m[1].m_val128, vFFF0fMask);
				mv2 = _mm_and_ps(m[2].m_val128, vFFF0fMask);

				// rv0
				rv00 = _mm_splat_ps(rv02, 0);
				rv01 = _mm_splat_ps(rv02, 1);
				rv02 = _mm_splat_ps(rv02, 2);

				rv00 = _mm_mul_ps(rv00, mv0);
				rv01 = _mm_mul_ps(rv01, mv1);
				rv02 = _mm_mul_ps(rv02, mv2);

				// rv1
				rv10 = _mm_splat_ps(rv12, 0);
				rv11 = _mm_splat_ps(rv12, 1);
				rv12 = _mm_splat_ps(rv12, 2);

				rv10 = _mm_mul_ps(rv10, mv0);
				rv11 = _mm_mul_ps(rv11, mv1);
				rv12 = _mm_mul_ps(rv12, mv2);

				// rv2
				rv20 = _mm_splat_ps(rv22, 0);
				rv21 = _mm_splat_ps(rv22, 1);
				rv22 = _mm_splat_ps(rv22, 2);

				rv20 = _mm_mul_ps(rv20, mv0);
				rv21 = _mm_mul_ps(rv21, mv1);
				rv22 = _mm_mul_ps(rv22, mv2);

				rv00 = _mm_add_ps(rv00, rv01);
				rv10 = _mm_add_ps(rv10, rv11);
				rv20 = _mm_add_ps(rv20, rv21);

				m_el[0].m_val128 = _mm_add_ps(rv00, rv02);
				m_el[1].m_val128 = _mm_add_ps(rv10, rv12);
				m_el[2].m_val128 = _mm_add_ps(rv20, rv22);
#else
				setValue(
					m.tdotx(m_el[0]), m.tdoty(m_el[0]), m.tdotz(m_el[0]),
					m.tdotx(m_el[1]), m.tdoty(m_el[1]), m.tdotz(m_el[1]),
					m.tdotx(m_el[2]), m.tdoty(m_el[2]), m.tdotz(m_el[2]));
#endif
				return *this;
			}
			AYA_FORCE_INLINE BaseVector3 operator * (const BaseVector3 &v) const {
#if defined(AYA_USE_SIMD)
				return v.dot3(m_el[0], m_el[1], m_el[2]);
#else
				return BaseVector3(m_el[0].dot(v), m_el[1].dot(v), m_el[2].dot(v));
#endif
			}
			friend AYA_FORCE_INLINE BaseVector3 operator * (const BaseVector3 &v, const Matrix3x3 &m) {
#if defined(AYA_USE_SIMD)
				const __m128 vv = v.m_val128;

				__m128 c0 = _mm_splat_ps(vv, 0);
				__m128 c1 = _mm_splat_ps(vv, 1);
				__m128 c2 = _mm_splat_ps(vv, 2);

				c0 = _mm_mul_ps(c0, _mm_and_ps(m[0].m_val128, vFFF0fMask));
				c1 = _mm_mul_ps(c1, _mm_and_ps(m[1].m_val128, vFFF0fMask));
				c0 = _mm_add_ps(c0, c1);
				c2 = _mm_mul_ps(c2, _mm_and_ps(m[2].m_val128, vFFF0fMask));

				return BaseVector3(_mm_add_ps(c0, c2));
#else
				return BaseVector3(m.tdotx(v), m.tdoty(v), m.tdotz(v));
#endif
			}

			AYA_FORCE_INLINE Matrix3x3 transpose() const {
#if defined(AYA_USE_SIMD)
				__m128 v0 = m_el[0].m_val128;
				__m128 v1 = m_el[1].m_val128;
				__m128 v2 = m_el[2].m_val128; // x2 y2 z2 w2
				__m128 vT;

				v2 = _mm_and_ps(v2, vFFF0fMask); // x2 y2 z2 0

				vT = _mm_unpackhi_ps(v0, v1); // z0 z1 * *
				v0 = _mm_unpacklo_ps(v0, v1); // x0 x1 y0 y1

				v1 = _mm_shuffle_ps(v0, v2, __MM_SHUFFLE(2, 3, 1, 3));
				v0 = _mm_shuffle_ps(v0, v2, __MM_SHUFFLE(0, 1, 0, 3));
				v2 = _mm_castpd_ps(_mm_move_sd(_mm_castps_pd(v2), _mm_castps_pd(vT)));

				return Matrix3x3(v0, v1, v2);
#else
				return Matrix3x3(m_el[0].x(), m_el[1].x(), m_el[2].x(),
					m_el[0].y(), m_el[1].y(), m_el[2].y(),
					m_el[0].z(), m_el[1].z(), m_el[2].z());
#endif
			}

			AYA_FORCE_INLINE Matrix3x3 absolute() const {
#if defined(AYA_USE_SIMD)
				return Matrix3x3(_mm_and_ps(m_el[0].m_val128, vAbsfMask),
					_mm_and_ps(m_el[1].m_val128, vAbsfMask),
					_mm_and_ps(m_el[2].m_val128, vAbsfMask));
#else
				return Matrix3x3(
					abs(m_el[0].x()), abs(m_el[0].y()), abs(m_el[0].z()),
					abs(m_el[1].x()), abs(m_el[1].y()), abs(m_el[1].z()),
					abs(m_el[2].x()), abs(m_el[2].y()), abs(m_el[2].z()));
#endif
			}
			AYA_FORCE_INLINE Matrix3x3 adjoin() const {
				return Matrix3x3(cofac(1, 1, 2, 2), cofac(0, 2, 2, 1), cofac(0, 1, 1, 2),
					cofac(1, 2, 2, 0), cofac(0, 0, 2, 2), cofac(0, 2, 1, 0),
					cofac(1, 0, 2, 1), cofac(0, 1, 2, 0), cofac(0, 0, 1, 1));
			}
			AYA_FORCE_INLINE Matrix3x3 inverse() const {
				BaseVector3 co(cofac(1, 1, 2, 2), cofac(1, 2, 2, 0), cofac(1, 0, 2, 1));
				float det = (*this)[0].dot(co);

				assert(det != 0.f);

				float s = 1.f / det;
				return Matrix3x3(co.x() * s, cofac(0, 2, 2, 1) * s, cofac(0, 1, 1, 2) * s,
					co.y() * s, cofac(0, 0, 2, 2) * s, cofac(0, 2, 1, 0) * s,
					co.z() * s, cofac(0, 1, 2, 0) * s, cofac(0, 0, 1, 1) * s);
			}

			AYA_FORCE_INLINE bool operator == (const Matrix3x3 &m) const {
#if defined(AYA_USE_SIMD)
				__m128 c0 = _mm_cmpeq_ps(m_el[0].m_val128, m[0].m_val128);
				__m128 c1 = _mm_cmpeq_ps(m_el[1].m_val128, m[1].m_val128);
				__m128 c2 = _mm_cmpeq_ps(m_el[2].m_val128, m[2].m_val128);

				c0 = _mm_and_ps(c0, c1);
				c0 = _mm_and_ps(c0, c2);

				return (0x7 == (_mm_movemask_ps((__m128)c0) & 0x7));
#else
				return (m_el[0][0] == m[0][0] && m_el[1][0] == m[1][0] && m_el[2][0] == m[2][0] &&
					m_el[0][1] == m[0][1] && m_el[1][1] == m[1][1] && m_el[2][1] == m[2][1] &&
					m_el[0][2] == m[0][2] && m_el[1][2] == m[1][2] && m_el[2][2] == m[2][2]);
#endif
			}
			AYA_FORCE_INLINE bool operator != (const Matrix3x3 &m) const {
				return !((*this) == m);
			}

			AYA_FORCE_INLINE float tdotx(const BaseVector3 &v) const {
				return m_el[0].x() * v.x() + m_el[1].x() * v.y() + m_el[2].x() * v.z();
			}
			AYA_FORCE_INLINE float tdoty(const BaseVector3 &v) const {
				return m_el[0].y() * v.x() + m_el[1].y() * v.y() + m_el[2].y() * v.z();
			}
			AYA_FORCE_INLINE float tdotz(const BaseVector3 &v) const {
				return m_el[0].z() * v.x() + m_el[1].z() * v.y() + m_el[2].z() * v.z();
			}
			AYA_FORCE_INLINE float cofac(const int &r1, const int &c1,
				const int &r2, const int &c2) const {
				return m_el[r1][c1] * m_el[r2][c2] - m_el[r1][c2] * m_el[r2][c1];
			}

			friend inline std::ostream &operator<<(std::ostream &os, const Matrix3x3 &v) {
				os << "[" << v.m_el[0] << ",\n";
				os << " " << v.m_el[1] << ",\n";
				os << " " << v.m_el[2] << "]";
				return os;
			}
	};
}

#endif