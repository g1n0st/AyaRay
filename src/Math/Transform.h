#ifndef AYA_MATH_TRANSFORM_H
#define AYA_MATH_TRANSFORM_H

#include "BBox.h"
#include "Mathutility.h"
#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "Quaternion.h"

#include "..\Core\Ray.h"

namespace Aya {
#if defined(AYA_USE_SIMD)
	__declspec(align(16))
#endif
		class AffineTransform {
		public:
			Matrix3x3 m_mat, m_inv;
			Vector3 m_trans;

			AffineTransform() {
				m_mat = m_inv = Matrix3x3().getIdentity();
				m_trans = Vector3(0, 0, 0);
			}
			explicit AYA_FORCE_INLINE AffineTransform(const Matrix3x3 &m, const Vector3 &t) :
				m_mat(m), m_inv(m.inverse()), m_trans(t) {}
			explicit AYA_FORCE_INLINE AffineTransform(const Matrix3x3 &m) :
				m_mat(m), m_inv(m.inverse()), m_trans(Vector3()) {}
			explicit AYA_FORCE_INLINE AffineTransform(const Matrix3x3 &m, const Matrix3x3 &inv, const Vector3 &t) :
				m_mat(m), m_inv(inv), m_trans(t) {}
			explicit AYA_FORCE_INLINE AffineTransform(const Matrix3x3 &m, const Matrix3x3 &inv) :
				m_mat(m), m_inv(inv), m_trans(Vector3()) {}
			explicit AYA_FORCE_INLINE AffineTransform(const Quaternion &q) { setRotation(q); }
			explicit AYA_FORCE_INLINE AffineTransform(const Quaternion &q, const BaseVector3 &v) { setRotation(q); m_trans = v; }
			explicit AYA_FORCE_INLINE AffineTransform(const Vector3 &t) :
				m_mat(Matrix3x3().getIdentity()), m_inv(Matrix3x3().getIdentity()), m_trans(t) {}
			AYA_FORCE_INLINE AffineTransform(const AffineTransform &rhs) :
				m_mat(rhs.m_mat), m_inv(rhs.m_inv), m_trans(rhs.m_trans) {}
			AYA_FORCE_INLINE AffineTransform& operator = (const AffineTransform &rhs) {
				m_mat = rhs.m_mat;
				m_inv = rhs.m_inv;
				m_trans = rhs.m_trans;
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

			AYA_FORCE_INLINE AffineTransform inverse() const {
				return AffineTransform(m_inv,
					m_mat,
					-(m_inv * m_trans));
			}

			AYA_FORCE_INLINE AffineTransform operator * (const AffineTransform &t) const {
				return AffineTransform(m_mat * t.m_mat,
					t.m_inv * m_inv,
					(m_mat * t.m_trans) + m_trans);
			}
			AYA_FORCE_INLINE AffineTransform& operator *= (const AffineTransform &t) {
				m_mat *= t.m_mat;
				m_inv = t.m_inv * m_inv;
				m_trans = (m_mat * t.m_trans) + m_trans;

				return *this;
			}


			AYA_FORCE_INLINE AffineTransform& setTranslate(const Vector3 &delta) {
				m_trans = delta;
				m_mat.setIdentity();
				m_inv.setIdentity();

				return *this;
			}
			AYA_FORCE_INLINE AffineTransform& setTranslate(const float &x, const float &y, const float &z) {
				m_trans.setValue(x, y, z);
				m_mat.setIdentity();
				m_inv.setIdentity();

				return *this;
			}
			AYA_FORCE_INLINE AffineTransform& setScale(const Vector3 &scale) {
				assert(scale.x() != 0 && scale.y() != 0 && scale.z() != 0);
				m_mat.setValue(scale.x(), 0, 0,
					0, scale.y(), 0,
					0, 0, scale.z());
				m_inv.setValue(1.f / scale.x(), 0, 0,
					0, 1.f / scale.y(), 0,
					0, 0, 1.f / scale.z());
				m_trans.setZero();

				return *this;
			}
			AYA_FORCE_INLINE AffineTransform& setScale(const float &x, const float &y, const float &z) {
				assert(x != 0 && y != 0 && z != 0);
				m_mat.setValue(x, 0, 0,
					0, y, 0,
					0, 0, z);
				m_inv.setValue(1.f / x, 0, 0,
					0, 1.f / y, 0,
					0, 0, 1.f / z);
				m_trans.setZero();

				return *this;
			}
			AYA_FORCE_INLINE AffineTransform& setRotateX(const float &angle) {
				float sin_t = sinf(Radian(angle));
				float cos_t = cosf(Radian(angle));
				m_mat.setValue(1, 0, 0,
					0, cos_t, sin_t,
					0, sin_t, -cos_t);
				m_inv = m_mat.transpose();
				m_trans.setZero();

				return *this;
			}
			AYA_FORCE_INLINE AffineTransform& setRotateY(const float &angle) {
				float sin_t = sinf(Radian(angle));
				float cos_t = cosf(Radian(angle));
				m_mat.setValue(cos_t, 0, sin_t,
					0, 1, 0,
					-sin_t, 0, cos_t);
				m_inv = m_mat.transpose();
				m_trans.setZero();

				return *this;
			}
			AYA_FORCE_INLINE AffineTransform& setRotateZ(const float &angle) {
				float sin_t = sinf(Radian(angle));
				float cos_t = cosf(Radian(angle));
				m_mat.setValue(cos_t, -sin_t, 0,
					sin_t, cos_t, 0,
					0, 0, 1);
				m_inv = m_mat.transpose();
				m_trans.setZero();

				return *this;
			}
			AYA_FORCE_INLINE AffineTransform& setRotation(const Vector3 &axis, const float &angle) {
				Vector3 a = axis.normalize();
				float x = a.x();
				float y = a.y();
				float z = a.z();
				float s = sinf(Radian(angle));
				float c = cosf(Radian(angle));

				m_mat.setValue(x * x + (1.f - x * x) * c, x * y * (1.f - c) - z * s, x * z * (1.f - c) + y * s,
					x * y * (1.f - c) + z * s, y * y + (1.f - y * y) * c, y * z * (1.f - c) - x * s,
					x * z * (1.f - c) - y * s, y * z * (1.f - c) + x * s, z * z + (1.f - z * z) * c);
				m_inv = m_mat.transpose();
				m_trans.setZero();

				return *this;
			}
			void setRotation(const Quaternion& q)
			{
				float d = q.length2();
				assert(d != 0.f);
				float s = 2.f / d;

#if defined(AYA_USE_SIMD)
				__m128 vs, Q = q.get128();
				__m128i Qi = _mm_castps_si128(Q);
				__m128 Y, Z;
				__m128 V1, V2, V3;
				__m128 V11, V21, V31;
				__m128 NQ = _mm_xor_ps(Q, vMzeroMask);
				__m128i NQi = _mm_castps_si128(NQ);

				V1 = _mm_castsi128_ps(_mm_shuffle_epi32(Qi, __MM_SHUFFLE(1, 0, 2, 3)));  // Y X Z W
				V2 = _mm_shuffle_ps(NQ, Q, __MM_SHUFFLE(0, 0, 1, 3));                 // -X -X  Y  W
				V3 = _mm_castsi128_ps(_mm_shuffle_epi32(Qi, __MM_SHUFFLE(2, 1, 0, 3)));  // Z Y X W
				V1 = _mm_xor_ps(V1, vMPPP);                                         //	change the sign of the first element

				V11 = _mm_castsi128_ps(_mm_shuffle_epi32(Qi, __MM_SHUFFLE(1, 1, 0, 3)));  // Y Y X W
				V21 = _mm_unpackhi_ps(Q, Q);                                         //  Z  Z  W  W
				V31 = _mm_shuffle_ps(Q, NQ, __MM_SHUFFLE(0, 2, 0, 3));                 //  X  Z -X -W

				V2 = _mm_mul_ps(V2, V1);   //
				V1 = _mm_mul_ps(V1, V11);  //
				V3 = _mm_mul_ps(V3, V31);  //

				V11 = _mm_shuffle_ps(NQ, Q, __MM_SHUFFLE(2, 3, 1, 3));                //	-Z -W  Y  W
				V11 = _mm_mul_ps(V11, V21);                                                    //
				V21 = _mm_xor_ps(V21, vMPPP);                                       //	change the sign of the first element
				V31 = _mm_shuffle_ps(Q, NQ, __MM_SHUFFLE(3, 3, 1, 3));                //	 W  W -Y -W
				V31 = _mm_xor_ps(V31, vMPPP);                                       //	change the sign of the first element
				Y = _mm_castsi128_ps(_mm_shuffle_epi32(NQi, __MM_SHUFFLE(3, 2, 0, 3)));  // -W -Z -X -W
				Z = _mm_castsi128_ps(_mm_shuffle_epi32(Qi, __MM_SHUFFLE(1, 0, 1, 3)));   //  Y  X  Y  W

				vs = _mm_load_ss(&s);
				V21 = _mm_mul_ps(V21, Y);
				V31 = _mm_mul_ps(V31, Z);

				V1 = _mm_add_ps(V1, V11);
				V2 = _mm_add_ps(V2, V21);
				V3 = _mm_add_ps(V3, V31);

				vs = _mm_splat3_ps(vs, 0);
				//	s ready
				V1 = _mm_mul_ps(V1, vs);
				V2 = _mm_mul_ps(V2, vs);
				V3 = _mm_mul_ps(V3, vs);

				V1 = _mm_add_ps(V1, v1000);
				V2 = _mm_add_ps(V2, v0100);
				V3 = _mm_add_ps(V3, v0010);

				m_mat[0] = V1;
				m_mat[1] = V2;
				m_mat[2] = V3;
#else
				float xs = q.x() * s, ys = q.y() * s, zs = q.z() * s;
				float wx = q.w() * xs, wy = q.w() * ys, wz = q.w() * zs;
				float xx = q.x() * xs, xy = q.x() * ys, xz = q.x() * zs;
				float yy = q.y() * ys, yz = q.y() * zs, zz = q.z() * zs;

				m_mat.setValue(
					1.f - (yy + zz), xy - wz, xz + wy,
					xy + wz, 1.f - (xx + zz), yz - wx,
					xz - wy, yz + wx, 1.f - (xx + yy));
#endif
				m_inv = m_mat.transpose();
				m_trans.setZero();
			}
			AYA_FORCE_INLINE AffineTransform& setEulerZYX(const float &e_x, const float &e_y, const float &e_z) {
				float ci(cosf(Radian(e_x)));
				float cj(cosf(Radian(e_y)));
				float ch(cosf(Radian(e_z)));
				float si(sinf(Radian(e_x)));
				float sj(sinf(Radian(e_y)));
				float sh(sinf(Radian(e_z)));
				float cc = ci * ch;
				float cs = ci * sh;
				float sc = si * ch;
				float ss = si * sh;

				m_mat.setValue(cj * ch, sj * sc - cs, sj * cc + ss,
					cj * sh, sj * ss + cc, sj * cs - sc,
					-sj, cj * si, cj * ci);
				m_inv = m_mat.transpose();
				m_trans.setZero();

				return *this;
			}
			AYA_FORCE_INLINE AffineTransform& setEulerYPR(const float &yaw, const float &pitch, const float &roll) {
				return setEulerZYX(roll, pitch, yaw);
			}

			AYA_FORCE_INLINE Vector3 operator() (const Vector3 &v) const {
				return m_mat * v;
			}
			AYA_FORCE_INLINE Point3 operator() (const Point3 &p) const {
				return m_mat * p + m_trans;
			}
			AYA_FORCE_INLINE Normal3 operator() (const Normal3 &n) const {
				return m_inv.transpose() * n;
			}
			AYA_FORCE_INLINE BBox operator() (const BBox &b) const {
				// const AffineTransform &M = *this;
				// BBox ret(M(Point3(b.m_pmin.x(), b.m_pmin.y(), b.m_pmin.z())));
				// ret.unity(M(Point3(b.m_pmax.x(), b.m_pmin.y(), b.m_pmin.z())));
				// ret.unity(M(Point3(b.m_pmin.x(), b.m_pmax.y(), b.m_pmin.z())));
				// ret.unity(M(Point3(b.m_pmin.x(), b.m_pmin.y(), b.m_pmax.z())));
				// ret.unity(M(Point3(b.m_pmin.x(), b.m_pmax.y(), b.m_pmax.z())));
				// ret.unity(M(Point3(b.m_pmax.x(), b.m_pmax.y(), b.m_pmin.z())));
				// ret.unity(M(Point3(b.m_pmax.x(), b.m_pmin.y(), b.m_pmax.z())));
				// ret.unity(M(Point3(b.m_pmax.x(), b.m_pmax.y(), b.m_pmax.z())));
				// return ret;

				BBox ret;

				Point3 mid = (b.m_pmax + b.m_pmin) * .5f;
				Vector3 c = (b.m_pmax - b.m_pmin) * .5f;

				Vector3 x = m_mat.getColumn(0) * c[0];
				Vector3 y = m_mat.getColumn(1) * c[1];
				Vector3 z = m_mat.getColumn(2) * c[2];
				mid = (*this)(mid);

#if defined(AYA_USE_SIMD)
				x.m_val128 = _mm_and_ps(x.m_val128, vAbsfMask);
				y.m_val128 = _mm_and_ps(y.m_val128, vAbsfMask);
				z.m_val128 = _mm_and_ps(z.m_val128, vAbsfMask);
				x.m_val128 = _mm_add_ps(x.m_val128, y.m_val128);
				x.m_val128 = _mm_add_ps(x.m_val128, z.m_val128);

				ret.m_pmax = mid + x;
				ret.m_pmin = mid - x;
#else
				Vector3 cro(abs(x.x()) + abs(y.x()) + abs(z.x()),
					abs(x.y()) + abs(y.y()) + abs(z.y()),
					abs(x.z()) + abs(y.z()) + abs(z.z()));
				ret.m_pmax = mid + cro;
				ret.m_pmin = mid - cro;
#endif
				return ret;
			}
			AYA_FORCE_INLINE Ray operator() (const Ray &r) const {
				Ray ret = r;
				ret.m_ori = (*this)(ret.m_ori);
				ret.m_dir = (*this)(ret.m_dir);

				return ret;
			}
			AYA_FORCE_INLINE RayDifferential operator() (const RayDifferential &r) const {
				RayDifferential ret = r;
				ret.m_ori = (*this)(ret.m_ori);
				ret.m_dir = (*this)(ret.m_dir);
				ret.m_rx_ori = (*this)(ret.m_rx_ori);
				ret.m_ry_ori = (*this)(ret.m_ry_ori);
				ret.m_rx_dir = (*this)(ret.m_rx_dir);
				ret.m_ry_dir = (*this)(ret.m_ry_dir);

				return ret;
			}

			friend inline std::ostream &operator<<(std::ostream &os, const AffineTransform &t) {
				os << t.m_mat << ",\n";
				os << t.m_inv << ",\n";
				os << t.m_trans;
				return os;
			}
	};

#if defined(AYA_USE_SIMD)
	__declspec(align(16))
#endif
		class Transform {
		public:
			Matrix4x4 m_mat, m_inv;

