#include <Media/Homogeneous.h>
#include <Core/Intersection.h>

namespace Aya {
	Spectrum HomogeneousMedium::tr(const Ray &ray, Sampler *sampler) const {
		return (-m_sigma_t *
			Min(ray.m_maxt * ray.m_dir.length(),
			std::numeric_limits<float>::max())
			).exp();
	}
	Spectrum HomogeneousMedium::sample(const Ray &ray, Sampler *sampler, MediumIntersection *mi) const {
		int channel = Min((int)(sampler->get1D() * Spectrum::nSamples), Spectrum::nSamples - 1);
		float dist = -std::logf(1.f - sampler->get1D()) / m_sigma_t[channel];
		float t = Min(dist / ray.m_dir.length(), ray.m_maxt);
		bool sampled_medium = t < ray.m_maxt;
		if (sampled_medium)
			*mi = MediumIntersection(ray(t), mp_func.get(), MediumInterface(ray.mp_medium));

		Spectrum tr = (-m_sigma_t * Min(t, std::numeric_limits<float>::max()) * ray.m_dir.length()).exp();
		Spectrum density = sampled_medium ? (m_sigma_t * tr) : tr;
		float pdf = 0.f;
		for (auto i = 0; i < Spectrum::nSamples; ++i) {
			pdf += density[i];
		}
		pdf /= float(Spectrum::nSamples);

		return sampled_medium ? (tr * m_sigma_s / pdf) : (tr / pdf);
	}
}