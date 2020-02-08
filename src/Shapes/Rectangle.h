#ifndef AYA_SHAPE_RECTANGLE_H
#define AYA_SHAPE_RECTANGLE_H

#include "../Core/Shape.h"

namespace Aya {
	class Rectangle : public Shape {
	public:
		/**@brief the length of three vertical sides */
		float m_x, m_y, m_z;

	public:
		Rectangle(const Transform *O2W, const Transform *W2O, const float &x, const float &y, const float &z);
		~Rectangle();

		virtual BBox objectBound() const;
		virtual bool intersect(const Ray &ray, float *hit_t, SurfaceInteraction *si) const;
	};
}

#endif