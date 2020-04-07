#ifndef AYA_CORE_BSDF_H
#define AYA_CORE_BSDF_H

#include "../Core/Ray.h"
#include "../Core/Intersection.h"
#include "../Core/Spectrum.h"
#include "../Core/Sampler.h"
#include "../Core/Sampling.h"
#include "../Core/Texture.h"
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"

namespace Aya {
	AYA_FORCE_INLINE float CosTheta(const Vector3 &v) {
		return v.z();
	}
	AYA_FORCE_INLINE float CosTheta2(const Vector3 &v) {
		return v.z() * v.z();
	}
	AYA_FORCE_INLINE float AbsCosTheta(const Vector3 &v) {
		return Abs(v.z());
	}
	AYA_FORCE_INLINE float SinTheta2(const Vector3 &v) {
		return Max(0.f, 1.f - CosTheta2(v));
	}
	AYA_FORCE_INLINE float SinTheta(const Vector3 &v) {
		return Sqrt(SinTheta2(v));
	}
	AYA_FORCE_INLINE float CosPhi(const Vector3 &v) {
		float sint = SinTheta(v);
		if (sint == 0.f)
			return 1.f;
		return Clamp(v.x() / sint, -1.f, 1.f);
	}
	AYA_FORCE_INLINE float SinPhi(const Vector3 &v) {
		float sint = SinTheta(v);
		if (sint == 0.f)
			return 0.f;
		return Clamp(v.y() / sint, -1.f, 1.f);
	}
	AYA_FORCE_INLINE float TanTheta(const Vector3 &v) {
		float tmp = 1.f - v.z() * v.z();
		if (tmp <= 0.f)
			return 0.f;
		return Sqrt(tmp) / v.z();
	}
	AYA_FORCE_INLINE float TanTheta2(const Vector3 &v) {
		float tmp = 1.f - v.z() * v.z();
		if (tmp <= 0.f)
			return 0.f;
		return tmp / (v.z() * v.z());
	}
	AYA_FORCE_INLINE bool sameHemisphere(const Vector3 &v1, const Vector3 &v2) {
		return v1.z() * v2.z() > 0.f;
	}

	enum ScatterType {
		BSDF_REFLECTION =	1 << 0,
		BSDF_TRANSMISSION = 1 << 1,
		BSDF_DIFFUSE =		1 << 2,
		BSDF_GLOSSY =		1 << 3,
		BSDF_SPECULAR =		1 << 4,
		BSDF_ALL_TYPES = 
			BSDF_DIFFUSE		| 
			BSDF_GLOSSY		| 
			BSDF_SPECULAR,
		BSDF_ALL_REFLECTION = 
			BSDF_REFLECTION | 
			BSDF_ALL_TYPES,
		BSDF_ALL_TRANSMISSION = 
			BSDF_TRANSMISSION | 
			BSDF_ALL_TYPES,
		BSDF_ALL = 
			BSDF_ALL_REFLECTION | 
			BSDF_ALL_TRANSMISSION
	};

	enum class BSDFType {
		LambertianDiffuse,
		Mirror,
		Glass,
		RoughConductor,
		RoughDielectric,
		Disney
	};

	class BSDF {
	protected:
		const ScatterType m_scatter_type;
		const BSDFType m_BSDF_type;
		UniquePtr<Texture2D<Spectrum>> mp_texture;
		UniquePtr<Texture2D<RGBSpectrum>> mp_normal_map;

	public:
		BSDF(ScatterType t1, BSDFType t2, const Spectrum &color);
		BSDF(ScatterType t1, BSDFType t2, UniquePtr<Texture2D<Spectrum>> tex, UniquePtr<Texture2D<RGBSpectrum>> normal);
		BSDF(ScatterType t1, BSDFType t2, const char *texture_file);
		BSDF(ScatterType t1, BSDFType t2, const char *texture_file, const char *normal_file);
		virtual ~BSDF() {}

		bool matchesTypes(ScatterType flags) const {
			return (m_scatter_type & flags) == m_scatter_type;
		}
		bool isSpecular() const {
			return (ScatterType(BSDF_SPECULAR | BSDF_DIFFUSE | BSDF_GLOSSY) & m_scatter_type) == ScatterType(BSDF_SPECULAR);
		}
		void setNormalMap(const char *normal_file) {
			mp_normal_map = MakeUnique<ImageTexture2D<RGBSpectrum, byteSpectrum>>(normal_file, 1.f);
		}
		void setTexture(const char *image_file) {
			mp_texture = MakeUnique<ImageTexture2D<Spectrum, byteSpectrum>>(image_file);
		}
		void setTexture(const Spectrum &color) {
			mp_texture = MakeUnique<ConstantTexture2D<Spectrum>>(color);
		}

		virtual Spectrum f(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const;
		virtual float pdf(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const;
		virtual Spectrum sample_f(const Vector3 &v_out, const Sample &sample,
			const SurfaceIntersection &intersection, Vector3 *v_in, float* pdf, ScatterType types = BSDF_ALL, ScatterType *sample_types = nullptr) const = 0;

		template<typename T>
		inline const T getValue(const Texture2D<T> *tex,
			const SurfaceIntersection &intersection,
			const TextureFilter filter = TextureFilter::Linear) const {
			Vector2f diffs[2] = {
				Vector2f(intersection.dudx, intersection.dvdx),
				Vector2f(intersection.dudy, intersection.dvdy)
			};
			return tex->sample(intersection.uv, diffs, filter);
		}

		const ScatterType getScatterType() const {
			return m_scatter_type;
		}
		const BSDFType getBSDFType() const {
			return m_BSDF_type;
		}
		const Texture2D<Spectrum>* getTexture() const {
			return mp_texture.get();
		}
		const Texture2D<RGBSpectrum>* getNormalMap() const {
			return mp_normal_map.get();
		}

		UniquePtr<Texture2D<Spectrum>> moveTexture() {
			return std::move(mp_texture);
		}
		UniquePtr<Texture2D<RGBSpectrum>> moveNormalMap() {
			return std::move(mp_normal_map);
		}

	protected:
		virtual float evalInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const = 0;
		virtual float pdfInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const = 0;

	public:
		// https://en.wikipedia.org/wiki/Schlick%27s_approximation
		static float fresnelDielectric(float cosi, float etai, float etat);
		static float fresnelConductor(float cosi, const float &eta, const float &k);

	protected:
		// for Cook Torrance Model
		// https://zhuanlan.zhihu.com/p/20091064
		// https://zhuanlan.zhihu.com/p/20119162

		// http://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
		static float GGX_D(const Vector3 &wh, float alpha);
		static Vector3 GGX_SampleNormal(float u1, float u2, float* pdf, float alpha);
		static float SmithG(const Vector3 &v, const Vector3 &wh, float alpha);
		static float	 GGX_G(const Vector3 &wo, const Vector3 &wi, const Vector3 &wh, float alpha);
		static float GGX_Pdf(const Vector3 &wh, float alpha);

		static Vector2f	ImportanceSampleGGX_VisibleNormal_Unit(float theta, float u1, float u2);
		static Vector3 GGX_SampleVisibleNormal(const Vector3 &wi, float u1, float u2, float* pdf, float roughness);
		static float GGX_Pdf_VisibleNormal(const Vector3 &wi, const Vector3 &h, float roughness);
 	};
}
#endif