#ifndef AYA_CORE_FILM_H
#define AYA_CORE_FILM_H

#include <Core/Config.h>
#include <Core/Filter.h>
#include <Core/Memory.h>
#include <Core/Spectrum.h>
#include <Math/Vector2.h>

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
		uint32_t m_sampleCount;
		BlockedArray<RGBSpectrum> m_pixelBuffer;
		BlockedArray<Pixel> m_accumulateBuffer;
		std::unique_ptr<Filter> mp_filter;

		mutable std::mutex m_mt;

		static const float INV_GAMMA;

	public:
		Film() = default;
		Film(int width, int height, Filter *filter) {
			init(width, height, filter);
		}
		virtual ~Film() {
			free();
		}
		virtual void init(int width, int height, Filter *filter);
		virtual void resize(int width, int height);
		virtual void free();

		virtual void clear();
		int getPixelCount() const {
			return m_width * m_height;
		}
		Vector2i getSize() const {
			return { m_width, m_height };
		}

		virtual void addSample(float x, float y, const Spectrum &L);
		virtual void addFilm(const Film *film, float weight = 1.f);
		virtual void splat(float x, float y, const Spectrum &L);
		void updateDisplay(const float splat_scale = 0.f);
		inline void addSampleCount() {
			++m_sampleCount;
			printf("\033[01;31m %d spp(s) \033[0m is rendered\n", m_sampleCount);
		}

		const RGBSpectrum* getPixelBuffer() const {
			return m_pixelBuffer.data();
		}
		const Spectrum getPixel(int x, int y) const {
			const Pixel &pixel = m_accumulateBuffer(x, y);
			return pixel.color / (pixel.weight + float(AYA_EPSILON)) + pixel.splat / static_cast<float>(m_sampleCount);
		}
		void setPixel(int x, int y, const Spectrum &L) {
			Pixel &pixel = m_accumulateBuffer(x, y);
			pixel.color = L;
			pixel.weight = 1.f;
			pixel.splat = Spectrum(0.f);
		}
		const int getSampleCount() const {
			return m_sampleCount;
		}
		virtual void denoise() {}
	};
}

#endif