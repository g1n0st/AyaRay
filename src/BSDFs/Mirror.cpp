#include "Mirror.h"

namespace Aya {
	Spectrum Mirror::f(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types) const {
		return Spectrum(0.f);
	}
	float Mirror::pdf(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types) const {
		return 0.f;
	}
	float Mirror::evalInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types) const {
		return 0.f;
	}
	float Mirror::pdfInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types) const {
		return 0.f;
	}

	Spectrum Mirror::sample_f(const Vector3 &v_out, const Sample &sample,
		const SurfaceIntersection &intersection, Vector3 *v_in, float *pdf,
		ScatterType types, ScatterType *sample_types) const {
		if (!matchesTypes(types)) {
			*pdf = 0.f;
			return Spectrum(0.f);
		}

		Vector3 wo = intersection.worldToLocal(v_out);
		Vector3 wi = Vector3(-wo.x, -wo.y, wo.z);

		*v_in = intersection.localToWorld(wi);
		*pdf = 1.f;
		if (sample_types != NULL)
			*sample_types = m_scatterType;

		return getValue(mp_texture.get(), intersection) / AbsCosTheta(wi);
	}
}