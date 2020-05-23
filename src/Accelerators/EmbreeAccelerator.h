#ifndef AYA_ACCELERATOR_EMBREEACCLERATOR_H
#define AYA_ACCELERATOR_EMBREEACCLERATOR_H

#include <Core/Accelerator.h>

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
		RTCScene m_rtcScene = nullptr;
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
			rtc_ray.org_x = ray.m_ori.x;
			rtc_ray.org_y = ray.m_ori.y;
			rtc_ray.org_z = ray.m_ori.z;
			rtc_ray.dir_x = ray.m_dir.x;
			rtc_ray.dir_y = ray.m_dir.y;
			rtc_ray.dir_z = ray.m_dir.z;
			rtc_ray.tnear = ray.m_mint;
			rtc_ray.tfar = ray.m_maxt;
			rtc_ray.flags = 0;
#elif (AYA_USE_EMBREE == 2)
			RTCRay rtc_ray;
			rtc_ray.org[0] = ray.m_ori.x;
			rtc_ray.org[1] = ray.m_ori.y;
			rtc_ray.org[2] = ray.m_ori.z;
			rtc_ray.dir[0] = ray.m_dir.x;
			rtc_ray.dir[1] = ray.m_dir.y;
			rtc_ray.dir[2] = ray.m_dir.z;
			rtc_ray.tnear = ray.m_mint;
			rtc_ray.tfar = ray.m_maxt;
			rtc_ray.time = 0.f;
			rtc_ray.geomID = RTC_INVALID_GEOMETRY_ID;
			rtc_ray.primID = RTC_INVALID_GEOMETRY_ID;
			rtc_ray.instID = RTC_INVALID_GEOMETRY_ID;
#endif
			return rtc_ray;
		}

#if (AYA_USE_EMBREE == 3) 
		static void alphaTest(const struct RTCFilterFunctionNArguments* args) {
			Primitive *prim = (Primitive*)args->geometryUserPtr;

			uint32_t prim_id = RTCHitN_primID(args->hit, 1, 0);
			float u = RTCHitN_u(args->hit, 1, 0);
			float v = RTCHitN_v(args->hit, 1, 0);
			if (prim->getBSDF(prim_id)->getTexture()->hasAlpha()) {
				const TriangleMesh *mesh = prim->getMesh();
				const Vector2f &uv1 = mesh->getUVAt(3 * prim_id + 0);
				const Vector2f &uv2 = mesh->getUVAt(3 * prim_id + 1);
				const Vector2f &uv3 = mesh->getUVAt(3 * prim_id + 2);

				const Vector2f uv = (1.0f - u - v) * uv1 +
					u * uv2 +
					v * uv3;

				if (!prim->getBSDF(prim_id)->getTexture()->alphaTest(uv))
					RTCHitN_geomID(args->hit, 1, 0) = RTC_INVALID_GEOMETRY_ID; // reject hit
			}
		}
#elif (AYA_USE_EMBREE == 2) 
		static void alphaTest(void *user_ptr, RTCRay& ray) {
			Primitive *prim = (Primitive*)user_ptr;

			if (prim->getBSDF(ray.primID)->getTexture()->hasAlpha()) {
				const TriangleMesh *mesh = prim->getMesh();
				const Vector2f &uv1 = mesh->getUVAt(3 * ray.primID + 0);
				const Vector2f &uv2 = mesh->getUVAt(3 * ray.primID + 1);
				const Vector2f &uv3 = mesh->getUVAt(3 * ray.primID + 2);

				const Vector2f uv = (1.0f - ray.u - ray.v) * uv1 +
					ray.u * uv2 +
					ray.v * uv3;

				if (!prim->getBSDF(ray.primID)->getTexture()->alphaTest(uv))
					ray.geomID = RTC_INVALID_GEOMETRY_ID; // reject hit
			}
	}
#endif
	};
#endif
}
#endif