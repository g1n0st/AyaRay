#ifndef AYA_BSDFS_DISNEY_H
#define AYA_BSDFS_DISNEY_H

#include "../Core/BSDF.h"

namespace Aya {
	class Disney : public BSDF {
	private:
		UniquePtr<Texture2D<float>> m_roughness;
		float m_specular;
		float m_metallic;
		float m_specular_tint;
		float m_sheen;
		float m_sheen_tint;
		float m_subsurface;
		float m_clear_coat;
		float m_clear_coat_gloss;

	public:
		Disney(const Spectrum &reflectance	= Spectrum(),
			float roughness					= 0.1f,
			float specular					= 0.5f,
			float metallic					= 0.0f,
			float specular_tint				= 0.0f,
			float sheen						= 0.0f,
			float sheen_tint					= 0.5f,
			float subsurface					= 0.0f,
			float clear_coat					= 0.0f,
			float clear_coat_gloss			= 0.0f
		) : BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE | BSDF_GLOSSY), BSDFType::Disney, reflectance),
			m_roughness(new ConstantTexture2D<float>(roughness)),
			m_specular(specular),
			m_metallic(metallic),
			m_specular_tint(specular_tint),
			m_sheen(sheen),
			m_sheen_tint(sheen_tint),
			m_subsurface(subsurface),
			m_clear_coat(clear_coat),
			m_clear_coat_gloss(clear_coat_gloss) {}

		Disney(UniquePtr<Texture2D<Spectrum>> texture,
			UniquePtr<Texture2D<Spectrum>> normal,
			float roughness = 0.1f,
			float specular = 0.5f,
			float metallic = 0.0f,
			float specular_tint = 0.0f,
			float sheen = 0.0f,
			float sheen_tint = 0.5f,
			float subsurface = 0.0f,
			float clear_coat = 0.0f,
			float clear_coat_gloss = 0.0f
		) : BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE | BSDF_GLOSSY), BSDFType::Disney, std::move(texture), std::move(normal)),
			m_roughness(new ConstantTexture2D<float>(roughness)),
			m_specular(specular),
			m_metallic(metallic),
			m_specular_tint(specular_tint),
			m_sheen(sheen),
			m_sheen_tint(sheen_tint),
			m_subsurface(subsurface),
			m_clear_coat(clear_coat),
			m_clear_coat_gloss(clear_coat_gloss) {}
		Disney(const char *file_tex,
			float roughness = 0.1f,
			float specular = 0.5f,
			float metallic = 0.0f,
			float specular_tint = 0.0f,
			float sheen = 0.0f,
			float sheen_tint = 0.5f,
			float subsurface = 0.0f,
			float clear_coat = 0.0f,
			float clear_coat_gloss = 0.0f
		) : BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE | BSDF_GLOSSY), BSDFType::Disney, file_tex),
			m_roughness(new ConstantTexture2D<float>(roughness)),
			m_specular(specular),
			m_metallic(metallic),
			m_specular_tint(specular_tint),
			m_sheen(sheen),
			m_sheen_tint(sheen_tint),
			m_subsurface(subsurface),
			m_clear_coat(clear_coat),
			m_clear_coat_gloss(clear_coat_gloss) {}
		Disney(const char *file_tex,
			const char *file_normal,
			float roughness = 0.1f,
			float specular = 0.5f,
			float metallic = 0.0f,
			float specular_tint = 0.0f,
			float sheen = 0.0f,
			float sheen_tint = 0.5f,
			float subsurface = 0.0f,
			float clear_coat = 0.0f,
			float clear_coat_gloss = 0.0f
		) : BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE | BSDF_GLOSSY), BSDFType::Disney, file_tex, file_normal),
			m_roughness(new ConstantTexture2D<float>(roughness)),
			m_specular(specular),
			m_metallic(metallic),
			m_specular_tint(specular_tint),
			m_sheen(sheen),
			m_sheen_tint(sheen_tint),
			m_subsurface(subsurface),
			m_clear_coat(clear_coat),
			m_clear_coat_gloss(clear_coat_gloss) {}

		Spectrum sample_f(const Vector3 &v_out, const Sample &sample,
			const SurfaceIntersection &intersection, Vector3 *v_in, float *pdf, ScatterType types = BSDF_ALL, ScatterType *sample_types = nullptr) const override;

	private:
		virtual float pdfInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const override;
	
		float Fresnel_Schlick(const float cosd, const float reflectance) const {
			float r = 1.f - cosd;
			float sqr_r = r * r;
			float fresnel = reflectance + (1.f - reflectance) * sqr_r * sqr_r * r;
			float fresnel_conductor = fresnelConductor(cosd, .4f, 1.6f);

			return Lerp(m_metallic, fresnel, fresnel_conductor);
		}
		float Fresnel_Schlick_Coat(const float cosd) const {
			float r = 1.f - cosd;
			float sqr_r = r * r;
			float fresnel = .04f + (1.f - .04f) * sqr_r * sqr_r * r;
			
			return fresnel;
		}
	};
}

#endif