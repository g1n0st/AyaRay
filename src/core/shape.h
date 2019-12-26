#ifndef AYA_SHAPE_H
#define AYA_SHAPE_H

#include <cstdint>
#include <cassert>
#include <vector>

#include "config.h"
#include "material.h"
#include "memory.h"
#include "..\math\transform.h"

namespace Aya {
	/**@brief The Shape base class specifies the methods that shapes must implement */
	class Shape {
	public:
#if defined(AYA_USE_SIMD)
		inline void  *operator new(size_t i) {
			return _mm_malloc(i, 16);
		}

		inline void operator delete(void *p) {
			_mm_free(p);
		}
#endif

		/*@brief Transformation between the world coordinate system and the object coordinate system */
		const Transform *o2w, *w2o;

	public:
		Shape(const Transform *O2W, const Transform *W2O);
		virtual ~Shape();

		/*@brief Returns the bounding box in the object coordinate system */
		virtual BBox objectBound() const = 0;
		/*@brief Returns the bounding box in the world coordinate system */
		virtual BBox worldBound() const;

		/*@brief Judge the shape can be intersected or not */
		virtual bool canIntersect() const;
		/*@brief Refine the shape into small pieces */
		virtual void refine(std::vector<SharedPtr<Shape> > &refined) const;
		/*@brief Test the intersection of a ray and an object
		*  @param ray the ray
		   @param hit_t T-parameter of intersecting ray
		   param dg Details of intersecting surfaces
		*/
		virtual bool intersect(const Ray &ray, float *hit_t, SurfaceInteraction *dg) const;
	};
}

#endif