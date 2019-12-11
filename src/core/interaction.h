#ifndef AYA_INTERACTION_H
#define AYA_INTERACTION_H

#include "config.h"
#include "memory.h"
#include "../math/vector3.h"

namespace Aya {

	class Primitive;
	class GeometricPrimitive;

	class SurfaceInteraction {
	public:
		float t, u, v;
		Point3 p;
		Normal3 n;
		const GeometricPrimitive *prim;

	public:
		SurfaceInteraction() {}
		SurfaceInteraction(const float &tt, const float &uu, const float &vv, const Point3 &pp, const Normal3 &nn, const GeometricPrimitive *p)
			: t(tt), u(uu), v(vv), p(pp), n(nn), prim(p) {}
	};
}

#endif