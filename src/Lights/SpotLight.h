#ifndef AYA_LIGHTS_SPOTLIGHT_H
#define AYA_LIGHTS_SPOTLIGHT_H

#include "../Core/Light.h"

namespace Aya {
	class SpotLight : public Light {
	private:
		Point3 m_pos;
		Spectrum m_intensity;
		Frame m_dir_frame;
		float m_cos_total, m_cos_falloff;

	public:
		SpotLight(const Point3 &pos,
			const Spectrum &intens,
			const Vector3 &dir,
			const float total,
			const float falloff,
			const uint32_t sample_count = 1) :
			Light(sample_count), m_pos(pos), m_intensity(intens), m_dir_frame(dir.normalize()),
				m_cos_total(std::cos(Radian(total))), m_cos_falloff(std::cos(Radian(falloff))) {
		}

		Spectrum illuminate(const Scatter &scatter,
			const Sample &light_sample,
			Vector3 *dir,
			VisibilityTester *tester,
			float *pdf,
			float *cos_at_light = nullptr,
			float *emit_pdf_w = nullptr) const override {
			const Point3 &pos = scatter.p;
			*dir = (m_pos - pos).normalize();
			*pdf = (m_pos - pos).length2();
			tester->setSegment(pos, m_pos);
			tester->setMedium(scatter.m_medium_interface.getMedium(*dir, scatter.n));

			if (cos_at_light)
				*cos_at_light = 1.f;
			if (emit_pdf_w)
				*emit_pdf_w = UniformConePDF(m_cos_total);

			return m_intensity * falloff(-*dir);
		}

		Spectrum sample(const Sample &light_sample0,
			const Sample &light_sample1,
			Ray *ray,
			Normal3 *normal,
			float *pdf,
			float *direct_pdf = nullptr) const override {
			Vector3 dir = UniformSampleCone(light_sample0.u, light_sample0.v, 
				m_cos_total,
				m_dir_frame.U(), m_dir_frame.V(), m_dir_frame.W());
			*ray = Ray(m_pos, dir);
			*normal = dir;
			*pdf = UniformConePDF(m_cos_total);
			if (direct_pdf)
				*direct_pdf = 1.f;

			return m_intensity * falloff(ray->m_dir);
		}

		Spectrum emit(const Vector3 &dir,
			const Normal3 &normal = Normal3(0.f, 0.f, 0.f),
			float *pdf = nullptr,
			float *direct_pdf = nullptr) const override {
			return Spectrum(0.f);
		}

		float pdf(const Point3 &pos, const Vector3 &dir) const override {
			return 0.f;
		}

		bool isDelta() const override {
			return true;
		}
		bool isFinite() const override {
			return true;
		}

	private:
		float falloff(const Vector3 &w) const {
			Vector3 wl = m_dir_frame.worldToLocal(w).normalize();
			float cos_theta = CosTheta(wl);
			if (cos_theta < m_cos_total)
				return 0.f;
			if (cos_theta >= m_cos_falloff)
				return 1.f;

			float delta = (cos_theta - m_cos_total) / (m_cos_falloff - m_cos_total);
			return (delta * delta) * (delta * delta);
		}
	};
}

#endif