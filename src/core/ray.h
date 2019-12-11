#ifndef AYA_RAY_H
#define AYA_RAY_H

#include "config.h"
#include "..\math\vector3.h"

namespace Aya {
	class Ray {
	public:
		Point3 m_ori;
		Vector3 m_dir;
		mutable float m_mint, m_maxt;
		float m_time;
		int m_depth;

		Ray() : m_mint(0.f), m_maxt(INFINITY), m_time(0.f), m_depth(0) {}
		inline Ray(const Point3 &ori, const Vector3 &dir,
			float start, float end, float t = 0.f, int d = 0)
			: m_ori(ori), m_dir(dir), m_mint(start), m_maxt(end), m_time(t), m_depth(d) {}
		inline Ray(const Point3 &ori, const Vector3 &dir, const float &time)
			: m_ori(ori), m_dir(dir), m_mint(0), m_maxt(INFINITY), m_time(time), m_depth(0) {}
		inline Ray(const Point3 &ori, const Vector3 &dir, const Ray &par,
			float start, float end = INFINITY)
			: m_ori(ori), m_dir(dir), m_mint(start), m_maxt(end),
			m_time(par.m_time), m_depth(par.m_depth + 1) {}

		inline Point3 operator() (const float &t) const {
			return m_ori + m_dir * t;
		}

		/**@brief cout debug function of Ray */
		friend __forceinline std::ostream &operator<<(std::ostream &os, const Ray &r) {
			os << "[ ori = " << r.m_ori << ", dir = " << r.m_dir << ", maxt = " << r.m_maxt
				<< ", time = " << r.m_time << "]";
			return os;
		}
	};
}

#endif