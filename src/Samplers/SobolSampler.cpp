#include <Samplers/SobolSampler.h>

namespace Aya {
	void SobolSampler::generateSamples(
		const int pixel_x,
		const int pixel_y,
		CameraSample *samples,
		RNG &rng) {
		assert(samples);

		samples->image_x = sobolSample(m_dim++) * m_res - pixel_x;
		samples->image_y = sobolSample(m_dim++) * m_res - pixel_y;
		samples->lens_u = sobolSample(m_dim++);
		samples->lens_v = sobolSample(m_dim++);
		samples->time = sobolSample(m_dim++);
	}

	void SobolSampler::advanceSampleIndex() {
		m_sampleIdx++;
	}
	void SobolSampler::startPixel(const int px, const int py) {
		m_sobolIdx = enumerateSampleIndex(px, py);
		m_dim = 0;
	}

	float SobolSampler::get1D() {
		return sobolSample(m_dim++);
	}
	Vector2f SobolSampler::get2D() {
		int dim = m_dim;
		Vector2f ret = Vector2f(
			sobolSample(m_dim),
			sobolSample(m_dim + 1)
		);
		m_dim += 2;

		return ret;
	}
	Sample SobolSampler::getSample() {
		int dim = m_dim;
		Sample ret;
		ret.u = sobolSample(m_dim);
		ret.v = sobolSample(m_dim + 1);
		ret.w = sobolSample(m_dim + 2);
		m_dim += 3;

		return ret;
	}

	std::unique_ptr<Sampler> SobolSampler::clone(const int seed) const {
		return std::make_unique<SobolSampler>(m_res, m_log2Res, m_scramble, m_sampleIdx);
	}
	std::unique_ptr<Sampler> SobolSampler::deepClone() const {
		SobolSampler *copy = new SobolSampler();
		memcpy_s(copy, sizeof(SobolSampler), this, sizeof(SobolSampler));
		return std::unique_ptr<SobolSampler>(copy);
	}

	uint64_t SobolSampler::enumerateSampleIndex(const uint32_t px, const uint32_t py) const {
		if (m_log2Res == 0) {
			return 0;
		}

		const uint32_t m2 = m_log2Res << 1;
		uint64_t idx = m_sampleIdx;
		uint64_t idx2 = idx << m2;

		uint64_t delta = 0;
		for (int c = 0; idx; idx >>= 1, c++) {
			if (idx & 1)  // Add flipped column m + c + 1.
				delta ^= VdC_sobol_matrices[m_log2Res - 1][c];
		}

		// Flipped b
		uint64_t b = (((uint64_t)px << m_log2Res) | py) ^ delta;

		for (int c = 0; b; b >>= 1, c++) {
			if (b & 1)  // Add column 2 * m - c.
				idx2 ^= VdC_sobol_matrices_inv[m_log2Res - 1][c];
		}

		return idx2;
	}

	float SobolSampler::sobolSample(const int dimension) const {
		if (dimension < num_sobol_dimensions) {
			uint64_t v = m_scramble;
			uint64_t idx = m_sobolIdx;
			for (int i = dimension * sobol_matrix_size + sobol_matrix_size - 1; idx != 0; idx >>= 1, i--) {
				if (idx & 1)
					v ^= sobol_matrices32[i];
			}

			return v * 2.3283064365386963e-10f; /* 1 / 2^32 */
		}
		else {
			return rng.drand48();
		}
	}
}