#include "Rectangle.h"

namespace Aya {
	Rectangle::Rectangle(const Transform *O2W, const Transform *W2O, const float &x, const float &y, const float &z)
		: Shape(O2W, W2O), m_x(x * .5f), m_y(y * .5f), m_z(z * .5f) {
		assert(m_x > 0 && m_y > 0 && m_z > 0);
	}
	Rectangle::~Rectangle() {}

	BBox Rectangle::objectBound() const {
		return BBox(
			Point3(-m_x, -m_y, -m_z),
			Point3(m_x, m_y, m_z));
	}

	bool Rectangle::intersect(const Ray &ray, float *hit_t, SurfaceInteraction *si) const {
		Ray r = (*w2o)(ray);

		float tx = INFINITY;
		float ty = INFINITY;
		float tz = INFINITY;
		float t1, t2;
		Point3 hit;

		if (abs(r.m_dir.x()) > AYA_EPSILON) {
			t1 = (m_x - r.m_ori.x()) / r.m_dir.x();
			t2 = (-m_x - r.m_ori.x()) / r.m_dir.x();
			if (t1 > 0) {
				hit = r(t1);
				if (hit.y() > -m_y && hit.y() < m_y &&
					hit.z() > -m_z && hit.z() < m_z) SetMin(tx, t1);
			}
			if (t2 > 0) {
				hit = r(t2);
				if (hit.y() > -m_y && hit.y() < m_y &&
					hit.z() > -m_z && hit.z() < m_z) SetMin(tx, t2);
			}
		}
		if (abs(r.m_dir.y()) > AYA_EPSILON) {
			t1 = (m_y - r.m_ori.y()) / r.m_dir.y();
			t2 = (-m_y - r.m_ori.y()) / r.m_dir.y();
			if (t1 > 0) {
				hit = r(t1);
				if (hit.x() > -m_x && hit.x() < m_x &&
					hit.z() > -m_z && hit.z() < m_z) SetMin(ty, t1);
			}
			if (t2 > 0) {
				hit = r(t2);
				if (hit.x() > -m_x && hit.x() < m_x &&
					hit.z() > -m_z && hit.z() < m_z) SetMin(ty, t2);
			}
		}
		if (abs(r.m_dir.z()) > AYA_EPSILON) {
			t1 = (m_z - r.m_ori.z()) / r.m_dir.z();
			t2 = (-m_z - r.m_ori.z()) / r.m_dir.z();
			if (t1 > 0) {
				hit = r(t1);
				if (hit.y() > -m_y && hit.y() < m_y &&
					hit.x() > -m_x && hit.x() < m_x) SetMin(tz, t1);
			}
			if (t2 > 0) {
				hit = r(t2);
				if (hit.y() > -m_y && hit.y() < m_y &&
					hit.x() > -m_x && hit.x() < m_x) SetMin(tz, t2);
			}
		}

		if (tx >= INFINITY && ty >= INFINITY && tz >= INFINITY) {
			return false;
		}

		if (tx < ty && tx < tz && tx < ray.m_maxt) {
			si->t = tx;
			*hit_t = tx;
			si->p = r(tx);
			if (si->p.x() < 0) {
				si->n = Normal3(-1, 0, 0);
			}
			else {
				si->n = Normal3(1, 0, 0);
			}
			si->u = (si->p.y() + m_y) / (m_y * 2.f);
			si->v = (si->p.z() + m_z) / (m_z * 2.f);

			si->p = (*o2w)(si->p);
			si->n = (*o2w)(si->n);
			return true;
		}
		if (ty < tz && ty < tx && ty < ray.m_maxt) {
			si->t = ty;
			*hit_t = ty;
			si->p = r(ty);
			if (si->p.y() < 0) {
				si->n = Normal3(0, -1, 0);
			}
			else {
				si->n = Normal3(0, 1, 0);
			}
			si->u = (si->p.x() + m_x) / (m_x * 2.f);
			si->v = (si->p.z() + m_z) / (m_z * 2.f);

			si->p = (*o2w)(si->p);
			si->n = (*o2w)(si->n);
			return true;
		}
		if (tz < ty && tz < tx && tz < ray.m_maxt) {
			si->t = tz;
			*hit_t = tz;
			si->p = r(tz);
			if (si->p.z() < 0) {
				si->n = Normal3(0, 0, -1);
			}
			else {
				si->n = Normal3(0, 0, 1);
			}
			si->u = (si->p.x() + m_x) / (m_x * 2.f);
			si->v = (si->p.y() + m_y) / (m_y * 2.f);

			si->p = (*o2w)(si->p);
			si->n = (*o2w)(si->n);
			return true;
		}

		return false;
	}
}