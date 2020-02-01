#ifndef AYA_HEART_H
#define AYA_HEART_H

#include "../core/shape.h"

#define AYA_HEART_DIV_ARGU_1 40
#define AYA_HEART_DIV_ARGU_2 30
namespace Aya {
	class Heart : public Shape {
	public:
		Heart(const Transform *O2W, const Transform *W2O);

		virtual BBox objectBound() const;
		virtual bool intersect(const Ray &ray, float *hit_t, SurfaceInteraction *si) const;

	private:
		inline bool intersectBBox(const Ray &ray, float &t_min, float &t_max) const;
	};
}

#endif