#ifndef AYA_BSDFS_ROUGHCONDUCTOR_H
#define AYA_BSDFS_ROUGHCONDUCTOR_H

#include "../Core/BSDF.h"

namespace Aya {
	class RoughConductor : public BSDF {
	private:
		UniquePtr<Texture2D<float>> m_roughness;

	public:
		RoughConductor(const Spectrum &color = Spectrum(), float roughness = .3f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_GLOSSY), BSDFType::RoughConductor, color),
			m_roughness(new ConstantTexture2D<float>(roughness)) {}
		RoughConductor(UniquePtr<Texture2D<Spectrum>> tex, UniquePtr<Texture2D<RGBSpectrum>> normal, float roughness = .3f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_GLOSSY), BSDFType::RoughConductor, std::move(tex), std::move(normal)),
			m_roughness(new ConstantTexture2D<float>(roughness)) {}
		RoughConductor(const char *texture_file, float roughness = .3f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_GLOSSY), BSDFType::RoughConductor, texture_file),
			m_roughness(new ConstantTexture2D<float>(roughness)) {}
		RoughConductor(const char *texture_file, const char *normal_file, float roughness = .3f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_GLOSSY), BSDFType::RoughConductor, texture_file, normal_file),
			m_roughness(new ConstantTexture2D<float>(roughness)) {}

		RoughConductor(const Spectrum &color, char *roughness_texture)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_GLOSSY), BSDFType::RoughConductor, color),
			m_roughness(new ImageTexture2D<float, float>(roughness_texture)) {}
		RoughConductor(UniquePtr<Texture2D<Spectrum>> tex, UniquePtr<Texture2D<RGBSpectrum>> normal, char *roughness_texture)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_GLOSSY), BSDFType::RoughConductor, std::move(tex), std::move(normal)),
			m_roughness(new ImageTexture2D<float, float>(roughness_texture)) {}
		RoughConductor(const char *texture_file, char *roughness_texture)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_GLOSSY), BSDFType::RoughConductor, texture_file),
			m_roughness(new ImageTexture2D<float, float>(roughness_texture)) {}
		RoughConductor(const char *texture_file, const char *normal_file, char *roughness_texture)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_GLOSSY), BSDFType::RoughConductor, texture_file, normal_file),
			m_roughness(new ImageTexture2D<float, float>(roughness_texture)) {}

		RoughConductor(const Spectrum &color, UniquePtr<Texture2D<float>> roughness)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_GLOSSY), BSDFType::RoughConductor, color),
			m_roughness(std::move(roughness)) {}
		RoughConductor(UniquePtr<Texture2D<Spectrum>> tex, UniquePtr<Texture2D<RGBSpectrum>> normal, UniquePtr<Texture2D<float>> roughness)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_GLOSSY), BSDFType::RoughConductor, std::move(tex), std::move(normal)),
			m_roughness(std::move(roughness)) {}
		RoughConductor(const char *texture_file, UniquePtr<Texture2D<float>> roughness)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_GLOSSY), BSDFType::RoughConductor, texture_file),
			m_roughness(std::move(roughness)) {}
		RoughConductor(const char *texture_file, const char *normal_file, UniquePtr<Texture2D<float>> roughness)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_GLOSSY), BSDFType::RoughConductor, texture_file, normal_file),
			m_roughness(std::move(roughness)) {}

		virtual Spectrum sample_f(const Vector3 &v_out, const Sample &sample,
			const SurfaceIntersection &intersection, Vector3 *v_in, float *pdf, ScatterType types = BSDF_ALL, ScatterType *sample_types = nullptr) const override;

	private:
		virtual float evalInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const override;
		virtual float pdfInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const override;
	};
}

#endif