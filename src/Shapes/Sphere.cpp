#include "Sphere.h"

namespace Aya {
	Sphere::Sphere(const Transform *O2W, const Transform *W2O, float radius)
		: Shape(O2W, W2O), m_radius(radius) {}
	Sphere::~Sphere() {}

	BBox Sphere::objectBound() const {
		return BBox(
			Point3(-m_radius, -m_radius, -m_radius),
			Point3(m_radius, m_radius, m_radius));
	}
	void Sphere::getUV(const Point3 &p, float *u, float *v) const {
		float phi = atan2f(p.x(), p.z());
		float theta = acosf(p.y());

		if (phi < 0.0f) phi += M_PI * 2;
		(*u) = phi * M_2_PI;
		(*v) = 1.f - theta * M_1_PI;
	}

	bool Sphere::intersect(const Ray &ray, float *hit_t, SurfaceInteraction *si) const {
		Ray r = (*w2o)(ray);
		Point3 o = r.m_ori;

		float a = r.m_dir.length2();
		float b = o.dot(r.m_dir);
		float c = o.length2() - m_radius * m_radius;
		float delta = b * b - a * c;

		// intersect
		if (delta > 0) {
			float t = (-b - Sqrt(delta)) / a; // x1
			if (t < ray.m_maxt && t > 0.f) {
				(*hit_t) = t;
				si->t = t;
				si->p = r(t);
				getUV(si->p / m_radius, &si->u, &si->v);
				si->n = si->p;
				si->n.normalize();

				si->p = (*o2w)(si->p);
				si->n = (*o2w)(si->n);

				return true;
			}

			t = (-b + Sqrt(delta)) / a; // x2
			if (t < ray.m_maxt && t > 0.f) {
				(*hit_t) = t;
				si->t = t;
				si->p = r(t);
				getUV(si->p / m_radius, &si->u, &si->v);
				si->n = si->p;
				si->n.normalize();

				si->p = (*o2w)(si->p);
				si->n = (*o2w)(si->n);

				return true;
			}
		}
		return false;
	}
}