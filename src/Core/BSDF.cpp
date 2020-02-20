#include "BSDF.h"

namespace Aya {
	BSDF::BSDF(ScatterType t1, BSDFType t2, const Spectrum & color)
		: m_scatter_type(t1), m_BSDF_type(t2), mp_texture(new ConstantTexture2D<Spectrum>(color)) {}
	BSDF::BSDF(ScatterType t1, BSDFType t2, UniquePtr<Texture2D<Spectrum>> tex, UniquePtr<Texture2D<Spectrum>> normal)
		: m_scatter_type(t1), m_BSDF_type(t2), 
		mp_texture(std::move(tex)), mp_normal_map(std::move(normal)) {}
	BSDF::BSDF(ScatterType t1, BSDFType t2, const char * texture_file)
		: m_scatter_type(t1), m_BSDF_type(t2),
		mp_texture(new ImageTexture2D<Spectrum, byteSpectrum>(texture_file)) {}
	BSDF::BSDF(ScatterType t1, BSDFType t2, const char * texture_file, const char * normal_file)
		: m_scatter_type(t1), m_BSDF_type(t2) , 
		mp_texture(new ImageTexture2D<Spectrum, byteSpectrum>(texture_file)), 
		mp_normal_map(new ImageTexture2D<Spectrum, byteSpectrum>(normal_file, 1.f)) {}

	Spectrum BSDF::f(const Vector3 & v_out, const Vector3 & v_in, const SurfaceIntersection & intersection, ScatterType types) const {
		if (v_out.dot(intersection.n) * v_in.dot(intersection.n) > 0.f)
			types = ScatterType(types & ~BSDF_TRANSMISSION);
		else
			types = ScatterType(types & ~BSDF_REFLECTION);

		if (!matchesTypes(types))
			return Spectrum(0.f);

		Vector3 l_out = intersection.W2O(v_out);
		Vector3 l_in = intersection.W2O(v_in);

		return getValue(mp_texture.get(), intersection) * 
			evalInner(l_out, l_in, intersection, types);
	}
	float BSDF::pdf(const Vector3 & v_out, const Vector3 & v_in, const SurfaceIntersection & intersection, ScatterType types) const
	{
		if (v_out.dot(intersection.n) * v_in.dot(intersection.n) > 0.f)
			types = ScatterType(types & ~BSDF_TRANSMISSION);
		else
			types = ScatterType(types & ~BSDF_REFLECTION);

		if (!matchesTypes(types))
			return 0.f;

		Vector3 l_out = intersection.W2O(v_out);
		Vector3 l_in = intersection.W2O(v_in);

		return pdfInner(l_out, l_in, intersection, types);
	}


	float BSDF::fresnelDielectric(float cosi, float etai, float etat) {
		cosi = Clamp(cosi, -1.f, 1.f);

		bool entering = cosi > 0.f;
		float ei = etai, et = etat;
		if (!entering)
			std::swap(ei, et);

		float sint = ei / et * Sqrt(Max(0.f, 1.f - cosi * cosi));

		if (sint >= 1.f) {
			// total reflection
			return 1.f;
		}
		else {
			float cost = Sqrt(Max(0.f, 1.f - sint * sint));
			cosi = Abs(cosi);

			float Rparl = ((etat * cosi) - (etai * cost)) /
				((etat * cosi) + (etai * cost));
			float Rperp = ((etai * cosi) - (etat * cost)) /
				((etai * cosi) + (etat * cost));

			return .5f * (Rparl * Rparl + Rperp * Rperp);
		}
	}

	float BSDF::fresnelConductor(float cosi, const float &eta, const float &k) {
		float tmp = (eta * eta + k * k) * cosi * cosi;
		float Rparl2 = (tmp - (2.f * eta * cosi) + 1.f) /
						(tmp + (2.f * eta * cosi) + 1.f);
		float tmp_f = eta * eta + k * k;
		float Rperp2 =
			(tmp_f - (2.f * eta * cosi) + cosi * cosi) /
			(tmp_f + (2.f * eta * cosi) + cosi * cosi);

		return .5f * (Rparl2 + Rperp2);
	}

	float BSDF::GGX_D(const Vector3 & wh, float alpha) {
		if (wh.z() <= 0.f)
			return 0.f;

		const float root = alpha / (CosTheta2(wh) * (alpha * alpha + TanTheta2(wh)));
		return float(M_1_PI) * (root * root);
	}

	Vector3 BSDF::GGX_SampleNormal(float u1, float u2, float* pdf, float alpha) {
		float alpha_sqr = alpha * alpha;
		float tan_sqr = alpha_sqr * u1 / (1.f - u1 + 1e-10f);
		float cos_1 = 1.f / std::sqrt(1.f + tan_sqr);

		float cos_2 = cos_1 * cos_1,
			cos_3 = cos_2 * cos_1,
			tmp = alpha_sqr + tan_sqr;

		*pdf = float(M_1_PI) * alpha_sqr / (cos_3 * tmp * tmp);

		float sin_1 = Sqrt(Max(0.f, 1.f - cos_2));
		float phi = u2 * float(M_PI) * 2.f;

		Vector3 dir = BaseVector3::sphericalDirection(sin_1, cos_1, phi);
		return Vector3(dir.x(), dir.z(), dir.y());
	}

