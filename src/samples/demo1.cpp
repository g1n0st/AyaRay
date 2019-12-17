#include "material.h"
#include "camera.h"
#include "shape.h"
#include "integrator.h"
#include "spectrum.h"

#include <iostream>
#include <fstream>
int main() {

	int nx = 800, ny = 800;
	int sample_times = 100;

	std::ofstream fout(
		"sample.ppm"
	);
	fout << "P3\n" << nx << " " << ny << "\n255\n";

	Transform t1;
	t1.setTranslate(7, 0, 0);
	Transform _t1;
	_t1 = t1.inverse();
	Transform t2;
	t2.setTranslate(6, -1, 0);
	Transform _t2;
	_t2 = t2.inverse();
	Transform t3;
	t3.setTranslate(6, 1, 0.7f);
	Transform _t3;
	_t3 = t3.inverse();
	Transform t4;
	t4.setTranslate(10, 0, -12);
	Transform _t4;
	_t4 = t4.inverse();
	Transform t5;
	t5.setTranslate(3, 1, 0);
	Transform _t5;
	_t5 = t5.inverse();
	Transform t6;
	t6.setTranslate(6, -1, -1);
	Transform _t6;
	_t6 = t6.inverse();

	Transform t7;
	t7.setTranslate(6.3f, 0.3f, 1.25f);
	Transform _t7;
	_t7 = t7.inverse();
	Transform t8;
	t8.setTranslate(6.3f, -0.6f, 1.3f);
	Transform _t8;
	_t8 = t8.inverse();

	Shape ** shape = new Shape*[10];
	shape[1] = new Sphere(&t5, &_t5, 0.6f, new DielectricMaterial(1.0f));
	shape[0] = new Sphere(&t1, &_t1, 1.0f, new MentalMaterial(Spectrum(0.7f, 0.7f, 0.7f), 0.f));
	shape[2] = new Sphere(&t2, &_t2, 0.4f, new LambertianMaterial(
		new CrossTexture(new ConstantTexture(Spectrum(1, 0, 0)), new ConstantTexture(Spectrum(0, 0, 1)))));
	shape[3] = new Sphere(&t3, &_t3, 0.15f, new DiffuseLight(new ConstantTexture(Spectrum(.5f, .5f, .5f))));
	shape[4] = new Sphere(&t4, &_t4, 10.00f, new LambertianMaterial(new NoiseTexture(1.f)));

	shape[5] = new Sphere(&t6, &_t6, 0.4f, new MentalMaterial(Spectrum(0.7f, 0.7f, 0.7f), 0.3f));

	shape[6] = new Sphere(&t7, &_t7, 0.3f, new LambertianMaterial(new ConstantTexture(Spectrum(0.0, 0.3, 1.0))));
	shape[7] = new Sphere(&t8, &_t8, 0.3f, new LambertianMaterial(new ConstantTexture(Spectrum(0.7, 0.6, 0.5))));

	int shape_size = 8;

	ProjectiveCamera cam(Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 0, 1), 40.f, float(nx) / float(ny), 0.0f, 1.f, 0.f, 0.f);
	SampleIntegrator integrator;
	RNG rng;

	int st_time = clock();

	Ray dr = cam.getRay(0.5, 0.5);
	//std::cout << dr << std::endl;
	//integrator.li(dr, shape, shape_size, 0);
	//return 0;
	//integrator.li(dr, shape, 4, 0);
	int total = ny, loading = 0;
	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {

			Spectrum col(0, 0, 0);
			for (int s = 0; s < sample_times; s++) {
				float u = float(i + rng.drand48()) / float(nx);
				float v = float(j + rng.drand48()) / float(ny);

				Ray r = cam.getRay(u, v);
				//std::cout << r << std::endl;
				col += integrator.li(r, shape, shape_size, 0);
			}
			col /= sample_times;
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
	return 0;
}