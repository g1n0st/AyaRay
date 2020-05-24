#include <Samplers/RandomSampler.h>

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

	std::unique_ptr<Sampler> RandomSampler::clone(const int seed) const {
		return std::make_unique<RandomSampler>(seed);
	}
	std::unique_ptr<Sampler> RandomSampler::deepClone() const {
		RandomSampler *copy = new RandomSampler();
		memcpy_s(copy, sizeof(RandomSampler), this, sizeof(RandomSampler));
		return std::unique_ptr<RandomSampler>(copy);
	}
}