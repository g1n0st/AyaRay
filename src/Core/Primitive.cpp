#include "Primitive.h"

#include "../BSDFs/LambertianDiffuse.h"
#include "../BSDFs/Glass.h"
#include "../BSDFs/Mirror.h"
#include "../BSDFs/Disney.h"

namespace Aya {
	Primitive::~Primitive() {
		SafeDeleteArray(mp_materialIdx);
		SafeDeleteArray(mp_subsetStartIdx);
		SafeDeleteArray(mp_subsetMaterialIdx);
	}

	void Primitive::loadMesh(const Transform &o2w,
		const char *path,
		std::function<UniquePtr<BSDF>(const ObjMaterial&)> mtl_parser,
		const bool force_compute_normal,
		const bool left_handed,
		const MediumInterface &medium_interface) {
		ObjMesh *mesh = new ObjMesh;
		mesh->loadObj(path, force_compute_normal, left_handed);
		mp_mesh = MakeUnique<TriangleMesh>();
		mp_mesh->loadMesh(o2w, mesh);

		const auto& mtl_info = mesh->getMaterialBuff();
		mp_BSDFs.resize(mtl_info.size());

		for (auto i = 0; i < mtl_info.size(); i++) {
			auto &mtl = mtl_info[i];
			mp_BSDFs[i] = mtl_parser(mtl);
			m_mediumInterface.emplace_back(medium_interface);
		}

		mp_materialIdx = new uint32_t[mesh->getTriangleCount()];

		for (uint32_t i = 0; i < mesh->getTriangleCount(); i++)
			mp_materialIdx[i] = mesh->getMaterialIdx(i);

		m_subsetCount = mesh->getSubsetCount();
		mp_subsetMaterialIdx = new uint32_t[m_subsetCount];
		mp_subsetStartIdx = new uint32_t[m_subsetCount];
		for (uint32_t i = 0; i < m_subsetCount; i++) {
			mp_subsetMaterialIdx[i] = mesh->getSubsetMtlIdx(i);
			mp_subsetStartIdx[i] = mesh->getSubsetStartIdx(i);
		}

		SafeDelete(mesh);
	}
	void Primitive::loadSphere(const Transform &o2w,
		const float radius,
		UniquePtr<BSDF> bsdf,
		const MediumInterface &medium_interface) {
		mp_mesh = MakeUnique<TriangleMesh>();
		mp_mesh->loadSphere(o2w, radius);

		setBSDF(std::move(bsdf), medium_interface);
	}
	void Primitive::loadPlane(const Transform &o2w,
		const float length,
		UniquePtr<BSDF> bsdf,
		const MediumInterface &medium_interface) {
		mp_mesh = MakeUnique<TriangleMesh>();
		mp_mesh->loadPlane(o2w, length);

		setBSDF(std::move(bsdf), medium_interface);
	}

	void Primitive::postIntersect(const RayDifferential &ray, SurfaceIntersection *intersection) const {
		intersection->bsdf = mp_BSDFs[mp_materialIdx[intersection->tri_id]].get();
		// BSSRDF Part
		intersection->arealight = mp_light;
		intersection->m_mediumInterface = m_mediumInterface[mp_materialIdx[intersection->tri_id]];
		mp_mesh->postIntersect(ray, intersection);
	}
	void Primitive::setBSDF(UniquePtr<BSDF> bsdf, const MediumInterface &medium_interface) {
		mp_BSDFs.resize(1);
		mp_BSDFs[0] = std::move(bsdf);
		m_mediumInterface.emplace_back(medium_interface);

		mp_materialIdx = new uint32_t[mp_mesh->getTriangleCount()];
		memset(mp_materialIdx, 0, sizeof(uint32_t) * mp_mesh->getTriangleCount());

		m_subsetCount = 1;
		mp_subsetMaterialIdx = new uint32_t[1];
		mp_subsetStartIdx = new uint32_t[1];
		mp_subsetMaterialIdx[0] = mp_subsetStartIdx[0] = 0;
	}
}