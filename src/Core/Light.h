#ifndef AYA_CORE_LIGHT_H
#define AYA_CORE_LIGHT_H

#include "../Core/Intersection.h"
#include "../Core/Ray.h"
#include "../Math/Vector3.h"
#include "../Core/Sampler.h"

namespace Aya {
	class Scene;

	class VisibilityTester {
	private:
		Ray ray;

	public:
		void setSegment(const Point3 &pt1, const Point3 &pt2) {
			float dist = pt1.distance(pt2);
			ray = Ray(pt1, (pt2 - pt1) / dist, nullptr, 0.f, dist);
		}
		void setRay(const Point3 &pt, const Vector3 &dir, const float max_length = INFINITY - AYA_RAY_EPS) {
			ray = Ray(pt, dir, nullptr, 0.f, max_length);
		}
		void setMedium(const Medium *medium) {
			ray.mp_medium = medium;
		}

		bool unoccluded(const Scene *scene) const;
		Spectrum tr(const Scene *scene, Sampler *sampler) const;
	};

	class Light {
	protected:
		uint32_t m_sampleCount;

	public:
		Light(uint32_t count) :
			m_sampleCount(count) {}

		virtual ~Light() = default;
		virtual Spectrum illuminate(const Scatter &scatter,
			const Sample &light_sample,
			Vector3 *dir,
			VisibilityTester *tester,
			float *pdf,
			float *cos_at_light = nullptr,
			float *emit_pdf_w = nullptr) const = 0;
		virtual Spectrum sample(const Sample &light_sample0,
			const Sample &light_sample1,
			Ray *ray,
			Normal3 *normal,
			float *pdf,
			float *direct_pdf = nullptr) const = 0;
		virtual Spectrum emit(const Vector3 &dir,
			const Normal3 &normal = Normal3(0.f, 0.f, 0.f),
			float *pdf = nullptr,
			float *direct_pdf = nullptr) const = 0;
		virtual float pdf(const Point3 &pos, const Vector3 &dir) const = 0;
		virtual bool isEnvironmentLight() const {
			return false;
		}
		virtual bool isAreaLight() const {
			return false;
		}
		virtual bool isDelta() const = 0;
		virtual bool isFinite() const = 0;
		uint32_t getSampleCount() const {
			return m_sampleCount;
		}
	};
}

#endif