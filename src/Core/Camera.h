#ifndef AYA_CORE_CAMERA_H
#define AYA_CORE_CAMERA_H

#include "../Core/Config.h"
#include "../Core/Ray.h"
#include "../Core/RNG.h"
#include "../Core/Sampling.h"

namespace Aya {
	class Camera {
	public:
		virtual Ray getRay(const float &s, const float &t) const = 0;
	};

	class ProjectiveCamera : public Camera {
	public:
		Point3 m_origin;
		Vector3 m_horizontal, m_vertical, m_lower_left_corner;
		Vector3 m_u, m_v, m_w;
		float m_t0, m_t1;
		float m_lens_radius;

	private:
		mutable RNG rng;

	public:
		ProjectiveCamera() {}
		ProjectiveCamera(const Point3 &lookform, const Point3 &lookat, const Vector3 &vup,
			float vfov, float aspect, float aperture, float focus_dist, float t0, float t1) {
			m_t0 = t0;
			m_t1 = t1;
			m_lens_radius = aperture / 2;

			float theta = vfov * (float)M_PI / 180.f;
			float half_height = tanf(theta / 2.f);
			float half_width = aspect * half_height;

			m_origin = lookform;
			m_w = (lookform - lookat).normalize();
			m_u = vup.cross(m_w).normalize();
			m_v = m_w.cross(m_u);

			m_lower_left_corner = m_origin - (m_u * half_width + m_v * half_height + m_w) * focus_dist;
			m_horizontal = m_u * half_width * focus_dist * 2;
			m_vertical = m_v * half_height * focus_dist * 2;
		}

		Ray getRay(const float &s, const float &t) const {
			float dx, dy;
			ConcentricSampleDisk(rng.drand48(), rng.drand48(), &dx, &dy);
			Point3 rd = m_lens_radius * Point3(dx, dy, 0.f);
			Vector3 offset = rd.x() * m_u + rd.y() * m_v;
			float time = Lerp(rng.drand48(), m_t0, m_t1);

			return Ray(m_origin + offset, (m_lower_left_corner + s * m_horizontal + t * m_vertical - m_origin - offset).normalize());
		}
	};
}
#endif