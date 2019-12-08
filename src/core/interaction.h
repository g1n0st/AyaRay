#ifndef AYA_INTERACTION_H
#define AYA_INTERACTION_H

#include "material.h"

class Material;

class SurfaceInteraction {
public:
	float t, u, v;
	Point3 p;
	Normal3 n;
	Material * mat;
	
public:
	SurfaceInteraction() {}
	SurfaceInteraction(const float &tt, const float &uu, const float &vv, const Point3 &pp, const Normal3 &nn, Material * m)
		: t(tt), u(uu), v(vv), p(pp), n(nn), mat(m) {}
};

#endif