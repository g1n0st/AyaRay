#ifndef AYA_PRIMITIVE_H
#define AYA_PRIMITIVE_H

#include "config.h"
#include "shape.h"
#include "material.h"
#include "memory.h"

namespace Aya {
	class Primitive {
	public:
		Primitive();
		virtual ~Primitive();
		virtual BBox worldBound() const = 0;
		virtual bool canIntersect() const;
		virtual bool intersect(const Ray &ray, SurfaceInteraction *si) const = 0;
		virtual void refine(std::vector<SharedPtr<Primitive> > &refined) const;
		virtual void fullyRefine(std::vector<SharedPtr<Primitive> > &refined) const;
	};

	class GeometricPrimitive : public Primitive {
	public:
		SharedPtr<Shape> m_shape;
		SharedPtr<Material> m_material;

	public:
		GeometricPrimitive(const SharedPtr<Shape> &s, const SharedPtr<Material> &m);
		virtual BBox worldBound() const;
		virtual bool canIntersect() const;
		virtual bool intersect(const Ray &ray, SurfaceInteraction *si) const;
		virtual void refine(std::vector<SharedPtr<Primitive> > &refined) const;
	};

	class Accelerator : public Primitive {
	public:
		Accelerator() {}

		virtual void construct(std::vector<SharedPtr<Primitive> > prims) = 0;
		virtual BBox worldBound() const = 0;
		virtual bool canIntersect() {
			return false;
		}
		virtual bool intersect(const Ray &ray, SurfaceInteraction *si) const = 0;
	};
}
#endif