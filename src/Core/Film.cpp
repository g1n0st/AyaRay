#include "Film.h"

using namespace concurrency;

namespace Aya {

	const float Film::INV_GAMMA = .454545f;

	void Film::init(int width, int height, Filter *filter) {
		resize(width, height);
		mp_filter.reset();
		mp_filter = UniquePtr<Filter>(filter);
	}
	void Film::resize(int width, int height) {
		m_width = width;
		m_height = height;

		m_pixel_buffer.free();
		m_pixel_buffer.init(height, width);
		m_accumulate_buffer.free();
		m_accumulate_buffer.init(width, height);
		m_sample_count = 0;
	}
	void Film::release() {
	}
	void Film::clear() {
		m_sample_count = 0;
		m_pixel_buffer.free();
		m_accumulate_buffer.free();
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
				Pixel& pixel = m_accumulate_buffer(j, m_height - 1 - i);
				float weight = mp_filter->evaluate(j - x, i - y);
				pixel.weight += weight;
				pixel.color += weight * L;
			}
		}
	}
	void Film::splat(float x, float y, const Spectrum& L) {
		std::lock_guard<std::mutex> lck(m_mt);

		int xx = Clamp((int)std::floor(x), 0, m_width - 1);
		int yy = Clamp((int)std::floor(y), 0, m_height - 1);
		m_accumulate_buffer(xx, m_height - 1 - yy).splat += L;
	}
	void Film::updateDisplay(const float ss) {
		std::lock_guard<std::mutex> lck(m_mt);

		float splat_scale = ss > 0.f ? ss : m_sample_count;

		parallel_for(0, m_height, [this, splat_scale](int y) {
			for (int x = 0; x < m_width; x++) {
				Pixel pixel = m_accumulate_buffer(x, y);
				pixel.color.clamp();
				m_pixel_buffer(y, x) =
					((Spectrum)(
						pixel.color / (pixel.weight + float(AYA_EPSILON)) + pixel.splat / splat_scale
						).pow(INV_GAMMA)).toRGBSpectrum();
			}
		});
	}
}