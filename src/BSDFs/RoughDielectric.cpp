#include "RoughDielectric.h"

namespace Aya {
	float RoughDielectric::pdf(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types) const {
		Vector3 l_out = intersection.worldToLocal(v_out);
		Vector3 l_in = intersection.worldToLocal(v_in);

		return pdfInner(l_out, l_in, intersection, types);
	}

	float RoughDielectric::pdfInner(const Vector3 &wo, const Vector3 &wi, const SurfaceIntersection &intersection, ScatterType types) const {
		bool sample_reflect = (types & reflect_scatter) == reflect_scatter;
		bool sample_refract = (types & refract_scatter) == refract_scatter;
		const float OdotN = CosTheta(wo);
		const float IdotN = CosTheta(wi);
		const float fac = OdotN * IdotN;
		if (fac == 0.f)
			return 0.f;

		bool reflect = fac > 0.f;
		bool entering = OdotN > 0.f;

		Vector3 wh;
		float dwh_dwi;
		if (reflect) {			// reflect
			if (!sample_reflect)
				return 0.f;

			wh = (wo + wi).normalize();
			dwh_dwi = 1.f / (4.f * wi.dot(wh));
		}
		else {					// refract
			if (!sample_refract)
				return 0.f;

			float etai = m_etai, etat = m_etat;
			if (!entering)
				std::swap(etai, etat);

			wh = -(etai * wo + etat * wi).normalize();

			const float OdotH = wo.dot(wh);
			const float IdotH = wi.dot(wh);
			float sqrt_denom = etai * OdotH + etat * IdotH;
			dwh_dwi = (etat * etat * IdotH) / (sqrt_denom * sqrt_denom);
		}

		wh *= (CosTheta(wh) >= 0.f ? 1.f : -1.f);

		float roughness = getValue(m_roughness.get(), intersection, TextureFilter::Linear);
		roughness = Clamp(roughness, .02f, 1.f);
		roughness = roughness * roughness;
		float wh_prob = GGX_Pdf_VisibleNormal(wo * (CosTheta(wo) >= 0.f ? 1.f : -1.f), wh, roughness);
		if (sample_reflect && sample_refract) {
			float F = fresnelDielectric(wo.dot(wh), m_etai, m_etat);
			//F = 0.5f * F + 0.25f;
			wh_prob *= reflect ? F : 1.f - F;
		}

		assert(!std::isnan(wh_prob));
		assert(!std::isnan(dwh_dwi));
		return Abs(wh_prob * dwh_dwi);
	}

	Spectrum RoughDielectric::f(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types) const {
		Vector3 l_out = intersection.worldToLocal(v_out);
		Vector3 l_in = intersection.worldToLocal(v_in);

		return getValue(mp_texture.get(), intersection) * evalInner(l_out, l_in, intersection, types);
	}

	float RoughDielectric::evalInner(const Vector3 &wo, const Vector3 &wi, const SurfaceIntersection &intersection, ScatterType types) const {
		bool sample_reflect = (types & reflect_scatter) == reflect_scatter;
		bool sample_refract = (types & refract_scatter) == refract_scatter;
		const float OdotN = CosTheta(wo);
		const float IdotN = CosTheta(wi);
		const float fac = OdotN * IdotN;
		if (fac == 0.f)
			return 0.f;

		bool reflect = fac > 0.f;
		bool entering = OdotN > 0.f;

		float etai = m_etai, etat = m_etat;
		if (!entering)
			std::swap(etai, etat);

		Vector3 wh;
		if (reflect) {			// reflect
			if (!sample_reflect)
				return 0.f;

			wh = wo + wi;
			if (wh.length2() <= 1e-5f)
				return 0.f;

			wh.normalized();
		}
		else {					// refract
			if (!sample_refract)
				return 0.f;

			wh = -(etai * wo + etat * wi).normalize();
		}

		wh *= (CosTheta(wh) >= 0.f ? 1.f : -1.f);

		float roughness = getValue(m_roughness.get(), intersection, TextureFilter::Linear);
		roughness = Clamp(roughness, .02f, 1.f);
		roughness = roughness * roughness;

		float D = GGX_D(wh, roughness);
		if (D == 0.f)
			return 0.f;

		float F = fresnelDielectric(wo.dot(wh), m_etai, m_etat);
		float G = GGX_G(wo, wi, wh, roughness);

		if (reflect) {
			return Abs(F * D * G / (4.f * CosTheta(wi) * CosTheta(wo)));
		}
		else {
			const float OdotH = wo.dot(wh);
			const float IdotH = wi.dot(wh);

			float sqrt_denom = etai * OdotH + etat * IdotH;
			float value = ((1.f - F) * D * G * etat * etat * OdotH * IdotH) /
				(sqrt_denom * sqrt_denom * OdotN * IdotN);

			// TODO: Fix solid angle compression when tracing radiance
			float factor = 1.0f;

			assert(!std::isnan(value));
			return Abs(value * factor * factor);
		}
	}

