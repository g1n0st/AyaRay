#ifndef AYA_SAMPLER_SOBOLSAMPLER_H
#define AYA_SAMPLER_SOBOLSAMPLER_H

#include <Samplers/SobolMatrices.h>
#include <Core/Sampler.h>

namespace Aya {
	class SobolSampler : public Sampler {
	private:
		int m_res, m_log2Res;
		uint64_t m_sampleIdx, m_sobolIdx;
		uint32_t m_dim;
		uint64_t m_scramble;

		mutable RNG rng;

	public:
		SobolSampler() = default;
		SobolSampler(const int rx, const int ry) :
			m_sampleIdx(0), m_sobolIdx(0), m_dim(0), m_scramble(0) {
			m_res = RoundUpToPowerOfTwo(Max(rx, ry));
			m_log2Res = FloorLog2(m_res);
			assert(m_res == 1 << m_log2Res);
		}
		SobolSampler(const int res, const int log2_res, const uint64_t scramble, const uint64_t sample_idx) :
			m_sampleIdx(sample_idx), m_res(res), m_log2Res(log2_res), m_scramble(scramble),
			m_sobolIdx(0), m_dim(0) {}

		void generateSamples(
			const int pixel_x,
			const int pixel_y,
			CameraSample *samples,
			RNG &rng
		) override;
		void advanceSampleIndex() override;

		void startPixel(const int pixel_x, const int pixel_y) override;
		float get1D() override;
		Vector2f get2D() override;
		Sample getSample() override;

		UniquePtr<Sampler> clone(const int seed) const override;
		UniquePtr<Sampler> deepClone() const override;

	private:
		uint64_t enumerateSampleIndex(const uint32_t pixel_x, const uint32_t pixel_y) const;
		float sobolSample(const int dim) const;
	};
}
#endif