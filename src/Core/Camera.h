#ifndef AYA_CORE_CAMERA_H
#define AYA_CORE_CAMERA_H

#include "../Core/Config.h"
#include "../Core/Ray.h"
#include "../Core/RNG.h"
#include "../Core/Sampling.h"
#include "../Core/Sampler.h"
#include "../Math/Transform.h"
#include "../Loaders/Bitmap.h"

namespace Aya {
	class Camera {
	public:
		Point3 m_pos;
		Point3 m_target;
		Vector3 m_up;
		Vector3 m_dir;

		float m_FOV;
		float m_ratio;
		float m_near;
		float m_far;

		float m_CoC_radius, m_focal_plane_dist;
		float m_image_plane_dist;
		float m_vignette_factor;

		Vector3 m_dx_cam, m_dy_cam;

	protected:
		int m_res_x, m_res_y;

		Transform m_view, m_view_inv;
		Transform m_proj;

		Transform m_screen2raster, m_raster2screen;
		Transform m_raster2camera, m_camera2raster;
		Transform m_raster2world, m_world2raster;

		UniquePtr<Distribution2D> mp_aperture;

	public:
		Camera();
		Camera(const Point3 &pos, const Point3 &tar, const Vector3 &up, int res_x, int res_y,
			float FOV = 35.f, float _near = .1f, float _far = 1000.f,
			const float blur_radius = 0.f, const float focal_dist = 0.f, const float vignette = 3.f);

		virtual ~Camera() {
		}

		virtual void init(const Point3 &pos, const Point3 &tar, const Vector3 &up, int res_x, int res_y,
			float FOV = 35.f, float _near = .1f, float _far = 1000.f,
			const float blur_radius = 0.f, const float focal_dist = 0.f, const float vignette = 3.f);
		virtual void resize(int width, int height);

		bool generateRay(const CameraSample &sample, Ray *ray, const bool force_pinhole = false) const;
		bool generateRayDifferential(const CameraSample &sample, RayDifferential *ray) const;

		inline const Matrix4x4& getViewMatrix() const {
			return m_view.m_mat;
		}
		inline const Matrix4x4& getViewInvMatrix() const {
			return m_view_inv.m_mat;
		}
		inline const Matrix4x4& getProjMatrix() const {
			return m_proj.m_mat;
		}
		inline const Matrix4x4& getRasterMatrix() const {
			return m_screen2raster.m_mat;
		}

		template<class T> inline T worldToRaster(const T elem) const {
			return m_world2raster(elem);
		}
		template<class T> inline T rasterToWorld(const T elem) const {
			return m_raster2world(elem);
		}
		template<class T> inline T rasterToCamera(const T elem) const {
			return m_raster2camera(elem);
		}
		template<class T> inline T cameraToRaster(const T elem) const {
			return m_camera2raster(elem);
		}

		inline int getResolusionX() const {
			return m_res_x;
		}
		inline int getResolusionY() const {
			return m_res_y;
		}
		inline bool checkRaster(const Point3 &pos) const {
			return pos.x() < float(m_res_x) && pos.x() >= 0.f
				&& pos.y() < float(m_res_y) && pos.y() >= 0.f;
		}

		void setApertureFunc(const char *path);

		float getCircleOfConfusionRadius() const {
			return m_CoC_radius;
		}
		float getFocusDistance() const {
			return m_focal_plane_dist;
		}
		float getImagePlaneDistance() const {
			return m_image_plane_dist;
		}
	};
}
#endif