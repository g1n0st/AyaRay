#ifndef AYA_CORE_PRIMITIVE__H
#define AYA_CORE_PRIMITIVE__H

#include "Config.h"
#include "Shape.h"
#include "Material.h"
#include "Memory.h"

namespace Aya {
	class _Primitive {
	public:
		_Primitive();
		virtual ~_Primitive();
		virtual BBox worldBound() const = 0;
		virtual bool canIntersect() const;
		virtual bool intersect(const Ray &ray, SurfaceInteraction *si) const = 0;
		virtual void refine(std::vector<SharedPtr<_Primitive> > &refined) const;
		virtual void fullyRefine(std::vector<SharedPtr<_Primitive> > &refined) const;
	};

	class GeometricPrimitive : public _Primitive {
	public:
		SharedPtr<Shape> m_shape;
		SharedPtr<Material> m_material;

	public:
		GeometricPrimitive(const SharedPtr<Shape> &s, const SharedPtr<Material> &m);
		virtual BBox worldBound() const;
		virtual bool canIntersect() const;
		virtual bool intersect(const Ray &ray, SurfaceInteraction *si) const;
		virtual void refine(std::vector<SharedPtr<_Primitive> > &refined) const;
	};

	class Accelerator : public _Primitive {
	public:
		Accelerator() {}

		virtual void construct(std::vector<SharedPtr<_Primitive> > prims) = 0;
		virtual BBox worldBound() const = 0;
		virtual bool canIntersect() {
			return false;
		}
		virtual bool intersect(const Ray &ray, SurfaceInteraction *si) const = 0;
	};
}
#endif