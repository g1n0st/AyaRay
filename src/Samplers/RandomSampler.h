#ifndef AYA_SAMPLER_RANDOMSAMPLER_H
#define AYA_SAMPLER_RANDOMSAMPLER_H

#include <Core/Sampler.h>

namespace Aya {
	class RandomSampler : public Sampler {
	private:
		RNG rng;

	public:
		RandomSampler() = default;
		RandomSampler(const uint64_t seed) : rng(seed) {}

		void generateSamples(
			const int pixel_x,
			const int pixel_y,
			CameraSample *samples,
			RNG &rng
		) override;

		void startPixel(const int pixel_x, const int pixel_y) override;
		float get1D() override;
		Vector2f get2D() override;
		Sample getSample() override;

		std::unique_ptr<Sampler> clone(const int seed) const override;
		std::unique_ptr<Sampler> deepClone() const override;
	};
}
#endif