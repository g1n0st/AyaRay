#include <Core/TriangleMesh.h>

namespace Aya {
	void TriangleMesh::loadMesh(const Transform &O2W, const ObjMesh * obj_mesh) {
		o2w = std::make_unique<Transform>(O2W);
		w2o = std::make_unique<Transform>(O2W.inverse());

		m_verts = obj_mesh->getVertexCount();
		m_tris = obj_mesh->getTriangleCount();
		mp_vertices = new MeshVertex[m_verts];
		for (auto i = (uint32_t)0; i < m_verts; ++i) {
			const MeshVertex &vertex = obj_mesh->getVertexAt(i);
			mp_vertices[i].p = (*o2w)(vertex.p);
			mp_vertices[i].n = (*o2w)(vertex.n);
			mp_vertices[i].uv = vertex.uv;
		}
		mp_vertIdx = new uint32_t[3 * m_tris];
		std::memcpy(mp_vertIdx, obj_mesh->getIndexAt(0), sizeof(uint32_t) * 3 * m_tris);
	}
	void TriangleMesh::loadSphere(const Transform & O2W, const float radius, const uint32_t slices, const uint32_t stacks) {
		o2w = std::make_unique<Transform>(O2W);
		w2o = std::make_unique<Transform>(O2W.inverse());

		const float theta_step = float(M_PI) / float(stacks);
		const float phi_step = float(M_PI) * 2.f / float(slices);

		mp_vertices = new MeshVertex[(stacks + 1) * (slices + 1)];

		float theta = 0.f;
		for (auto i = (uint32_t)0; i <= stacks; ++i) {
			float phi = 0.f;
			for (auto j = (uint32_t)0; j <= slices; ++j) {
				Vector3 dir = BaseVector3::sphericalDirection(sinf(theta), cosf(theta), phi);
				Point3 pt = dir * radius;
				mp_vertices[i * (1 + slices) + j] = MeshVertex(
					(*o2w)(pt),
					(*o2w)(dir),
					phi * float(M_1_PI) * 0.5f, theta * float(M_1_PI)
				);
				phi += phi_step;
			}
			theta += theta_step;
		}

		mp_vertIdx = new uint32_t[stacks * slices * 6];

		for (auto i = (uint32_t)0; i < stacks; ++i) {
			for (auto j = (uint32_t)0; j < slices; ++j) {
				auto idx = (i * slices + j) * 6;
				mp_vertIdx[idx + 0] = i * (slices + 1) + j;
				mp_vertIdx[idx + 1] = i * (slices + 1) + j + 1;
				mp_vertIdx[idx + 2] = (i + 1) * (slices + 1) + j;

				mp_vertIdx[idx + 3] = i * (slices + 1) + j + 1;
				mp_vertIdx[idx + 4] = (i + 1) * (slices + 1) + j + 1;
				mp_vertIdx[idx + 5] = (i + 1) * (slices + 1) + j;
			}
		}

		m_verts = (stacks + 1) * (slices + 1);
		m_tris = stacks * slices * 2;
	}
	void TriangleMesh::loadPlane(const Transform & O2W, const float length) {
		o2w = std::make_unique<Transform>(O2W);
		w2o = std::make_unique<Transform>(O2W.inverse());

		const float length_2 = length * .5f;
		const Normal3 n = (*o2w)(Normal3(0.f, 1.f, 0.f));

		mp_vertices = new MeshVertex[4];
		mp_vertices[0] = MeshVertex((*o2w)(Point3(-length_2, 0.f, length_2)), n, 0.f, 0.f);
		mp_vertices[1] = MeshVertex((*o2w)(Point3(-length_2, 0.f, -length_2)), n, 0.f, 1.f);
		mp_vertices[2] = MeshVertex((*o2w)(Point3(length_2, 0.f, -length_2)), n, 1.f, 1.f);
		mp_vertices[3] = MeshVertex((*o2w)(Point3(length_2, 0.f, length_2)), n, 1.f, 0.f);

		mp_vertIdx = new uint32_t[6];
		mp_vertIdx[0] = 0;
		mp_vertIdx[1] = 2;
		mp_vertIdx[2] = 1;
		mp_vertIdx[3] = 2;
		mp_vertIdx[4] = 0;
		mp_vertIdx[5] = 3;

		m_verts = (uint32_t)4;
		m_tris = (uint32_t)2;
	}
	void TriangleMesh::postIntersect(const RayDifferential &ray, SurfaceIntersection *intersection) const {
		assert(intersection);

		const uint32_t id1 = 3 * intersection->tri_id + 0;
		const uint32_t id2 = 3 * intersection->tri_id + 1;
		const uint32_t id3 = 3 * intersection->tri_id + 2;

		const Point3 &vt1 = getPositionAt(id1);
		const Point3 &vt2 = getPositionAt(id2);
		const Point3 &vt3 = getPositionAt(id3);

		const Normal3 &n1 = getNormalAt(id1);
		const Normal3 &n2 = getNormalAt(id2);
		const Normal3 &n3 = getNormalAt(id3);

		const float u = intersection->u;
		const float v = intersection->v;
		const float w = 1.f - u - v;

		intersection->p = w * vt1 + u * vt2 + v * vt3;
		intersection->n = w * n1 + u * n2 + v * n3;

		Vector3 e1 = vt1 - vt2;
		Vector3 e2 = vt3 - vt1;
		intersection->gn = e2.cross(e1).normalize();

		if (intersection->n == Normal3(0.f, 0.f, 0.f))
			intersection->n = intersection->gn;

		if (intersection->bsdf->getTexture() || intersection->bsdf->getNormalMap()) {
			const Vector2f &uv1 = getUVAt(id1);
			const Vector2f &uv2 = getUVAt(id2);
			const Vector2f &uv3 = getUVAt(id3);

			intersection->uv = w * uv1 + u * uv2 + v * uv3;

			Vector2f d1 = uv1 - uv2;
			Vector2f d2 = uv3 - uv1;
			float det = d1.u * d2.v - d2.u * d1.v;

			Normal3 dn1 = n1 - n2;
			Normal3 dn2 = n3 - n1;

			if (det != 0.f) {
				float inv_det = 1.f / det;
				intersection->dpdu = (d2.v * e1 - d1.v * e2) * inv_det;
				intersection->dpdv = (-d2.u * e1 + d1.u * e2) * inv_det;
				intersection->dndu = (d2.v * dn1 - d1.v * dn2) * inv_det;
				intersection->dndv = (-d2.u * dn1 + d1.u * dn2) * inv_det;
			}
			else {
				BaseVector3::coordinateSystem(intersection->n, &intersection->dpdu, &intersection->dpdv);
				intersection->dndu = intersection->dndv = Normal3(0.f, 0.f, 0.f);
			}

			intersection->computeDifferentials(ray);

			auto normal_map = intersection->bsdf->getNormalMap();
			if (det != 0.f && normal_map) {
				intersection->frame = Frame(
					intersection->dpdu.normalize(),
					intersection->dpdv.normalize(),
					intersection->n);

				Vector2f diffs[2] = {
					Vector2f(intersection->dudx, intersection->dvdx),
					Vector2f(intersection->dudy, intersection->dvdy)
				};
				float rgb[3];
				normal_map->sample(intersection->uv, diffs, TextureFilter::TriLinear).toRGB(rgb);
				Vector3 tan_n = 2.f * (Vector3(rgb[0] - .5f, rgb[1] - .5f, rgb[2] - .5f));
				Vector3 normal = intersection->frame.localToWorld(tan_n);
				intersection->n = normal;
				intersection->frame = Frame(normal);
			}
			else
				intersection->frame = Frame(intersection->n);
		}
		else
			intersection->frame = Frame(intersection->n);
	}
}