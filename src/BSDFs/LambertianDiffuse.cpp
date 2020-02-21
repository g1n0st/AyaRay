#include "LambertianDiffuse.h"

namespace Aya {
	float LambertianDiffuse::evalInner(const Vector3 &wo, const Vector3 &wi, const SurfaceIntersection &intersection, ScatterType types) const {
		if (CosTheta(wo) <= 0.f || !sameHemisphere(wo, wi))
			return 0.0f;
		return float(M_1_PI);
	}
	float LambertianDiffuse::pdfInner(const Vector3 &wo, const Vector3 &wi, const SurfaceIntersection &intersection, ScatterType types) const {
		if (CosTheta(wo) <= 0.f || !sameHemisphere(wo, wi))
			return 0.0f;
		return Abs(CosTheta(wi)) * float(M_1_PI);
	}
	Spectrum LambertianDiffuse::sample_f(const Vector3 &v_out, const Sample &sample, 
		const SurfaceIntersection &intersection, Vector3 *v_in, float *pdf, 
		ScatterType types, ScatterType *sample_types) const {
		if (!matchesTypes(types)) {
			*pdf = 0.f;
			return Spectrum(0.f);
		}

		Vector3 wo = intersection.W2O(v_out);
		Vector3 wi = CosineSampleHemisphere(sample.u, sample.v);
		if (CosTheta(wo) <= 0.f || !sameHemisphere(wo, wi))
			return 0.f;

		if (wo.z() < 0.f)
			wi[2] *= -1.f;

		*v_in = intersection.O2W(wi);

		if (v_out.dot(intersection.n) * v_in->dot(intersection.n) > 0.f)
			types = ScatterType(types & ~BSDF_TRANSMISSION);
		else
			types = ScatterType(types & ~BSDF_REFLECTION);

		if (!matchesTypes(types)) {
			*pdf = 0.f;
			return Spectrum(0.f);
		}

		*pdf = pdfInner(wo, wi, intersection, types);

		if (sample_types != NULL)
			*sample_types = m_scatter_type;

		return getValue(mp_texture.get(), intersection) * evalInner(wo, wi, intersection, types);
	}
}