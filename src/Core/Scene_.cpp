#include "Scene_.h"

namespace Aya {

	Scene_::Scene_(const int & x, const int & y, const int & time) : m_screen_x(x), m_screen_y(y), m_sample_times(time) {}

	void Scene_::render(const char *output) {

		std::ofstream fout(
			output
		);

		fout << "P3\n" << m_screen_x << " " << m_screen_y << "\n255\n";

		MitchellNetravaliFilter *filter = new MitchellNetravaliFilter();
		//BoxFilter *filter = new BoxFilter();
		Film film(m_screen_x, m_screen_y, filter);
		
		Point3 pp;
		int st_time = clock();

		int total = m_sample_times, loading = 0;
		for (int s = 0; s < m_sample_times; s++) {
			for (int j = m_screen_y - 1; j >= 0; j--) {
				for (int i = 0; i < m_screen_x; i++) {
					Spectrum col;
					float u = float(i + rng.drand48());
					float v = float(j + rng.drand48());

					Ray r = m_cam->getRay(u / m_screen_x, v / m_screen_y);
					film.addSample(u, v, m_int->li(r, m_acc, 0));
				}
			}
			loading++;
			std::cout << loading << " / " << total << "(" << (float)loading / (float)total << ")\n";
			film.addSampleCount();
		}
		film.updateDisplay();
		const RGBSpectrum * offset = film.getPixelBuffer();
		int cnt = 0;
		for (int j = 0; j < m_screen_y; j++)
			for (int i = 0; i < m_screen_x; i++) {
				byteSpectrum b = offset[cnt];
				fout << b.r << ' ' << b.g << ' ' << b.b << std::endl;
				cnt++;
			}
		Bitmap::save("output.bmp", (float*)film.getPixelBuffer(), m_screen_x, m_screen_y, RGBA_32);
		int ed_time = clock();
		std::cout << "used " << (float)(ed_time - st_time) / 1000.0f << " sec.\n";
	}
}