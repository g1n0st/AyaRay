#ifndef AYA_BSDFS_ROUGHDIELECTRIC_H
#define AYA_BSDFS_ROUGHDIELECTRIC_H

#include <Core/BSDF.h>

namespace Aya {
	class RoughDielectric : public BSDF {
	private:
		std::unique_ptr<Texture2D<float>> m_roughness;
		float m_etai, m_etat;

		static const ScatterType reflect_scatter = ScatterType(BSDF_REFLECTION | BSDF_GLOSSY);
		static const ScatterType refract_scatter = ScatterType(BSDF_TRANSMISSION | BSDF_GLOSSY);

	public:
		RoughDielectric(const Spectrum &color = Spectrum(), float roughness = .3f, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_GLOSSY), BSDFType::RoughDielectric, color),
			m_roughness(new ConstantTexture2D<float>(roughness)),
			m_etai(etai), m_etat(etat) {}
		RoughDielectric(std::unique_ptr<Texture2D<Spectrum>> tex, std::unique_ptr<Texture2D<RGBSpectrum>> normal, float roughness = .3f, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_GLOSSY), BSDFType::RoughDielectric, std::move(tex), std::move(normal)),
			m_roughness(new ConstantTexture2D<float>(roughness)),
			m_etai(etai), m_etat(etat) {}
		RoughDielectric(const char *texture_file, float roughness = .3f, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_GLOSSY), BSDFType::RoughDielectric, texture_file),
			m_roughness(new ConstantTexture2D<float>(roughness)),
			m_etai(etai), m_etat(etat) {}
		RoughDielectric(const char *texture_file, const char *normal_file, float roughness = .3f, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_GLOSSY), BSDFType::RoughDielectric, texture_file, normal_file),
			m_roughness(new ConstantTexture2D<float>(roughness)),
			m_etai(etai), m_etat(etat) {}

		RoughDielectric(const Spectrum &color, char *roughness_texture, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_GLOSSY), BSDFType::RoughDielectric, color),
			m_roughness(new ImageTexture2D<float, float>(roughness_texture)),
			m_etai(etai), m_etat(etat) {}
		RoughDielectric(std::unique_ptr<Texture2D<Spectrum>> tex, std::unique_ptr<Texture2D<RGBSpectrum>> normal, char *roughness_texture, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_GLOSSY), BSDFType::RoughDielectric, std::move(tex), std::move(normal)),
			m_roughness(new ImageTexture2D<float, float>(roughness_texture)),
			m_etai(etai), m_etat(etat) {}
		RoughDielectric(const char *texture_file, char *roughness_texture, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_GLOSSY), BSDFType::RoughDielectric, texture_file),
			m_roughness(new ImageTexture2D<float, float>(roughness_texture)),
			m_etai(etai), m_etat(etat) {}
		RoughDielectric(const char *texture_file, const char *normal_file, char *roughness_texture, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_GLOSSY), BSDFType::RoughDielectric, texture_file, normal_file),
			m_roughness(new ImageTexture2D<float, float>(roughness_texture)),
			m_etai(etai), m_etat(etat) {}

		RoughDielectric(const Spectrum &color, std::unique_ptr<Texture2D<float>> roughness, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_GLOSSY), BSDFType::RoughDielectric, color),
			m_roughness(std::move(roughness)),
			m_etai(etai), m_etat(etat) {}
		RoughDielectric(std::unique_ptr<Texture2D<Spectrum>> tex, std::unique_ptr<Texture2D<RGBSpectrum>> normal, std::unique_ptr<Texture2D<float>> roughness, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_GLOSSY), BSDFType::RoughDielectric, std::move(tex), std::move(normal)),
			m_roughness(std::move(roughness)),
			m_etai(etai), m_etat(etat) {}
		RoughDielectric(const char *texture_file, std::unique_ptr<Texture2D<float>> roughness, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_GLOSSY), BSDFType::RoughDielectric, texture_file),
			m_roughness(std::move(roughness)),
			m_etai(etai), m_etat(etat) {}
		RoughDielectric(const char *texture_file, const char *normal_file, std::unique_ptr<Texture2D<float>> roughness, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_GLOSSY), BSDFType::RoughDielectric, texture_file, normal_file),
			m_roughness(std::move(roughness)),
			m_etai(etai), m_etat(etat) {}

		virtual Spectrum f(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const override;
		virtual float pdf(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const override;
		virtual Spectrum sample_f(const Vector3 &v_out, const Sample &sample,
			const SurfaceIntersection &intersection, Vector3 *v_in, float *pdf, ScatterType types = BSDF_ALL, ScatterType *sample_types = nullptr) const override;

	private:
		virtual float evalInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const override;
		virtual float pdfInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const override;
	};
}

#endif