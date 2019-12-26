#include "scene.h"

namespace Aya {

	Scene::Scene(const int & x, const int & y, const int & time) : m_screen_x(x), m_screen_y(y), m_sample_times(time) {}

	void Scene::render(const char *output) {

		std::ofstream fout(
			output
		);

		fout << "P3\n" << m_screen_x << " " << m_screen_y << "\n255\n";

		int st_time = clock();

		int total = m_screen_y, loading = 0;
		for (int j = m_screen_y - 1; j >= 0; j--) {
			for (int i = 0; i < m_screen_x; i++) {

				Aya::Spectrum col(0, 0, 0);
				for (int s = 0; s < m_sample_times; s++) {
					float u = float(i + rng.drand48()) / float(m_screen_x);
					float v = float(j + rng.drand48()) / float(m_screen_y);

					Aya::Ray r = m_cam->getRay(u, v);
					//std::cout << r << std::endl;
					col += m_int->li(r, m_acc, 0);
				}
				col /= (float)m_sample_times;
				col.sqrt();
				int ir = (int)(255.99f * col[0]);
				int ig = (int)(255.99f * col[1]);
				int ib = (int)(255.99f * col[2]);
				fout << ir << ' ' << ig << ' ' << ib << std::endl;
			}
			loading++;
			std::cout << loading << " / " << total << "(" << (float)loading / (float)total << ")\n";
		}

		int ed_time = clock();
		std::cout << "used " << (float)(ed_time - st_time) / 1000.0f << " sec.\n";
	}
}