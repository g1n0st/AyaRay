#include "Disney.h"

namespace Aya {
	float Disney::pdfInner(const Vector3 &l_out, const Vector3 &l_in, const SurfaceIntersection &intersection, ScatterType types) const {
		Vector3 wh = (l_out + l_in).normalize();
		if (wh.length2() == 0.f)
			return 0.f;

		float roughness = getValue(mp_roughness.get(), intersection, TextureFilter::Linear);
		roughness = Clamp(roughness, .02f, 1.f);

		float microfacet_pdf = GGX_Pdf_VisibleNormal(l_out, wh, roughness * roughness);
		float pdf = 0.f;
		float dwh_dwi = 1.f / (4.f * Abs(l_in.dot(wh)));
		float spec_pdf = microfacet_pdf * dwh_dwi;

		float normal_ref = Lerp(m_specular, .0f, .08f);
		float OdotH = l_out.dot(wh);
		float prob_spec = Fresnel_Schlick(OdotH, normal_ref);

		pdf += spec_pdf * prob_spec;
		pdf += AbsCosTheta(l_in) * float(M_1_PI) * (1.f - prob_spec);

		if (m_clear_coat > 0.f) {
			Spectrum albedo = getValue(mp_texture.get(), intersection);
			float coat_weight = m_clear_coat / (m_clear_coat + albedo.y());
			float fresnel_coat = Fresnel_Schlick_Coat(AbsCosTheta(l_out));
			float prob_coat = (fresnel_coat * coat_weight) /
				(fresnel_coat * coat_weight +
				(1.f - fresnel_coat) * (1.f - coat_weight));
			float coat_rough = Lerp(m_clear_coat_gloss, .005f, .10f);
			float coat_half_pdf = GGX_Pdf_VisibleNormal(l_out, wh, coat_rough);
			float coat_pdf = coat_half_pdf * dwh_dwi;

			pdf *= 1.f - prob_coat;
			pdf += coat_pdf * prob_coat;
		}

		return pdf;
	}

	float Disney::evalInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types) const {
		return 0.f;
	}

	Spectrum Disney::f(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types) const {
		if (v_out.dot(intersection.gn) * v_in.dot(intersection.gn) > 0.f)
			types = ScatterType(types & ~BSDF_TRANSMISSION);
		else
			types = ScatterType(types & ~BSDF_REFLECTION);

		if (!matchesTypes(types))
			return Spectrum(0.f);

		Vector3 l_out = intersection.worldToLocal(v_out);
		Vector3 l_in = intersection.worldToLocal(v_in);

		return evaluate(l_out, l_in, intersection, types);
	}

	Spectrum Disney::evaluate(const Vector3 &l_out, const Vector3 &l_in, const SurfaceIntersection &intersection, ScatterType types) const {
		Spectrum albedo = getValue(mp_texture.get(), intersection);
		float roughness = getValue(mp_roughness.get(), intersection, TextureFilter::Linear);
		roughness = Clamp(roughness, .02f, 1.f);

		Spectrum Ctint = albedo.y(); // luminance approx.
		Spectrum Cspec0 = Lerp(m_metallic, // dielectric -> metallic
			Lerp(1.f - m_specular_tint, albedo, Ctint), // baseColor -> Colorless
			albedo);
		Spectrum Csheen = Lerp(m_sheen_tint, Ctint, albedo); // Colorless -> baseColor

		Vector3 wh = (l_out + l_in).normalize();
		float OdotH = l_out.dot(wh);
		float IdotH = l_in.dot(wh);
		float _1_OdotH = 1.f - OdotH;
		Cspec0 = Lerp(_1_OdotH * _1_OdotH * _1_OdotH, Cspec0, Ctint);

		// sheen
		float FH = Fresnel_Schlick(OdotH, 0.f);
		Spectrum sheen = FH * m_sheen * Csheen;

		return (1.f - m_metallic)
			* (albedo * Lerp(m_subsurface, diffuseTerm(l_out, l_in, IdotH, roughness), subsurfaceTerm(l_out, l_in, IdotH, roughness))
				+ sheen)
			+ Cspec0 * specularTerm(l_out, l_in, wh, OdotH, roughness, nullptr)
			+ Spectrum(clearCoatTerm(l_out, l_in, wh, IdotH, m_clear_coat_gloss));
	}

	Spectrum Disney::sample_f(const Vector3 &v_out, const Sample &sample, const SurfaceIntersection &intersection, 
		Vector3 *v_in, float *pdf, ScatterType types, ScatterType *sample_types) const {
		if (!matchesTypes(types)) {
			*pdf = 0.f;
			return Spectrum(0.f);
		}

		const Vector3 l_out = intersection.worldToLocal(v_out);
		Vector3 l_in, wh;
		bool sample_coat = false;
		Sample remapped_sample = sample;

		if (m_clear_coat > 0.f) {
			Spectrum albedo = getValue(mp_texture.get(), intersection);
			float coat_weight = m_clear_coat / (m_clear_coat + albedo.y());
			float fresnel_coat = Fresnel_Schlick_Coat(AbsCosTheta(l_out));
			float prob_coat = (fresnel_coat * coat_weight) /
				(fresnel_coat * coat_weight +
				(1.f - fresnel_coat) * (1.f - coat_weight));

			if (sample.v < prob_coat) {
				sample_coat = true;
				remapped_sample.v /= prob_coat;

				float coat_rough = Lerp(m_clear_coat_gloss, .005f, .1f);
				float coat_pdf;
				wh = GGX_SampleVisibleNormal(l_out, remapped_sample.u, remapped_sample.v, &coat_pdf, coat_rough);
				l_in = -l_out + 2.f * l_out.dot(wh) * wh;
				*sample_types = ScatterType(BSDF_REFLECTION | BSDF_GLOSSY);
			}
			else {
				sample_coat = false;
				remapped_sample.v = (sample.v - prob_coat) / (1.f - prob_coat);
			}
		}

		if (!sample_coat) {
			float microfacet_pdf;
			float roughness = getValue(mp_roughness.get(), intersection, TextureFilter::Linear);
			roughness = Clamp(roughness, .02f, 1.f);

			wh = GGX_SampleVisibleNormal(l_out, remapped_sample.u, remapped_sample.v, &microfacet_pdf, roughness * roughness);

			float normal_ref = Lerp(m_specular, .0f, .08f);
			float OdotH = l_out.dot(wh);
			float prob_spec = Fresnel_Schlick(OdotH, normal_ref);

			if (remapped_sample.w <= prob_spec) {
				l_in = -l_out + 2.f * l_out.dot(wh) * wh;
				*sample_types = ScatterType(BSDF_REFLECTION | BSDF_GLOSSY);
			}
			else {
				l_in = CosineSampleHemisphere(remapped_sample.u, remapped_sample.v);
				if (l_out.z() < 0.f)
					l_in.setZ(l_in.z() * -1.f);

				*sample_types = ScatterType(BSDF_REFLECTION | BSDF_DIFFUSE);
			}
		}

		if (l_in.z() <= 0.0f) {
			*pdf = 0.f;
			return Spectrum(0.f);
		}

		*v_in = intersection.localToWorld(l_in);
		*pdf = pdfInner(l_out, l_in, intersection, types);

		if (v_out.dot(intersection.gn) * v_in->dot(intersection.gn) > 0.f)
			types = ScatterType(types & ~BSDF_TRANSMISSION);
		else
			types = ScatterType(types & ~BSDF_REFLECTION);

		if (!matchesTypes(types)) {
			*pdf = 0.f;
			return Spectrum(0.f);
		}

		return evaluate(l_out, l_in, intersection, types);
	}
}