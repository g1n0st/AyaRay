#include "Primitive.h"

#include "../BSDFs/LambertianDiffuse.h"
#include "../BSDFs/Glass.h"
#include "../BSDFs/Mirror.h"

namespace Aya {
	Primitive::~Primitive() {
		SafeDeleteArray(mp_material_idx);
		SafeDeleteArray(mp_subset_start_idx);
		SafeDeleteArray(mp_subset_material_idx);
	}

	void Primitive::loadMesh(const Transform &o2w,
		const char *path,
		const bool force_compute_normal,
		UniquePtr<BSDF> bsdf,
		const MediumInterface &medium_interface) {
		ObjMesh *mesh = new ObjMesh;
		mesh->loadObj(path, force_compute_normal);
		mp_mesh = MakeUnique<TriangleMesh>();
		mp_mesh->loadMesh(o2w, mesh);

		const auto& mtl_info = mesh->getMaterialBuff();
		mp_BSDFs.resize(mtl_info.size());
;		for (auto i = 0; i < mtl_info.size(); i++) {
			auto &mtl = mtl_info[i];
			if (bsdf.get() == nullptr) {
				if (mtl.texture_path[0]) {
					if (mtl.bump_path[0])
						mp_BSDFs[i] = MakeUnique<LambertianDiffuse>(mtl.texture_path, mtl.bump_path);
					else
						mp_BSDFs[i] = MakeUnique<LambertianDiffuse>(mtl.texture_path);
				}
				else {
					if (mtl.trans_color != Spectrum(0.f))
						mp_BSDFs[i] = MakeUnique<Glass>(mtl.trans_color, 1.f, mtl.refractive_index);
					else if (mtl.specular_color != Spectrum(0.f))
						mp_BSDFs[i] = MakeUnique<Mirror>(mtl.specular_color);
					else
						mp_BSDFs[i] = MakeUnique<LambertianDiffuse>(mtl.diffuse_color);

					if (mtl.bump_path[0])
						mp_BSDFs.back()->setNormalMap(mtl.bump_path);
				}
			}
			else
				mp_BSDFs[i] = std::move(bsdf);

			m_medium_interface.emplace_back(medium_interface);
		}

		mp_material_idx = new uint32_t[mesh->getTriangleCount()];
		for (uint32_t i = 0; i < mesh->getTriangleCount(); i++)
			mp_material_idx[i] = mesh->getMaterialIdx(i);

		m_subset_count = mesh->getSubsetCount();
		mp_subset_material_idx = new uint32_t[m_subset_count];
		mp_subset_start_idx = new uint32_t[m_subset_count];
		for (uint32_t i = 0; i < m_subset_count; i++) {
			mp_subset_material_idx[i] = mesh->getSubsetMtlIdx(i);
			mp_subset_start_idx[i] = mesh->getSubsetStartIdx(i);
		}
	}
	void Primitive::loadSphere(const Transform &o2w,
		const float radius,
		UniquePtr<BSDF> bsdf,
		const MediumInterface &medium_interface) {
		mp_mesh = MakeUnique<TriangleMesh>();
		mp_mesh->loadSphere(o2w, radius);

		mp_BSDFs.resize(1);
		mp_BSDFs[0] = std::move(bsdf);
		m_medium_interface.emplace_back(medium_interface);

		mp_material_idx = new uint32_t[mp_mesh->getTriangleCount()];
		memset(mp_material_idx, 0, sizeof(mp_material_idx));

		m_subset_count = 1;
		mp_subset_material_idx = new uint32_t[1];
		mp_subset_start_idx = new uint32_t[1];
		mp_subset_material_idx[0] = mp_subset_start_idx[0] = 0;
	}
	void Primitive::loadPlane(const Transform &o2w,
		const float length,
		UniquePtr<BSDF> bsdf,
		const MediumInterface &medium_interface) {
		mp_mesh = MakeUnique<TriangleMesh>();
		mp_mesh->loadPlane(o2w, length);

		mp_BSDFs.resize(1);
		mp_BSDFs[0] = std::move(bsdf);
		m_medium_interface.emplace_back(medium_interface);

		mp_material_idx = new uint32_t[mp_mesh->getTriangleCount()];
		memset(mp_material_idx, 0, sizeof(mp_material_idx));

		m_subset_count = 1;
		mp_subset_material_idx = new uint32_t[1];
		mp_subset_start_idx = new uint32_t[1];
		mp_subset_material_idx[0] = mp_subset_start_idx[0] = 0;
	}

	void Primitive::postIntersect(const Ray &ray, SurfaceIntersection *intersection) const {
		intersection->bsdf = mp_BSDFs[mp_material_idx[intersection->tri_id]].get();
		// BSSRDF Part
		// Light Part
		intersection->m_medium_interface = m_medium_interface[mp_material_idx[intersection->tri_id]];
		mp_mesh->postIntersect(ray, intersection);
	}
}