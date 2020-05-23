#ifndef AYA_CORE_SAMPLER_H
#define AYA_CORE_SAMPLER_H

#include <Core/Config.h>
#include <Core/Memory.h>
#include <Core/RNG.h>
#include <Math/Vector2.h>

namespace Aya {
	struct CameraSample {
		float image_x, image_y;
		float lens_u, lens_v;
		float time;
	};

	struct Sample {
		float u, v, w;

		Sample() : u(0.f), v(0.f), w(0.f) {}
		Sample(RNG &rng);
	};

	class Sampler {
	public:
		virtual ~Sampler() = default;

		virtual void generateSamples(
			const int pixel_x,
			const int pixel_y,
			CameraSample *samples,
			RNG &rng
		) = 0;
		virtual void advanceSampleIndex() {}

		virtual void startPixel(const int pixel_x, const int pixel_y) {}
		virtual float get1D() = 0;
		virtual Vector2f get2D() = 0;
		virtual Sample getSample() = 0;

		virtual UniquePtr<Sampler> clone(const int seed) const = 0;
		virtual UniquePtr<Sampler> deepClone() const = 0;
	};
}
#endif