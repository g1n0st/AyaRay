#ifndef AYA_LOADERS_OBJMESH_H
#define AYA_LOADERS_OBJMESH_H

#include "../Math/Vector2.h"
#include "../Math/Vector3.h"
#include "../Core/Spectrum.h"
#include "../Core/Memory.h"

#include <cstring>
#include <string>
#include <vector>
#include <cstdio>
#include <functional>

#define AYA_MAX_PATH 1024

namespace Aya {
	struct MeshVertex {
		Point3 p;
		Normal3 n;
		Vector2f uv;

		MeshVertex() {
			p = Point3(0.f, 0.f, 0.f);
			n = Normal3(0.f, 0.f, 0.f);
			uv = Vector2f(0.f, 0.f);
		}
		MeshVertex(const Point3 &pp, const Normal3 &nn, const float uu, const float vv) :
			p(pp), n(nn), uv(uu, vv) {}

		friend bool operator == (const MeshVertex &a, const MeshVertex &b) {
			return (a.p == b.p) && (a.n == b.n) && (a.uv == b.uv);
		}
	};
	struct MeshFace {
		int idx[3];
		int smoothing_group;

		MeshFace() = default;
		MeshFace(const int idx0, const int idx1, const int idx2, const int sg) :
			smoothing_group(sg) {
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
		char name[AYA_MAX_PATH];			// newmtl name
		char map_Kd[AYA_MAX_PATH];		// texture map
		char map_Ks[AYA_MAX_PATH];		// specular map
		char map_Bump[AYA_MAX_PATH];	// bump map
		RGBSpectrum Ka;					// ambient color
		RGBSpectrum Ke;					// missive color
		RGBSpectrum Kd;					// diffuse color
		RGBSpectrum Ks;					// specular colore
		RGBSpectrum Tf;					// transmission filter
		float Ns;						// refractive index
		float Ni;						// reflection index
		int illum; // illumination

		// 0 Color on and Ambient off
		// 1 Color on and Ambient on
		// 2 Highlight on
		// 3 Reflection on and Ray trace on
		// 4 Transparency: Glass on
		//	 Reflection : Ray trace on
		// 5 Reflection : Fresnel on and Ray trace on
		// 6 Transparency : Refraction on
		//	 Reflection : Fresnel off and Ray trace on
		// 7 Transparency : Refraction on
		//	 Reflection : Fresnel on and Ray trace on
		// 8 Reflection on and Ray trace off
		// 9 Transparency : Glass on Reflection : Ray trace off
		// 10 Casts shadows onto invisible surfaces

		ObjMaterial(const char *_name = "") :
			Ka(0.f),
			Ke(0.f),
			Kd(0.85f),
			Ks(0.f),
			Tf(0.f),
			Ni(0.f),
			Ns(0.f),
			illum(0) {
			strcpy_s(name, AYA_MAX_PATH, _name);
			std::memset(map_Kd, 0, AYA_MAX_PATH);
			std::memset(map_Bump, 0, AYA_MAX_PATH);
			std::memset(map_Ks, 0, AYA_MAX_PATH);
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
		uint32_t m_subsetCount;

		bool m_normaled;
		bool m_textured;

	public:
		ObjMesh() :
			m_vertex_count(0),
			m_triangle_count(0),
			m_subsetCount(0),
			m_normaled(false),
			m_textured(false) {}

		bool loadObj(const char *path, const bool force_compute_normal = false, const bool left_handed = true);
		void loadMtl(const char *path);
		uint32_t addVertex(uint32_t hash, const MeshVertex *vertex);
		void computeVertexNormals();

		inline const uint32_t* getIndexAt(int num) const {
			return m_indices.data() + 3 * num;
		}
		inline const MeshFace& getFaceAt(int idx) const {
			return m_faces[idx];
		}
		inline const std::vector<MeshFace>& getFacesBuff() const {
			return m_faces;
		}
		inline const MeshVertex& getVertexAt(int idx) const {
			return m_vertices[idx];
		}
		inline const std::vector<MeshVertex>& getVerticesBuff() const {
			return m_vertices;
		}
		inline uint32_t getVertexCount() const {
			return m_vertex_count;
		}
		inline uint32_t getTriangleCount() const {
			return m_triangle_count;
		}
		inline bool isNormaled() const {
			return m_normaled;
		}
		inline bool isTextured() const {
			return m_textured;
		}

		inline const std::vector<ObjMaterial>& getMaterialBuff() const {
			return m_materials;
		}
		inline const std::vector<uint32_t>& getMaterialIdxBuff() const {
			return m_material_idx;
		}
		inline uint32_t getMaterialIdx(int idx) const {
			return m_material_idx[idx];
		}

		inline const uint32_t getSubsetCount() const {
			return m_subsetCount;
		}
		inline const uint32_t getSubsetStartIdx(int idx) const {
			return m_subset_start_idx[idx];
		}
		inline const uint32_t getSubsetMtlIdx(int idx) const {
			return m_subset_mtl_idx[idx];
		}

	private:
		void parserFramework(const char *filename, std::function<void(char*, char *)> callback);
	};
}

#endif