		public:
			Transform() {
				m_mat = m_inv = Matrix4x4().getIdentity();
			}
			explicit AYA_FORCE_INLINE Transform(const Matrix4x4 &m) :
				m_mat(m), m_inv(m.inverse()) {}
			explicit AYA_FORCE_INLINE Transform(const Matrix4x4 &m, const Matrix4x4 &inv) :
				m_mat(m), m_inv(inv) {}
			explicit AYA_FORCE_INLINE Transform(const Quaternion &q) { setRotation(q); }
			AYA_FORCE_INLINE Transform(const AffineTransform &tr) {
				const Matrix3x3 &mat = tr.m_mat;
				const Matrix3x3 &inv = tr.m_inv;
				const Vector3 &trans = tr.m_trans;
				m_mat.setValue(mat[0][0], mat[0][1], mat[0][2], trans[0],
					mat[1][0], mat[1][1], mat[1][2], trans[1],
					mat[2][0], mat[2][1], mat[2][2], trans[2],
					0, 0, 0, 1);
			}

			AYA_FORCE_INLINE Transform(const Transform &rhs) :
				m_mat(rhs.m_mat), m_inv(rhs.m_inv) {}
			AYA_FORCE_INLINE Transform& operator = (const Transform &rhs) {
				m_mat = rhs.m_mat;
				m_inv = rhs.m_inv;
				return *this;
			}
			AYA_FORCE_INLINE Transform& operator = (const AffineTransform &tr) {
				const Matrix3x3 &mat = tr.m_mat;
				const Matrix3x3 &inv = tr.m_inv;
				const Vector3 &trans = tr.m_trans;
				m_mat.setValue(mat[0][0], mat[0][1], mat[0][2], trans[0],
					mat[1][0], mat[1][1], mat[1][2], trans[1],
					mat[2][0], mat[2][1], mat[2][2], trans[2],
					0, 0, 0, 1);

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

			AYA_FORCE_INLINE Transform inverse() const {
				return Transform(m_inv, m_mat);
			}

			AYA_FORCE_INLINE Transform operator * (const Transform &t) const {
				return Transform(m_mat * t.m_mat, t.m_inv * m_inv);
			}
			AYA_FORCE_INLINE Transform& operator *= (const Transform &t) {
				m_mat *= t.m_mat;
				m_inv = t.m_inv * m_inv;

				return *this;
			}

			AYA_FORCE_INLINE Transform& setTranslate(const Vector3 &delta) {
				m_mat.setValue(1, 0, 0, delta.x(),
					0, 1, 0, delta.y(),
					0, 0, 1, delta.z(),
					0, 0, 0, 1);
				m_inv.setValue(1, 0, 0, -delta.x(),
					0, 1, 0, -delta.y(),
					0, 0, 1, -delta.z(),
					0, 0, 0, 1);

				return *this;
			}
			AYA_FORCE_INLINE Transform& setTranslate(const float &x, const float &y, const float &z) {
				m_mat.setValue(1, 0, 0, x,
					0, 1, 0, y,
					0, 0, 1, z,
					0, 0, 0, 1);
				m_inv.setValue(1, 0, 0, -x,
					0, 1, 0, -y,
					0, 0, 1, -z,
					0, 0, 0, 1);

				return *this;
			}
			AYA_FORCE_INLINE Transform& setScale(const Vector3 &scale) {
				assert(scale.x() != 0 && scale.y() != 0 && scale.z() != 0);
				m_mat.setValue(scale.x(), 0, 0, 0,
					0, scale.y(), 0, 0,
					0, 0, scale.z(), 0,
					0, 0, 0, 1);
				m_inv.setValue(1.f / scale.x(), 0, 0, 0,
					0, 1.f / scale.y(), 0, 0,
					0, 0, 1.f / scale.z(), 0,
					0, 0, 0, 1);

				return *this;
			}
			AYA_FORCE_INLINE Transform& setScale(const float &x, const float &y, const float &z) {
				assert(x != 0 && y != 0 && z != 0);
				m_mat.setValue(x, 0, 0, 0,
					0, y, 0, 0,
					0, 0, z, 0,
					0, 0, 0, 1);
				m_inv.setValue(1.f / x, 0, 0, 0,
					0, 1.f / y, 0, 0,
					0, 0, 1.f / z, 0,
					0, 0, 0, 1);

				return *this;
			}
			AYA_FORCE_INLINE Transform& setRotateX(const float &angle) {
				float sin_t = sinf(Radian(angle));
				float cos_t = cosf(Radian(angle));
				m_mat.setValue(1, 0, 0, 0,
					0, cos_t, sin_t, 0,
					0, sin_t, -cos_t, 0,
					0, 0, 0, 1);
				m_inv = m_mat.transpose();

				return *this;
			}
			AYA_FORCE_INLINE Transform& setRotateY(const float &angle) {
				float sin_t = sinf(Radian(angle));
				float cos_t = cosf(Radian(angle));
				m_mat.setValue(cos_t, 0, sin_t, 0,
					0, 1, 0, 0,
					-sin_t, 0, cos_t, 0,
					0, 0, 0, 1);
				m_inv = m_mat.transpose();

				return *this;
			}
			AYA_FORCE_INLINE Transform& setRotateZ(const float &angle) {
				float sin_t = sinf(Radian(angle));
				float cos_t = cosf(Radian(angle));
				m_mat.setValue(cos_t, -sin_t, 0, 0,
					sin_t, cos_t, 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1);
				m_inv = m_mat.transpose();

				return *this;
			}
			AYA_FORCE_INLINE Transform& setRotation(const Vector3 &axis, const float &angle) {
				Vector3 a = axis.normalize();
				float x = a.x();
				float y = a.y();
				float z = a.z();
				float s = sinf(Radian(angle));
				float c = cosf(Radian(angle));

				m_mat.setValue(x * x + (1.f - x * x) * c, x * y * (1.f - c) - z * s, x * z * (1.f - c) + y * s, 0,
					x * y * (1.f - c) + z * s, y * y + (1.f - y * y) * c, y * z * (1.f - c) - x * s, 0,
					x * z * (1.f - c) - y * s, y * z * (1.f - c) + x * s, z * z + (1.f - z * z) * c, 0,
					0, 0, 0, 1);
				m_inv = m_mat.transpose();

				return *this;
			}
			void setRotation(const Quaternion& q)
			{
				float d = q.length2();
				assert(d != 0.f);
				float s = 2.f / d;

#if defined(AYA_USE_SIMD)
				__m128 vs, Q = q.get128();
				__m128i Qi = _mm_castps_si128(Q);
				__m128 Y, Z;
				__m128 V1, V2, V3;
				__m128 V11, V21, V31;
				__m128 NQ = _mm_xor_ps(Q, vMzeroMask);
				__m128i NQi = _mm_castps_si128(NQ);

				V1 = _mm_castsi128_ps(_mm_shuffle_epi32(Qi, __MM_SHUFFLE(1, 0, 2, 3)));  // Y X Z W
				V2 = _mm_shuffle_ps(NQ, Q, __MM_SHUFFLE(0, 0, 1, 3));                 // -X -X  Y  W
				V3 = _mm_castsi128_ps(_mm_shuffle_epi32(Qi, __MM_SHUFFLE(2, 1, 0, 3)));  // Z Y X W
				V1 = _mm_xor_ps(V1, vMPPP);                                         //	change the sign of the first element

				V11 = _mm_castsi128_ps(_mm_shuffle_epi32(Qi, __MM_SHUFFLE(1, 1, 0, 3)));  // Y Y X W
				V21 = _mm_unpackhi_ps(Q, Q);                                         //  Z  Z  W  W
				V31 = _mm_shuffle_ps(Q, NQ, __MM_SHUFFLE(0, 2, 0, 3));                 //  X  Z -X -W

				V2 = _mm_mul_ps(V2, V1);   //
				V1 = _mm_mul_ps(V1, V11);  //
				V3 = _mm_mul_ps(V3, V31);  //

				V11 = _mm_shuffle_ps(NQ, Q, __MM_SHUFFLE(2, 3, 1, 3));                //	-Z -W  Y  W
				V11 = _mm_mul_ps(V11, V21);                                                    //
				V21 = _mm_xor_ps(V21, vMPPP);                                       //	change the sign of the first element
				V31 = _mm_shuffle_ps(Q, NQ, __MM_SHUFFLE(3, 3, 1, 3));                //	 W  W -Y -W
				V31 = _mm_xor_ps(V31, vMPPP);                                       //	change the sign of the first element
				Y = _mm_castsi128_ps(_mm_shuffle_epi32(NQi, __MM_SHUFFLE(3, 2, 0, 3)));  // -W -Z -X -W
				Z = _mm_castsi128_ps(_mm_shuffle_epi32(Qi, __MM_SHUFFLE(1, 0, 1, 3)));   //  Y  X  Y  W

				vs = _mm_load_ss(&s);
				V21 = _mm_mul_ps(V21, Y);
				V31 = _mm_mul_ps(V31, Z);

				V1 = _mm_add_ps(V1, V11);
				V2 = _mm_add_ps(V2, V21);
				V3 = _mm_add_ps(V3, V31);

				vs = _mm_splat3_ps(vs, 0);
				//	s ready
				V1 = _mm_mul_ps(V1, vs);
				V2 = _mm_mul_ps(V2, vs);
				V3 = _mm_mul_ps(V3, vs);

				V1 = _mm_add_ps(V1, v1000);
				V2 = _mm_add_ps(V2, v0100);
				V3 = _mm_add_ps(V3, v0010);

				m_mat[0] = V1;
				m_mat[1] = V2;
				m_mat[2] = V3;
				m_mat[3] = v0001;
#else
				float xs = q.x() * s, ys = q.y() * s, zs = q.z() * s;
				float wx = q.w() * xs, wy = q.w() * ys, wz = q.w() * zs;
				float xx = q.x() * xs, xy = q.x() * ys, xz = q.x() * zs;
				float yy = q.y() * ys, yz = q.y() * zs, zz = q.z() * zs;

				m_mat.setValue(
					1.f - (yy + zz), xy - wz, xz + wy, 0,
					xy + wz, 1.f - (xx + zz), yz - wx, 0,
					xz - wy, yz + wx, 1.f - (xx + yy), 0,
					0, 0, 0, 1);
#endif
				m_inv = m_mat.transpose();
			}
			AYA_FORCE_INLINE Transform& setEulerZYX(const float &e_x, const float &e_y, const float &e_z) {
				float ci(cosf(Radian(e_x)));
				float cj(cosf(Radian(e_y)));
				float ch(cosf(Radian(e_z)));
				float si(sinf(Radian(e_x)));
				float sj(sinf(Radian(e_y)));
				float sh(sinf(Radian(e_z)));
				float cc = ci * ch;
				float cs = ci * sh;
				float sc = si * ch;
				float ss = si * sh;

				m_mat.setValue(cj * ch, sj * sc - cs, sj * cc + ss, 0,
					cj * sh, sj * ss + cc, sj * cs - sc, 0,
					-sj, cj * si, cj * ci, 0,
					0, 0, 0, 1);
				m_inv = m_mat.transpose();

				return *this;
			}
			AYA_FORCE_INLINE Transform& setEulerYPR(const float &yaw, const float &pitch, const float &roll) {
				return setEulerZYX(roll, pitch, yaw);
			}

			AYA_FORCE_INLINE Vector3 operator() (const Vector3 &v) const {
				QuadWord r = m_mat * QuadWord(v.x(), v.y(), v.z(), 0.f);
				return Vector3(r.x(), r.y(), r.z());
			}
			AYA_FORCE_INLINE Point3 operator() (const Point3 &p) const {
				QuadWord r = m_mat * QuadWord(p.x(), p.y(), p.z(), 1.f);
				assert(r.w() != 0.f);
				if (r.w() == 1.f)
					return Point3(r.x(), r.y(), r.z());
				else {
					float inv = 1.f / r.w();
					return Point3(r.x() * inv, r.y() * inv, r.z() * inv);
				}
			}
			AYA_FORCE_INLINE Normal3 operator() (const Normal3 &n) const {
				QuadWord r = m_inv.transpose() * QuadWord(n.x(), n.y(), n.z(), 0.f);
				return Normal3(r.x(), r.y(), r.z());
			}
			AYA_FORCE_INLINE BBox operator() (const BBox &b) const {
				const Transform &M = *this;
				BBox ret(M(Point3(b.m_pmin.x(), b.m_pmin.y(), b.m_pmin.z())));
				ret.unity(M(Point3(b.m_pmax.x(), b.m_pmin.y(), b.m_pmin.z())));
				ret.unity(M(Point3(b.m_pmin.x(), b.m_pmax.y(), b.m_pmin.z())));
				ret.unity(M(Point3(b.m_pmin.x(), b.m_pmin.y(), b.m_pmax.z())));
				ret.unity(M(Point3(b.m_pmin.x(), b.m_pmax.y(), b.m_pmax.z())));
				ret.unity(M(Point3(b.m_pmax.x(), b.m_pmax.y(), b.m_pmin.z())));
				ret.unity(M(Point3(b.m_pmax.x(), b.m_pmin.y(), b.m_pmax.z())));
				ret.unity(M(Point3(b.m_pmax.x(), b.m_pmax.y(), b.m_pmax.z())));
				return ret;
			}
			AYA_FORCE_INLINE Ray operator() (const Ray &r) const {
				Ray ret = r;
				ret.m_ori = (*this)(ret.m_ori);
				ret.m_dir = (*this)(ret.m_dir);

				return ret;
			}
			AYA_FORCE_INLINE RayDifferential operator() (const RayDifferential &r) const {
				RayDifferential ret = r;
				ret.m_ori = (*this)(ret.m_ori);
				ret.m_dir = (*this)(ret.m_dir);
				ret.m_rx_ori = (*this)(ret.m_rx_ori);
				ret.m_ry_ori = (*this)(ret.m_ry_ori);
				ret.m_rx_dir = (*this)(ret.m_rx_dir);
				ret.m_ry_dir = (*this)(ret.m_ry_dir);

				return ret;
			}

			friend inline std::ostream &operator<<(std::ostream &os, const Transform &t) {
				os << t.m_mat << ",\n";
				os << t.m_inv;

				return os;
			}
	};
}
#endif