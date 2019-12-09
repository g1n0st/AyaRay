#ifndef AYA_MATERIAL_H
#define AYA_MATERIAL_H

#include "ray.h"
#include "config.h"
#include "texture.h"
#include "interaction.h"

/*
bool refract(const Vector3 &v, const Vector3 &n, float iot, Vector3& refracted) {
	Vector3 uv = v;
	uv.normalize();

	float dt = uv.dot(n);
	float delta = 1.f - iot * iot *(1 - dt * dt);
	if (delta > 0) {
		refracted = iot * (uv - n * dt) - n * Sqrt(delta);
		return true;
	}

	return false;
}*/

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

	virtual bool scatter(const Ray &r_in, const SurfaceInteraction &si, Vector3 &attenuation, Ray &scattered) const = 0;
	virtual Vector3 emitted(float u, float v, const Vector3 &p) const {
		return Vector3(0.f, 0.f, 0.f);
	}
};

class DiffuseLight : public Material {
public:
	Texture *m_light;

public:
	DiffuseLight(Texture *light) : m_light(light) {}
	virtual bool scatter(const Ray &r_in, const SurfaceInteraction &si, Vector3 &attenuation, Ray &scattered) const {
		return false;
	}
	virtual Vector3 emitted(float u, float v, const Vector3 &p) const {
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
	virtual bool scatter(const Ray &r_in, const SurfaceInteraction &si, Vector3 &attenuation, Ray &scattered) const {
		scattered = Ray(si.p, (Vector3)rng.randomUnitDisk(), r_in.m_time);
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

	virtual bool scatter(const Ray &r_in, const SurfaceInteraction &si, Vector3 &attenuation, Ray &scattered) const {
		Vector3 target = si.p + si.n + rng.randomUnitDisk();
		scattered = Ray(si.p, target - si.p, r_in.m_time);
		attenuation = m_albedo->value(si.u, si.v, si.p);
		return true;
	}
};

class MentalMaterial : public Material {
public:
	Vector3 m_albedo;
	float m_fuzz;

private:
	mutable RNG rng;

public:
	MentalMaterial(const Vector3 &albedo, float fuzz) : m_albedo(albedo), m_fuzz(Min(fuzz, 1.f)) {}

	virtual bool scatter(const Ray &r_in, const SurfaceInteraction &si, Vector3 &attenuation, Ray &scattered) const {
		Vector3 nd = r_in.m_dir;
		nd.normalize();
		
		Vector3 reflected = reflect(nd, si.n);
		scattered = Ray(si.p, reflected + rng.randomUnitDisk() * m_fuzz, r_in.m_time);
		attenuation = m_albedo;
		return scattered.m_dir.dot(si.n) > 0;
	}

private:
	Vector3 reflect(const Vector3 &v, const Normal3 &n) const {
		return v - v.dot(n) * n * 2;
	}
};

#endif