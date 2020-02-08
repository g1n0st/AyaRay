#ifndef AYA_CORE_SHAPE_H
#define AYA_CORE_SHAPE_H

#include "Config.h"
#include "Material.h"
#include "Memory.h"

#include "..\Math\Transform.h"

#include <cstdint>
#include <cassert>
#include <vector>

namespace Aya {
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

		const Transform *o2w, *w2o;

	public:
		Shape(const Transform *O2W, const Transform *W2O);
		virtual ~Shape();

		virtual BBox objectBound() const = 0;
		virtual BBox worldBound() const;

		virtual bool canIntersect() const;
		virtual void refine(std::vector<SharedPtr<Shape> > &refined) const;
		virtual bool intersect(const Ray &ray, float *hit_t, SurfaceInteraction *dg) const;
	};
}

#endif