	Spectrum RoughDielectric::sample_f(const Vector3 &v_out, const Sample &sample,
		const SurfaceIntersection &intersection, Vector3 *v_in, float *pdf, ScatterType types, ScatterType *sample_types) const {
		bool sample_reflect = (types & reflect_scatter) == reflect_scatter;
		bool sample_refract = (types & refract_scatter) == refract_scatter;

		if (!sample_reflect && !sample_refract) {
			*pdf = 0.f;
			return Spectrum(0.f);
		}

		bool sample_both = sample_reflect == sample_refract;
		const Vector3 wo = intersection.worldToLocal(v_out);

		float roughness = getValue(m_roughness.get(), intersection, TextureFilter::Linear);
		roughness = Clamp(roughness, .02f, 1.f);
		roughness = roughness * roughness;

		float microfacet_pdf;
		const Vector3 wh = GGX_SampleVisibleNormal(wo * (CosTheta(wo) >= 0.f ? 1.f : -1.f), sample.u, sample.v, &microfacet_pdf, roughness);
		if (microfacet_pdf == 0.f)
			return 0.f;

		float D = GGX_D(wh, roughness);
		if (D == 0.f)
			return 0.f;

		float F = fresnelDielectric(wo.dot(wh), m_etai, m_etat);
		float prob = F;

		Vector3 wi;
		if (sample.w <= prob && sample_both || (sample_reflect && !sample_both)) { // Sample reflection
			wi = -wo + Vector3(2.f * wo.dot(wh) * wh);
			if (CosTheta(wi) * CosTheta(wo) <= 0.f) {
				*pdf = 0.f;
				return Spectrum(0.f);
			}

			*v_in = intersection.localToWorld(wi);

			*pdf = !sample_both ? microfacet_pdf : microfacet_pdf * prob;
			float dwh_dwi = 1.f / (4.f * wi.dot(wh));
			*pdf *= Abs(dwh_dwi);

			float G = GGX_G(wo, wi, wh, roughness);

			if (sample_types != nullptr)
				*sample_types = reflect_scatter;

			return getValue(mp_texture.get(), intersection) *
				Abs(F * D * G / (4.f * CosTheta(wi) * CosTheta(wo)));
		}
		else if (sample.w > prob && sample_both || (sample_refract && !sample_both)) { // Sample refraction
			float eta = m_etat / m_etai;
			float odoth = -wo.dot(wh);
			if (odoth < 0.f)
				eta = 1.f / eta;

			float idoth2 = 1.f - (1.f - odoth * odoth) * (eta * eta);

			if (idoth2 <= 0.f) {
				*pdf = 0.f;
				return Spectrum(0.f);
			}

			float sign = (odoth >= 0.f ? 1.f : -1.f);
			wi = wh * (-odoth * eta + sign * Sqrt(idoth2)) + (-wo) * eta;

			if (CosTheta(wi) * CosTheta(wo) >= 0.f) {
				*pdf = 0.f;
				return Spectrum(0.f);
			}

			*v_in = intersection.localToWorld(wi);

			*pdf = !sample_both ? microfacet_pdf : microfacet_pdf * (1.f - prob);
			bool entering = CosTheta(wo) > 0.f;
			float etai = m_etai, etat = m_etat;
			if (!entering)
				std::swap(etai, etat);

			const float OdotH = wo.dot(wh);
			const float IdotH = wi.dot(wh);
			float sqrt_denom = etai * OdotH + etat * IdotH;
			if (sqrt_denom == 0.f) {
				*pdf = 0.f;
				return Spectrum(0.f);
			}

			float dwh_dwi = (etat * etat * IdotH) / (sqrt_denom * sqrt_denom);
			*pdf *= Abs(dwh_dwi);

			if (sample_types != nullptr)
				*sample_types = refract_scatter;

			float G = GGX_G(wo, wi, wh, roughness);

			float value = ((1.f - F) * D * G * etat * etat * OdotH * IdotH) /
				(sqrt_denom * sqrt_denom * CosTheta(wo) * CosTheta(wi));

			// TODO: Fix solid angle compression when tracing radiance
			float factor = 1.0f;

			return getValue(mp_texture.get(), intersection) * Abs(value * factor * factor);
		}

		return Spectrum(0.f);
	}
}