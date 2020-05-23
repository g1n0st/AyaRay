#include <BSDFs/Glass.h>

namespace Aya {
	Spectrum Glass::f(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types) const {
		return Spectrum(0.f);
	}
	float Glass::pdf(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types) const {
		return 0.f;
	}
	float Glass::evalInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types) const {
		return 0.f;
	}
	float Glass::pdfInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types) const {
		return 0.f;
	}

	Spectrum Glass::sample_f(const Vector3 &v_out, const Sample &sample,
		const SurfaceIntersection &intersection, Vector3 *v_in, float *pdf,
		ScatterType types, ScatterType *sample_types) const {
		bool sample_reflect = (types & (BSDF_REFLECTION | BSDF_SPECULAR)) == (BSDF_REFLECTION | BSDF_SPECULAR);
		bool sample_refract = (types & (BSDF_TRANSMISSION | BSDF_SPECULAR)) == (BSDF_TRANSMISSION | BSDF_SPECULAR);

		if (!sample_reflect && !sample_refract) {
			*pdf = 0.f;
			return Spectrum(0.f);
		}

		bool sample_both = (sample_reflect == sample_refract);

		Vector3 wo = intersection.worldToLocal(v_out), wi;

		float fresnel = fresnelDielectric(CosTheta(wo), m_etai, m_etat);
		float prob = .5f * fresnel + .25f;

		// reflect
		if (sample.w <= prob && sample_both || (sample_reflect && !sample_refract)) {
			wi = Vector3(-wo.x, -wo.y, wo.z);

			*v_in = intersection.localToWorld(wi);
			*pdf = !sample_both ? 1.f : prob;
			if (sample_types != NULL)
				*sample_types = ScatterType(BSDF_REFLECTION | BSDF_SPECULAR);
			return fresnel * Spectrum(1.f) / AbsCosTheta(wi);
		}
		// refract
		else if (sample.w > prob && sample_both || (sample_refract && !sample_both)) {
			bool entering = CosTheta(wo) > 0.f;
			float etai = m_etai, etat = m_etat;
			if (!entering)
				std::swap(etai, etat);

			float sini2 = SinTheta2(wo);
			float eta = etai / etat;
			float sint2 = eta * eta * sini2;

			if (sint2 > 1.f) // total reflection
				return Spectrum(0.f);

			float cost = Sqrt(Max(0.f, 1.f - sint2));
			if (entering)
				cost = -cost;
			float toi = eta;

			wi = Vector3(toi * -wo.x, toi * -wo.y, cost);

			*v_in = intersection.localToWorld(wi);
			*pdf = !sample_both ? 1.f : 1.f - prob;
			if (sample_types != NULL)
				*sample_types = ScatterType(BSDF_TRANSMISSION | BSDF_SPECULAR);

			return (1.f - fresnel) * eta * eta * getValue(mp_texture.get(), intersection) / AbsCosTheta(wi);
		}

		return Spectrum(0.f);
	}
}
