#include "BVH.h"

namespace Aya {
	void BVHAccel::construct(std::vector<SharedPtr<Primitive> > prims) {
		// refine all the primitives
		for (auto p : prims) {
			p->fullyRefine(m_prims);
		}
		construct(&m_root, 0, (int)m_prims.size() - 1);
		return;
	}
	BBox BVHAccel::worldBound() const {
		return m_root->m_box;
	}
	bool BVHAccel::intersect(const Ray &ray, SurfaceInteraction *si) const {
		return intersect(m_root, ray, si);
	}

	bool BVHAccel::intersect(BVHNode *node, const Ray &ray, SurfaceInteraction *si) const {
		bool is_leaf = false;
		bool hit_object = node->intersect(ray, si, is_leaf);
		if (is_leaf) {
			return hit_object;
		}
		if (hit_object) {
			SurfaceInteraction l_si, r_si;
			bool hit_l = false;
			if (node->l_l) {
				hit_l = intersect(node->l_l, ray, &l_si);
			}
			bool hit_r = false;
			if (node->r_l) {
				hit_r = intersect(node->r_l, ray, &r_si);
			}

			if (hit_l && hit_r) {
				(*si) = l_si.t < r_si.t ? l_si : r_si;
				return true;
			}
			else if (hit_l) {
				(*si) = l_si;
				return true;
			}
			else if (hit_r) {
				(*si) = r_si;
				return true;
			}
			return false;
		}
		return false;
	}
	inline bool xBVHCmp(const SharedPtr<Primitive> &a, const SharedPtr<Primitive> &b) {
		BBox ab = a->worldBound();
		BBox bb = b->worldBound();
		return ab.m_pmin.x() < bb.m_pmin.x();
	}
	inline bool yBVHCmp(const SharedPtr<Primitive> &a, const SharedPtr<Primitive> &b) {
		BBox ab = a->worldBound();
		BBox bb = b->worldBound();
		return ab.m_pmin.y() < bb.m_pmin.y();
	}
	inline bool zBVHCmp(const SharedPtr<Primitive> &a, const SharedPtr<Primitive> &b) {
		BBox ab = a->worldBound();
		BBox bb = b->worldBound();
		return ab.m_pmin.z() < bb.m_pmin.z();
	}

	void BVHAccel::construct(BVHNode **node, const int &L, const int &R) {
		if (L > R) {
			node = NULL;
			return;
		}
		*node = new BVHNode();
		static int axis;
		axis = (axis + 1) % 3;
		switch (axis) {
		case 0:
			std::sort(m_prims.begin() + L, m_prims.begin() + R + 1, xBVHCmp);
			break;
		case 1:
			std::sort(m_prims.begin() + L, m_prims.begin() + R + 1, yBVHCmp);
			break;
		default:
			std::sort(m_prims.begin() + L, m_prims.begin() + R + 1, zBVHCmp);
		}

		if (L == R) {
			*node = new BVHLeaf(m_prims[L]);
			return;
		}

		int mid = (L + R) >> 1;
		*node = new BVHNode();
		construct(&(*node)->l_l, L, mid);
		construct(&(*node)->r_l, mid + 1, R);
		(*node)->unity();
	}
	void BVHAccel::freeNode(BVHNode **node) {
		if ((*node)->l_l != NULL) {
			freeNode(&(*node)->l_l);
		}
		if ((*node)->r_l != NULL) {
			freeNode(&(*node)->r_l);
		}
		delete *node;
	}
}