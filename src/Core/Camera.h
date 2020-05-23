#ifndef AYA_CORE_CAMERA_H
#define AYA_CORE_CAMERA_H

#include <Core/Config.h>
#include <Core/Ray.h>
#include <Core/RNG.h>
#include <Core/Sampling.h>
#include <Core/Sampler.h>
#include <Math/Transform.h>
#include <Loaders/Bitmap.h>

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

		float m_CoCRadius, m_focalPlaneDist;
		float m_imagePlaneDist;
		float m_vignetteFactor;

		Vector3 m_dxCam, m_dyCam;

	protected:
		int m_resX, m_resY;

		Transform m_view, m_viewInv;
		Transform m_proj;

		Transform m_screenToRaster, m_rasterToScreen;
		Transform m_rasterToCamera, m_cameraToRaster;
		Transform m_rasterToWorld, m_worldToRaster;

		UniquePtr<Distribution2D> mp_aperture;

	public:
		Camera() = default;
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

		template<class T> inline T view(const T elem) const {
			return m_view(elem);
		}
		template<class T> inline T viewInv(const T elem) const {
			return m_viewInv(elem);
		}
		template<class T> inline T proj(const T elem) const {
			return m_proj(elem);
		}
		template<class T> inline T worldToRaster(const T elem) const {
			return m_worldToRaster(elem);
		}
		template<class T> inline T rasterToWorld(const T elem) const {
			return m_rasterToWorld(elem);
		}
		template<class T> inline T rasterToCamera(const T elem) const {
			return m_rasterToCamera(elem);
		}
		template<class T> inline T cameraToRaster(const T elem) const {
			return m_cameraToRaster(elem);
		}

		inline int getResolusionX() const {
			return m_resX;
		}
		inline int getResolusionY() const {
			return m_resY;
		}
		inline bool checkRaster(const Point3 &pos) const {
			return pos.x < float(m_resX) && pos.x >= 0.f
				&& pos.y < float(m_resY) && pos.y >= 0.f;
		}

		void setApertureFunc(const char *path);

		float getCircleOfConfusionRadius() const {
			return m_CoCRadius;
		}
		float getFocusDistance() const {
			return m_focalPlaneDist;
		}
		float getImagePlaneDistance() const {
			return m_imagePlaneDist;
		}
	};
}
#endif