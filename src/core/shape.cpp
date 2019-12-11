#include "shape.h"

namespace Aya {
	Shape::Shape(const Transform *O2W, const Transform *W2O) : o2w(O2W), w2o(W2O) {
	}
	Shape::~Shape() {}

	BBox Shape::worldBound() const {
		return (*o2w)(objectBound());
	}

	bool Shape::canIntersect() const {
		return true;
	}

	void Shape::refine(std::vector<SharedPtr<Shape> > &refined) const {
		assert(0);
	}
	bool Shape::intersect(const Ray &ray, float *hit_t, SurfaceInteraction *dg) const {
		assert(0);
		return false;
	}
}