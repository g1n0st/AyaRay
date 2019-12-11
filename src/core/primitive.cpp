#include "primitive.h"

namespace Aya {
	Primitive::Primitive() {}
	Primitive::~Primitive() {}

	bool Primitive::canIntersect() const {
		return false;
	}

	void Primitive::refine(std::vector<SharedPtr<Primitive>>& refined) const {
		assert(0);
	}
	void Primitive::fullyRefine(std::vector<SharedPtr<Primitive>>& refined) const {
		std::vector<SharedPtr<Primitive> > r;
		r.push_back(const_cast<Primitive *>(this));

		while (r.size()) {
			SharedPtr<Primitive> p = r.back();
			r.pop_back();
			if (p->canIntersect()) {
				refined.push_back(p);
			}
			else {
				p->refine(r);
			}
		}
	}
	

	GeometricPrimitive::GeometricPrimitive(const SharedPtr<Shape> &s, const SharedPtr<Material> &m) 
		: m_shape(s), m_material(m) {}

	BBox GeometricPrimitive::worldBound() const {
		return m_shape->worldBound();
	}

	bool GeometricPrimitive::canIntersect() const {
		return m_shape->canIntersect();
	}

	bool GeometricPrimitive::intersect(const Ray &ray, SurfaceInteraction * si) const {
		float thit;
		if (!m_shape->intersect(ray, &thit, si)) {
			return false;
		}
		si->prim = this;
		ray.m_maxt = thit;

		return true;
	}

	void GeometricPrimitive::refine(std::vector<SharedPtr<Primitive> >& refined) const {
		std::vector<SharedPtr<Shape> > r;
		m_shape->refine(r);

		for (auto s : r) {
			GeometricPrimitive *gp = new GeometricPrimitive(s, m_material);
			refined.push_back(gp);
		}
	}

}