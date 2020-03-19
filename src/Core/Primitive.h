#ifndef AYA_CORE_PRIMITIVE_H
#define AYA_CORE_PRIMITIVE_H

#include "../Core/BSDF.h"
#include "../Core/Medium.h"
#include "../Core/TriangleMesh.h"

namespace Aya {
	class AreaLight;
	
	class Primitive {
	private:
		friend class Scene_;

		UniquePtr<TriangleMesh> mp_mesh;

		std::vector<UniquePtr<BSDF>> mp_BSDFs;
		//std::vector<UniquePtr<BSSRDF>> mp_BSSRDFs;
		const AreaLight *mp_light = nullptr;

		uint32_t *mp_material_idx = nullptr;
		uint32_t *mp_subset_start_idx  = nullptr;
		uint32_t *mp_subset_material_idx = nullptr;
		uint32_t m_subset_count;

		std::vector<MediumInterface> m_medium_interface;

	public:
		Primitive() {
		}
		~Primitive();

		void loadMesh(const Transform &o2w,
			const char *path,
			const bool force_compute_normal = false,
			UniquePtr<BSDF> bsdf = UniquePtr<BSDF>(nullptr),
			const MediumInterface &medium_interface = MediumInterface());
		void loadSphere(const Transform &o2w,
			const float radius,
			UniquePtr<BSDF> bsdf,
			const MediumInterface &medium_interface = MediumInterface());
		void loadPlane(const Transform &o2w,
			const float length,
			UniquePtr<BSDF> bsdf,
			const MediumInterface &medium_interface = MediumInterface());

		void postIntersect(const Ray &ray, SurfaceIntersection *intersection) const;

		const BSDF* getBSDF(const uint32_t id) const {
			return mp_BSDFs[mp_material_idx[id]].get();
		}
		const MediumInterface* getMediumInterface(const uint32_t id) const {
			return &m_medium_interface[mp_material_idx[id]];
		}
		void setBSDF(const uint32_t id, UniquePtr<BSDF> bsdf) {
			mp_BSDFs[mp_material_idx[id]] = std::move(bsdf);
		}
		void setMediumInterface(const uint32_t id, const MediumInterface &medium_interface) {
			m_medium_interface[mp_material_idx[id]] = medium_interface;
		}

		const TriangleMesh* getMesh() const {
			return mp_mesh.get();
		}
		const uint32_t* getMaterialIdx() const {
			return mp_material_idx;
		}
		const uint32_t* getSubsetStartIdx() const {
			return mp_subset_start_idx;
		}
		const uint32_t* getSubsetMaterialIdx() const {
			return mp_subset_material_idx;
		}
		const uint32_t getSubsetCount() const {
			return m_subset_count;
		}

		void setAreaLight(const AreaLight *light) {
			mp_light = light;
		}

	private:
		void setBSDF(UniquePtr<BSDF> bsdf,
			const MediumInterface &medium_interface = MediumInterface());
	};
}

#endif