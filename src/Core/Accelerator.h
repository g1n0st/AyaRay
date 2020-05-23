#ifndef AYA_CORE_ACCELERATOR_H
#define AYA_CORE_ACCELERATOR_H

#include "../Core/Accelerator.h"
#include "../Core/Primitive.h"

namespace Aya {
	class Accelerator {
	public:
		Accelerator() = default;
		virtual ~Accelerator();

		virtual void construct(const std::vector<Primitive*> &prims) = 0;
		virtual BBox worldBound() const = 0;
		virtual bool intersect(const Ray &ray, Intersection *si) const = 0;
		virtual bool occluded(const Ray &ray) const = 0;
	};
}

#endif