#ifndef AYA_MATH_MATRIX4X4_H
#define AYA_MATH_MATRIX4X4_H

#include "Vector3.h"

namespace Aya {
#if defined(AYA_USE_SIMD)
	__declspec(align(16))
#endif
		class Matrix4x4 {
		public:
			QuadWord m_el[4];

			Matrix4x4() {}
			Matrix4x4(const float &xx, const float &xy, const float &xz, const float &xw,
				const float &yx, const float &yy, const float &yz, const float &yw,
				const float &zx, const float &zy, const float &zz, const float &zw,
				const float &wx, const float &wy, const float &wz, const float &ww) {
				setValue(xx, xy, xz, xw,
					yx, yy, yz, yw,
					zx, zy, zz, zw,
					wx, wy, wz, ww);
			}

#if defined(AYA_USE_SIMD)
			AYA_FORCE_INLINE Matrix4x4(const __m128  &v0, const __m128  &v1, const __m128  &v2, const __m128  &v3) {
				m_el[0].m_val128 = v0;
				m_el[1].m_val128 = v1;
				m_el[2].m_val128 = v2;
				m_el[3].m_val128 = v3;
			}
			AYA_FORCE_INLINE Matrix4x4(const QuadWord &v0, const QuadWord &v1, const QuadWord &v2, const QuadWord &v3) {
				m_el[0] = v0;
				m_el[1] = v1;
				m_el[2] = v2;
				m_el[2] = v3;
			}
			AYA_FORCE_INLINE Matrix4x4(const Matrix4x4 &rhs) {
				m_el[0].m_val128 = rhs.m_el[0].m_val128;
				m_el[1].m_val128 = rhs.m_el[1].m_val128;
				m_el[2].m_val128 = rhs.m_el[2].m_val128;
				m_el[3].m_val128 = rhs.m_el[3].m_val128;
			}
			AYA_FORCE_INLINE Matrix4x4& operator = (const Matrix4x4 &rhs) {
				m_el[0].m_val128 = rhs.m_el[0].m_val128;
				m_el[1].m_val128 = rhs.m_el[1].m_val128;
				m_el[2].m_val128 = rhs.m_el[2].m_val128;
				m_el[3].m_val128 = rhs.m_el[3].m_val128;

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
			AYA_FORCE_INLINE Matrix4x4(const QuadWord &v0, const QuadWord &v1, const QuadWord &v2, const QuadWord &v3) {
				m_el[0] = v0;
				m_el[1] = v1;
				m_el[2] = v2;
				m_el[3] = v3;
			}
			AYA_FORCE_INLINE Matrix4x4(const Matrix4x4 &rhs) {
				m_el[0] = rhs.m_el[0];
				m_el[1] = rhs.m_el[1];
				m_el[2] = rhs.m_el[2];
				m_el[3] = rhs.m_el[3];
			}
			AYA_FORCE_INLINE Matrix4x4& operator = (const Matrix4x4 &rhs) {
				m_el[0] = rhs.m_el[0];
				m_el[1] = rhs.m_el[1];
				m_el[2] = rhs.m_el[2];
				m_el[3] = rhs.m_el[3];

				return *this;
			}
#endif
			AYA_FORCE_INLINE QuadWord getColumn(int x) const {
				assert(x >= 0 && x < 4);
				return QuadWord(m_el[0][x], m_el[1][x], m_el[2][x], m_el[3][x]);
			}
			AYA_FORCE_INLINE QuadWord getRow(int x) const {
				assert(x >= 0 && x < 4);
				return m_el[x];
			}
			AYA_FORCE_INLINE QuadWord& operator [](int x) {
				assert(x >= 0 && x < 4);
				return m_el[x];
			}
			AYA_FORCE_INLINE const QuadWord& operator [](int x) const {
				assert(x >= 0 && x < 4);
				return m_el[x];
			}

			void setValue(const float &xx, const float &xy, const float &xz, const float &xw,
				const float &yx, const float &yy, const float &yz, const float &yw,
				const float &zx, const float &zy, const float &zz, const float &zw,
				const float &wx, const float &wy, const float &wz, const float &ww) {
#if defined(AYA_USE_SIMD)
				m_el[0].m_val128 = _mm_set_ps(xw, xz, xy, xx);
				m_el[1].m_val128 = _mm_set_ps(yw, yz, yy, yx);
				m_el[2].m_val128 = _mm_set_ps(zw, zz, zy, zx);
				m_el[3].m_val128 = _mm_set_ps(ww, wz, wy, wx);
#else
				m_el[0][0] = xx;
				m_el[0][1] = xy;
				m_el[0][2] = xz;
				m_el[0][3] = xw;

				m_el[1][0] = yx;
				m_el[1][1] = yy;
				m_el[1][2] = yz;
				m_el[1][3] = yw;

				m_el[2][0] = zx;
				m_el[2][1] = zy;
				m_el[2][2] = zz;
				m_el[2][3] = zw;

				m_el[3][0] = wx;
				m_el[3][1] = wy;
				m_el[3][2] = wz;
				m_el[3][3] = ww;
#endif
			}

			void setIdentity() {
#if defined(AYA_USE_SIMD)
				m_el[0] = v1000;
				m_el[1] = v0100;
				m_el[2] = v0010;
				m_el[3] = v0001;
#else
				setValue(1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1);
#endif
			}

			Matrix4x4 getIdentity() {
#if defined(AYA_USE_SIMD)
				return Matrix4x4(v1000, v0100, v0010, v0001);
#else
				return Matrix4x4(1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1);
#endif
			}

			AYA_FORCE_INLINE Matrix4x4 operator + (const Matrix4x4& m1) const {
#if defined(AYA_USE_SIMD)
				return Matrix4x4(_mm_add_ps(m_el[0].m_val128, m1.m_el[0].m_val128),
					_mm_add_ps(m_el[1].m_val128, m1.m_el[1].m_val128),
					_mm_add_ps(m_el[2].m_val128, m1.m_el[2].m_val128),
					_mm_add_ps(m_el[3].m_val128, m1.m_el[3].m_val128));
#else
				return Matrix4x4(
					m_el[0][0] + m1[0][0],
					m_el[0][1] + m1[0][1],
					m_el[0][2] + m1[0][2],
					m_el[0][3] + m1[0][3],

					m_el[1][0] + m1[1][0],
					m_el[1][1] + m1[1][1],
					m_el[1][2] + m1[1][2],
					m_el[1][3] + m1[1][3],

					m_el[2][0] + m1[2][0],
					m_el[2][1] + m1[2][1],
					m_el[2][2] + m1[2][2],
					m_el[2][3] + m1[2][3],

					m_el[3][0] + m1[3][0],
					m_el[3][1] + m1[3][1],
					m_el[3][2] + m1[3][2],
					m_el[3][3] + m1[3][3]
				);
#endif
			}
			AYA_FORCE_INLINE Matrix4x4& operator += (const Matrix4x4& m) {
#if defined(AYA_USE_SIMD)
				m_el[0].m_val128 = _mm_add_ps(m_el[0].m_val128, m.m_el[0].m_val128);
				m_el[1].m_val128 = _mm_add_ps(m_el[1].m_val128, m.m_el[1].m_val128);
				m_el[2].m_val128 = _mm_add_ps(m_el[2].m_val128, m.m_el[2].m_val128);
				m_el[3].m_val128 = _mm_add_ps(m_el[3].m_val128, m.m_el[3].m_val128);
#else
				m_el[0][0] += m[0][0];
				m_el[0][1] += m[0][1];
				m_el[0][2] += m[0][2];
				m_el[0][3] += m[0][3];

				m_el[1][0] += m[1][0];
				m_el[1][1] += m[1][1];
				m_el[1][2] += m[1][2];
				m_el[1][3] += m[1][3];

				m_el[2][0] += m[2][0];
				m_el[2][1] += m[2][1];
				m_el[2][2] += m[2][2];
				m_el[2][3] += m[2][3];

				m_el[3][0] += m[3][0];
				m_el[3][1] += m[3][1];
				m_el[3][2] += m[3][2];
				m_el[3][3] += m[3][3];
#endif
				return *this;
			}
			AYA_FORCE_INLINE Matrix4x4 operator - (const Matrix4x4& m1) const {
#if defined(AYA_USE_SIMD)
				return Matrix4x4(_mm_sub_ps(m_el[0].m_val128, m1.m_el[0].m_val128),
					_mm_sub_ps(m_el[1].m_val128, m1.m_el[1].m_val128),
					_mm_sub_ps(m_el[2].m_val128, m1.m_el[2].m_val128),
					_mm_sub_ps(m_el[3].m_val128, m1.m_el[3].m_val128));
#else
				return Matrix4x4(
					m_el[0][0] - m1[0][0],
					m_el[0][1] - m1[0][1],
					m_el[0][2] - m1[0][2],
					m_el[0][3] - m1[0][3],

					m_el[1][0] - m1[1][0],
					m_el[1][1] - m1[1][1],
					m_el[1][2] - m1[1][2],
					m_el[1][3] - m1[1][3],

					m_el[2][0] - m1[2][0],
					m_el[2][1] - m1[2][1],
					m_el[2][2] - m1[2][2],
					m_el[2][3] - m1[2][3],

					m_el[3][0] - m1[3][0],
					m_el[3][1] - m1[3][1],
					m_el[3][2] - m1[3][2],
					m_el[3][3] - m1[3][3]
				);
#endif
			}
			AYA_FORCE_INLINE Matrix4x4& operator -= (const Matrix4x4& m) {
#if defined(AYA_USE_SIMD)
				m_el[0].m_val128 = _mm_sub_ps(m_el[0].m_val128, m.m_el[0].m_val128);
				m_el[1].m_val128 = _mm_sub_ps(m_el[1].m_val128, m.m_el[1].m_val128);
				m_el[2].m_val128 = _mm_sub_ps(m_el[2].m_val128, m.m_el[2].m_val128);
				m_el[3].m_val128 = _mm_sub_ps(m_el[3].m_val128, m.m_el[3].m_val128);
#else
				m_el[0][0] -= m[0][0];
				m_el[0][1] -= m[0][1];
				m_el[0][2] -= m[0][2];
				m_el[0][3] -= m[0][3];

				m_el[1][0] -= m[1][0];
				m_el[1][1] -= m[1][1];
				m_el[1][2] -= m[1][2];
				m_el[1][3] -= m[1][3];

				m_el[2][0] -= m[2][0];
				m_el[2][1] -= m[2][1];
				m_el[2][2] -= m[2][2];
				m_el[2][3] -= m[2][3];

				m_el[3][0] -= m[3][0];
				m_el[3][1] -= m[3][1];
				m_el[3][2] -= m[3][2];
				m_el[3][3] -= m[3][3];
#endif
				return *this;
			}
			AYA_FORCE_INLINE Matrix4x4 operator * (const float &s) const {
#if defined(AYA_USE_SIMD)
				__m128 vs = _mm_load_ss(&s);
				vs = _mm_pshufd_ps(vs, 0x00);
				return Matrix4x4(_mm_mul_ps(m_el[0].m_val128, vs),
					_mm_mul_ps(m_el[1].m_val128, vs),
					_mm_mul_ps(m_el[2].m_val128, vs),
					_mm_mul_ps(m_el[3].m_val128, vs));
#else
				return Matrix4x4(
					m_el[0][0] * s,
					m_el[0][1] * s,
					m_el[0][2] * s,
					m_el[0][3] * s,

					m_el[1][0] * s,
					m_el[1][1] * s,
					m_el[1][2] * s,
					m_el[1][3] * s,

					m_el[2][0] * s,
					m_el[2][1] * s,
					m_el[2][2] * s,
					m_el[2][3] * s,

					m_el[3][0] * s,
					m_el[3][1] * s,
					m_el[3][2] * s,
					m_el[3][3] * s
				);
#endif
			}
			AYA_FORCE_INLINE Matrix4x4& operator *= (const float &s) {
#if defined(AYA_USE_SIMD)
				__m128 vs = _mm_load_ss(&s);
				vs = _mm_pshufd_ps(vs, 0x00);
				m_el[0].m_val128 = _mm_mul_ps(m_el[0].m_val128, vs);
				m_el[1].m_val128 = _mm_mul_ps(m_el[1].m_val128, vs);
				m_el[2].m_val128 = _mm_mul_ps(m_el[2].m_val128, vs);
				m_el[3].m_val128 = _mm_mul_ps(m_el[3].m_val128, vs);
#else
				m_el[0][0] *= s;
				m_el[0][1] *= s;
				m_el[0][2] *= s;
				m_el[0][3] *= s;

				m_el[1][0] *= s;
				m_el[1][1] *= s;
				m_el[1][2] *= s;
				m_el[1][3] *= s;

				m_el[2][0] *= s;
				m_el[2][1] *= s;
				m_el[2][2] *= s;
				m_el[2][3] *= s;

				m_el[3][0] *= s;
				m_el[3][1] *= s;
				m_el[3][2] *= s;
				m_el[3][3] *= s;
#endif
				return *this;
			}
			AYA_FORCE_INLINE friend Matrix4x4 operator * (const float &s, const Matrix4x4 &v) {
				return v * s;
			}
			AYA_FORCE_INLINE Matrix4x4 operator / (const float &s) const {
				assert(s != 0);
				return (*this) * (1.f / s);
			}
			AYA_FORCE_INLINE Matrix4x4& operator /= (const float &s) {
				assert(s != 0);
				return (*this) *= (1.f / s);
			}

			AYA_FORCE_INLINE Matrix4x4 operator * (const Matrix4x4 &m) const {
#if defined(AYA_USE_SIMD)
				__m128 m10 = m_el[0].m_val128;
				__m128 m11 = m_el[1].m_val128;
				__m128 m12 = m_el[2].m_val128;
				__m128 m13 = m_el[3].m_val128;

				//__m128 m2v = _mm_and_ps(m[0].m_val128, vFFFFfMask);

				__m128 c0 = _mm_splat_ps(m10, 0);
				__m128 c1 = _mm_splat_ps(m11, 0);
				__m128 c2 = _mm_splat_ps(m12, 0);
				__m128 c3 = _mm_splat_ps(m13, 0);

				c0 = _mm_mul_ps(c0, m[0].m_val128);
				c1 = _mm_mul_ps(c1, m[0].m_val128);
				c2 = _mm_mul_ps(c2, m[0].m_val128);
				c3 = _mm_mul_ps(c3, m[0].m_val128);

				//m2v = _mm_and_ps(m[1].m_val128, vFFFFfMask);

				__m128 c0_1 = _mm_splat_ps(m10, 1);
				__m128 c1_1 = _mm_splat_ps(m11, 1);
				__m128 c2_1 = _mm_splat_ps(m12, 1);
				__m128 c3_1 = _mm_splat_ps(m13, 1);

				c0_1 = _mm_mul_ps(c0_1, m[1].m_val128);
				c1_1 = _mm_mul_ps(c1_1, m[1].m_val128);
				c2_1 = _mm_mul_ps(c2_1, m[1].m_val128);
				c3_1 = _mm_mul_ps(c3_1, m[1].m_val128);

				//m2v = _mm_and_ps(m[2].m_val128, vFFFFfMask);

				__m128 c0_2 = _mm_splat_ps(m10, 2);
				__m128 c1_2 = _mm_splat_ps(m11, 2);
				__m128 c2_2 = _mm_splat_ps(m12, 2);
				__m128 c3_2 = _mm_splat_ps(m13, 2);

				c0_2 = _mm_mul_ps(c0_2, m[2].m_val128);
				c1_2 = _mm_mul_ps(c1_2, m[2].m_val128);
				c2_2 = _mm_mul_ps(c2_2, m[2].m_val128);
				c3_2 = _mm_mul_ps(c3_2, m[2].m_val128);

				//m2v = _mm_and_ps(m[3].m_val128, vFFFFfMask);

				c0 = _mm_add_ps(c0, c0_1);
				c1 = _mm_add_ps(c1, c1_1);
				c2 = _mm_add_ps(c2, c2_1);
				c3 = _mm_add_ps(c3, c3_1);

				c0 = _mm_add_ps(c0, c0_2);
				c1 = _mm_add_ps(c1, c1_2);
				c2 = _mm_add_ps(c2, c2_2);
				c3 = _mm_add_ps(c3, c3_2);

				m10 = _mm_splat_ps(m10, 3);
				m11 = _mm_splat_ps(m11, 3);
				m12 = _mm_splat_ps(m12, 3);
				m13 = _mm_splat_ps(m13, 3);

				m10 = _mm_mul_ps(m10, m[3].m_val128);
				m11 = _mm_mul_ps(m11, m[3].m_val128);
				m12 = _mm_mul_ps(m12, m[3].m_val128);
				m13 = _mm_mul_ps(m13, m[3].m_val128);

				c0 = _mm_add_ps(c0, m10);
				c1 = _mm_add_ps(c1, m11);
				c2 = _mm_add_ps(c2, m12);
				c3 = _mm_add_ps(c3, m13);

				return Matrix4x4(c0, c1, c2, c3);
#else
				return Matrix4x4(
					m.tdotx(m_el[0]), m.tdoty(m_el[0]), m.tdotz(m_el[0]), m.tdotw(m_el[0]),
					m.tdotx(m_el[1]), m.tdoty(m_el[1]), m.tdotz(m_el[1]), m.tdotw(m_el[1]),
					m.tdotx(m_el[2]), m.tdoty(m_el[2]), m.tdotz(m_el[2]), m.tdotw(m_el[2]),
					m.tdotx(m_el[3]), m.tdoty(m_el[3]), m.tdotz(m_el[3]), m.tdotw(m_el[3]));
#endif
			}
			AYA_FORCE_INLINE Matrix4x4& operator *= (const Matrix4x4 &m) {
#if defined(AYA_USE_SIMD)
				__m128 m10 = m_el[0].m_val128;
				__m128 m11 = m_el[1].m_val128;
				__m128 m12 = m_el[2].m_val128;
				__m128 m13 = m_el[3].m_val128;

				//__m128 m2v = _mm_and_ps(m[0].m_val128, vFFFFfMask);

				__m128 c0 = _mm_splat_ps(m10, 0);
				__m128 c1 = _mm_splat_ps(m11, 0);
				__m128 c2 = _mm_splat_ps(m12, 0);
				__m128 c3 = _mm_splat_ps(m13, 0);

				c0 = _mm_mul_ps(c0, m[0].m_val128);
				c1 = _mm_mul_ps(c1, m[0].m_val128);
				c2 = _mm_mul_ps(c2, m[0].m_val128);
				c3 = _mm_mul_ps(c3, m[0].m_val128);

				//m2v = _mm_and_ps(m[1].m_val128, vFFFFfMask);

				__m128 c0_1 = _mm_splat_ps(m10, 1);
				__m128 c1_1 = _mm_splat_ps(m11, 1);
				__m128 c2_1 = _mm_splat_ps(m12, 1);
				__m128 c3_1 = _mm_splat_ps(m13, 1);

				c0_1 = _mm_mul_ps(c0_1, m[1].m_val128);
				c1_1 = _mm_mul_ps(c1_1, m[1].m_val128);
				c2_1 = _mm_mul_ps(c2_1, m[1].m_val128);
				c3_1 = _mm_mul_ps(c3_1, m[1].m_val128);

				//m2v = _mm_and_ps(m[2].m_val128, vFFFFfMask);

				__m128 c0_2 = _mm_splat_ps(m10, 2);
				__m128 c1_2 = _mm_splat_ps(m11, 2);
				__m128 c2_2 = _mm_splat_ps(m12, 2);
				__m128 c3_2 = _mm_splat_ps(m13, 2);

				c0_2 = _mm_mul_ps(c0_2, m[2].m_val128);
				c1_2 = _mm_mul_ps(c1_2, m[2].m_val128);
				c2_2 = _mm_mul_ps(c2_2, m[2].m_val128);
				c3_2 = _mm_mul_ps(c3_2, m[2].m_val128);

				//m2v = _mm_and_ps(m[3].m_val128, vFFFFfMask);

				c0 = _mm_add_ps(c0, c0_1);
				c1 = _mm_add_ps(c1, c1_1);
				c2 = _mm_add_ps(c2, c2_1);
				c3 = _mm_add_ps(c3, c3_1);

				c0 = _mm_add_ps(c0, c0_2);
				c1 = _mm_add_ps(c1, c1_2);
				c2 = _mm_add_ps(c2, c2_2);
				c3 = _mm_add_ps(c3, c3_2);

				m10 = _mm_splat_ps(m10, 3);
				m11 = _mm_splat_ps(m11, 3);
				m12 = _mm_splat_ps(m12, 3);
				m13 = _mm_splat_ps(m13, 3);

				m10 = _mm_mul_ps(m10, m[3].m_val128);
				m11 = _mm_mul_ps(m11, m[3].m_val128);
				m12 = _mm_mul_ps(m12, m[3].m_val128);
				m13 = _mm_mul_ps(m13, m[3].m_val128);

				m_el[0].m_val128 = _mm_add_ps(c0, m10);
				m_el[1].m_val128 = _mm_add_ps(c1, m11);
				m_el[2].m_val128 = _mm_add_ps(c2, m12);
				m_el[3].m_val128 = _mm_add_ps(c3, m13);
#else
				setValue(
					m.tdotx(m_el[0]), m.tdoty(m_el[0]), m.tdotz(m_el[0]), m.tdotw(m_el[0]),
					m.tdotx(m_el[1]), m.tdoty(m_el[1]), m.tdotz(m_el[1]), m.tdotw(m_el[1]),
					m.tdotx(m_el[2]), m.tdoty(m_el[2]), m.tdotz(m_el[2]), m.tdotw(m_el[2]),
					m.tdotx(m_el[3]), m.tdoty(m_el[3]), m.tdotz(m_el[3]), m.tdotw(m_el[3]));
#endif
				return *this;
			}
			
			AYA_FORCE_INLINE QuadWord operator * (const QuadWord &v) const {
#if defined(AYA_USE_SIMD)
				__m128 a0 = _mm_mul_ps(m_el[0].m_val128, v.m_val128);
				__m128 a1 = _mm_mul_ps(m_el[1].m_val128, v.m_val128);
				__m128 a2 = _mm_mul_ps(m_el[2].m_val128, v.m_val128);
				__m128 a3 = _mm_mul_ps(m_el[3].m_val128, v.m_val128);
				__m128 b0 = _mm_add_ps(_mm_unpacklo_ps(a0, a1), _mm_unpackhi_ps(a0, a1));
				__m128 b1 = _mm_add_ps(_mm_unpacklo_ps(a2, a3), _mm_unpackhi_ps(a2, a3));

				return _mm_add_ps(_mm_movelh_ps(b0, b1), _mm_movehl_ps(b1, b0));
#else
				return QuadWord(m_el[0].x() * v.x() + m_el[0].y() * v.y() + m_el[0].z() * v.z() + m_el[0].w() * v.w(),
					m_el[1].x() * v.x() + m_el[1].y() * v.y() + m_el[1].z() * v.z() + m_el[1].w() * v.w(),
					m_el[2].x() * v.x() + m_el[2].y() * v.y() + m_el[2].z() * v.z() + m_el[2].w() * v.w(),
					m_el[3].x() * v.x() + m_el[3].y() * v.y() + m_el[3].z() * v.z() + m_el[3].w() * v.w());
#endif
			}
			friend AYA_FORCE_INLINE QuadWord operator * (const QuadWord &v, const Matrix4x4 &m) {
#if defined(AYA_USE_SIMD)
				const __m128 vv = v.m_val128;

				__m128 c0 = _mm_splat_ps(vv, 0);
				__m128 c1 = _mm_splat_ps(vv, 1);
				__m128 c2 = _mm_splat_ps(vv, 2);
				__m128 c3 = _mm_splat_ps(vv, 3);

				c0 = _mm_mul_ps(c0, m[0].m_val128);
				c1 = _mm_mul_ps(c1, m[1].m_val128);
				c2 = _mm_mul_ps(c2, m[2].m_val128);
				c3 = _mm_mul_ps(c3, m[3].m_val128);

				return QuadWord(_mm_add_ps(_mm_add_ps(c0, c1), _mm_add_ps(c2, c3)));
#else
				return QuadWord(m.tdotx(v), m.tdoty(v), m.tdotz(v), m.tdotw(v));
#endif
			}

			// https://www.cnblogs.com/esing/p/4471543.html
			AYA_FORCE_INLINE Matrix4x4 transpose() const {
#if defined(AYA_USE_SIMD)
				__m128 t0 = _mm_unpacklo_ps(m_el[0].m_val128, m_el[2].m_val128);
				__m128 t1 = _mm_unpackhi_ps(m_el[0].m_val128, m_el[2].m_val128);
				__m128 t2 = _mm_unpacklo_ps(m_el[1].m_val128, m_el[3].m_val128);
				__m128 t3 = _mm_unpackhi_ps(m_el[1].m_val128, m_el[3].m_val128);

				return Matrix4x4(_mm_unpacklo_ps(t0, t2),
					_mm_unpackhi_ps(t0, t2),
					_mm_unpacklo_ps(t1, t3),
					_mm_unpackhi_ps(t1, t3));
#else
				return Matrix4x4(m_el[0].x(), m_el[1].x(), m_el[2].x(), m_el[3].x(),
					m_el[0].y(), m_el[1].y(), m_el[2].y(), m_el[3].y(),
					m_el[0].z(), m_el[1].z(), m_el[2].z(), m_el[3].z(),
					m_el[0].w(), m_el[1].w(), m_el[2].w(), m_el[3].w());
#endif
			}
			AYA_FORCE_INLINE Matrix4x4 absolute() const {
#if defined(AYA_USE_SIMD)
				return Matrix4x4(_mm_and_ps(m_el[0].m_val128, vAbsfMask),
					_mm_and_ps(m_el[1].m_val128, vAbsfMask),
					_mm_and_ps(m_el[2].m_val128, vAbsfMask),
					_mm_and_ps(m_el[3].m_val128, vAbsfMask));
#else
				return Matrix4x4(Abs(m_el[0].x()), Abs(m_el[0].y()), Abs(m_el[0].z()), Abs(m_el[0].w()),
					Abs(m_el[1].x()), Abs(m_el[1].y()), Abs(m_el[1].z()), Abs(m_el[1].w()),
					Abs(m_el[2].x()), Abs(m_el[2].y()), Abs(m_el[2].z()), Abs(m_el[2].w()),
					Abs(m_el[3].x()), Abs(m_el[3].y()), Abs(m_el[3].z()), Abs(m_el[3].w()));
#endif
			}
			AYA_FORCE_INLINE Matrix4x4 adjoint() const {
				return Matrix4x4(
					cofac(1, 2, 3, 1, 2, 3),
					-cofac(0, 2, 3, 1, 2, 3),
					cofac(0, 1, 3, 1, 2, 3),
					-cofac(0, 1, 2, 1, 2, 3),

					-cofac(1, 2, 3, 0, 2, 3),
					cofac(0, 2, 3, 0, 2, 3),
					-cofac(0, 1, 3, 0, 2, 3),
					cofac(0, 1, 2, 0, 2, 3),

					cofac(1, 2, 3, 0, 1, 3),
					-cofac(0, 2, 3, 0, 1, 3),
					cofac(0, 1, 3, 0, 1, 3),
					-cofac(0, 1, 2, 0, 1, 3),

					-cofac(1, 2, 3, 0, 1, 2),
					cofac(0, 2, 3, 0, 1, 2),
					-cofac(0, 1, 3, 0, 1, 2),
					cofac(0, 1, 2, 0, 1, 2)
				);
			}

			// https://lxjk.github.io/2020/02/07/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained-JP.html
			AYA_FORCE_INLINE Matrix4x4 inverse() const {
#if defined(AYA_USE_SIMD)
				auto Mat2Mul = [](__m128 v1, __m128 v2) {
					return
						_mm_add_ps(_mm_mul_ps(v1, _mm_swizzle(v2, 0, 3, 0, 3)),
							_mm_mul_ps(_mm_swizzle(v1, 1, 0, 3, 2), _mm_swizzle(v2, 2, 1, 2, 1)));
				};
				// 2x2 row major Matrix adjugate multiply (A#)*B
				auto Mat2AdjMul = [](__m128 v1, __m128 v2) {
					return
						_mm_sub_ps(_mm_mul_ps(_mm_swizzle(v1, 3, 3, 0, 0), v2),
							_mm_mul_ps(_mm_swizzle(v1, 1, 1, 2, 2), _mm_swizzle(v2, 2, 3, 0, 1)));

				};
				// 2x2 row major Matrix multiply adjugate A*(B#)
				auto Mat2MulAdj = [](__m128 v1, __m128 v2) {
					return
						_mm_sub_ps(_mm_mul_ps(v1, _mm_swizzle(v2, 3, 0, 3, 0)),
							_mm_mul_ps(_mm_swizzle(v1, 1, 0, 3, 2), _mm_swizzle(v2, 2, 1, 2, 1)));
				};
				// use block matrix method
// A is a matrix, then i(A) or iA means inverse of A, A# (or A_ in code) means adjugate of A, |A| (or detA in code) is determinant, tr(A) is trace

// sub matrices
				__m128 A = _mm_movelh_ps(m_el[0].m_val128, m_el[1].m_val128);
				__m128 B = _mm_movehl_ps(m_el[1].m_val128, m_el[0].m_val128);
				__m128 C = _mm_movelh_ps(m_el[2].m_val128, m_el[3].m_val128);
				__m128 D = _mm_movehl_ps(m_el[3].m_val128, m_el[2].m_val128);

				// determinant as (|A| |B| |C| |D|)
				__m128 detSub = _mm_sub_ps(
					_mm_mul_ps(_mm_shuffle2(m_el[0].m_val128, m_el[2].m_val128, 0, 2, 0, 2), _mm_shuffle2(m_el[1].m_val128, m_el[3].m_val128, 1, 3, 1, 3)),
					_mm_mul_ps(_mm_shuffle2(m_el[0].m_val128, m_el[2].m_val128, 1, 3, 1, 3), _mm_shuffle2(m_el[1].m_val128, m_el[3].m_val128, 0, 2, 0, 2))
				);
				__m128 detA = _mm_swizzle1(detSub, 0);
				__m128 detB = _mm_swizzle1(detSub, 1);
				__m128 detC = _mm_swizzle1(detSub, 2);
				__m128 detD = _mm_swizzle1(detSub, 3);

				// let iM = 1/|M| * | X  Y |
				//                  | Z  W |

				// D#C
				__m128 D_C = Mat2AdjMul(D, C);
				// A#B
				__m128 A_B = Mat2AdjMul(A, B);
				// X# = |D|A - B(D#C)
				__m128 X_ = _mm_sub_ps(_mm_mul_ps(detD, A), Mat2Mul(B, D_C));
				// W# = |A|D - C(A#B)
				__m128 W_ = _mm_sub_ps(_mm_mul_ps(detA, D), Mat2Mul(C, A_B));

				// |M| = |A|*|D| + ... (continue later)
				__m128 detM = _mm_mul_ps(detA, detD);

				// Y# = |B|C - D(A#B)#
				__m128 Y_ = _mm_sub_ps(_mm_mul_ps(detB, C), Mat2MulAdj(D, A_B));
				// Z# = |C|B - A(D#C)#
				__m128 Z_ = _mm_sub_ps(_mm_mul_ps(detC, B), Mat2MulAdj(A, D_C));

				// |M| = |A|*|D| + |B|*|C| ... (continue later)
				detM = _mm_add_ps(detM, _mm_mul_ps(detB, detC));

				// tr((A#B)(D#C))
				__m128 tr = _mm_mul_ps(A_B, _mm_swizzle(D_C, 0, 2, 1, 3));
				tr = _mm_hadd_ps(tr, tr);
				tr = _mm_hadd_ps(tr, tr);
				// |M| = |A|*|D| + |B|*|C| - tr((A#B)(D#C)
				detM = _mm_sub_ps(detM, tr);

				const __m128 adjSignMask = _mm_setr_ps(1.f, -1.f, -1.f, 1.f);
				// (1/|M|, -1/|M|, -1/|M|, 1/|M|)
				__m128 rDetM = _mm_div_ps(adjSignMask, detM);

				X_ = _mm_mul_ps(X_, rDetM);
				Y_ = _mm_mul_ps(Y_, rDetM);
				Z_ = _mm_mul_ps(Z_, rDetM);
				W_ = _mm_mul_ps(W_, rDetM);

				// apply adjugate and store, here we combine adjugate shuffle and store shuffle
				return Matrix4x4(_mm_shuffle2(X_, Y_, 3, 1, 3, 1),
				_mm_shuffle2(X_, Y_, 2, 0, 2, 0),
				_mm_shuffle2(Z_, W_, 3, 1, 3, 1),
				_mm_shuffle2(Z_, W_, 2, 0, 2, 0));
#else
				float cofac00 = cofac(1, 2, 3, 1, 2, 3);
				float cofac01 = -cofac(1, 2, 3, 0, 2, 3);
				float cofac02 = cofac(1, 2, 3, 0, 1, 3);
				float cofac03 = -cofac(1, 2, 3, 0, 1, 2);
				float det = m_el[0][0] * cofac00 + m_el[0][1] * cofac01 + m_el[0][2] * cofac02 + m_el[0][3] * cofac03;
				float det_inv = 1.f / det;
				return Matrix4x4(
					cofac00,
					-cofac(0, 2, 3, 1, 2, 3),
					cofac(0, 1, 3, 1, 2, 3),
					-cofac(0, 1, 2, 1, 2, 3),

					cofac01,
					cofac(0, 2, 3, 0, 2, 3),
					-cofac(0, 1, 3, 0, 2, 3),
					cofac(0, 1, 2, 0, 2, 3),

					cofac02,
					-cofac(0, 2, 3, 0, 1, 3),
					cofac(0, 1, 3, 0, 1, 3),
					-cofac(0, 1, 2, 0, 1, 3),

					cofac03,
					cofac(0, 2, 3, 0, 1, 2),
					-cofac(0, 1, 3, 0, 1, 2),
					cofac(0, 1, 2, 0, 1, 2)
				) * det_inv;
#endif
			}

			AYA_FORCE_INLINE bool operator == (const Matrix4x4 &m) const {
#if defined(AYA_USE_SIMD)
				__m128 c0 = _mm_cmpeq_ps(m_el[0].m_val128, m[0].m_val128);
				__m128 c1 = _mm_cmpeq_ps(m_el[1].m_val128, m[1].m_val128);
				__m128 c2 = _mm_cmpeq_ps(m_el[2].m_val128, m[2].m_val128);
				__m128 c3 = _mm_cmpeq_ps(m_el[3].m_val128, m[3].m_val128);

				c0 = _mm_and_ps(c0, c1);
				c0 = _mm_and_ps(c0, c2);
				c0 = _mm_and_ps(c0, c3);

				return (0x0 == _mm_movemask_ps((__m128)c0));
#else
				return (m_el[0][0] == m[0][0] && m_el[1][0] == m[1][0] && m_el[2][0] == m[2][0] && m_el[3][0] == m[3][0] &&
					m_el[0][1] == m[0][1] && m_el[1][1] == m[1][1] && m_el[2][1] == m[2][1] && m_el[3][1] == m[3][1] &&
					m_el[0][2] == m[0][2] && m_el[1][2] == m[1][2] && m_el[2][2] == m[2][2] && m_el[3][2] == m[3][2] &&
					m_el[0][3] == m[0][3] && m_el[1][3] == m[1][3] && m_el[2][3] == m[2][3] && m_el[3][3] == m[3][3]);
#endif
			}
			AYA_FORCE_INLINE bool operator != (const Matrix4x4 &m) const {
				return !((*this) == m);
			}

			AYA_FORCE_INLINE float tdotx(const QuadWord &v) const {
				return m_el[0].x() * v.x() + m_el[1].x() * v.y() + m_el[2].x() * v.z() + m_el[3].x() * v.w();
			}
			AYA_FORCE_INLINE float tdoty(const QuadWord &v) const {
				return m_el[0].y() * v.x() + m_el[1].y() * v.y() + m_el[2].y() * v.z() + m_el[3].y() * v.w();
			}
			AYA_FORCE_INLINE float tdotz(const QuadWord &v) const {
				return m_el[0].z() * v.x() + m_el[1].z() * v.y() + m_el[2].z() * v.z() + m_el[3].z() * v.w();
			}
			AYA_FORCE_INLINE float tdotw(const QuadWord &v) const {
				return m_el[0].w() * v.x() + m_el[1].w() * v.y() + m_el[2].w() * v.z() + m_el[3].w() * v.w();
			}
			AYA_FORCE_INLINE float cofac(const int &r1, const int &c1,
				const int &r2, const int &c2) const {
				return m_el[r1][c1] * m_el[r2][c2] - m_el[r1][c2] * m_el[r2][c1];
			}
			AYA_FORCE_INLINE float cofac(const int &r1, const int &r2, const int &r3,
				const int &c1, const int &c2, const int &c3) const {
				return  m_el[r1].m_val[c1] * (m_el[r2].m_val[c2] * m_el[r3].m_val[c3] - m_el[r3].m_val[c2] * m_el[r2].m_val[c3]) -
					m_el[r1].m_val[c2] * (m_el[r2].m_val[c1] * m_el[r3].m_val[c3] - m_el[r3].m_val[c1] * m_el[r2].m_val[c3]) +
					m_el[r1].m_val[c3] * (m_el[r2].m_val[c1] * m_el[r3].m_val[c2] - m_el[r2].m_val[c2] * m_el[r3].m_val[c1]);
			}

			friend inline std::ostream &operator<<(std::ostream &os, const Matrix4x4 &m) {
				os << "[" << m.m_el[0] << ",\n";
				os << " " << m.m_el[1] << ",\n";
				os << " " << m.m_el[2] << ",\n";
				os << " " << m.m_el[3] << "]";
				return os;
			}
	};
};

#endif