#ifndef AYA_FILMS_FILMRHF_H
#define AYA_FILMS_FILMRHF_H

#include <Core/Film.h>

#include <array>

namespace Aya {
	class FilmRHF : public Film {
	protected:
		struct Histogram {
			static const int NUM_BINS = 20;
			static const float MAX_VAL;

			BlockedArray<RGBSpectrum> histogramWeights[NUM_BINS];
			BlockedArray<float> totalWeights;

			void init(int w, int h) {
				totalWeights.init(w, h);
				for (size_t i = 0; i < NUM_BINS; i++) {
					histogramWeights[i].init(w, h);
				}
			}

			void free() {
				totalWeights.free();
				for (size_t i = 0; i < NUM_BINS; i++) {
					histogramWeights[i].free();
				}
			}
		};

		static const int MAX_SCALE = 3;
		Histogram m_sampleHistogram;
		float m_maxDist;
		int m_halfPatchSize;
		int m_halfWindowSize;
		int m_scale;

		mutable std::mutex m_RHFLock;

	public:
		void init(int width, int height, Filter *filter) override;
		void resize(int width, int height) override;
		void free() override;
		void clear() override;

		void addSample(float x, float y, const Spectrum &L) override;
		void denoise() override;

	private:
		void histogramFusion(BlockedArray<RGBSpectrum> &input, const Histogram &histogram);
		float chiSquareDistance(const Vector2i &x, const Vector2i &y, const int halfPathSize, const Histogram &histogram);

		template<typename T>
		void gaussianDownSample(const BlockedArray<T> &input, BlockedArray<T> &output, float scale);
		void bicubicInterpolation(const BlockedArray<RGBSpectrum> &input, BlockedArray<RGBSpectrum> &output);
	};
}

#endif