#include <BSDFs/RoughConductor.h>

namespace Aya {
	Spectrum RoughConductor::sample_f(const Vector3 &v_out, const Sample &sample,
		const SurfaceIntersection &intersection, Vector3 *v_in, float *pdf, ScatterType types, ScatterType *sample_types) const {
		if (!matchesTypes(types)) {
			*pdf = 0.f;
			return Spectrum(0.f);
		}

		Vector3 wo = intersection.worldToLocal(v_out), wi;

		float microfacet_pdf;
		float roughness = getValue(m_roughness.get(), intersection, TextureFilter::Linear);
		roughness = Clamp(roughness, .02f, 1.f);
		roughness = roughness * roughness;
		Vector3 wh = GGX_SampleVisibleNormal(wo * (CosTheta(wo) >= 0.f ? 1.f : -1.f), sample.u, sample.v, &microfacet_pdf, roughness);

		if (microfacet_pdf == 0.f)
			return 0.f;

		wi = -wo + Vector3(2.f * wo.dot(wh) * wh);
		if (!sameHemisphere(wo, wi))
			return Spectrum(0.f);

		*v_in = intersection.localToWorld(wi);

		float dwh_dwi = 1.f / (4.f * Abs(wi.dot(wh)));
		*pdf = microfacet_pdf * dwh_dwi;

		if (v_out.dot(intersection.gn) * v_in->dot(intersection.gn) > 0.f)
			types = ScatterType(types & ~BSDF_TRANSMISSION);
		else
			types = ScatterType(types & ~BSDF_REFLECTION);

		if (!matchesTypes(types)) {
			*pdf = 0.f;
			return Spectrum(0.f);
		}

		if (sample_types != nullptr)
			*sample_types = m_scatterType;

		float D = GGX_D(wh, roughness);
		if (D == 0.f)
			return 0.f;
		float F = fresnelConductor(wh.dot(wo), .4f, 1.6f);
		float G = GGX_G(wo, wi, wh, roughness);

		// Cook-Torrance microfacet model
		return getValue(mp_texture.get(), intersection) * F * D * G / (4.f * AbsCosTheta(wo) * AbsCosTheta(wi));
	}

	float RoughConductor::evalInner(const Vector3 &wo, const Vector3 &wi, const SurfaceIntersection &intersection, ScatterType types) const {
		if (!sameHemisphere(wo, wi))
			return 0.0f;

		Vector3 wh = (wo + wi).normalize();
		wh *= (CosTheta(wh) >= 0.f ? 1.f : -1.f);

		float roughness = getValue(m_roughness.get(), intersection, TextureFilter::Linear);
		roughness = Clamp(roughness, .02f, 1.f);
		roughness = roughness * roughness;

		float D = GGX_D(wh, roughness);
		if (D == 0.f)
			return 0.f;
		float F = fresnelConductor(wh.dot(wo), .4f, 1.6f);
		float G = GGX_G(wo, wi, wh, roughness);

		// Cook-Torrance microfacet model
		return F * D * G / (4.f * AbsCosTheta(wo) * AbsCosTheta(wi));
	}

	float RoughConductor::pdfInner(const Vector3 &wo, const Vector3 &wi, const SurfaceIntersection &intersection, ScatterType types) const {
		if (!sameHemisphere(wo, wi))
			return 0.0f;

		Vector3 wh = (wo + wi).normalize();
		wh *= (CosTheta(wh) >= 0.f ? 1.f : -1.f);

		float roughness = getValue(m_roughness.get(), intersection, TextureFilter::Linear);
		roughness = Clamp(roughness, .02f, 1.f);
		roughness = roughness * roughness;

		float dwh_dwi = 1.f / (4.f * wi.dot(wh));
		float wh_prob = GGX_Pdf_VisibleNormal(wo * (CosTheta(wo) >= 0.f ? 1.f : -1.f), wh, roughness);

		return Abs(wh_prob * dwh_dwi);
	}
}