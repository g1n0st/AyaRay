#include "Medium.h"

namespace Aya {
	float PhaseFunctionHG::f(const Vector3 &wo, const Vector3 &wi) const {
		return HenyeyGreenstein(wo.dot(wi), m_g);
	}
	float PhaseFunctionHG::sample_f(const Vector3 &wo, Vector3 *v_in, const Vector2f &sample) const {
		float cos_theta;
		float g = m_g;
		if (Abs(g) < 1e-3) {
			cos_theta = 1.f - 2.f * sample.u;
		}
		else {
			float sqr_term = (1.f - g * g) / 
				(1.f - g + 2.f * g * sample.u);
			cos_theta = (1.f + g * g - sqr_term * sqr_term) / (2.f * g);
		}

		float sin_theta = Sqrt(Max(0.f, 1.f - cos_theta * cos_theta));
		float phi = float(M_PI) * 2.f * sample.v;

		Vector3 v1, v2;
		BaseVector3::coordinateSystem(wo, &v1, &v2);
		*v_in = BaseVector3::sphericalDirection(sin_theta, cos_theta, phi, v1, -wo, v2);

		return HenyeyGreenstein(-cos_theta, g);
	}
}