#ifndef AYA_LOADERS_OBJMESH_H
#define AYA_LOADERS_OBJMESH_H

#include "../Math/Vector3.h"
#include "../Core/Spectrum.h"
#include "../Core/Memory.h"

#include <cstring>
#include <string>
#include <vector>
#include <cstdio>
#include <map>

#define MAX_PATH 260

namespace Aya {
	struct MeshVertex {
		Point3 p;
		Normal3 n;
		float u, v;

		MeshVertex() = default;
		MeshVertex(const Point3 &pp, const Normal3 &nn, const float uu, const float vv) :
			p(pp), n(nn), u(uu), v(vv) {}
	};
	struct MeshFace {
		int idx[3];
		int smoothing_group;

		MeshFace() = default;
		MeshFace(const int idx0, const int idx1, const int idx2, const int sg) :
			smoothing_group(sg)
		{
			idx[0] = idx0;
			idx[1] = idx1;
			idx[2] = idx2;
		}
	};
	struct Cache {
		uint32_t idx;
		Cache *next;
	};
	struct ObjMaterial {
		char name[MAX_PATH];
		char texture_path[MAX_PATH];
		char bump_path[MAX_PATH];
		Spectrum diffuse_color;
		Spectrum specular_color;
		Spectrum trans_color;
		float refractive_index;

		ObjMaterial(const char *_name = "") :
			diffuse_color(0.85f),
			specular_color(0.f),
			trans_color(0.f),
			refractive_index(0.f) {
			strcpy_s(name, MAX_PATH, _name);
			std::memset(texture_path, 0, MAX_PATH);
			std::memset(bump_path, 0, MAX_PATH);
		}

		inline bool operator == (const ObjMaterial &rhs) const {
			return 0 == strcmp(name, rhs.name);
		}
	};

	class ObjMesh {
	protected:
		uint32_t m_vertex_count, m_triangle_count;

		std::vector<MeshVertex> m_vertices;
		std::vector<uint32_t> m_indices;
		std::vector<MeshFace> m_faces;
		std::vector<Cache*> m_caches;

		std::vector<ObjMaterial> m_materials;
		std::vector<uint32_t> m_material_idx;
		std::vector<uint32_t> m_subset_start_idx;
		std::vector<uint32_t> m_subset_mtl_idx;
		uint32_t m_subset_count;

		bool m_normaled;
		bool m_textured;

	public:
		ObjMesh() :
			m_vertex_count(0),
			m_triangle_count(0),
			m_subset_count(0),
			m_normaled(false),
			m_textured(false) {}
		
		bool loadObj(const char *path, const bool force_compute_normal = false, const bool left_handed = true);
		void loadMtl(const char *path);
		uint32_t addVertex(uint32_t hash, const MeshVertex *vertex);
		void computeVertexNormals();
		
		__forceinline const uint32_t* getIndexAt(int num) const {
			return m_indices.data() + 3 * num;
		}
		__forceinline const MeshVertex& getVertexAt(int idx) const {
			return m_vertices[idx];
		}
		__forceinline const std::vector<MeshVertex>& getVerticesBuff() const {
			return m_vertices;
		}
		__forceinline uint32_t getVertexCount() const {
			return m_vertex_count;
		}
		__forceinline uint32_t getTriangleCount() const {
			return m_triangle_count;
		}
		__forceinline bool isNormaled() const {
			return m_normaled;
		}
		__forceinline bool isTextured() const {
			return m_textured;
		}

		__forceinline const std::vector<ObjMaterial>& getMaterialBuff() const {
			return m_materials;
		}
		__forceinline const std::vector<uint32_t>& getMaterialIdxBuff() const {
			return m_material_idx;
		}
		__forceinline uint32_t getMaterialIdx(int idx) const {
			return m_material_idx[idx];
		}

		__forceinline const uint32_t getSubsetCount() const {
			return m_subset_count;
		}
		__forceinline const uint32_t getSubsetStartIdx(int idx) const {
			return m_subset_start_idx[idx];
		}
		__forceinline const uint32_t getSubsetMtlIdx(int idx) const {
			return m_subset_mtl_idx[idx];
		}
	};
}

#endif