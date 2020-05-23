#ifndef AYA_BSDFS_DISNEY_H
#define AYA_BSDFS_DISNEY_H

#include <Core/BSDF.h>

namespace Aya {
	class Disney : public BSDF {
	private:
		UniquePtr<Texture2D<float>> mp_roughness;
		UniquePtr<Texture2D<float>> mp_specular;
		float m_metallic;
		float m_specularTint;
		float m_sheen;
		float m_sheenTint;
		float m_subsurface;
		float m_clearCoat;
		float m_clearCoatGloss;

	public:
		Disney(const Spectrum &reflectance,
			UniquePtr<Texture2D<float>> roughness,
			UniquePtr<Texture2D<float>> specular,
			float metallic					= 0.0f,
			float specular_tint				= 0.0f,
			float sheen						= 0.0f,
			float sheen_tint					= 0.5f,
			float subsurface					= 0.0f,
			float clear_coat					= 0.0f,
			float clear_coat_gloss			= 0.0f
		) : BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE | BSDF_GLOSSY), BSDFType::Disney, reflectance),
			mp_roughness(std::move(roughness)),
			mp_specular(std::move(specular)),
			m_metallic(metallic),
			m_specularTint(specular_tint),
			m_sheen(sheen),
			m_sheenTint(sheen_tint),
			m_subsurface(subsurface),
			m_clearCoat(clear_coat),
			m_clearCoatGloss(clear_coat_gloss) {}

		Disney(UniquePtr<Texture2D<Spectrum>> texture,
			UniquePtr<Texture2D<RGBSpectrum>> normal,
			UniquePtr<Texture2D<float>> roughness,
			UniquePtr<Texture2D<float>> specular,
			float metallic					= 0.0f,
			float specular_tint				= 0.0f,
			float sheen						= 0.0f,
			float sheen_tint					= 0.5f,
			float subsurface					= 0.0f,
			float clear_coat					= 0.0f,
			float clear_coat_gloss			= 0.0f
		) : BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE | BSDF_GLOSSY), BSDFType::Disney, std::move(texture), std::move(normal)),
			mp_roughness(std::move(roughness)),
			mp_specular(std::move(specular)),
			m_metallic(metallic),
			m_specularTint(specular_tint),
			m_sheen(sheen),
			m_sheenTint(sheen_tint),
			m_subsurface(subsurface),
			m_clearCoat(clear_coat),
			m_clearCoatGloss(clear_coat_gloss) {}
		Disney(const char *file_tex,
			UniquePtr<Texture2D<float>> roughness,
			UniquePtr<Texture2D<float>> specular,
			float metallic					= 0.0f,
			float specular_tint				= 0.0f,
			float sheen						= 0.0f,
			float sheen_tint					= 0.5f,
			float subsurface					= 0.0f,
			float clear_coat					= 0.0f,
			float clear_coat_gloss			= 0.0f
		) : BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE | BSDF_GLOSSY), BSDFType::Disney, file_tex),
			mp_roughness(std::move(roughness)),
			mp_specular(std::move(specular)),
			m_metallic(metallic),
			m_specularTint(specular_tint),
			m_sheen(sheen),
			m_sheenTint(sheen_tint),
			m_subsurface(subsurface),
			m_clearCoat(clear_coat),
			m_clearCoatGloss(clear_coat_gloss) {}
		Disney(const char *file_tex,
			const char *file_normal,
			UniquePtr<Texture2D<float>> roughness,
			UniquePtr<Texture2D<float>> specular,
			float metallic					= 0.0f,
			float specular_tint				= 0.0f,
			float sheen						= 0.0f,
			float sheen_tint					= 0.5f,
			float subsurface					= 0.0f,
			float clear_coat					= 0.0f,
			float clear_coat_gloss			= 0.0f
		) : BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE | BSDF_GLOSSY), BSDFType::Disney, file_tex, file_normal),
			mp_roughness(std::move(roughness)),
			mp_specular(std::move(specular)),
			m_metallic(metallic),
			m_specularTint(specular_tint),
			m_sheen(sheen),
			m_sheenTint(sheen_tint),
			m_subsurface(subsurface),
			m_clearCoat(clear_coat),
			m_clearCoatGloss(clear_coat_gloss) {}

		Spectrum sample_f(const Vector3 &v_out, const Sample &sample,
			const SurfaceIntersection &intersection, Vector3 *v_in, float *pdf, ScatterType types = BSDF_ALL, ScatterType *sample_types = nullptr) const override;
		Spectrum f(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const override;

		Spectrum evaluate(const Vector3 &l_out, const Vector3 &l_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const;

		void setRoughness(const char *path) {
			mp_roughness = MakeUnique<ImageTexture2D<float, float>>(path);
		}
		void setRoughness(const float roughness) {
			mp_roughness = MakeUnique<ConstantTexture2D<float>>(roughness);
		}
		void setSpecular(const char *path) {
			mp_specular = MakeUnique<ImageTexture2D<float, float>>(path);
		}
		void setSpecular(const float specular) {
			mp_specular = MakeUnique<ConstantTexture2D<float>>(specular);
		}
		
	private:
		float pdfInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const override;
		float evalInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const override;

		float Fresnel_Schlick(const float cosd) const {
			float R = 1.f - cosd;
			float sqr_R = R * R;
			return sqr_R * sqr_R * R;
		}
		float Fresnel_Schlick(const float cosd, const float reflectance) const {
			float R = 1.f - cosd;
			float sqr_R = R * R;
			float fresnel = reflectance + (1.f - reflectance) * sqr_R * sqr_R * R;
			float fresnel_conductor = fresnelConductor(cosd, .4f, 1.6f);

			return Lerp(m_metallic, fresnel, fresnel_conductor);
		}
		float Fresnel_Schlick_Coat(const float cosd) const {
			float R = 1.f - cosd;
			float sqr_R = R * R;
			float fresnel = .04f + (1.f - .04f) * sqr_R * sqr_R * R;
			
			return fresnel;
		}

		// Each Disney independent Term
		float diffuseTerm(const Vector3 &l_out, const Vector3 &l_in, const float IdotH, const float roughness) const {
			if (m_metallic == 1.f)
				return 0.f;

			// Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
			// and mix in diffuse retro-reflection based on roughness
			float FL = Fresnel_Schlick(AbsCosTheta(l_in));
			float FV = Fresnel_Schlick(AbsCosTheta(l_out));
			float F_D90 = .5f + 2.f * IdotH * IdotH * roughness;

			return (1.f + (F_D90 - 1.f) * FL) * (1.f + (F_D90 - 1.f) * FV) * float(M_1_PI);
		}
		float specularTerm(const Vector3 &l_out, const Vector3 &l_in, const Vector3 &wh,
			const float OdotH, const float roughness, const float specular, const float *fresnel = nullptr) const {
			if (CosTheta(l_out) * CosTheta(l_in) <= 0.f)
				return 0.f;

			float Ds = GGX_D(wh, roughness * roughness);
			if (Ds == 0.f)
				return 0.f;

			float normal_ref = Lerp(specular, .0f, .08f);
			float Fs = fresnel ? *fresnel : Fresnel_Schlick(OdotH, normal_ref);

			float rough_G = (.5f + .5f * roughness);
			float Gs = GGX_G(l_out, l_in, wh, rough_G * rough_G);

			return Fs * Ds * Gs / 
				(4.f * AbsCosTheta(l_out) * AbsCosTheta(l_in));
		}
		float subsurfaceTerm(const Vector3 &l_out, const Vector3 &l_in, const float IdotH, const float roughness) const {
			if (m_subsurface == 0.f)
				return 0.f;
			
			float FL = Fresnel_Schlick(AbsCosTheta(l_in));
			float FV = Fresnel_Schlick(AbsCosTheta(l_out));

			// Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
			// 1.25 scale is used to (roughly) preserve albedo
			// Fss90 used to "flatten" retroreflection based on roughness
			float Fss90 = IdotH * IdotH * roughness;
			float Fss = (1.f + (Fss90 - 1.f) * FL) * (1.f + (Fss90 - 1.f) * FV);

			return 1.25f * (Fss * (1.f / (AbsCosTheta(l_in) + AbsCosTheta(l_out)) - .5f) + .5f) * float(M_1_PI);
		}
		float clearCoatTerm(const Vector3 &l_out, const Vector3 &l_in, const Vector3 &wh,
			const float IdotH, const float roughness) const {
			if (m_clearCoat == 0.f)
				return 0.f;

			float rough = Lerp(roughness, .005f, .1f);

			// clearcoat (ior = 1.5 -> F0 = 0.04)
			float Dr = GGX_D(wh, rough);
			if (Dr == 0.f)
				return 0.f;
			float Fr = Fresnel_Schlick_Coat(IdotH);
			float Gr = GGX_G(l_out, l_in, wh, .25f);

			return m_clearCoat * Fr * Dr * Gr /
				(4.f * AbsCosTheta(l_out) * AbsCosTheta(l_in));
		}
	};
}

#endif