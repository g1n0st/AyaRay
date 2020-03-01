#ifndef AYA_ACCELERATOR_BVH__H
#define AYA_ACCELERATOR_BVH__H

#include <algorithm>

#include "../core/primitive_.h"

namespace Aya {
	class BVHNode_{
	public:
		BVHNode_ *l_l, *r_l;
		BBox m_box;

		BVHNode_() {
			l_l = r_l = NULL;
			m_box = BBox();
		}
		virtual inline bool intersect(const Ray &ray, SurfaceInteraction * si, bool &is_leaf) const {
			is_leaf = false;
			return m_box.intersect(ray);
		}
		inline void unity() {
			m_box = l_l->m_box;
			m_box.unity(r_l->m_box);
		}
	};
	class BVHLeaf_ : public BVHNode_ {
	public:
		SharedPtr<_Primitive> m_prim;

		BVHLeaf_() {
			l_l = r_l = NULL;
			m_box = BBox();
		}
		BVHLeaf_(const SharedPtr<_Primitive> &prim) {
			m_prim = prim;
			m_box = m_prim->worldBound();
		}
		virtual inline bool intersect(const Ray &ray, SurfaceInteraction * si, bool &is_leaf) const {
			is_leaf = true;
			if (!m_box.intersect(ray)) return false;
			return m_prim->intersect(ray, si);
		}
	};

	class BVHAccel_ : public Accelerator {
	public:
		std::vector<SharedPtr<_Primitive> > m_prims;
		BVHNode_ *m_root;
	public:
		BVHAccel_() {
			m_root = NULL;
		}
		~BVHAccel_() {
			freeNode(&m_root);
			m_prims.clear();
		}

		virtual void construct(std::vector<SharedPtr<_Primitive> > prims);
		virtual BBox worldBound() const;
		virtual bool canIntersect() {
			return false;
		}
		virtual bool intersect(const Ray &ray, SurfaceInteraction *si) const;

	private:
		bool intersect(BVHNode_ *node, const Ray &ray, SurfaceInteraction *si) const;
		void construct(BVHNode_ **node, const int &L, const int &R);
		void freeNode(BVHNode_ **node);
	};
}

#endif