#include "material.h"
#include "memory.h"
#include "camera.h"
#include "integrator.h"
#include "../shapes/rectangle.h"
#include "../shapes/sphere.h"
#include "../shapes/rectangle.h"
#include "../accelerators/BVH.h"



#include <iostream>
#include <fstream>
int main() {

	int nx = 600, ny = 600;
	int sample_times = 1000;

	std::ofstream fout(
		"sample.ppm"
	);
	fout << "P3\n" << nx << " " << ny << "\n255\n";

	Aya::Transform t1= Aya::Transform().setTranslate(3, 0.6f, 0.6f) * Aya::Transform().setEulerZYX(20, 40, 50);
	Aya::Transform _t1;
	_t1 = t1.inverse();

	Aya::Transform t2;
	t2.setTranslate(5.2, -0.3, -0.3);
	Aya::Transform _t2;
	_t2 = t2.inverse();

	Aya::Transform t3;
	t3.setTranslate(3, 0.6f, -0.3f);
	Aya::Transform _t3;
	_t3 = t3.inverse();

	Aya::Transform t4;
	t4.setTranslate(0, 0, -9.7f);
	t4 *= Aya::Transform().setRotation(Aya::Vector3(0, 1, 0), 8.0f);
	Aya::Transform _t4;
	_t4 = t4.inverse();

	Aya::SharedPtr<Aya::Shape>  shape1(new Aya::Rectangle(&t1, &_t1, 1, 1, 1));
	Aya::SharedPtr<Aya::Shape>  shape2(new Aya::Sphere(&t2, &_t2, 0.6f));
	Aya::SharedPtr<Aya::Shape>  shape3(new Aya::Sphere(&t3, &_t3, 0.3f));
	Aya::SharedPtr<Aya::Shape>  shape4(new Aya::Rectangle(&t4, &_t4, 18, 18, 18));
	Aya::SharedPtr<Aya::Material> mat1(new Aya::LambertianMaterial(new Aya::NoiseTexture()));
	Aya::SharedPtr<Aya::Material> mat2(new Aya::MentalMaterial(Aya::Spectrum(1.0f, 0.84f, 0.0f), 0.05f));
	Aya::SharedPtr<Aya::Material> mat3(
		new Aya::DiffuseLight(
			new Aya::CrossTexture(
				new Aya::ConstantTexture(Aya::Spectrum(1, 0, 0)), 
				new Aya::NoiseTexture()
				)
			)
		);
	Aya::SharedPtr<Aya::Material> mat4(new Aya::MentalMaterial(Aya::Spectrum(0.1f, 0.1f, 0.95f), 0.05f));
	Aya::GeometricPrimitive **prim = new Aya::GeometricPrimitive* [5];
	prim[0] = new Aya::GeometricPrimitive(shape1, mat3);
	prim[1] = new Aya::GeometricPrimitive(shape2, mat2);
	prim[2] = new Aya::GeometricPrimitive(shape3, mat3);
	prim[3] = new Aya::GeometricPrimitive(shape4, mat4);
	int prim_size = 4;

	Aya::ProjectiveCamera cam(Aya::Vector3(-3, 0, 0.2), Aya::Vector3(1, 0, -0.2), Aya::Vector3(0, 0, 1), 40.f, float(nx) / float(ny), 0.0f, 1.f, 0.f, 0.f);
	Aya::SampleIntegrator integrator;
	Aya::RNG rng;

	int st_time = clock();

	std::vector<Aya::SharedPtr<Aya::Primitive>> pr;
	pr.push_back(prim[0]);
	pr.push_back(prim[1]);
	pr.push_back(prim[2]);
	pr.push_back(prim[3]);
	Aya::BVHAccel *acc = new Aya::BVHAccel();
	acc->construct(pr);

	Aya::Ray r = cam.getRay(0.2, 0.2);
	integrator.li(r, acc, 0);

	int total = ny, loading = 0;
	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {

			Aya::Spectrum col(0, 0, 0);
			for (int s = 0; s < sample_times; s++) {
				float u = float(i + rng.drand48()) / float(nx);
				float v = float(j + rng.drand48()) / float(ny);

				Aya::Ray r = cam.getRay(u, v);
				//std::cout << r << std::endl;
				col += integrator.li(r, acc, 0);
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