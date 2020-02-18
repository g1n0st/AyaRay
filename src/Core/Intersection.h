#ifndef AYA_CORE_INTERSECTION_H
#define AYA_CORE_INTERSECTION_H

#include "../Math/Vector2.h"
#include "../Math/Vector3.h"
#include "../Core/Ray.h"

namespace Aya {
	//class BSDF;
	//class BSSRDF;
	//class AreaLight;
	//class MediumInterface;

	class Frame {
	private:
		Vector3 u, v, w;

	public:
		Frame() {
			u = Vector3(1.f, 0, 0);
			v = Vector3(0, 1.f, 0);
			w = Vector3(0, 0, 1.f);
		}
		Frame(const Vector3 &x, const Vector3 &y, const Vector3 &z) 
		 : u(x), v(y), w(z) {}
		__forceinline Frame(const Normal3 &normal) {
			w = normal;
			BaseVector3::coordinateSystem(w, &u, &v);
		}

		Vector3 O2W(const Vector3 &vec) const {
			return u * vec.x() + v * vec.y() + w * vec.z();
		}
		Vector3 W2O(const Vector3 &vec) const {
			return vec.dot3(u, v, w);
		}

		const Vector3& U() const {
			return u;
		}
		const Vector3& V() const {
			return v;
		}
		const Vector3& W() const {
			return w;
		}
	};

	class Scatter {
	public:
		Point3 p;
		Normal3 n;
		//MediumInterface medium_interface;

	public:
		Scatter(const Point3 pos = Point3(0.f, 0.f, 0.f), 
			const Normal3 norm = Normal3(0.f, 0.f, 1.f)) :
			p(pos), n(pos) {}
		virtual bool isSurfaceScatter() const = 0;
	};

	class Intersection {
	public:
		uint32_t prim_id, tri_id;
		float u, v;
		float dist;

		Intersection() : dist(float(INFINITY)), u(0.f), v(0.f) {}
	};

	class SurfaceIntersection : public Intersection, public Scatter {
	public:
		Vector2f tex_coord;
		Vector3 geom_normal;
		Vector3 dpdu, dpdv;
		Vector3 dndu, dndv;

		mutable Vector3 dpdx, dpdy;
		mutable float dudx, dudy, dvdx, dvdy;

		Frame shading_frame;

	public:
		SurfaceIntersection()
			: dudx(0.f), dudy(0.f), dvdx(0.f), dvdy(0.f) {}

		__forceinline Vector3 W2O(const Vector3 &vec) const {
			return shading_frame.W2O(vec);
		}
		__forceinline Vector3 O2W(const Vector3 &vec) const {
			return shading_frame.O2W(vec);
		}

		void computeDifferentials(const RayDifferential& ray) const;
		bool isSurfaceScatter() const override {
			return true;
		}
	};
}
#endif