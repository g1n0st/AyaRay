#ifndef AYA_ACCELERATOR_EMBREEACCLERATOR_H
#define AYA_ACCELERATOR_EMBREEACCLERATOR_H

#include "../Core/Accelerator.h"

#if defined AYA_USE_EMBREE

#if defined (AYA_USE_EMBREE_STATIC_LIB)
#define EMBREE_STATIC_LIB
#endif

#if (AYA_USE_EMBREE == 3)
#include <embree3/rtcore.h>

#elif (AYA_USE_EMBREE == 2)
#include <embree2/rtcore_ray.h>
#include <embree2/rtcore.h>

#else
#error "Wrong Intel Embree Version!"
#endif
namespace Aya {
	class EmbreeAccel : public Accelerator {
	private:
		RTCScene m_rtc_scene = nullptr;
		RTCDevice m_device = nullptr;

	public:
		EmbreeAccel() {}
		~EmbreeAccel() override;

		void construct(const std::vector<Primitive*> &prims) override;
		BBox worldBound() const override;
		bool intersect(const Ray &ray, Intersection *si) const override;
		bool occluded(const Ray &ray) const override;

	private:

		inline RTCRay toRTCRay(const Ray &ray) const {
#if (AYA_USE_EMBREE == 3)
			RTCRay rtc_ray;
			rtc_ray.org_x = ray.m_ori.x();
			rtc_ray.org_y = ray.m_ori.y();
			rtc_ray.org_z = ray.m_ori.z();
			rtc_ray.dir_x = ray.m_dir.x();
			rtc_ray.dir_y = ray.m_dir.y();
			rtc_ray.dir_z = ray.m_dir.z();
			rtc_ray.tnear = ray.m_mint;
			rtc_ray.tfar = ray.m_maxt;
			rtc_ray.flags = 0;
#elif (AYA_USE_EMBREE == 2)
			RTCRay rtc_ray;
			rtc_ray.org[0] = ray.m_ori.x();
			rtc_ray.org[1] = ray.m_ori.y();
			rtc_ray.org[2] = ray.m_ori.z();
			rtc_ray.dir[0] = ray.m_dir.x();
			rtc_ray.dir[1] = ray.m_dir.y();
			rtc_ray.dir[2] = ray.m_dir.z();
			rtc_ray.tnear = ray.m_mint;
			rtc_ray.tfar = ray.m_maxt;
			rtc_ray.time = 0.f;
			rtc_ray.geomID = RTC_INVALID_GEOMETRY_ID;
			rtc_ray.primID = RTC_INVALID_GEOMETRY_ID;
			rtc_ray.instID = RTC_INVALID_GEOMETRY_ID;
#endif
			return rtc_ray;
		}
	};
}
#endif

#endif