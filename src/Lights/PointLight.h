#ifndef AYA_LIGHTS_POINTLIGHT_H
#define AYA_LIGHTS_POINTLIGHT_H

#include "../Core/Light.h"
#include "../Core/Intersection.h"
#include "../Core/Sampler.h"
#include "../Core/Sampling.h"

namespace Aya {
	class PointLight : public Light {
	private:
		Point3 m_pos;
		Spectrum m_intensity;

	public:
		PointLight(const Point3 &pos,
			const Spectrum &intens,
			const uint32_t sample_count = 1) :
			Light(sample_count), m_pos(pos), m_intensity(intens) {
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
			tester->setMedium(scatter.m_mediumInterface.getMedium(*dir, scatter.n));

			if (cos_at_light)
				*cos_at_light = 1.f;
			if (emit_pdf_w)
				*emit_pdf_w = UniformSpherePDF();

			return m_intensity;
		}

		Spectrum sample(const Sample &light_sample0,
			const Sample &light_sample1,
			Ray *ray,
			Normal3 *normal,
			float *pdf,
			float *direct_pdf = nullptr) const override {
			Vector3 dir = UniformSampleSphere(light_sample0.u, light_sample0.v);
			*ray = Ray(m_pos, dir);
			*normal = dir;
			*pdf = UniformSpherePDF();
			if (direct_pdf)
				*direct_pdf = 1.f;

			return m_intensity;
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
	};
}

#endif