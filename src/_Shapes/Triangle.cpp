#include "Triangle.h"

namespace Aya {
	TriangleMesh::TriangleMesh(const Transform *O2W, const Transform *W2O, int tris, int verts,
		const int *vert, const Point3 *P, const Normal3 *N, const float *UV) 
			: Shape(O2W, W2O){
		m_tris = tris;
		m_verts = verts;
		m_vert_idx = new int[3 * m_tris];
		memcpy(m_vert_idx, vert, 3 * m_tris * sizeof(int));
		// copy P, N, UV
		if (UV) {
			m_uvs = new float[2 * m_verts];
			memcpy(m_uvs, UV, 2 * m_verts * sizeof(float));
		}
		else {
			m_uvs = NULL;
		}
		if (N) {
			m_n = new Normal3[m_verts];
			memcpy(m_n, N, m_verts * sizeof(Normal3));
		}
		else {
			m_n = NULL;
		}
		m_p = new Point3[m_verts];
		for (int i = 0; i < m_verts; i++) {
			m_p[i] = (*o2w)(P[i]);
		}
	}
	TriangleMesh::~TriangleMesh() {
		delete[] m_vert_idx;
		delete[] m_p;
		delete[] m_n;
		delete[] m_uvs;
	}

	BBox TriangleMesh::objectBound() const {
		BBox aabb;
		for (int i = 0; i < m_verts; i++) {
			aabb.unity((*w2o)(m_p[i]));
		}
		return aabb;
	}
	BBox TriangleMesh::worldBound() const {
		BBox aabb;
		for (int i = 0; i < m_verts; i++) {
			aabb.unity(m_p[i]);
		}
		return aabb;
	}
	inline bool TriangleMesh::canIntersect() const {
		return false;
	}
	void TriangleMesh::refine(std::vector<SharedPtr<Shape> > &refined) const {
		for (int i = 0; i < m_tris; i++) {
			refined.push_back(new Triangle(o2w, w2o, (TriangleMesh*)this, i));
		}

	}

	Triangle::Triangle(const Transform *O2W, const Transform *W2O, TriangleMesh * m, int n) :
		Shape(O2W, W2O){
		m_mesh = m;
		m_v = &m_mesh->m_vert_idx[3 * n];
	}

	BBox Triangle::objectBound() const {
		const Point3 &p1 = m_mesh->m_p[m_v[0]];
		const Point3 &p2 = m_mesh->m_p[m_v[1]];
		const Point3 &p3 = m_mesh->m_p[m_v[2]];
		return BBox((*w2o)(p1), (*w2o)(p2)).unity((*w2o)(p3));
	}
	BBox Triangle::worldBound() const {
		const Point3 &p1 = m_mesh->m_p[m_v[0]];
		const Point3 &p2 = m_mesh->m_p[m_v[1]];
		const Point3 &p3 = m_mesh->m_p[m_v[2]];
		return BBox(p1, p2).unity(p3);
	}

	inline void Triangle::getUVs(float uv[3][2]) const {
		if (m_mesh->m_uvs) {
			uv[0][0] = m_mesh->m_uvs[2 * m_v[0]];
			uv[0][1] = m_mesh->m_uvs[2 * m_v[0] + 1];
			uv[1][0] = m_mesh->m_uvs[2 * m_v[1]];
			uv[1][1] = m_mesh->m_uvs[2 * m_v[1] + 1];
			uv[2][0] = m_mesh->m_uvs[2 * m_v[2]];
			uv[2][1] = m_mesh->m_uvs[2 * m_v[2] + 1];
		}
		else {
			uv[0][0] = 0.; uv[0][1] = 0.;
			uv[1][0] = 1.; uv[1][1] = 0.;
			uv[2][0] = 1.; uv[2][1] = 1.;
		}
	}
	bool Triangle::intersect(const Ray &ray, float *hit_t, SurfaceInteraction *si) const {
		const Point3 &p1 = m_mesh->m_p[m_v[0]];
		const Point3 &p2 = m_mesh->m_p[m_v[1]];
		const Point3 &p3 = m_mesh->m_p[m_v[2]];
		Vector3 e1 = p2 - p1;
		Vector3 e2 = p3 - p1;
		Vector3 s1 = ray.m_dir.cross(e2);
		float divisor = s1.dot(e1);

		if (std::abs(divisor) < AYA_EPSILON) {
			return false;
		}
		float inv_div = 1.f / divisor;

		Vector3 s = ray.m_ori - p1;
		float b1 = s.dot(s1) * inv_div;
		if (b1 < 0.f || b1 > 1.f) {
			return false;
		}

		Vector3 s2 = s.cross(e1);
		float b2 = ray.m_dir.dot(s2) * inv_div;
		if (b2 < 0.f || b1 + b2 > 1.f) {
			return false;
		}

		float t = e2.dot(s2) * inv_div;
		if (t >= ray.m_maxt || t <= 0.f) {
			return false;
		}

		float uvs[3][2];
		getUVs(uvs);
		float b0 = 1.f - b1 - b2;

		(*hit_t) = t;
		si->t = t;
		si->p = ray(t - AYA_EPSILON);
		//si->p = b0 * p1 + b1 * p2 + b2 * p3;
		if (m_mesh->m_n != NULL) {
			si->n = (*o2w)((Normal3)(b0 * m_mesh->m_n[m_v[0]]
				+ b1 * m_mesh->m_n[m_v[1]]
				+ b2 * m_mesh->m_n[m_v[2]]).normalize());
		}
		else {
			Normal3 nn = (*o2w)((Normal3)e1.cross(e2).normalize());
			if (nn.dot(ray.m_dir) > 0) {
				nn = -nn;
			}
			si->n = nn;
		}
		si->u = b0 * uvs[0][0] + b1 * uvs[1][0] + b2 * uvs[2][0];
		si->v = b0 * uvs[0][1] + b1 * uvs[1][1] + b2 * uvs[2][1];
		return true;
	}
}