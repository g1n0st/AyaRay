#ifndef AYA_CORE_TRIANGLEMESH_H
#define AYA_CORE_TRIANGLEMESH_H

#include <Core/Config.h>
#include <Math/Transform.h>
#include <Core/Memory.h>
#include <Loaders/ObjMesh.h>
#include <Core/BSDF.h>

namespace Aya {
	class TriangleMesh {
		std::unique_ptr<Transform> w2o, o2w;
		uint32_t m_tris, m_verts;
		uint32_t *mp_vertIdx;
		MeshVertex *mp_vertices;

	public:
		TriangleMesh()
			: m_tris(0), m_verts(0),
			mp_vertIdx(nullptr),
			mp_vertices(nullptr) {}
		~TriangleMesh() {
			release();
		}
		void release() {
			SafeDeleteArray(mp_vertices);
			m_tris = 0;
			m_verts = 0;
		}

		void loadMesh(const Transform &o2w, const ObjMesh *obj_mesh);
		void loadSphere(const Transform &o2w, const float radius, const uint32_t slices = 64, const uint32_t stacks = 64);
		void loadPlane(const Transform &o2w, const float length);

		void postIntersect(const RayDifferential &ray, SurfaceIntersection *intersection) const;

		// Data Interface
		inline const Point3& getPositionAt(uint32_t idx) const {
			assert(idx < 3 * m_tris);
			assert(mp_vertices);
			assert(mp_vertIdx);
			return mp_vertices[mp_vertIdx[idx]].p;
		}
		inline const Normal3& getNormalAt(uint32_t idx) const {
			assert(idx < 3 * m_tris);
			assert(mp_vertices);
			assert(mp_vertIdx);
			return mp_vertices[mp_vertIdx[idx]].n;
		}
		inline const Vector2f& getUVAt(uint32_t idx) const {
			assert(idx < 3 * m_tris);
			assert(mp_vertices);
			assert(mp_vertIdx);
			return mp_vertices[mp_vertIdx[idx]].uv;
		}
		inline const uint32_t *getIndexAt(uint32_t idx) const {
			assert(idx < 3 * m_tris);
			assert(mp_vertices);
			assert(mp_vertIdx);
			return &mp_vertIdx[3 * idx];
		}

		inline const uint32_t* getIndexBuffer() const {
			return mp_vertIdx;
		}
		inline const MeshVertex* getVertexBuffer() const {
			return mp_vertices;
		}

		inline uint32_t getTriangleCount() const {
			return m_tris;
		}
		inline uint32_t getVertexCount() const {
			return m_verts;
		}
	};
}

#endif