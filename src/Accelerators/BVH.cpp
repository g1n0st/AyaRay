#include "BVH.h"

namespace Aya {
	void BVHAccel::construct(const std::vector<Primitive*> &prims) {
		for (uint32_t i = 0; i < prims.size(); i++) {
			auto mesh = prims[i]->getMesh();
			for (uint32_t j = 0; j < mesh->getTriangleCount(); j++) {
				m_leafs.emplace_back(
					mesh->getPositionAt(3 * j + 0),
					mesh->getPositionAt(3 * j + 1),
					mesh->getPositionAt(3 * j + 2),
					i,
					j
				);
			}
		}
		construct(&m_root, 0, (int)m_leafs.size() - 1);
		return;
	}
	BBox BVHAccel::worldBound() const {
		return m_root->m_box;
	}
	bool BVHAccel::intersect(const Ray &ray, Intersection *si) const {
		return intersect(m_root, ray, si);
	}
	bool BVHAccel::occluded(const Ray &ray) const {
		return occluded(m_root, ray);
	}
	bool BVHAccel::occluded(BVHNode *node, const Ray &ray) const {
		bool is_leaf = false;
		bool hit_object = node->occluded(ray, is_leaf);
		if (is_leaf) {
			return hit_object;
		}
		if (hit_object) {
			Intersection l_si, r_si;
			bool hit_l = false;
			if (node->l_l) {
				hit_l = occluded(node->l_l, ray);
			}
			bool hit_r = false;
			if (node->r_l) {
				hit_r = occluded(node->r_l, ray);
			}

			if (hit_l || hit_r) return true;
			return false;
		}
		return false;
	}
	bool BVHAccel::intersect(BVHNode *node, const Ray &ray, Intersection *si) const {
		bool is_leaf = false;
		bool hit_object = node->intersect(ray, si, is_leaf);
		if (is_leaf) {
			return hit_object;
		}
		if (hit_object) {
			Intersection l_si, r_si;
			bool hit_l = false;
			if (node->l_l) {
				hit_l = intersect(node->l_l, ray, &l_si);
			}
			bool hit_r = false;
			if (node->r_l) {
				hit_r = intersect(node->r_l, ray, &r_si);
			}

			if (hit_l && hit_r) {
				(*si) = l_si.dist < r_si.dist ? l_si : r_si;
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
	inline bool xBVHCmp(const BVHLeaf &a, const BVHLeaf &b) {
		return a.m_box.m_pmin.x() < b.m_box.m_pmin.x();
	}
	inline bool yBVHCmp(const BVHLeaf &a, const BVHLeaf &b) {
		return a.m_box.m_pmin.y() < b.m_box.m_pmin.y();
	}
	inline bool zBVHCmp(const BVHLeaf &a, const BVHLeaf &b) {
		return a.m_box.m_pmin.z() < b.m_box.m_pmin.z();
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
			std::sort(m_leafs.begin() + L, m_leafs.begin() + R + 1, xBVHCmp);
			break;
		case 1:
			std::sort(m_leafs.begin() + L, m_leafs.begin() + R + 1, yBVHCmp);
			break;
		default:
			std::sort(m_leafs.begin() + L, m_leafs.begin() + R + 1, zBVHCmp);
		}

		if (L == R) {
			*node = new BVHLeaf(m_leafs[L]);
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