#ifndef AYA_CORE_INTERSECTION_H
#define AYA_CORE_INTERSECTION_H

#include "../Math/Vector2.h"
#include "../Math/Vector3.h"
#include "../Core/Medium.h"
#include "../Core/Ray.h"

namespace Aya {
	class BSDF;
	//class BSSRDF;
	class AreaLight;

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
		AYA_FORCE_INLINE Frame(const Normal3 &normal) {
			w = normal;
			BaseVector3::coordinateSystem(w, &u, &v);
		}

		Vector3 localToWorld(const Vector3 &vec) const {
			return u * vec.x + v * vec.y + w * vec.z;
		}
		Vector3 worldToLocal(const Vector3 &vec) const {
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
		MediumInterface m_medium_interface;

	public:
		Scatter(const Point3 pos = Point3(0.f, 0.f, 0.f), 
			const Normal3 norm = Normal3(0.f, 0.f, 1.f),
			const MediumInterface &medium_interface = MediumInterface()) :
			p(pos), n(pos), m_medium_interface(medium_interface) {}
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
		Vector2f uv;
		Normal3 gn;
		Vector3 dpdu, dpdv;
		Vector3 dndu, dndv;

		mutable Vector3 dpdx, dpdy;
		mutable float dudx, dudy, dvdx, dvdy;

		Frame frame;

		const BSDF *bsdf				= nullptr;
		const AreaLight *arealight	= nullptr;

	public:
		SurfaceIntersection()
			: dudx(0.f), dudy(0.f), dvdx(0.f), dvdy(0.f) {}

		inline Vector3 worldToLocal(const Vector3 &vec) const {
			return frame.worldToLocal(vec);
		}
		inline Vector3 localToWorld(const Vector3 &vec) const {
			return frame.localToWorld(vec);
		}

		Spectrum emit(const Vector3& dir) const;
		void computeDifferentials(const RayDifferential& ray) const;

		bool isSurfaceScatter() const override {
			return true;
		}
	};

	class MediumIntersection : public Scatter {
	public:
		const PhaseFunctionHG *mp_func;

	public:
		MediumIntersection(const Vector3 &pos = Vector3(0.f, 0.f, 0.f), 
			PhaseFunctionHG *func = nullptr, 
			const MediumInterface &medium_interface = MediumInterface())
			: Scatter(pos, Normal3(0.f, 0.f, 0.f), medium_interface) ,
			mp_func(func) {}

		bool isValid() {
			return mp_func != nullptr;
		}
		bool isSurfaceScatter() const override {
			return false;
		}
	};
}
#endif