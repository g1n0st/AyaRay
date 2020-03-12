#include "Scene.h"

#include "../Lights/AreaLight.h"

namespace Aya {
	bool Scene::intersect(const Ray &ray0, Intersection *isect) const {
		Ray ray = m_scene_scale(ray0);
		if (!mp_accel->intersect(ray, isect))
			return false;

		ray0.m_maxt = isect->dist;
		return true;
	}
	void Scene::postIntersect(const Ray &ray, SurfaceIntersection *intersection) const {
		assert(intersection);
		m_primitves[intersection->prim_id]->postIntersect(ray, intersection);

		intersection->p = m_scene_scale(intersection->p);
		intersection->n = m_scene_scale_inv(intersection->n).normalize();
		intersection->gn = m_scene_scale_inv(intersection->gn).normalize();
		intersection->frame = Frame(intersection->n);

		intersection->dpdu = m_scene_scale(intersection->dpdu);
		intersection->dpdv = m_scene_scale(intersection->dpdv);
		intersection->dndu = m_scene_scale(intersection->dndu);
		intersection->dndv = m_scene_scale(intersection->dndv);
	}
	bool Scene::occluded(const Ray &ray0) const {
		Ray ray = m_scene_scale(ray0);
		return mp_accel->occluded(ray);
	}
	BBox Scene::worldBound() const {
		return m_scene_scale(mp_accel->worldBound());
	}
	void Scene::addPrimitive(Primitive *prim) {
		m_primitves.resize(m_primitves.size() + 1);
		m_primitves[m_primitves.size() - 1] = UniquePtr<Primitive>(prim);
	}
	void Scene::addLight(Light *light) {
		if (light->isEnvironmentLight()) {
			bool found_light = false;
			for (auto& it : m_lights) {
				if (it->isEnvironmentLight()) {
					found_light = true;
					it.reset(light);
				}
			}
			if (!found_light)
				m_lights.push_back(UniquePtr<Light>(light));

			mp_env_light = light;
		}
		else if (light->isAreaLight()) {
			m_primitves.emplace_back(UniquePtr<Primitive>(((AreaLight*)light)->getPrimitive()));
			m_lights.emplace_back(UniquePtr<Light>(light));
		}
		else
			m_lights.emplace_back(UniquePtr<Light>(light));
	}

	void Scene::initAccelerator() {
		if (m_dirty) {
			mp_accel = MakeUnique<BVHAccel>();
			std::vector<Primitive*> prims;
			for (const auto& it : m_primitves) {
				prims.push_back(it.get());
			}

			mp_accel->construct(prims);
			m_dirty = false;
		}
	}

}