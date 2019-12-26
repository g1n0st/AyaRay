#ifndef AYA_INTERACTION_H
#define AYA_INTERACTION_H

#include "config.h"
#include "memory.h"
#include "../math/vector3.h"

namespace Aya {

	class Primitive;
	class GeometricPrimitive;

	/**@brief SurfaceInteraction class records the details where the surface and the ray intersected */
	class SurfaceInteraction {
	public:
		/**@brief t is the param of the ray */
		float t;
		/**@brief (u,v) coordinates of intersecting surfaces */
		float u, v;
		/**@brief the point in the world coordinate where the surface and the ray intersected */
		Point3 p;
		/**@brief the normal of the surface */
		Normal3 n;
		/**@brief the primitive which own the surface */
		const GeometricPrimitive *prim;

	public:
		SurfaceInteraction() {}
		SurfaceInteraction(const float &tt, const float &uu, const float &vv, const Point3 &pp, const Normal3 &nn, const GeometricPrimitive *p)
			: t(tt), u(uu), v(vv), p(pp), n(nn), prim(p) {}
	};
}

#endif