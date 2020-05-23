#include <Core/Sampler.h>

namespace Aya {
	Sample::Sample(RNG &rng) {
		u = rng.drand48();
		v = rng.drand48();
		w = rng.drand48();
	}
}