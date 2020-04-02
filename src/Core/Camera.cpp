#include "Camera.h"

namespace Aya {
	Camera::Camera(){
	}
	Camera::Camera(const Point3 &pos, const Point3 &tar, const Vector3 &up, 
		int res_x, int res_y, float FOV, float _near, float _far,
		const float blur_radius, const float focal_dist, const float vignette) {
		init(pos, tar, up, res_x, res_y, FOV, _near, _far, blur_radius, focal_dist, vignette);
	}

	void Camera::init(const Point3 &pos, const Point3 &tar, const Vector3 &up, 
		int res_x, int res_y, float FOV, float _near, float _far,
		const float blur_radius, const float focal_dist, const float vignette) {
		m_pos = pos;
		m_target = tar;
		m_dir = (m_target - m_pos).normalize();
		m_up = up;
		m_res_x = res_x;
		m_res_y = res_y;
		m_FOV = FOV;
		m_near = _near;
		m_far = _far;
		

		m_view = Transform::lookAt(m_pos, m_target, m_up);
		m_view_inv = m_view.inverse();
		resize(m_res_x, m_res_y);

		m_CoC_radius = blur_radius;
		m_focal_plane_dist = focal_dist;
		m_vignette_factor = 3.f - vignette;

		float tanHalfAngle = tanf(Radian(m_FOV * .5f));
		m_image_plane_dist = m_res_y * .5f / tanHalfAngle;

		const int size = 128;
		BlockedArray<float> apeture_func(size, size);

		for (int i = 0; i < size; i++) {
			float y = 2.f * i / float(size) - 1.f;
			for (int j = 0; j < size; j++) {
				float x = 2.f * j / float(size) - 1.f;
				apeture_func(i, j) = Vector2f(x, y).length() < 1.f ? 1.f : 0.f;
			}
		}

		mp_aperture = MakeUnique<Distribution2D>(apeture_func.data(), size, size);
	}
	void Camera::resize(int width, int height) {
		m_res_x = width;
		m_res_y = height;

		m_ratio = float(m_res_x) / float(m_res_y);
		m_proj = Transform::perspective(m_FOV, m_ratio, m_near, m_far);
		m_screen2raster = Transform().setScale(float(m_res_x), float(m_res_y), 1.f)
			* Transform().setScale(.5f, -.5f, 1.f)
			* Transform().setTranslate(1.f, -1.f, 0.f);
		m_raster2screen = m_screen2raster.inverse();
		m_raster2camera = m_proj.inverse() * m_raster2screen;
		m_camera2raster = m_raster2camera.inverse();
		m_raster2world = m_view_inv * m_raster2camera;
		m_world2raster = m_raster2world.inverse();

		m_dx_cam = rasterToCamera(Point3(1.f, 0.f, 0.f))
			- rasterToCamera(Point3(0.f, 0.f, 0.f));
		m_dy_cam = rasterToCamera(Point3(0.f, 1.f, 0.f))
			- rasterToCamera(Point3(0.f, 0.f, 0.f));

		float tanHalfAngle = tanf(Radian(m_FOV * .5f));
		m_image_plane_dist = m_res_y * .5f / tanHalfAngle;
	}

	bool Camera::generateRay(const CameraSample &sample, Ray *ray, const bool force_pinhole) const {
		Point3 cam_coord = rasterToCamera(Point3(sample.image_x, sample.image_y, 0.f));

		ray->m_ori = Point3(0.f, 0.f, 0.f);
		ray->m_dir = cam_coord.normalize();

		if (m_CoC_radius > 0.f && !force_pinhole) {
			float focal_hit = m_focal_plane_dist / ray->m_dir.z();
			Point3 focal = (*ray)(focal_hit);

			Vector2f scr_coord(2.f * float(sample.image_x) / float(m_res_x) - 1.f,
				2.f * float(sample.image_y) / float(m_res_y) - 1.f);
			scr_coord.x *= m_ratio;
			scr_coord.y *= -1.f;

			float u, v, pdf;
			mp_aperture->sampleContinuous(sample.lens_u, sample.lens_v, &u, &v, &pdf);
			u = 2.f * u - 1.f;
			v = 2.f * v - 1.f;

			if (Vector2f(scr_coord.x + u, scr_coord.y + v).length() > m_vignette_factor)
				return false;

			u *= m_CoC_radius;
			v *= m_CoC_radius;

			ray->m_ori = Point3(u, v, 0.f);
			ray->m_dir = (focal - ray->m_ori).normalize();
		}

		*ray = m_view_inv(*ray);
		ray->m_mint = float(AYA_RAY_EPS);
		ray->m_maxt = float(INFINITY - AYA_RAY_EPS);

		return true;
	}
	bool Camera::generateRayDifferential(const CameraSample &sample, RayDifferential *ray) const {
		Point3 cam_coord = rasterToCamera(Point3(sample.image_x, sample.image_y, 0.f));

		ray->m_ori = Point3(0.f, 0.f, 0.f);
		ray->m_dir = cam_coord.normalize();

		if (m_CoC_radius > 0.f) {
			float focal_hit = m_focal_plane_dist / ray->m_dir.z();
			Point3 focal = (*ray)(focal_hit);

			Vector2f scr_coord(2.f * float(sample.image_x) / float(m_res_x) - 1.f,
				2.f * float(sample.image_y) / float(m_res_y) - 1.f);
			scr_coord.x *= m_ratio;
			scr_coord.y *= -1.f;

			float u, v, pdf;
			mp_aperture->sampleContinuous(sample.lens_u, sample.lens_v, &u, &v, &pdf);
			u = 2.f * u - 1.f;
			v = 2.f * v - 1.f;

			if (Vector2f(scr_coord.x + u, scr_coord.y + v).length() > m_vignette_factor)
				return false;

			u *= m_CoC_radius;
			v *= m_CoC_radius;

			ray->m_ori = Point3(u, v, 0.f);
			ray->m_dir = (focal - ray->m_ori).normalize();
		}
		ray->m_rx_ori = ray->m_ry_ori = ray->m_ori;
		ray->m_rx_dir = (cam_coord + m_dx_cam).normalize();
		ray->m_ry_dir = (cam_coord + m_dy_cam).normalize();
		ray->m_has_differentials = true;

		*ray = m_view_inv(*ray);
		ray->m_mint = float(AYA_RAY_EPS);
		ray->m_maxt = float(INFINITY - AYA_RAY_EPS);

		return true;
	}

	void Camera::setApertureFunc(const char *path) {
		int width, height, channel;
		float *func = Bitmap::read<float>(path, &width, &height, &channel);
		mp_aperture = MakeUnique<Distribution2D>(func, height, width);
		SafeDeleteArray(func);
	}
}