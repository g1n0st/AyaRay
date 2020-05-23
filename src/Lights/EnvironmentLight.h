#ifndef AYA_LIGHTS_ENVIRONMENTLIGHT_H
#define AYA_LIGHTS_ENVIRONMENTLIGHT_H

#include "../Core/Light.h"
#include "../Core/Intersection.h"
#include "../Core/Primitive.h"
#include "../Core/Sampler.h"
#include "../Core/Sampling.h"
#include "../Core/Scene.h"

namespace Aya {
	class EnvironmentLight : public Light {
	private:
		UniquePtr<Texture2D<Spectrum>> mp_map;
		UniquePtr<Distribution2D> mp_distribution;
		BlockedArray<float> m_luminance;
		const Scene *mp_scene;
		bool is_texture;

		mutable float m_scale;
		mutable float m_rotation;

	public:
		EnvironmentLight(const Spectrum &intens,
			const Scene* scene,
			const uint32_t sample_count = 1) :
			Light(sample_count) {
			mp_scene = scene;
			is_texture = false;
			m_scale = 1.f;
			mp_map = MakeUnique<ConstantTexture2D<Spectrum>>(intens);
		}
		EnvironmentLight(const char *path,
			const Scene *scene,
			const float scale = 1.f,
			const float rotate = 0.f,
			const uint32_t sample_count = 1) :
			Light(sample_count) {
			mp_scene = scene;
			is_texture = true;
			m_scale = scale;
			m_rotation = Radian(rotate);
			mp_map = MakeUnique<ImageTexture2D<Spectrum, Spectrum>>(path, 1.f);
			calcLuminanceDistribution();
		}

		Spectrum illuminate(const Scatter &scatter,
			const Sample &light_sample,
			Vector3 *dir,
			VisibilityTester *tester,
			float *pdf,
			float *cos_at_light = nullptr,
			float *emit_pdf_w = nullptr) const override {
			const Point3 &pos = scatter.p;
			float u, v;
			if (is_texture) {
				mp_distribution->sampleContinuous(light_sample.u, light_sample.v, &u, &v, pdf);
				if (*pdf == 0.f)
					return Spectrum(0.f);

				float phi = u * float(M_PI) * 2.f;
				phi = applyRotation(phi, -1.f);
				float theta = v * float(M_PI);
				float sin_theta = std::sinf(theta);
				*pdf = sin_theta != 0.f ? *pdf * float(M_1_PI) * float(M_1_PI) * .5f / sin_theta : 0.f;
				*dir = BaseVector3::sphericalDirection(sin_theta, std::cosf(theta), phi);
			}
			else {
				u = light_sample.u;
				v = light_sample.v;
				Vector3 d = UniformSampleSphere(u, v);
				*dir = Vector3(d.x, d.z, -d.y);
				*pdf = UniformSpherePDF();
			}

			if (cos_at_light)
				*cos_at_light = 1.f;

			Point3 center;
			float radius;
			mp_scene->worldBound().boundingSphere(&center, &radius);

			if (emit_pdf_w)
				*emit_pdf_w = *pdf * ConcentricDiscPdf() / (radius * radius);

			tester->setRay(pos, *dir, 2.f * radius);
			tester->setMedium(scatter.m_mediumInterface.getMedium(*dir, scatter.n));

			Vector2f diff[2] = {Vector2f(), Vector2f()};
			return mp_map->sample(Vector2f(u, v), diff, TextureFilter::Linear) * m_scale;
		}

		Spectrum sample(const Sample &light_sample0,
			const Sample &light_sample1,
			Ray *ray,
			Normal3 *normal,
			float *pdf,
			float *direct_pdf = nullptr) const override {
			float u, v, map_pdf, sin_theta;

			if (is_texture) {
				mp_distribution->sampleContinuous(light_sample0.u, light_sample0.v, &u, &v, &map_pdf);
				if (map_pdf == 0.f) {
					*pdf = 0.f;
					if (direct_pdf)
						*direct_pdf = 0.f;

					return Spectrum(0.f);
				}

				float phi = u * float(M_PI) * 2.f;
				phi = applyRotation(phi, -1.f);
				float theta = v * float(M_PI);
				sin_theta = std::sinf(theta);
				*normal = -BaseVector3::sphericalDirection(sin_theta, std::cosf(theta), phi);
			}
			else {
				u = light_sample0.u;
				v = light_sample0.v;
				*normal = -UniformSampleSphere(u, v);
				map_pdf = UniformSpherePDF();

				float theta = v * float(M_PI);
				sin_theta = std::sinf(theta);
			}

			Point3 center;
			float radius;
			mp_scene->worldBound().boundingSphere(&center, &radius);

			Vector3 v1, v2;
			BaseVector3::coordinateSystem(-*normal, &v1, &v2);
			float f1, f2;
			ConcentricSampleDisk(light_sample1.u, light_sample1.v, &f1, &f2);

			Point3 origin = center + radius * (f1 * v1 + f2 * v2);
			*ray = Ray(origin + radius * -*normal, *normal);

			float pdfW = sin_theta != 0.f ? map_pdf * float(M_1_PI) * float(M_1_PI) * .5f / sin_theta : 0.f;
			float pdfA = ConcentricDiscPdf() / (radius * radius);
			*pdf = pdfW * pdfA;
			if (direct_pdf)
				*direct_pdf = pdfW;

			Vector2f diff[2] = { Vector2f(), Vector2f() };
			return mp_map->sample(Vector2f(u, v), diff, TextureFilter::Linear) * m_scale;
		}

