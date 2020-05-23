#ifndef AYA_LIGHTS_DIRECTIONALLIGHT_H
#define AYA_LIGHTS_DIRECTIONALLIGHT_H

#include <Core/Light.h>
#include <Core/Sampling.h>
#include <Core/Scene.h>

namespace Aya {
	class DirectionalLight : public Light {
	private:
		Spectrum m_intensity;
		Vector3 m_dir;
		Frame m_dirFrame;
		float m_coneCos;
		const Scene *mp_scene;

	public:
		DirectionalLight(const Vector3 &dir,
			const Spectrum &intens,
			const Scene *scene,
			const float cone_deg = 0.f,
			const uint32_t sample_count = 1) :
			Light(sample_count), m_intensity(intens),
			m_dir(-dir.normalize()), m_dirFrame(-dir.normalize()),
			mp_scene(scene), m_coneCos(std::cos(Radian(cone_deg))) {
				}

		Spectrum illuminate(const Scatter &scatter,
			const Sample &light_sample,
			Vector3 *dir,
			VisibilityTester *tester,
			float *pdf,
			float *cos_at_light = nullptr,
			float *emit_pdf_w = nullptr) const override {
			const Point3 &pos = scatter.p;
			*dir = UniformSampleCone(light_sample.u, light_sample.v,
				m_coneCos,
				m_dirFrame.U(),
				m_dirFrame.V(),
				m_dirFrame.W());
			*pdf = UniformConePDF(m_coneCos);

			Point3 center;
			float radius;
			mp_scene->worldBound().boundingSphere(&center, &radius);
			tester->setSegment(pos, pos + 2.f * radius * (*dir));
			tester->setMedium(scatter.m_mediumInterface.getMedium(*dir, scatter.n));

			if (cos_at_light)
				*cos_at_light = dir->dot(m_dir);
			if (emit_pdf_w)
				*emit_pdf_w = ConcentricDiscPdf() / (radius * radius) * (*pdf);

			return m_intensity;
		}

		Spectrum sample(const Sample &light_sample0,
			const Sample &light_sample1,
			Ray *ray,
			Normal3 *normal,
			float *pdf,
			float *direct_pdf = nullptr) const override {
			float f1, f2;
			ConcentricSampleDisk(light_sample0.u, light_sample0.v, &f1, &f2);

			Point3 center;
			float radius;
			mp_scene->worldBound().boundingSphere(&center, &radius);

			Point3 origin = center + radius * 
				(f1 * m_dirFrame.U() + 
				 f2 * m_dirFrame.V());
			Vector3 dir = -UniformSampleCone(light_sample1.u, light_sample1.v,
				m_coneCos,
				m_dirFrame.U(),
				m_dirFrame.V(),
				m_dirFrame.W());
			*ray = Ray(origin + radius * m_dir, dir);
			*normal = dir;

			float dir_pdf = UniformConePDF(m_coneCos);
			*pdf = ConcentricDiscPdf() / (radius * radius) * dir_pdf;
			if (direct_pdf)
				*direct_pdf = dir_pdf;

			return m_intensity;
		}

		Spectrum emit(const Vector3 &dir,
			const Normal3 &normal = Normal3(0.f, 0.f, 0.f),
			float *pdf = nullptr,
			float *direct_pdf = nullptr) const override {
			if (dir.dot(-m_dir) > m_coneCos) {
				float dir_pdf = UniformConePDF(m_coneCos);
				if (pdf) {
					Point3 center;
					float radius;
					mp_scene->worldBound().boundingSphere(&center, &radius);
					*pdf = ConcentricDiscPdf() / (radius * radius) * dir_pdf;
				}
				if (direct_pdf)
					*direct_pdf = dir_pdf;
			}

			return Spectrum(0.f);
		}

		float pdf(const Point3 &pos, const Vector3 &dir) const override {
			if (dir.dot(-m_dir) > m_coneCos)
				return UniformConePDF(m_coneCos);
			return 0.f;
		}

		bool isEnvironmentLight() const override {
			return true;
		}
		bool isDelta() const override {
			return m_coneCos == 1.f;
		}
		bool isFinite() const override {
			return false;
		}
	};
}

#endif