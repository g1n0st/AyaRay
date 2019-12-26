#ifndef AYA_SPHERE_H
#define AYA_SPHERE_H

#include "../core/shape.h"

namespace Aya {
	/**@brief Sphere class inherits Shape base class and represents a sphere*/
	class Sphere : public Shape {
	public:
		/**@brief Sphere radius*/
		float m_radius;

	public:
		Sphere(const Transform *O2W, const Transform *W2O, float radius);
		~Sphere();

		virtual BBox objectBound() const;
		/**@brief Get UV coordinates on a sphere */
		void getUV(const Point3 &p, float *u, float *v) const;
		virtual bool intersect(const Ray &ray, float *hit_t, SurfaceInteraction *si) const;
	};
}

#endif