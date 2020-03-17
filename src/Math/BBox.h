#ifndef AYA_MATH_BBOX_H
#define AYA_MATH_BBOX_H

#include "Vector3.h"
#include "../Core/Ray.h"

namespace Aya {
#if defined(AYA_USE_SIMD)
	__declspec(align(16))
#endif
		class BBox {
		public:
			Point3 m_pmin, m_pmax;

			BBox() {
				m_pmin = Point3(INFINITY, INFINITY, INFINITY);
				m_pmax = Point3(-INFINITY, -INFINITY, -INFINITY);
			}
			AYA_FORCE_INLINE BBox(const Point3 &p) : m_pmin(p), m_pmax(p) {}
			AYA_FORCE_INLINE BBox(const Point3 &p1, const Point3 &p2) {
#if defined(AYA_USE_SIMD)
				m_pmax.m_val128 = _mm_max_ps(p1.m_val128, p2.m_val128);
				m_pmin.m_val128 = _mm_min_ps(p1.m_val128, p2.m_val128);
#else
				m_pmax = Point3(Max(p1.x(), p2.x()), Max(p1.y(), p2.y()), Max(p1.z(), p2.z()));
				m_pmin = Point3(Min(p1.x(), p2.x()), Min(p1.y(), p2.y()), Min(p1.z(), p2.z()));
#endif
			}
#if defined(AYA_USE_SIMD)
			AYA_FORCE_INLINE void  *operator new(size_t i) {
				return _mm_malloc(i, 16);
			}

			AYA_FORCE_INLINE void operator delete(void *p) {
				_mm_free(p);
			}
#endif
			AYA_FORCE_INLINE bool overlaps(const BBox &b) const {
#if defined(AYA_USE_SIMD)
				bool b1 = (0xf == _mm_movemask_ps((__m128)_mm_cmpeq_ps(
					_mm_min_ps(b.m_pmax.m_val128, m_pmin.m_val128), m_pmin.m_val128)));
				bool b2 = (0xf == _mm_movemask_ps((__m128)_mm_cmpeq_ps(
					_mm_max_ps(b.m_pmin.m_val128, m_pmax.m_val128), m_pmax.m_val128)));

				return b1 && b2;
#else
				bool x = (b.m_pmax.x() >= m_pmin.x()) && (m_pmax.x() >= b.m_pmin.x());
				bool y = (b.m_pmax.y() >= m_pmin.y()) && (m_pmax.y() >= b.m_pmin.y());
				bool z = (b.m_pmax.z() >= m_pmin.z()) && (m_pmax.z() >= b.m_pmin.z());

				return x && y && z;
#endif
			}
			AYA_FORCE_INLINE bool inside(const Point3 &p) const {
#if defined(AYA_USE_SIMD)
				bool b1 = (0xf == _mm_movemask_ps((__m128)_mm_cmpeq_ps(
					_mm_max_ps(p.m_val128, m_pmin.m_val128), p.m_val128)));
				bool b2 = (0xf == _mm_movemask_ps((__m128)_mm_cmpeq_ps(
					_mm_min_ps(p.m_val128, m_pmax.m_val128), p.m_val128)));

				return b1 && b2;
#else
				return	(p.x() >= m_pmin.x() && p.x() <= m_pmax.x()) &&
					(p.y() >= m_pmin.y() && p.y() <= m_pmax.y()) &&
					(p.z() >= m_pmin.z() && p.z() <= m_pmax.z());
#endif
			}
			AYA_FORCE_INLINE void expand(const float &d) {
#if defined(AYA_USE_SIMD)
				__m128 vd = _mm_load_ss(&d);
				vd = _mm_pshufd_ps(vd, 0x80);

				m_pmax.m_val128 = _mm_add_ps(m_pmax.m_val128, vd);
				m_pmin.m_val128 = _mm_sub_ps(m_pmin.m_val128, vd);
#else
				m_pmax += Point3(d, d, d);
				m_pmin -= Point3(d, d, d);
#endif
			}
			AYA_FORCE_INLINE BBox unity(const Point3 &p) {
#if defined(AYA_USE_SIMD)
				m_pmax.m_val128 = _mm_max_ps(m_pmax.m_val128, p.m_val128);
				m_pmin.m_val128 = _mm_min_ps(m_pmin.m_val128, p.m_val128);
#else
				m_pmin[0] = Min(m_pmin[0], p[0]);
				m_pmin[1] = Min(m_pmin[1], p[1]);
				m_pmin[2] = Min(m_pmin[2], p[2]);

				m_pmax[0] = Max(m_pmax[0], p[0]);
				m_pmax[1] = Max(m_pmax[1], p[1]);
				m_pmax[2] = Max(m_pmax[2], p[2]);
#endif

				return *this;
			}
			AYA_FORCE_INLINE BBox unity(const BBox &b) {
#if defined(AYA_USE_SIMD)
				m_pmax.m_val128 = _mm_max_ps(m_pmax.m_val128, b.m_pmax.m_val128);
				m_pmin.m_val128 = _mm_min_ps(m_pmin.m_val128, b.m_pmin.m_val128);
#else
				m_pmin[0] = Min(m_pmin[0], b.m_pmin[0]);
				m_pmin[1] = Min(m_pmin[1], b.m_pmin[1]);
				m_pmin[2] = Min(m_pmin[2], b.m_pmin[2]);

				m_pmax[0] = Max(m_pmax[0], b.m_pmax[0]);
				m_pmax[1] = Max(m_pmax[1], b.m_pmax[1]);
				m_pmax[2] = Max(m_pmax[2], b.m_pmax[2]);
#endif
				return *this;
			}
			AYA_FORCE_INLINE bool intersect(const Ray &r) const {
				float t0, t1;
				float tmin = 0.f, tmax = r.m_maxt;
				for (int a = 0; a < 3; a++) {
					t0 = Min((m_pmin[a] - r.m_ori[a]) / r.m_dir[a],
						(m_pmax[a] - r.m_ori[a]) / r.m_dir[a]);
					t1 = Max((m_pmin[a] - r.m_ori[a]) / r.m_dir[a],
						(m_pmax[a] - r.m_ori[a]) / r.m_dir[a]);
					tmin = Max(t0, tmin);
					tmax = Min(t1, tmax);
					if (tmax < tmin) {
						return false;
					}
				}
				return true;
			}
			AYA_FORCE_INLINE void boundingSphere(Point3 *center, float *radius) {
				*center = (m_pmin + m_pmax) * .5f;
				*radius = inside(*center) ? center->distance(m_pmax) : 0.f;
			}

			friend inline std::ostream &operator<<(std::ostream &os, const BBox &b) {
				os << "[pmin = " << b.m_pmin << ", pmax = " << b.m_pmax << "]";
				return os;
			}
	};
}

#endif