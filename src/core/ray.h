#ifndef AYA_RAY_H
#define AYA_RAY_H

#include "config.h"
#include "..\math\vector3.h"

namespace Aya {
	class Ray {
	public:
		Point3 m_ori;
		Vector3 m_dir;
		mutable float m_maxt;
		float m_time;

		Ray() : m_maxt(INFINITY), m_time(0.f) {}
		inline Ray(const Point3 &ori, const Vector3 &dir,
			float start, float end = INFINITY, float t = 0.f)
			: m_ori(ori), m_dir(dir), m_maxt(end), m_time(t) {}

		inline Point3 operator() (const float &t) const {
			return m_ori + m_dir * t;
		}

		friend __forceinline std::ostream &operator<<(std::ostream &os, const Ray &r) {
			os << "[ ori = " << r.m_ori << ", dir = " << r.m_dir << ", maxt = " << r.m_maxt
				<< ", time = " << r.m_time << "]";
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
			float start, float end = INFINITY, float t = 0.f) : Ray(ori, dir, end, t){
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