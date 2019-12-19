#include "material.h"
#include "memory.h"
#include "camera.h"
#include "integrator.h"
#include "../shapes/rectangle.h"
#include "../shapes/sphere.h"
#include "../shapes/triangle.h"
#include "../accelerators/BVH.h"



#include <iostream>
#include <fstream>
using Aya::Point3;

Point3 p_sb[] = {
Point3(130.0f,165.0f,65.0f),
Point3(82.0,165.0,225.0),
Point3(240.0,165.0,272.0),
Point3(290.0,165.0,114.0),

Point3(290.0, 0.0,114.0),
Point3(290.0, 165.0, 114.0),
Point3(240.0, 165.0, 272.0),
Point3(240.0,   0.0, 272.0),

Point3(130.0,   0.0 , 65.0),
Point3(130.0 ,165.0 , 65.0),
Point3(290.0 ,165.0 ,114.0),
Point3(290.0 ,  0.0, 114.0),

 Point3(82.0 ,  0.0, 225.0),
 Point3(82.0 ,165.0, 225.0),
Point3(130.0, 165.0 , 65.0),
Point3(130.0 ,  0.0,  65.0),

Point3(240.0,   0.0 ,272.0),
Point3(240.0 ,165.0, 272.0),
 Point3(82.0 ,165.0, 225.0),
 Point3(82.0 ,  0.0, 225.0)
};
int vert_sb[] = {
	0, 1, 2,
	2, 3, 0,

	4, 5, 6,
	4, 6, 7,

	8, 9, 10,
	8, 10, 11,

	12, 13, 14,
	12, 14, 15,

	16, 17, 18,
	16, 18, 19
};
using Aya::Shape;
using Aya::SharedPtr;
using Aya::Transform;
using Aya::Material;
using Aya::Spectrum;
using Aya::Vector3;
int main() {

	int nx = 10, ny = 10;
	int sample_times = 1;

	std::ofstream fout(
		"sample.ppm"
	);
	fout << "P3\n" << nx << " " << ny << "\n255\n";

	Transform t1 = Transform().setTranslate(0, 0, 0);
	Transform _t1 = t1.inverse();
	

	Transform t2 = Transform().setTranslate(0, 50, -50) * Transform().setScale(0.2f, 0.2f, 0.2f);
	Transform _t2 = t2.inverse();


	SharedPtr<Shape> shape0(new Aya::TriangleMesh(&t1, &_t1, 10, 20, vert_sb, p_sb, NULL, NULL));
	SharedPtr<Shape> shape1(new Aya::TriangleMesh(&t2, &_t2, 10, 20, vert_sb, p_sb, NULL, NULL));

	SharedPtr<Material> mat1 = new Aya::LambertianMaterial(new Aya::ConstantTexture(Spectrum(1, 1, 1)));
	SharedPtr<Material> mat2 = new Aya::MentalMaterial(Spectrum(0.8, 0.8, 0.8), 0.0f);

	std::vector<SharedPtr<Aya::Primitive> > prims;
	prims.push_back(new Aya::GeometricPrimitive(shape0, mat2));
	prims.push_back(new Aya::GeometricPrimitive(shape1, mat1));

	Aya::Accelerator *acc = new Aya::BVHAccel();
	acc->construct(prims);

	Aya::ProjectiveCamera cam(Point3(278, 273, -800), Vector3(0, 0, 1), Vector3(0, 1, 0), 40.f, float(nx) / float(ny), 0.0f, 0.035f, 0.f, 0.f);
	Aya::SampleIntegrator integrator;
	Aya::RNG rng;

	Aya::Ray ray = cam.getRay(0.6f, 0.2f);
	integrator.li(ray, acc, 0);
	//return 0;

	int st_time = clock();

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