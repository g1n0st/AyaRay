#ifndef AYA_BSDFS_GLASS_H
#define AYA_BSDFS_GLASS_H

#include <Core/BSDF.h>

namespace Aya {
	class Glass : public BSDF {
	private:
		float m_etai, m_etat;

	public:
		Glass(const Spectrum &color = Spectrum(), float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR), BSDFType::Glass, color)
			, m_etai(etai), m_etat(etat) {}
		Glass(std::unique_ptr<Texture2D<Spectrum>> tex, std::unique_ptr<Texture2D<RGBSpectrum>> normal, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR), BSDFType::Glass, std::move(tex), std::move(normal))
			, m_etai(etai), m_etat(etat) {}
		Glass(const char *texture_file, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR), BSDFType::Glass, texture_file)
			, m_etai(etai), m_etat(etat) {}
		Glass(const char *texture_file, const char *normal_file, float etai = 1.0f, float etat = 1.5f)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR), BSDFType::Glass, texture_file, normal_file)
			, m_etai(etai), m_etat(etat) {}

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