#ifndef AYA_CORE_SCENE_H
#define AYA_CORE_SCENE_H

#include <Core/Config.h>
#include <Math/Transform.h>
#include <Core/Light.h>
#include <Core/Primitive.h>

#if defined(AYA_USE_EMBREE)
#include <Accelerators/EmbreeAccelerator.h>
#else
#include <Accelerators/BVH.h>
#endif

#include <vector>

namespace Aya {
	class Scene {
	private:
		std::vector<std::unique_ptr<Primitive>> m_primitves;
		std::vector<std::unique_ptr<Light>> m_lights;
		Light* mp_envLight;
		std::unique_ptr<Accelerator> mp_accel;
		bool m_dirty;
		std::vector<std::unique_ptr<const Medium>> m_media;

		Transform m_sceneScale, m_sceneScaleInv;

	public:
		Scene() : mp_envLight(nullptr), m_dirty(true) {}
		~Scene() {}

		bool intersect(const Ray &ray, Intersection *isect) const;
		void postIntersect(const RayDifferential &ray, SurfaceIntersection *intersection) const;
		bool occluded(const Ray &ray) const;

		BBox worldBound() const;

		void addPrimitive(Primitive *prim);
		void addLight(Light *light);

		inline const std::vector<std::unique_ptr<Primitive>>& getPrimitives() const {
			return m_primitves;
		}
		inline const std::vector<std::unique_ptr<Light>>& getLights() const {
			return m_lights;
		}
		inline const Light* getLight(const uint32_t idx) const {
			return m_lights[idx].get();
		}
		inline const Light* getEnviromentLight() const {
			return mp_envLight;
		}
		inline uint32_t getLightCount() const {
			return uint32_t(m_lights.size());
		}

		const Light* chooseLightSource(const float light_sample, float *pdf) const {
			int light_count = int(m_lights.size());
			int light_id = Min(FloorToInt(light_sample * light_count), light_count - 1);
			
			if (pdf)
				*pdf = 1.f / float(light_count);

			return getLight(light_id);
 		}
		float lightPdf(const Light *light) const {
			return 1.f / float(m_lights.size());
		}

		void initAccelerator();

		inline void setScale(const float scale) {
			assert(scale > 0.f);
			m_sceneScale.setScale(scale, scale, scale);
			
			float inv = 1.f / scale;
			m_sceneScaleInv.setScale(inv, inv, inv);
		}
		inline const Transform& getScale() const {
			return m_sceneScale;
		}
	};
}

#endif