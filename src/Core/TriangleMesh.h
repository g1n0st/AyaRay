#ifndef AYA_CORE_TRIANGLEMESH_H
#define AYA_CORE_TRIANGLEMESH_H

#include "../Math/Vector2.h"
#include "../Math/Vector3.h"
#include "../Math/Transform.h"
#include "../Core/Memory.h"
#include "../Loaders/ObjMesh.h"
#include "../Core/Ray.h"
#include "../Core/Intersection.h"
#include "../Core/BSDF.h"

namespace Aya {
	class TriangleMesh {
		UniquePtr<Transform> w2o, o2w;
		uint32_t m_tris, m_verts;
		uint32_t *mp_vert_idx;
		MeshVertex *mp_vertices;

	public:
		TriangleMesh()
			: m_tris(0), m_verts(0),
			mp_vert_idx(nullptr),
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

		void postIntersect(const Ray &ray, SurfaceIntersection *intersection) const;

		// Data Interface
		__forceinline const Point3& getPositionAt(uint32_t idx) const {
			assert(idx < 3 * m_tris);
			assert(mp_vertices);
			assert(mp_vert_idx);
			return mp_vertices[mp_vert_idx[idx]].p;
		}
		__forceinline const Normal3& getNormalAt(uint32_t idx) const {
			assert(idx < 3 * m_tris);
			assert(mp_vertices);
			assert(mp_vert_idx);
			return mp_vertices[mp_vert_idx[idx]].n;
		}
		__forceinline const Vector2f& getUVAt(uint32_t idx) const {
			assert(idx < 3 * m_tris);
			assert(mp_vertices);
			assert(mp_vert_idx);
			return mp_vertices[mp_vert_idx[idx]].uv;
		}
		__forceinline const uint32_t *getIndexAt(uint32_t idx) const {
			assert(idx < 3 * m_tris);
			assert(mp_vertices);
			assert(mp_vert_idx);
			return &mp_vert_idx[3 * idx];
		}

		__forceinline const uint32_t* getIndexBuffer() const {
			return mp_vert_idx;
		}

		__forceinline uint32_t getTriangleCount() const {
			return m_tris;
		}
		__forceinline uint32_t getVertexCount() const {
			return m_verts;
		}
	};
}

#endif