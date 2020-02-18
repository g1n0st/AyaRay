#ifndef AYA_CORE_BSDF_H
#define AYA_CORE_BSDF_H

#include "../Core/Ray.h"
#include "../Core/Intersection.h"
#include "../Core/Spectrum.h"
#include "../Core/Sampler.h"
#include "../Core/Texture.h"
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"

namespace Aya {
	enum ScatterType {
		BSDF_REFLECTION = 1 << 0,
		BSDF_TRANSMISSION = 1 << 1,
		BSDF_DIFFUSE = 1 << 2,
		BSDF_GLOSSY = 1 << 3,
		BSDF_SPECULAR = 1 << 4,
		BSDF_ALL_TYPES = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR,
		BSDF_ALL_REFLECTION = BSDF_REFLECTION | BSDF_ALL_TYPES,
		BSDF_ALL_TRANSMISSION = BSDF_TRANSMISSION | BSDF_ALL_TYPES,
		BSDF_ALL = BSDF_ALL_REFLECTION | BSDF_ALL_TRANSMISSION
	};

	enum class BSDFType {
		Diffuse,
		Mirror,
		RoughConductor,
		RoughDielectric,
		Disney
	};

	class BSDF {
	protected:
		const ScatterType m_scatter_type;
		const BSDFType m_BSDF_type;
		UniquePtr<Texture2D<Spectrum>> m_texture;
		UniquePtr<Texture2D<Spectrum>> m_normal_map;

	public:
		BSDF(ScatterType t1, BSDFType t2, const Spectrum &color);
		BSDF(ScatterType t, BSDFType t2, UniquePtr<Texture2D<Spectrum>> tex, UniquePtr<Texture2D<Spectrum>> normal);
		BSDF(ScatterType t, BSDFType t2, const char* file1, const char* file2);
		virtual ~BSDF() {}

		bool matchesTypes(ScatterType flags) const {
			return (m_scatter_type & flags) == m_scatter_type;
		}
		bool isSpecular() const {
			return (ScatterType(BSDF_SPECULAR | BSDF_DIFFUSE | BSDF_GLOSSY) & m_scatter_type) == ScatterType(BSDF_SPECULAR);
		}

		virtual Spectrum f(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const;
		virtual float pdf(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const;
		virtual Spectrum sample_f(const Vector3 &v_out, const Sample &sample,
			const SurfaceIntersection &intersection, Vector3* v_in, float* pdf, ScatterType types = BSDF_ALL, ScatterType *sample_types = nullptr) const = 0;

		template<typename T>
		__forceinline const T getValue(const Texture2D<T> *tex,
			const SurfaceIntersection &intersection,
			const TextureFilter filter = TextureFilter::TriLinear) const {
			Vector2 diffs[2] = {
				(intersection.dudx, intersection.dvdx),
				(intersection.dudy, intersection.dvdy)
			};
			return tex->sample(intersection.tex_coord, diffs, filter);
		}

		const ScatterType getScatterType() const {
			return m_scatter_type;
		}
		const BSDFType getBSDFType() const {
			return m_BSDF_type;
		}
		const Texture2D<Spectrum>* getTexture() const {
			return m_texture.get();
		}
		const Texture2D<Spectrum>* getNormalMap() const {
			return m_normal_map.get();
		}

		UniquePtr<Texture2D<Spectrum>> moveTexture() {
			return std::move(m_texture);
		}
		UniquePtr<Texture2D<Spectrum>> moveNormalMap() {
			return std::move(m_normal_map);
		}

	protected:
		virtual float evalInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const = 0;
		virtual float pdfInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const = 0;

	public:
		static float fresnelDielectric(float cosi, float etai, float etat);
		static float fresnelConductor(float cosi, const float &eta, const float k);

	protected:
		static float GGX_D(const Vector3 &wh, float alpha);
		static Vector3 GGX_SampleNormal(float u1, float u2, float* pdf, float alpha);
		static float SmithG(const Vector3 &v, const Vector3 &wh, float alpha);
		static float	 GGX_G(const Vector3 &wo, const Vector3 &wi, const Vector3 &wh, float alpha);
		static float GGX_Pdf(const Vector3 &wh, float alpha);

		static Vector2f	ImportanceSampleGGX_VisibleNormal_Unit(float theta, float u1, float u2);
		static Vector3 GGX_SampleVisibleNormal(const Vector3 &wi, float u1, float u2, float* pdf, float roughness);
		static float GGX_Pdf_VisibleNormal(const Vector3 &wi, const Vector3 &h, float roughness);
 	};

	class LambertianDiffuse : public BSDF {
	public:
		LambertianDiffuse(const Spectrum &color = RGBSpectrum().toSpectrum())
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE), BSDFType::Diffuse, color) {}
		LambertianDiffuse(UniquePtr<Texture2D<Spectrum>> tex, UniquePtr<Texture2D<Spectrum>> normal)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE), BSDFType::Diffuse, std::move(tex), std::move(normal)) {}
		LambertianDiffuse(const char* file1, const char* file2)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE), BSDFType::Diffuse, file1, file2) {}

		virtual Spectrum sample_f(const Vector3 &v_out, const Sample &sample,
			const SurfaceIntersection &intersection, Vector3* v_in, float* pdf, ScatterType types = BSDF_ALL, ScatterType *sample_types = nullptr) const override;

	private:
		virtual float evalInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const override;
		virtual float pdfInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const override;
	};

	//class Mirror : public BSDF {
	//public:

	//};
}

#endif