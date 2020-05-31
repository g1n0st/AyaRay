#include <Films/FilmRHF.h>

namespace Aya {
	const float FilmRHF::Histogram::MAX_VAL = 7.5f;

	void FilmRHF::init(int width, int height, Filter *filter) {
		Film::init(width, height, filter);
		m_sampleHistogram.init(width, height);

		m_maxDist = { .6f };
		m_halfPatchSize = { 1 };
		m_halfWindowSize = { 6 };
		m_scale = { 3 };
	}

	void FilmRHF::resize(int width, int height) {
		Film::resize(width, height);
		m_sampleHistogram.free();
		m_sampleHistogram.init(width, height);
	}

	void FilmRHF::free() {
		Film::free();
		m_sampleHistogram.free();
	}

	void FilmRHF::clear() {
		Film::clear();
		concurrency::parallel_for(0, m_height, [this](int y) {
			for (int x = 0; x < m_width; x++) {
				m_sampleHistogram.totalWeights(x, y) = 0.f;
				for (int i = 0; i < Histogram::NUM_BINS; i++) {
					m_sampleHistogram.histogramWeights[i](x, y) = RGBSpectrum(0.f);
				}
			}
		});
	}

	void FilmRHF::addSample(float x, float y, const Spectrum &sample) {
		x -= .5f;
		y -= .5f;
		int min_x = CeilToInt(x - mp_filter->getRadius());
		int max_x = FloorToInt(x + mp_filter->getRadius());
		int min_y = CeilToInt(y - mp_filter->getRadius());
		int max_y = FloorToInt(y + mp_filter->getRadius());
		min_x = Max(0, min_x);
		max_x = Min(max_x, m_width - 1);
		min_y = Max(0, min_y);
		max_y = Min(max_y, m_height - 1);

		for (int i = min_y; i <= max_y; i++) {
			for (int j = min_x; j <= max_x; j++) {
				int row = m_height - 1 - j;
				int col = j;
				Pixel &pixel = m_accumulateBuffer(col, row);

				float weight = mp_filter->evaluate(j - x, i - y);
				RGBSpectrum weighted_sample = Spectrum(weight * sample).toRGBSpectrum();
				
				weighted_sample[0] = Max(0.f, weighted_sample[0]);
				weighted_sample[1] = Max(0.f, weighted_sample[1]);
				weighted_sample[2] = Max(0.f, weighted_sample[2]);
				RGBSpectrum normalized_sample = weighted_sample.pow(INV_GAMMA) / Histogram::MAX_VAL;

				const float S = 2.f;
				normalized_sample[0] = Min(S, normalized_sample[0]);
				normalized_sample[1] = Min(S, normalized_sample[1]);
				normalized_sample[2] = Min(S, normalized_sample[2]);

				std::array<float, 3> bin = {
					float(Histogram::NUM_BINS - 2) * normalized_sample[0], 
					float(Histogram::NUM_BINS - 2) * normalized_sample[1],
					float(Histogram::NUM_BINS - 2) * normalized_sample[2] };
				std::array<int, 3> bin_low = {
					FloorToInt(bin[0]),
					FloorToInt(bin[1]),
					FloorToInt(bin[2]) };

				for (auto c = 0; c < 3; ++c) {
					float weight_H = { 0.f }, weight_L = { 0.f };
					if (bin_low[c] < Histogram::NUM_BINS - 2) {
						weight_H = bin[c] - bin_low[c];
						weight_L = 1.f - weight_H;

						m_sampleHistogram.histogramWeights[bin_low[c]](col, row)[c] += weight_L;
						m_sampleHistogram.histogramWeights[bin_low[c] + 1](col, row)[c] += weight_H;
					}
					else {
						weight_H = (normalized_sample[c] - 1.f) / (S - 1.f);
						weight_L = 1.f - weight_H;

						m_sampleHistogram.histogramWeights[Histogram::NUM_BINS - 2](col, row)[c] += weight_L;
						m_sampleHistogram.histogramWeights[Histogram::NUM_BINS - 1](col, row)[c] += weight_H;
					}
				}

				m_sampleHistogram.totalWeights(col, row) += 1.f;
			}
		}
	}

	void FilmRHF::denoise() {
		BlockedArray<RGBSpectrum> scaled_image;
		BlockedArray<RGBSpectrum> prev_image;
		Histogram scaled_histogram;

		float total_weight = { 0.f };
		for (size_t i = 0; i < m_sampleHistogram.totalWeights.linearSize(); ++i) {
			total_weight += m_sampleHistogram.totalWeights.data()[i];
		}

		for (auto s = m_scale - 1; s >= 0; --s) {
			float scale = 1.f / float(1 << s);
			if (s > 0) {
				concurrency::parallel_for(0, Histogram::NUM_BINS, [this, scale](int i) {
					gaussianDownSample(m_sampleHistogram.histogramWeights[i], m_sampleHistogram.histogramWeights[i], scale);
				});
				gaussianDownSample(m_sampleHistogram.totalWeights, m_sampleHistogram.totalWeights, scale);

				float scaled_total_weight = { 0.f };
				for (size_t i = 0; i < scaled_histogram.totalWeights.linearSize(); ++i) {
					scaled_total_weight += scaled_histogram.totalWeights.data()[i];
				}

				float ratio = total_weight / scaled_total_weight;
				concurrency::parallel_for(0, Histogram::NUM_BINS, [this, ratio, &scaled_histogram](int b) {
					for (size_t i = 0; i < scaled_histogram.histogramWeights[b].linearSize(); ++i) {
						scaled_histogram.histogramWeights[b].data()[i] *= ratio;
					}
				});

				gaussianDownSample(m_pixelBuffer, scaled_image, scale);
			}
			else {
				scaled_image = m_pixelBuffer;
				scaled_histogram = m_sampleHistogram;
			}

			histogramFusion(scaled_image, scaled_histogram);

			if (s < m_scale - 1) {
				BlockedArray<RGBSpectrum> pos_term;
				pos_term.init(scaled_image.u(), scaled_image.v());
				bicubicInterpolation(prev_image, pos_term);

				BlockedArray<RGBSpectrum> neg_termD;
				gaussianDownSample(scaled_image, neg_termD, .5f);
				BlockedArray<RGBSpectrum> neg_term;
				neg_term.init(scaled_image.u(), scaled_image.v());
				bicubicInterpolation(neg_termD, neg_term);

				for (size_t i = 0; i < scaled_image.linearSize(); ++i) {
					scaled_image.data()[i] += pos_term.data()[i] - neg_term.data()[i];
				}
			}

			prev_image = scaled_image;
		}

		m_pixelBuffer = scaled_image;
	}
}