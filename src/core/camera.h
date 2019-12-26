#ifndef AYA_CAMERA_H
#define AYA_CAMERA_H

#include "config.h"
#include "ray.h"
#include "rng.h"

namespace Aya {
	/**@brief The Camera base class specifies the methods that cameras must implement */
	class Camera {
	public:
		/**@brief give the coordinate(s,t) in screen, return the ray contributes to (s,t) */
		virtual Ray getRay(const float &s, const float &t) const = 0;
	};

	/**@brief ProjectiveCamera class
	* assumes a pinhole projection system, the image is formed by the intersection of light from an object through the center of the lens 
	* (the center of the projection), and there is a focal plane */
	class ProjectiveCamera : public Camera {
	public:
		/**@brief the original point */
		Point3 m_origin;
		/**@brief the horizontal and vertical vectors */
		Vector3 m_horizontal, m_vertical, m_lower_left_corner;
		Vector3 m_u, m_v, m_w;
		float m_t0, m_t1;
		/**@brief the lens radius */
		float m_lens_radius;

	private:
		mutable RNG rng;

	public:
		ProjectiveCamera() {}
		/**@brief the construction function
		* @param lookform the position of the camera
		* @param lookat the direction where camera look at
		* @param vup the up direction of camera
		* @param screen ratio
		* @param aperture the lens radius
		* @param focus_dist the distance that camera focus on
		* @param t0 the camera open time (for moving object)
		* @param t1 the camera close time (for moving object)
		*/
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
			Point3 rd = m_lens_radius * rng.randomInUnitDisk();
			Vector3 offset = rd.x() * m_u + rd.y() * m_v;
			float time = Lerp(rng.drand48(), m_t0, m_t1);

			return Ray(m_origin + offset, m_lower_left_corner + s * m_horizontal + t * m_vertical - m_origin - offset, time);
		}
	};
}
#endif