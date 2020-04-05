#ifndef AYA_ACCELERATORS_BVH_H
#define AYA_ACCELERATORS_BVH_H

#include "../Core/Accelerator.h"

namespace Aya {
	class BVHTriangle {
		uint32_t mesh_id, tri_id;
		Point3 v0;
		Vector3 e1, e2;
		Normal3 n;

	public:
		BVHTriangle() {}
		BVHTriangle(const Point3 p1,
			const Point3 p2,
			const Point3 p3,
			const uint32_t mid, const uint32_t tid) :
			mesh_id(mid), tri_id(tid) {
			v0 = p1;
			e1 = p1 - p2;
			e2 = p3 - p1;
			n = e1.cross(e2);
		}

		AYA_FORCE_INLINE bool intersect(const Ray &ray, Intersection *isect) const {
			Point3 ori = ray.m_ori;
			Vector3 dir = ray.m_dir;
			Vector3 C = v0 - ori;
			Vector3 R = dir.cross(C);
			float det = n.dot(dir);
			float absdet = Abs(det);
			float U = R.dot(e2);
			if (det < 0) U = -U;
			float V = R.dot(e1);
			if (det < 0) V = -V;
			bool valid = (absdet > 1e-7) & (U >= 0.0f) & (V >= 0.0f) & (U + V <= absdet);
			if (!valid)
				return false;

			float T = n.dot(C);
			if (det < 0) T = -T;
			valid &= (T > absdet * ray.m_mint) & (T < absdet * isect->dist);
			if (!valid)
				return false;

			const float inv = 1.f / absdet;
			const float u = U * inv;
			const float v = V * inv;
			const float t = T * inv;

			ray.m_maxt = t;
			isect->dist = t;
			isect->u = u;
			isect->v = v;
			isect->prim_id = mesh_id;
			isect->tri_id = tri_id;
			
			return true;
		}
		AYA_FORCE_INLINE bool occluded(const Ray &ray) const {
			Point3 ori = ray.m_ori;
			Vector3 dir = ray.m_dir;
			Vector3 C = v0 - ori;
			Vector3 R = dir.cross(C);
			float det = n.dot(dir);
			float absdet = Abs(det);
			float U = R.dot(e2);
			if (det < 0) U = -U;
			float V = R.dot(e1);
			if (det < 0) V = -V;
			bool valid = (absdet > 1e-7) & (U >= 0.0f) & (V >= 0.0f) & (U + V <= absdet);
			if (!valid)
				return false;

			float T = n.dot(C);
			if (det < 0) T = -T;
			valid &= (T > absdet * ray.m_mint) & (T < absdet * ray.m_maxt);
			if (!valid)
				return false;

			return true;
		}
	};

	class BVHNode {
	public:
		BVHNode *l_l, *r_l;
		BBox m_box;

		BVHNode() {
			l_l = r_l = NULL;
			m_box = BBox();
		}
		virtual inline bool intersect(const Ray &ray, Intersection* si, bool &is_leaf) const {
			is_leaf = false;
			return m_box.intersect(ray);
		}
		virtual inline bool occluded(const Ray &ray, bool &is_leaf) const {
			is_leaf = false;
			return m_box.intersect(ray);
		}
		inline void unity() {
			m_box = l_l->m_box;
			m_box.unity(r_l->m_box);
		}
	};
	class BVHLeaf : public BVHNode {
	public:
		BVHTriangle triangle;

		BVHLeaf() {
			l_l = r_l = NULL;
			m_box = BBox();
		}
		BVHLeaf(const Point3 &p1,
			const Point3 &p2,
			const Point3 &p3,
			const uint32_t mid, const uint32_t tid) :
			triangle(p1, p2, p3, mid, tid){
			m_box = BBox(p1, p2);
			m_box.unity(p3);
		}
		virtual inline bool intersect(const Ray &ray, Intersection * si, bool &is_leaf) const {
			is_leaf = true;
			if (!m_box.intersect(ray)) return false;
			return triangle.intersect(ray, si);
		}
		virtual inline bool occluded(const Ray &ray, bool &is_leaf) const {
			is_leaf = true;
			if (!m_box.intersect(ray)) return false;
			return triangle.occluded(ray);
		}
	};

	class BVHAccel : public Accelerator{
	private:
		BVHNode *m_root;

		std::vector<BVHLeaf> m_leafs;

		bool intersect(BVHNode *node, const Ray &ray, Intersection *si) const;
		bool occluded(BVHNode *node, const Ray &ray) const;
		void construct(BVHNode **node, const int &L, const int &R);
		void freeNode(BVHNode **node);

	public:
		BVHAccel() {}
		~BVHAccel() {

		}

		void construct(const std::vector<Primitive*> &prims) override;
		BBox worldBound() const override;
		bool intersect(const Ray &ray, Intersection *si) const override;
		bool occluded(const Ray &ray) const override;
	};
}

#endif