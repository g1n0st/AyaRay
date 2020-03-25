#ifndef AYA_BSDFS_LAMBERTIANDIFFUSE_H
#define AYA_BSDFS_LAMBERTIANDIFFUSE_H

#include "../Core/BSDF.h"

namespace Aya {
	class LambertianDiffuse : public BSDF {
	public:
		LambertianDiffuse(const Spectrum &color = Spectrum())
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE), BSDFType::LambertianDiffuse, color) {}
		LambertianDiffuse(UniquePtr<Texture2D<Spectrum>> tex, UniquePtr<Texture2D<RGBSpectrum>> normal)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE), BSDFType::LambertianDiffuse, std::move(tex), std::move(normal)) {}
		LambertianDiffuse(const char *texture_file)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE), BSDFType::LambertianDiffuse, texture_file) {}
		LambertianDiffuse(const char *texture_file, const char *normal_file)
			: BSDF(ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE), BSDFType::LambertianDiffuse, texture_file, normal_file) {}

		virtual Spectrum sample_f(const Vector3 &v_out, const Sample &sample,
			const SurfaceIntersection &intersection, Vector3 *v_in, float *pdf, ScatterType types = BSDF_ALL, ScatterType *sample_types = nullptr) const override;

	private:
		virtual float evalInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const override;
		virtual float pdfInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types = BSDF_ALL) const override;
	};
}

#endif