#include "Disney.h"

namespace Aya {
	float Disney::pdfInner(const Vector3 &v_out, const Vector3 &v_in, const SurfaceIntersection &intersection, ScatterType types) const {
		Vector3 wh = (v_out + v_in).normalize();
		if (wh.length2() == 0.f)
			return 0.f;

		float roughness = getValue(m_roughness.get(), intersection, TextureFilter::Linear);
		roughness = Clamp(roughness, .02f, 1.f);

		float microfacet_pdf = GGX_Pdf_VisibleNormal(v_out, wh, roughness * roughness);
		float pdf = 0.f;
		float dwh_dwi = 1.f / (4.f * Abs(v_in.dot(wh)));
		float spec_pdf = microfacet_pdf * dwh_dwi;

		float norm_ref = Lerp(m_specular, .0f, .08f);
		float odh = v_out.dot(wh);
		float prob_spec = Fresnel_
	}
}