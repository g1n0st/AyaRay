#ifndef AYA_LIGHTS_AREALIGHT_H
#define AYA_LIGHTS_AREALIGHT_H

#include "../Core/Light.h"
#include "../Core/Intersection.h"
#include "../Core/Primitive.h"
#include "../Core/Sampler.h"
#include "../Core/Sampling.h"
#include "../Accelerators/BVH.h"

namespace Aya {
	class AreaLight : public Light {
	private:
		Primitive *mp_prim;
		Spectrum m_intensity;
		uint32_t m_triangle_count;
		float m_area, m_area_inv;
		UniquePtr<BVHAccel> m_BVH;

	public:
		AreaLight(Primitive *prim,
			const Spectrum &intens,
			const uint32_t sample_count = 1) :
			Light(sample_count), mp_prim(prim), m_intensity(intens) {
			m_triangle_count = mp_prim->getMesh()->getTriangleCount();
			m_area = 0.f;
			for (uint32_t i = 0; i < m_triangle_count; i++) {
				float area = triangleArea(i);
				m_area += area;
			}

			m_area_inv = 1.f / m_area;
			mp_prim->setAreaLight(this);

			m_BVH = MakeUnique<BVHAccel>();
			m_BVH->construct({ mp_prim });
		}

		Spectrum illuminate(const Scatter &scatter,
			const Sample &light_sample,
			Vector3 *dir,
			VisibilityTester *tester,
			float *pdf,
			float *cos_at_light = nullptr,
			float *emit_pdf_w = nullptr) const override {
			const Point3 &pos = scatter.p;
			uint32_t tri_id = uint32_t(light_sample.w * m_triangle_count);
			tri_id = Clamp(tri_id, 0, m_triangle_count - 1);

			float b0, b1;
			UniformSampleTriangle(light_sample.u, light_sample.v, &b0, &b1);

			Point3 light_p;
			Normal3 light_n;
			sampleTriangle(tri_id, b0, b1, light_p, light_n);

			const Vector3 light_v = light_p - pos;
			*dir = light_v.normalize();
			tester->setSegment(pos, light_p);
			tester->setMedium(scatter.m_medium_interface.getMedium(*dir, scatter.n));

			const float dist2 = light_v.length2();
			*pdf = dist2 / Abs(light_n.dot(-*dir)) * m_area_inv;

			const float cos = light_n.dot(-*dir);
			if (cos < 1e-6f)
				return Spectrum(0.f);

			if (cos_at_light)
				*cos_at_light = cos;
			if (emit_pdf_w)
				*emit_pdf_w = CosineHemispherePDF(cos) * m_area_inv;

			return m_intensity;
		}

		Spectrum sample(const Sample &light_sample0,
			const Sample &light_sample1,
			Ray *ray,
			Normal3 *normal,
			float *pdf,
			float *direct_pdf = nullptr) const override {
			uint32_t tri_id = uint32_t(light_sample0.w * m_triangle_count);
			tri_id = Clamp(tri_id, 0, m_triangle_count - 1);

			float b0, b1;
			UniformSampleTriangle(light_sample0.u, light_sample0.v, &b0, &b1);

			Point3 light_p;
			Normal3 light_n;
			sampleTriangle(tri_id, b0, b1, light_p, light_n);
			*normal = light_n.normalize();

			Vector3 local_dir_out = CosineSampleHemisphere(light_sample1.u, light_sample1.v);
			local_dir_out.setZ(Max(local_dir_out.z(), 1e-6f));

			Frame light_frame = Frame(light_n);
			Vector3 world_dir_out = light_frame.localToWorld(local_dir_out);

			*ray = Ray(light_p, world_dir_out);
			*pdf = m_area_inv * CosineHemispherePDF(CosTheta(local_dir_out));

			if (direct_pdf)
				*direct_pdf = m_area_inv;

			return m_intensity * CosTheta(local_dir_out);
		}

		Spectrum emit(const Vector3 &dir,
			const Normal3 &normal = Normal3(0.f, 0.f, 0.f),
			float *pdf = nullptr,
			float *direct_pdf = nullptr) const override {
			if (normal.dot(dir) <= 0.f)
				return Spectrum(0.f);

			if (direct_pdf)
				*direct_pdf = m_area_inv;
			if (pdf)
				*pdf = m_area_inv * CosineHemispherePDF(normal, dir);

			return m_intensity;
		}

		float pdf(const Point3 &pos, const Vector3 &dir) const override {
			SurfaceIntersection isect;
			Ray ray = Ray(pos, dir);
			if (m_BVH->intersect(ray, &isect)) {
				mp_prim->postIntersect(ray, &isect);
				const float dist = isect.dist;
				return (dist * dist) / Abs(isect.gn.dot(-dir)) * m_area_inv;
			}
			else
				return 0.f;
		}

		bool isAreaLight() const override {
			return true;
		}
		bool isDelta() const override {
			return false;
		}
		bool isFinite() const override {
			return true;
		}

		Primitive* getPrimitive() {
			return mp_prim;
		}

	private:
		inline void sampleTriangle(const uint32_t tri_id,
			const float b0,
			const float b1,
			Point3 &pos,
			Normal3 &normal) const {
			auto mesh = mp_prim->getMesh();
			const Point3 &p0 = mesh->getPositionAt(3 * tri_id + 0);
			const Point3 &p1 = mesh->getPositionAt(3 * tri_id + 1);
			const Point3 &p2 = mesh->getPositionAt(3 * tri_id + 2);
			pos = b0 * p0 + b1 * p1 + (1.f - b0 - b1) * p2;

			const Normal3 &n0 = mesh->getNormalAt(3 * tri_id + 0);
			const Normal3 &n1 = mesh->getNormalAt(3 * tri_id + 1);
			const Normal3 &n2 = mesh->getNormalAt(3 * tri_id + 2);
			normal = b0 * n0 + b1 * n1 + (1.f - b0 - b1) * n2;
		}
		inline float triangleArea(const uint32_t tri_id) const {
			auto mesh = mp_prim->getMesh();
			const Point3 &p0 = mesh->getPositionAt(3 * tri_id + 0);
			const Point3 &p1 = mesh->getPositionAt(3 * tri_id + 1);
			const Point3 &p2 = mesh->getPositionAt(3 * tri_id + 2);
			return .5f * (p1 - p0).cross(p2 - p0).length();
		}
	};
}

#endif