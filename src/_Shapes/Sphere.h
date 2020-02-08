#ifndef AYA_CORE_SPHERE_H
#define AYA_CORE_SPHERE_H

#include "../Core/Shape.h"

namespace Aya {
	class Sphere : public Shape {
	public:
		float m_radius;

	public:
		Sphere(const Transform *O2W, const Transform *W2O, float radius);
		~Sphere();

		virtual BBox objectBound() const;
		void getUV(const Point3 &p, float *u, float *v) const;
		virtual bool intersect(const Ray &ray, float *hit_t, SurfaceInteraction *si) const;
	};
}

#endif