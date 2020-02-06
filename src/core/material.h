#ifndef AYA_MATERIAL_H
#define AYA_MATERIAL_H

#include "ray.h"
#include "config.h"
#include "texture.h"
#include "interaction.h"

namespace Aya {
	class Material {
	public:
#if defined(AYA_USE_SIMD)
		inline void  *operator new(size_t i) {
			return _mm_malloc(i, 16);
		}

		inline void operator delete(void *p) {
			_mm_free(p);
		}
#endif

		virtual bool scatter(const Ray &r_in, const SurfaceInteraction &si, Spectrum &attenuation, Ray &scattered) const = 0;
		virtual Spectrum emitted(float u, float v, const Vector3 &p) const {
			return Spectrum();
		}
	};

	class DiffuseLight : public Material {
	public:
		Texture *m_light;

	public:
		DiffuseLight(Texture *light) : m_light(light) {}
		virtual bool scatter(const Ray &r_in, const SurfaceInteraction &si, Spectrum &attenuation, Ray &scattered) const {
			return false;
		}
		virtual Spectrum emitted(float u, float v, const Vector3 &p) const {
			return m_light->value(u, v, p);
		}
	};

	class IsotropicMaterial : public Material {
	public:
		Texture *m_albedo;

	private:
		mutable RNG rng;

	public:
		IsotropicMaterial(Texture *albedo) : m_albedo(albedo) {}
		virtual bool scatter(const Ray &r_in, const SurfaceInteraction &si, Spectrum &attenuation, Ray &scattered) const {
			scattered = Ray(si.p, (Vector3)rng.randomInUnitSphere(), r_in.m_time);
			attenuation = m_albedo->value(si.u, si.v, si.p);
			return true;
		}
	};

	class LambertianMaterial : public Material {
	public:
		Texture *m_albedo;

	private:
		mutable RNG rng;

	public:
		LambertianMaterial(Texture *albedo) : m_albedo(albedo) {}

		virtual bool scatter(const Ray &r_in, const SurfaceInteraction &si, Spectrum &attenuation, Ray &scattered) const {
			Vector3 target = si.p + si.n + rng.randomInUnitSphere();
			scattered = Ray(si.p, target - si.p, r_in.m_time);
			attenuation = m_albedo->value(si.u, si.v, si.p);
			return true;
		}
	};

	class MentalMaterial : public Material {
	public:
		Spectrum m_albedo;
		float m_fuzz;

	private:
		mutable RNG rng;

	public:
		MentalMaterial(const Spectrum &albedo, float fuzz) : m_albedo(albedo), m_fuzz(Min(fuzz, 1.f)) {}

		virtual bool scatter(const Ray &r_in, const SurfaceInteraction &si, Spectrum &attenuation, Ray &scattered) const {
			Vector3 nd = r_in.m_dir;
			nd.normalize();

			Vector3 reflected = reflect(nd, si.n);
			scattered = Ray(si.p, reflected + rng.randomInUnitSphere() * m_fuzz, r_in.m_time);
			attenuation = m_albedo;
			return scattered.m_dir.dot(si.n) > 0;
		}

	private:
		inline Vector3 reflect(const Vector3 &v, const Normal3 &n) const {
			return v - v.dot(n) * n * 2.f;
		}
	};

	class DielectricMaterial : public Material {
	public:
		float m_refractive_idx;

	private:
		mutable RNG rng;

	public:
		DielectricMaterial() { m_refractive_idx = 0; }
		DielectricMaterial(const float &re_idx) : m_refractive_idx(re_idx) {}

		virtual bool scatter(const Ray &r_in, const SurfaceInteraction &si, Spectrum &attenuation, Ray &scattered) const {
			Normal3 outward_normal;
			Vector3 reflected = reflect(r_in.m_dir, si.n);

			float iot;
			// The value represents the energy ratio of refraction and reflection with probability
			float reflect_prob;
			float cosine;
			Vector3 refracted;

			float rgb[3] = { 1.f, 1.f, 1.f };
			attenuation = Spectrum::fromRGB(rgb);

			// in to out
			if (r_in.m_dir.dot(si.n) > 0) {
				outward_normal = -si.n;
				iot = m_refractive_idx;
				cosine = m_refractive_idx * r_in.m_dir.dot(si.n) / r_in.m_dir.length();
			}
			// out to in
			else {
				outward_normal = si.n;
				iot = 1.f / m_refractive_idx;
				cosine = -r_in.m_dir.dot(si.n) / r_in.m_dir.length();
			}

			if (refract(r_in.m_dir, outward_normal, iot, refracted)) {
				reflect_prob = schlick(cosine, m_refractive_idx);
			}
			else {
				scattered = Ray(si.p, reflected, r_in.m_time);
				reflect_prob = 1.f;
			}

			if (rng.drand48() < reflect_prob) {
				scattered = Ray(si.p, reflected, r_in.m_time);
			}
			else {
				scattered = Ray(si.p, refracted, r_in.m_time);
			}

			return true;
		}

	private:
		inline bool refract(const Vector3 &v, const Normal3 &n, float iot, Vector3& refracted) const {
			Vector3 uv = v;
			uv.normalize();

			float dt = uv.dot(n);
			float delta = 1.f - iot * iot *(1 - dt * dt);
			if (delta > 0) {
				refracted = iot * (uv - n * dt) - n * Sqrt(delta);
				return true;
			}

			return false;
		}
		inline Vector3 reflect(const Vector3 &v, const Normal3 &n) const {
			return v - v.dot(n) * n * 2.f;
		}

		// Schlick approximation of Fresnel reflectance
		inline float schlick(const float &cosine, const float &idx) const {
			float r0 = (1 - idx) / (1.f + idx);
			r0 = r0 * r0;
			return r0 + (1 - r0) * pow((1 - cosine), 5);
		}
	};
}

#endif