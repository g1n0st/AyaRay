#ifndef AYA_BBOX_H
#define AYA_BBOX_H

#include "vector3.h"

__declspec(align(16))
class BBox {
public:
		Point3 m_pmin, m_pmax;

		BBox() {
			m_pmin = Point3(INFINITY, INFINITY, INFINITY);
			m_pmax = Point3(-INFINITY, -INFINITY, -INFINITY);
		}
		inline BBox(const Point3 &p) : m_pmin(p), m_pmax(p) {}
		inline BBox(const Point3 &p1, const Point3 &p2) {
#if defined(AYA_USE_SIMD)
			m_pmax.m_val128 = _mm_max_ps(p1.m_val128, p2.m_val128);
			m_pmin.m_val128 = _mm_min_ps(p1.m_val128, p2.m_val128);
#else
			m_pmax = Point3(Max(p1.x(), p2.x()), Max(p1.y(), p2.y()), Max(p1.z(), p2.z()));
			m_pmin = Point3(Min(p1.x(), p2.x()), Min(p1.y(), p2.y()), Min(p1.z(), p2.z()));
#endif
		}
#if defined(AYA_USE_SIMD)
		inline void  *operator new(size_t i) {
			return _mm_malloc(i, 16);
		}

		inline void operator delete(void *p) {
			_mm_free(p);
		}
#endif

		inline bool overlaps(const BBox &b) const {
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
		inline bool inside(const Point3 &p) const {
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
		inline void expand(const float &d) {
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
		inline float surfaceArea() const {
			Vector3 d = m_pmax - m_pmin;
			return 2 * d.length2();
		}
		inline float volume() const {
			Vector3 d = m_pmax - m_pmin;
			return d.x() * d.y() * d.z();
		}
		inline Point3 lerp(const float &tx, const float &ty, const float &tz) const {
#if defined(AYA_USE_SIMD)
			Vector3 t(tx, ty, tz), ft(1.f - tx, 1.f - ty, 1.f - tz);

			return Point3(_mm_add_ps(_mm_mul_ps(t.m_val128, m_pmin.m_val128), 
									 _mm_mul_ps(ft.m_val128, m_pmax.m_val128)));
#else
			return Point3(
				Lerp(tx, m_pmin.x(), m_pmax.x()),
				Lerp(ty, m_pmin.y(), m_pmax.y()),
				Lerp(tz, m_pmin.z(), m_pmax.z())
			);
#endif
		}
		inline void boundingSphere(Point3 *p, float *rad) const {
#if defined(AYA_USE_SIMD)
			p->m_val128 = _mm_mul_ps(v0_5, _mm_add_ps(m_pmax.m_val128, m_pmin.m_val128));
			
#else
			*p = .5f * (m_pmin + m_pmax);
#endif
			*rad = inside(*p) ? m_pmax.distance(*p) : 0.f;
		}
		inline BBox unity(const Point3 &p) {
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
		inline BBox unity(const BBox &b) {
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

		friend inline std::ostream &operator<<(std::ostream &os, const BBox &b) {
			os << "[pmin = " << b.m_pmin << ", pmax = " << b.m_pmax << "]";
			return os;
		}
};

#endif