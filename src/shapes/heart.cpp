#include "heart.h"

namespace Aya {

	inline float g0(const float &x, const float &y, const float &z) {
		return 2.f * x * x + y * y + z * z - 1.f;
	}
	inline float f(const float &x, const float &y, const float &z) {
		float g0_v = g0(x, y, z);
		return g0_v * g0_v * g0_v - .1f * x * x * z * z * z - y * y * z * z * z;
	}
	inline float f(const Point3 &p) {
		return f(p.x(), p.y(), p.z());
	}

	inline float Fx(const float &x0, const float &y0, const float &z0) {
		float g0_v = g0(x0, y0, z0);
		return 12.f * g0_v * g0_v * x0 - .2f * x0 * z0 * z0 * z0;
	}
	inline float Fy(const float &x0, const float &y0, const float &z0) {
		float g0_v = g0(x0, y0, z0);
		return 6.f * g0_v * g0_v * y0 - 2.f * y0 * z0 * z0 * z0;
	}
	inline float Fz(const float &x0, const float &y0, const float &z0) {
		float g0_v = g0(x0, y0, z0);
		return 6.f * g0_v * g0_v * z0 - .3f * x0 * x0 * z0 * z0 - 3.f * y0 * y0 * z0 * z0;
	}
	inline Normal3 getNormal(Point3 &p) {
		//return Normal3(1, 0, 0);
		return Normal3(Fx(p.x(), p.y(), p.z()),
			Fy(p.x(), p.y(), p.z()),
			Fz(p.x(), p.y(), p.z()));

	}

	inline bool Heart::intersectBBox(const Ray &r, float &t_min, float &t_max) const {
		t_min = INFINITY;
		t_max = -INFINITY;

		const float m_x = 0.72f, m_y = 0.7f, m_z = 1.2f;
		float t1, t2;
		Point3 hit;

		if (abs(r.m_dir.x()) > SIMD_EPSILON) {
			t1 = (m_x - r.m_ori.x()) / r.m_dir.x();
			t2 = (-m_x - r.m_ori.x()) / r.m_dir.x();
			if (t1 > 0) {
				SetMin(t_min, t1); SetMax(t_max, t1);
			}
			if (t2 > 0) {
				SetMin(t_min, t2); SetMax(t_max, t2);
			}
		}
		if (abs(r.m_dir.y()) > SIMD_EPSILON) {
			t1 = (m_y - r.m_ori.y()) / r.m_dir.y();
			t2 = (-m_y - r.m_ori.y()) / r.m_dir.y();
			if (t1 > 0) {
				SetMin(t_min, t1); SetMax(t_max, t1);
			}
			if (t2 > 0) {
				SetMin(t_min, t2); SetMax(t_max, t2);
			}
		}
		if (abs(r.m_dir.z()) > SIMD_EPSILON) {
			t1 = (m_z - r.m_ori.z()) / r.m_dir.z();
			t2 = (-m_z - r.m_ori.z()) / r.m_dir.z();
			if (t1 > 0) {
				SetMin(t_min, t1); SetMax(t_max, t1);
			}
			if (t2 > 0) {
				SetMin(t_min, t2); SetMax(t_max, t2);
			}
		}
		if (t_max < t_min) return false;
		return true;
	}
	inline float getT(const Ray &r, float t_min, float t_max) {
		float mid;
		for (int i = 0; i < AYA_HEART_DIV_ARGU_2; i++) {
			mid = (t_min + t_max) / 2.f;
			if (std::abs(f(r(mid))) < SIMD_EPSILON) return mid;
			if (f(r(mid)) > 0) t_min = mid;
			else t_max = mid;
		}
		return mid;
	}

	Heart::Heart(const Transform *O2W, const Transform *W2O) : Shape(O2W, W2O) {
	}
	BBox Heart::objectBound() const {
		return BBox(Point3(-0.72f, -0.7f, -1.2f),
			Point3(0.72f, 0.7f, 1.2f)
		);
	}

	bool Heart::intersect(const Ray &ray, float *hit_t, SurfaceInteraction *si) const {
		Ray r = (*w2o)(ray);

		float t_min, t_max, t1 = -1.f;
		if (!intersectBBox(r, t_min, t_max)) return false;
		float flip = (t_max - t_min) / (float)AYA_HEART_DIV_ARGU_1;

		float iter_s = t_min - flip;
		int cnt = 0;

		do {
			flip /= 2.f;
			if (flip <= 1e-5) {
				if (f(r(t_min)) > 0 && f(r(t_max)) > 0) return false;
				t1 = getT(r, t_min, t_max);
				break;
			}
			if (cnt > 7) return false;
			cnt++;

			for (float t0 = t_min - flip; t0 < t_max + flip; t0 += flip) {
				float v1 = f(r(t0));
				float v2 = f(r(t0 + flip));
				if (v1 >= 0 && v2 <= 0) {
					t1 = getT(r, t0, t0 + flip); break;
				}
				//if (v2 > 0 && v2 < v1) iter_s = t0;
				//if (v1 > 0 && v1 < v2) break;
			}
		} while (t1 < 0);

		if (t1 >= ray.m_maxt || t1 <= ray.m_mint) return false;
		*hit_t = t1;
		si->t = t1;
		si->u = si->v = 0;
		si->p = r(t1);
		si->n = getNormal(si->p);
		si->n.normalize();

		si->p = (*o2w)(si->p);
		si->n = (*o2w)(si->n);
		return true;
	}
}