		Spectrum emit(const Vector3 &dir,
			const Normal3 &normal = Normal3(0.f, 0.f, 0.f),
			float *pdf = nullptr,
			float *direct_pdf = nullptr) const override {
			Vector3 neg_dir = (-dir).normalize();
			float phi = BaseVector3::sphericalPhi(neg_dir);
			float s = phi * float(M_1_PI) * .5f;
			float theta = BaseVector3::sphericalTheta(neg_dir);
			float t = theta * float(M_1_PI);

			if (direct_pdf || pdf) {
				float pdfW = 0.f;
				if (is_texture) {
					float map_pdf = mp_distribution->pdf(s, t);
					float sin_theta = std::sinf(theta);
					float pdfW = sin_theta != 0.f ? map_pdf * float(M_1_PI) * float(M_1_PI) * .5f / sin_theta : 0.f;
				}
				else {
					float pdfW = UniformSpherePDF();
				}

				if (direct_pdf)
					*direct_pdf = pdfW;
				if (pdf) {
					Point3 center;
					float radius;
					mp_scene->worldBound().boundingSphere(&center, &radius);
					float pdfA = ConcentricDiscPdf() / (radius * radius);
					*pdf = pdfW * pdfA;
				}
			}

			Vector2f diff[2] = { Vector2f(), Vector2f() };
			return mp_map->sample(Vector2f(s, t), diff, TextureFilter::Linear) * m_scale;
		}
		float pdf(const Point3 &pos, const Vector3 &dir) const override {
			if (is_texture) {
				Vector3 normalized_dir = dir.normalize();
				float theta = BaseVector3::sphericalTheta(normalized_dir);
				float phi = BaseVector3::sphericalPhi(normalized_dir);
				phi = applyRotation(phi);
				float sin_theta = std::sinf(theta);
				return mp_distribution->pdf(phi * float(M_1_PI) * .5f, theta * float(M_1_PI)) /
					(2.f * float(M_PI) * float(M_PI) * sin_theta);
			}
			else
				return UniformSpherePDF();
		}

		bool isEnvironmentLight() const override {
			return true;
		}
		bool isDelta() const override {
			return false;
		}
		bool isFinite() const override {
			return false;
		}

		Texture2D<Spectrum>* getTexture() const {
			return mp_map.get();
		}
		bool isTexture() const {
			return is_texture;
		}

		float getRotation() const {
			return m_rotation;
		}
		void setRotation(const float rot) const {
			m_rotation = rot;
		}
		float getScaling() const {
			return m_scale;
		}
		void setScaling(const float scl) const {
			m_scale = scl;
		}

	private:
		void calcLuminanceDistribution() {
			auto width = mp_map->width();
			auto height = mp_map->height();
			m_luminance.init(height, width);
			for (auto y = 0; y < height; ++y) {
				float v = (y + .5f) / float(height);
				float sin_theta = std::sinf(float(M_PI) * v);
				for (auto x = 0; x < width; ++x) {
					float u = (x + .5f) / float(width);
					Vector2f diff[2] = {
						Vector2f(),
						Vector2f()
					};
					m_luminance(y, x) = 
						mp_map->sample(Vector2f(u, v), diff, TextureFilter::Linear).luminance() * sin_theta;
				}
			}

			mp_distribution = MakeUnique<Distribution2D>(m_luminance.data(), width, height);
		}

		inline float applyRotation(const float phi, const float scl = 1.f) const {
			float ret = phi;
			ret += scl * m_rotation;
			if (ret < 0.f)
				ret += float(M_PI) * 2.f;
			else if (ret > float(M_PI) * 2.f)
				ret -= float(M_PI) * 2.f;

			return ret;
		}
	};
}

#endif