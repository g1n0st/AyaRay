#ifndef AYA_CORE_FILM_H
#define AYA_CORE_FILM_H

#include "../Core/Filter.h"
#include "../Core/Memory.h"
#include "../Core/Spectrum.h"

#include <ppl.h>
#include <thread>
#include <mutex>

namespace Aya {
	class Film {
	protected :
		struct Pixel {
			Spectrum color;
			Spectrum splat;
			float weight;
		};

		int m_width, m_height;
		uint32_t m_sample_count;
		BlockedArray<RGBSpectrum> m_pixel_buffer;
		BlockedArray<Pixel> m_accumulate_buffer;
		UniquePtr<Filter> mp_filter;

		mutable std::mutex m_mt;

		static const float INV_GAMMA;

	public:
		Film() {}
		Film(int width, int height, Filter * filter) {
			init(width, height, filter);
		}
		virtual ~Film() {
			release();
		}
		virtual void init(int width, int height, Filter *filter);
		virtual void resize(int width, int height);
		virtual void clear();

		void release();
		int getPixelCount() const {
			return m_width * m_height;
		}

		virtual void addSample(float x, float y, const Spectrum &L);
		virtual void splat(float x, float y, const Spectrum &L);
		void updateDisplay(const float splat_scale = 0.f);
		inline void addSampleCount() {
			++m_sample_count;
		}

		const RGBSpectrum* getPixelBuffer() const {
			return m_pixel_buffer.data();
		}
		const int getSampleCount() const {
			return m_sample_count;
		}
		virtual void denoise() {}
	};
}

#endif