#ifndef AYA_CORE_MEDIUM_H
#define AYA_CORE_MEDIUM_H

#include "../Core/Ray.h"
#include "../Core/Memory.h"
#include "../Core/Spectrum.h"
#include "../Core/Sampler.h"
#include "../Math/Vector3.h"
#include "../Math/Vector2.h"

namespace Aya {
	class PhaseFunctionHG {
	private:
		const float m_g;

	public:
		PhaseFunctionHG(float g) : m_g(g) {}

		float f(const Vector3 &wo, const Vector3 &wi) const;
		float sample_f(const Vector3 &wo, Vector3 *v_in, const Vector2f &sample) const;

		inline float HenyeyGreenstein(const float cos_theta, const float g) const {
			float denom = 1.f + g * g + 2 * g * cos_theta;
			return float(M_1_PI * .25f) * (1 - g * g) / (denom * Sqrt(denom));
		}
	};

	class MediumIntersection;
	class Medium {
	protected:
		UniquePtr<PhaseFunctionHG> mp_func;

	public:
		Medium(const float g) {
			mp_func = MakeUnique<PhaseFunctionHG>(g);
		}

		virtual Spectrum tr(const Ray &ray, Sampler *sampler) const = 0;
		virtual Spectrum sample(const Ray &ray, Sampler *sampler, MediumIntersection *mi) const = 0;
	};

	class MediumInterface {
	private:
		const Medium *mp_inside;
		const Medium *mp_outside;

	public:
		MediumInterface() : mp_inside(nullptr), mp_outside(nullptr) {}
		MediumInterface(const Medium *medium) : mp_inside(medium), mp_outside(medium) {}
		MediumInterface(const Medium *inside, const Medium *outside) :
			mp_inside(inside), mp_outside(outside) {}

		const Medium* getOutside() const {
			return mp_outside;
		}
		const Medium* getInside() const {
			return mp_inside;
		}
		void setOutside(const Medium *medium) {
			SafeDelete(mp_outside);
			mp_outside = medium;
		}
		void setInside(const Medium *medium) {
			SafeDelete(mp_inside);
			mp_inside = medium;
		}
		const Medium* getMedium(const Vector3 &d, const Normal3 &n) const {
			return d.dot(n) > 0.f ? mp_outside : mp_inside;
		}

		bool isMediumTransition() const {
			return mp_inside != mp_outside;
		}
	};
}

#endif