#ifndef AYA_TRIANGLE_H
#define AYA_TRIANGLE_H

#include "../core/shape.h"

namespace Aya {
	/**@brief TriangleMesh class inherits Shape base class and represents a triangle mesh*/
	class TriangleMesh : public Shape {
	protected:
		/**@brief Number of vertices and patches*/
		int m_tris, m_verts;
		/**@brief Pointer to vertex index*/
		int *m_vert_idx;
		/**@brief Pointer to points*/
		Point3 *m_p;
		/**@brief Pointer to normals*/
		Normal3 *m_n;
		/**@brief Pointer to uvs*/
		float *m_uvs;

	public:
		TriangleMesh(const Transform *O2W, const Transform *W2O, int tris, int verts,
			const int *vert, const Point3 *P, const Normal3 *N, const float *UV);
		~TriangleMesh();

		friend class Triangle;

		virtual BBox objectBound() const;
		virtual BBox worldBound() const;
		virtual inline bool canIntersect() const;
		/**@brief Refine triangle mesh into triangles */
		virtual void refine(std::vector<SharedPtr<Shape> > &refined) const;
	};

	class Triangle : public Shape {
	private:
		/**@brief Pointer to the TriangleMesh where store the data */
		SharedPtr<TriangleMesh> m_mesh;
		/**@brief vertexs pointers */
		int *m_v;

	public:
		Triangle(const Transform *O2W, const Transform *W2O, TriangleMesh * m, int n);

		virtual BBox objectBound() const;
		virtual BBox worldBound() const;

		/**@brief Get UV coordinates on a triangle */
		inline void getUVs(float uv[3][2]) const;
		virtual bool intersect(const Ray &ray, float *hit_t, SurfaceInteraction *si) const;
	};
}

#endif