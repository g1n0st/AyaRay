#ifndef AYA_CORE_RAY_H
#define AYA_CORE_RAY_H

#include "../Core/Config.h"
#include "../Math/Vector3.h"

#define AYA_RAY_EPS 1e-4f

namespace Aya {
	class Medium;

	class Ray {
	public:
		Point3 m_ori;
		Vector3 m_dir;
		mutable float m_mint, m_maxt;
		int m_depth;

		const Medium *mp_medium;

		Ray() : m_mint(AYA_RAY_EPS), m_maxt(INFINITY), mp_medium(nullptr), m_depth(0) {}
		inline Ray(const Point3 &ori, const Vector3 &dir,
			const Medium *medium = nullptr,
			float start = AYA_RAY_EPS, float end = INFINITY, int depth = 0)
			: m_ori(ori), m_dir(dir), mp_medium(medium), m_mint(start + AYA_RAY_EPS), m_maxt(end - AYA_RAY_EPS), m_depth(depth) {}

		inline Point3 operator() (const float &t) const {
			return m_ori + m_dir * t;
		}

		friend __forceinline std::ostream &operator<<(std::ostream &os, const Ray &r) {
			os << "[ ori = " << r.m_ori << ", dir = " << r.m_dir << ", maxt = " << r.m_maxt
				<< ", depth = " << r.m_depth << "]";
			return os;
		}
	};

	class RayDifferential : public Ray {
	public:
		bool m_has_differentials;
		Point3 m_rx_ori, m_ry_ori;
		Vector3 m_rx_dir, m_ry_dir;

		RayDifferential() { m_has_differentials = false; }
		RayDifferential(const Point3 &ori, const Vector3 &dir,
			const Medium *medium = nullptr,
			float start = AYA_RAY_EPS, float end = INFINITY, int depth = 0) : Ray(ori, dir, medium, start, end, depth){
			m_has_differentials = false;
		}
		RayDifferential(const Ray &ray) : Ray(ray) {
			m_has_differentials = false;
		}

		void scale(const float &s) {
			m_rx_ori = m_ori + (m_rx_ori - m_ori) * s;
			m_ry_ori = m_ori + (m_ry_ori - m_ori) * s;
			m_rx_dir = m_dir + (m_rx_dir - m_dir) * s;
			m_ry_dir = m_dir + (m_ry_dir - m_dir) * s;
		}
		friend std::ostream & operator << (std::ostream &os, const RayDifferential &r) {
			os << "[ " << (Ray &)r << " has differentials: " <<
				(r.m_has_differentials ? "true" : "false") << ", xo = " << r.m_rx_ori <<
				", xd = " << r.m_rx_dir << ", yo = " << r.m_ry_ori << ", yd = " <<
				r.m_ry_dir;
			return os;
		}
	};
}

#endif