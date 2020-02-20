#ifndef AYA_CORE_FILM_H
#define AYA_CORE_FILM_H

#include "Filter.h"
#include "Memory.h"
#include "Spectrum.h"

#include <ppl.h>

namespace Aya {
	class Film {
	public :
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

		// there may need a thread lock

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
			return m_pixel_buffer.linearSize();
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