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
		uint32_t m_depth;

		const Medium *mp_medium;

		Ray() : m_mint(AYA_RAY_EPS), m_maxt(INFINITY), mp_medium(nullptr), m_depth(0) {}
		inline Ray(const Point3 &ori, const Vector3 &dir,
			const Medium *medium = nullptr,
			float start = AYA_RAY_EPS, float end = INFINITY, uint32_t depth = 0)
			: m_ori(ori), m_dir(dir), mp_medium(medium), m_mint(start + AYA_RAY_EPS), m_maxt(end - AYA_RAY_EPS), m_depth(depth) {}

		inline Point3 operator() (const float &t) const {
			return m_ori + m_dir * t;
		}

		friend inline std::ostream &operator<<(std::ostream &os, const Ray &r) {
			os << "[ ori = " << r.m_ori << ", dir = " << r.m_dir << ", maxt = " << r.m_maxt
				<< ", depth = " << r.m_depth << "]";
			return os;
		}
	};

	class RayDifferential : public Ray {
	public:
		bool m_hasDifferentials;
		Point3 m_rxOri, m_ryOri;
		Vector3 m_rxDir, m_ryDir;

		RayDifferential() { m_hasDifferentials = false; }
		RayDifferential(const Point3 &ori, const Vector3 &dir,
			const Medium *medium = nullptr,
			float start = AYA_RAY_EPS, float end = INFINITY, uint32_t depth = 0) : Ray(ori, dir, medium, start, end, depth){
			m_hasDifferentials = false;
		}
		RayDifferential(const Ray &ray) : Ray(ray) {
			m_hasDifferentials = false;
		}

		void scale(const float &s) {
			m_rxOri = m_ori + (m_rxOri - m_ori) * s;
			m_ryOri = m_ori + (m_ryOri - m_ori) * s;
			m_rxDir = m_dir + (m_rxDir - m_dir) * s;
			m_ryDir = m_dir + (m_ryDir - m_dir) * s;
		}
		friend std::ostream & operator << (std::ostream &os, const RayDifferential &r) {
			os << "[ " << (Ray &)r << " has differentials: " <<
				(r.m_hasDifferentials ? "true" : "false") << ", xo = " << r.m_rxOri <<
				", xd = " << r.m_rxDir << ", yo = " << r.m_ryOri << ", yd = " <<
				r.m_ryDir;
			return os;
		}
	};
}

#endif