#ifndef AYA_CORE_PRIMITIVE_H
#define AYA_CORE_PRIMITIVE_H

#include <Core/Config.h>
#include <Core/BSDF.h>
#include <Core/Medium.h>
#include <Core/TriangleMesh.h>

namespace Aya {
	class AreaLight;
	
	class Primitive {
	private:
		friend class Scene;

		UniquePtr<TriangleMesh> mp_mesh;

		std::vector<UniquePtr<BSDF>> mp_BSDFs;
		//std::vector<UniquePtr<BSSRDF>> mp_BSSRDFs;
		const AreaLight *mp_light = nullptr;

		uint32_t *mp_materialIdx = nullptr;
		uint32_t *mp_subsetStartIdx  = nullptr;
		uint32_t *mp_subsetMaterialIdx = nullptr;
		uint32_t m_subsetCount;

		std::vector<MediumInterface> m_mediumInterface;

	public:
		Primitive() {
		}
		~Primitive();

		void loadMesh(const Transform &o2w,
			const char *path,
			std::function<UniquePtr<BSDF>(const ObjMaterial&)> mtl_parser,
			const bool force_compute_normal = false,
			const bool left_handed = true,
			const MediumInterface &medium_interface = MediumInterface());
		void loadSphere(const Transform &o2w,
			const float radius,
			UniquePtr<BSDF> bsdf,
			const MediumInterface &medium_interface = MediumInterface());
		void loadPlane(const Transform &o2w,
			const float length,
			UniquePtr<BSDF> bsdf,
			const MediumInterface &medium_interface = MediumInterface());

		void postIntersect(const RayDifferential &ray, SurfaceIntersection *intersection) const;

		const BSDF* getBSDF(const uint32_t id) const {
			return mp_BSDFs[mp_materialIdx[id]].get();
		}
		const MediumInterface* getMediumInterface(const uint32_t id) const {
			return &m_mediumInterface[mp_materialIdx[id]];
		}
		void setBSDF(const uint32_t id, UniquePtr<BSDF> bsdf) {
			mp_BSDFs[mp_materialIdx[id]] = std::move(bsdf);
		}
		void setMediumInterface(const uint32_t id, const MediumInterface &medium_interface) {
			m_mediumInterface[mp_materialIdx[id]] = medium_interface;
		}

		const TriangleMesh* getMesh() const {
			return mp_mesh.get();
		}
		const uint32_t* getMaterialIdx() const {
			return mp_materialIdx;
		}
		const uint32_t* getSubsetStartIdx() const {
			return mp_subsetStartIdx;
		}
		const uint32_t* getSubsetMaterialIdx() const {
			return mp_subsetMaterialIdx;
		}
		const uint32_t getSubsetCount() const {
			return m_subsetCount;
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