	float BSDF::SmithG(const Vector3 &v, const Vector3 &wh, float alpha) {
		const float tan_theta = Abs(TanTheta(v));

		if (tan_theta == 0.f)
			return 1.f;

		if (v.dot(wh) * CosTheta(v) <= 0.f)
			return 0.f;

		const float root = alpha * tan_theta;
		return 2.f / (1.f + std::sqrt(1.f + root * root));
	}

	float BSDF::GGX_G(const Vector3 &wo, const Vector3 &wi, const Vector3 &wh, float alpha) {
		return SmithG(wo, wh, alpha) * SmithG(wi, wh, alpha);
	}

	float BSDF::GGX_Pdf(const Vector3 &wh, float alpha) {
		return GGX_D(wh, alpha) * CosTheta(wh);
	}

	Vector2f BSDF::ImportanceSampleGGX_VisibleNormal_Unit(float theta, float u1, float u2) {
		Vector2f slope;

		// Special case (normal incidence)
		if (theta < 1e-4f) {
			float r = Sqrt(Max(u1 / ((1.f - u1) + 1e-6f), 0.f));
			float sin_phi = std::sinf(2.f * float(M_PI) * u2);
			float cos_phi = std::cosf(2.f * float(M_PI) * u2);
			return Vector2f(r * cos_phi, r * sin_phi);
		}

		// Precomputations
		float tan_theta = std::tanf(theta);
		float a = 1.f / tan_theta;
		float G1 = 2.f / (1.f + Sqrt(Max(1.f + 1.f / (a * a), 0.f)));

		// Simulate X component
		float A = 2.f * u1 / G1 - 1.f;
		if (Abs(A) == 1.f)
			A -= (A >= 0.f ? 1.f : -1.f) * 1e-4f;

		float tmp = 1.f / (A * A - 1.f);
		float B = tan_theta;
		float D = Sqrt(Max(B * B * tmp * tmp - (A * A - B * B) * tmp, 0.f));
		float slope_x_1 = B * tmp - D;
		float slope_x_2 = B * tmp + D;
		slope.x = (A < 0.f || slope_x_2 > 1.f / tan_theta) ? slope_x_1 : slope_x_2;

		// Simulate Y component
		float S;
		if (u2 > .5f) {
			S = 1.f;
			u2 = 2.f * (u2 - .5f);
		}
		else {
			S = -1.f;
			u2 = 2.f * (.5f - u2);
		}

		// Improved fit
		float z =
			(u2 * (u2 * (u2 * (-0.365728915865723f) + 0.790235037209296f) -
				0.424965825137544f) + 0.000152998850436920f) /
				(u2 * (u2 * (u2 * (u2 * 0.169507819808272f - 0.397203533833404f) -
					0.232500544458471f) + 1.f) - 0.539825872510702f);

		slope.y = S * z * std::sqrt(1.f + slope.x * slope.x);

		return slope;
	}

	Vector3 BSDF::GGX_SampleVisibleNormal(const Vector3 &_wi, float u1, float u2, float * pdf, float alpha) {
		// Stretch wi
		Vector3 wi = Vector3(
			alpha * wi.x(),
			alpha * wi.y(),
			wi.z()
		).normalize();

		// Get polar coordinates
		float theta = 0.f, phi = 0.f;
		if (wi.z() < float(0.99999f)) {
			theta = std::acosf(wi.z());
			phi = std::atan2f(wi.y(), wi.x());
		}
		float sin_phi = std::sinf(phi);
		float cos_phi = std::cosf(phi);

		// Simulate P22_{wi}(Slope.x, Slope.y, 1, 1)
		Vector2f slope = ImportanceSampleGGX_VisibleNormal_Unit(theta, u1, u2);

		// Step 3: rotate
		slope = Vector2f(
			cos_phi * slope.x - sin_phi * slope.y,
			sin_phi * slope.x + cos_phi * slope.y);

		// Unstretch
		slope.x *= alpha;
		slope.y *= alpha;

		// Compute normal
		float normalization = 1.f / Sqrt(slope.x * slope.x
			+ slope.y * slope.y + 1.f);

		Vector3 H = Vector3(
			-slope.x * normalization,
			-slope.y * normalization,
			normalization
		);

		*pdf = GGX_Pdf_VisibleNormal(_wi, H, alpha);

		return H;
	}

	float BSDF::GGX_Pdf_VisibleNormal(const Vector3 &wi, const Vector3 &h, float alpha) {
		float D = GGX_D(h, alpha);
		return SmithG(wi, h, alpha) * Abs(wi.dot(h)) * D / (AbsCosTheta(wi) + 1e-4f);
	}

}