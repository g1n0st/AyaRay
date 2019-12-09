#ifndef AYA_SHAPE_H
#define AYA_SHAPE_H

#include <stdint.h>
#include <assert.h>

#include "config.h"
#include "interaction.h"
#include "material.h"
#include "..\math\transform.h"

class Shape {
public:
	const Transform *o2w, *w2o;

public:
	Shape(const Transform *O2W, const Transform *W2O) : o2w(O2W), w2o(W2O) {}
	virtual ~Shape() {}

	virtual BBox objectBound() const = 0;
	virtual BBox worldBound() const {
		return (*o2w)(objectBound());
	}

	virtual bool intersect(const Ray &ray, float *hit_t, SurfaceInteraction *dg) const {
		assert(0);
		return false;
	}
};

class Sphere : public Shape {
public:
	float m_radius;
	Material *m_mat;

public:
	Sphere(const Transform *O2W, const Transform *W2O, float radius, Material *material) 
		: Shape(O2W, W2O), m_radius(radius), m_mat(material) {}
	~Sphere() {
		delete m_mat;
	}

	virtual BBox objectBound() const {
		return BBox(
			Point3(-m_radius, -m_radius, -m_radius),
			Point3(m_radius, m_radius, m_radius));
	}
	void getUV(const Point3 &p, float *u, float *v) const {
		float phi = atan2f(p.z(), p.x());
		float theta = asinf(p.y());

		(*u) = (float)(1 - (phi + M_PI) / (2 * M_PI));
		(*v) = (float)((theta + M_PI_2) / M_PI);
	}

	virtual bool intersect(const Ray &ray, float *hit_t, SurfaceInteraction *si) const {
		Ray r = (*w2o)(ray);
		Point3 o = r.m_ori;

		float a = r.m_dir.length2();
		float b = o.dot(r.m_dir);
		float c = o.length2() - m_radius * m_radius;
		float delta = b * b - a * c;

		// intersect
		if (delta > 0) {
			float t = (-b - Sqrt(delta)) / a; // x1
			if (t < ray.m_maxt && t > ray.m_mint) {
				(*hit_t) = t;
				si->t = t;
				si->p = (*o2w)(r(t));
				getUV(si->p / m_radius, &si->u, &si->v);
				si->n = (*o2w)((Normal3)si->p.normalize());
				si->mat = m_mat;

				return true;
			}

			t = (-b + Sqrt(delta)) / a; // x2
			if (t < ray.m_maxt && t > ray.m_mint) {
				(*hit_t) = t;
				si->t = t;
				si->p = (*o2w)(r(t));
				getUV(si->p / m_radius, &si->u, &si->v);
				si->n = (*o2w)((Normal3)si->p.normalize());
				si->mat = m_mat;

				return true;
			}
		}
		return false;
	}
};
#endif