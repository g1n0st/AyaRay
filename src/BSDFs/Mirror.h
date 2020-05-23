#ifndef AYA_BSDFS_MIRROR_H
#define AYA_BSDFS_MIRROR_H

#include <Core/BSDF.h>

namespace Aya {
	class Mirror : public BSDF {
	public:
		Mirror(const Spectrum &color = Spectrum())
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_SPECULAR), BSDFType::Mirror, color) {}
		Mirror(UniquePtr<Texture2D<Spectrum>> tex, UniquePtr<Texture2D<RGBSpectrum>> normal)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_SPECULAR), BSDFType::Mirror, std::move(tex), std::move(normal)) {}
		Mirror(const char *texture_file)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_SPECULAR), BSDFType::Mirror, texture_file) {}
		Mirror(const char *texture_file, const char *normal_file)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_SPECULAR), BSDFType::Mirror, texture_file, normal_file) {}

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