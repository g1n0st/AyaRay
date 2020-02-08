#include "RandomSampler.h"

namespace Aya {
	void RandomSampler::generateSamples(
		const int pixel_x,
		const int pixel_y,
		CameraSample *samples,
		RNG &rng) {
		assert(samples);

		samples->image_x = rng.drand48();
		samples->image_y = rng.drand48();
		samples->lens_u = rng.drand48();
		samples->lens_v = rng.drand48();
		samples->time = rng.drand48();
	}
	void RandomSampler::startPixel(const int pixel_x, const int pixel_y) {}
	float RandomSampler::get1D() {
		return rng.drand48();
	}
	Vector2f RandomSampler::get2D() {
		return Vector2f(rng.drand48(), rng.drand48());
	}
	Sample RandomSampler::getSample() {
		return Sample(rng);
	}

	UniquePtr<Sampler> RandomSampler::clone(const int seed) const {
		return MakeUnique<RandomSampler>(seed);
	}
}