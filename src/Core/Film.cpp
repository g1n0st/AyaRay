#include <Core/Film.h>

namespace Aya {

	const float Film::INV_GAMMA = .454545f;
		
	void Film::init(int width, int height, Filter *filter) {
		resize(width, height);
		mp_filter.reset();
		mp_filter = std::unique_ptr<Filter>(filter);
	}
	void Film::resize(int width, int height) {
		m_width = width;
		m_height = height;

		m_pixelBuffer.free();
		m_pixelBuffer.init(height, width);
		m_accumulateBuffer.free();
		m_accumulateBuffer.init(width, height);
		m_sampleCount = 0;
	}
	void Film::clear() {
		std::lock_guard<std::mutex> lck(m_mt);

		concurrency::parallel_for(0, m_height, [this](int y) {
			for (int x = 0; x < m_width; x++) {
				Pixel &pixel = m_accumulateBuffer(x, y);
				pixel = { Spectrum(0.f), Spectrum(0.f), 0.f };
			}
		});

		m_sampleCount = 0;
	}
	void Film::free() {
		m_sampleCount = 0;
		m_pixelBuffer.free();
		m_accumulateBuffer.free();
	}

	void Film::addSample(float x, float y, const Spectrum& L) {
		std::lock_guard<std::mutex> lck(m_mt);

		x -= .5f;
		y -= .5f;
		int min_x = Clamp((int)std::ceil(x - mp_filter->getRadius()), 0, m_width - 1);
		int max_x = Clamp((int)std::floor(x + mp_filter->getRadius()), 0, m_width - 1);
		int min_y = Clamp((int)std::ceil(y - mp_filter->getRadius()), 0, m_height - 1);
		int max_y = Clamp((int)std::floor(y + mp_filter->getRadius()), 0, m_height - 1);

		for (auto i = min_y; i <= max_y; i++) {
			for (auto j = min_x; j <= max_x; j++) {
				Pixel &pixel = m_accumulateBuffer(j, m_height - 1 - i);
				float weight = mp_filter->evaluate(j - x, i - y);
				pixel.weight += weight;
				pixel.color += weight * L;
			}
		}
	}

	void Film::addFilm(const Film *film, float weight) {
		assert(m_width == film->m_width && m_height == film->m_height);
		std::lock_guard<std::mutex> lck(m_mt);

		concurrency::parallel_for(0, m_height, [this, film, weight](int y) {
			for (int x = 0; x < m_width; x++) {
				Pixel &pixel = m_accumulateBuffer(x, y);
				const Pixel &pixel0 = film->m_accumulateBuffer(x, y);

				pixel.color += pixel0.color * weight;
				pixel.weight += pixel0.weight * weight;
				pixel.splat += pixel0.splat * weight;
			}
		});

		m_sampleCount += film->m_sampleCount;
	}

	void Film::splat(float x, float y, const Spectrum& L) {
		std::lock_guard<std::mutex> lck(m_mt);

		int xx = Clamp((int)std::floor(x), 0, m_width - 1);
		int yy = Clamp((int)std::floor(y), 0, m_height - 1);
		m_accumulateBuffer(xx, m_height - 1 - yy).splat += L;
	}
	void Film::updateDisplay(const float ss) {
		std::lock_guard<std::mutex> lck(m_mt);

		float splat_scale = ss > 0.f ? ss : m_sampleCount;

		concurrency::parallel_for(0, m_height, [this, splat_scale](int y) {
			for (int x = 0; x < m_width; x++) {
				Pixel pixel = m_accumulateBuffer(x, y);
				pixel.color.clamp();

				Spectrum L = ((Spectrum)(
					pixel.color / (pixel.weight + float(AYA_EPSILON)) + pixel.splat / splat_scale
					).pow(INV_GAMMA)).toRGBSpectrum().clamp(0.f, 1.f);
				L[3] = 1.f;
				m_pixelBuffer(y, x) = L;
					
			}
		});
	}
}