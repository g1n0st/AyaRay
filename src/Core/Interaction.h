#ifndef AYA_CORE_INTERACTION_H
#define AYA_CORE_INTERACTION_H

#include "Config.h"
#include "Memory.h"
#include "../Math/Vector3.h"

namespace Aya {

	class Primitive;
	class GeometricPrimitive;

	class SurfaceInteraction {
	public:
		float t;
		float u, v;
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