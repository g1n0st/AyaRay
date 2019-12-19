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

int *VV;
Aya::Point3 *PP;
Aya::Normal3 *NN;
int VS, TS;

void readFromFile() {
	std::vector <Aya::Point3> P;
	std::vector <Aya::Normal3> N;
	std::vector<int> V;
	int vs = 0, ts = 0;
	std::ifstream fin(
		"bunny.obj"
	);
	char str[512];
	while (fin.getline(str, 512)) {
		char type1, type2;
		float a, _a, b, _b, c, _c;
		type1 = str[0];
		type2 = str[1];
		if (type1 == 'v' && type2 == ' ') {
			sscanf(str + 2, "%f%f%f", &a, &b, &c);
			P.push_back(Aya::Point3(a, b, c));
			vs++;

		}
		else if (type1 == 'v' && type2 == 'n') {
			sscanf(str + 3, "%f%f%f", &a, &b, &c);
			N.push_back(Aya::Normal3(a, b, c));
		}
		else {
			int a, b, c, _a, _b, _c;
			sscanf(str + 2, "%d//%d %d//%d %d//%d", &a, &_a, &b, &_b, &c, &_c);
			V.push_back(a - 1);
			V.push_back(b - 1);
			V.push_back(c - 1);
			ts++;
		}
	}
	VS = vs;
	TS = ts;
	VV = new int[V.size()];
	for (int i = 0; i < V.size(); i++) VV[i] = V[i];
	NN = new Aya::Normal3[N.size()];
	for (int i = 0; i < N.size(); i++) NN[i] = N[i];
	PP = new Aya::Point3[P.size()];
	for (int i = 0; i < P.size(); i++) PP[i] = P[i];
}

void readFromSimFile() {
	std::vector <Aya::Point3> P;
	std::vector<int> V;
	int vs = 0, ts = 0;
	std::ifstream fin(
		"teapot.obj"
	);
	char str[512];
	while (fin.getline(str, 512)) {
		char type1, type2;
		float a, b, c;
		type1 = str[0];
		if (type1 == 'v') {
			sscanf(str + 2, "%f%f%f", &a, &b, &c);
			P.push_back(Aya::Point3(a, b, c));
			vs++;

		}
		else {
			int a, b, c, _a, _b, _c;
			sscanf(str + 2, "%d %d %d", &a, &b, &c);
			V.push_back(a - 1);
			V.push_back(b - 1);
			V.push_back(c - 1);
			ts++;
		}
	}
	VS = vs;
	TS = ts;
	VV = new int[V.size()];
	for (int i = 0; i < V.size(); i++) VV[i] = V[i];
	PP = new Aya::Point3[P.size()];
	for (int i = 0; i < P.size(); i++) PP[i] = P[i];
}

int main() {

	readFromFile();

	int nx = 100, ny = 100;
	int sample_times = 800 * 4;

	std::ofstream fout(
		"sample.ppm"
	);
	fout << "P3\n" << nx << " " << ny << "\n255\n";

	Aya::Transform t1 = Aya::Transform().setTranslate(7, 0, 0);
		//* Aya::Transform().setEulerZYX(-20, 90, 90);
	Aya::Transform _t1 = t1.inverse();


	Aya::Transform t2 = Aya::Transform().setTranslate(15, 1, 0.21);
	Aya::Transform _t2 = t2.inverse();

	Aya::Transform t3 = Aya::Transform().setTranslate(12.5, 3.5, -2);
	Aya::Transform _t3 = t3.inverse();

	Aya::Transform t4 = Aya::Transform().setTranslate(0, -10, 0);
	Aya::Transform _t4 = t4.inverse();


	Aya::SharedPtr<Aya::Shape> shape0(new Aya::TriangleMesh(&t1, &_t1, TS, VS, VV, PP, NN, NULL));
	Aya::SharedPtr<Aya::Shape> shape1(new Aya::Rectangle(&t1, &_t1, 1, 1, 1));
	Aya::SharedPtr<Aya::Shape> shape2(new Aya::Sphere(&t2, &_t2, 1.5f));
	Aya::SharedPtr<Aya::Shape> light(new Aya::Sphere(&t3, &_t3, 0.7f));
	Aya::SharedPtr<Aya::Shape> box(new Aya::Rectangle(&t4, &_t4, 400.0f, 18.5f, 400.0f));

	Aya::SharedPtr<Aya::Material> mat2 = new Aya::MentalMaterial(Aya::Spectrum(191.0 / 256.0, 173.0 / 256.0, 111.0 / 256.0), 0.0f);
	Aya::SharedPtr<Aya::Material> mat = new Aya::LambertianMaterial(new Aya::ConstantTexture(Aya::Spectrum(0.3, 0.3, 0.3)));
	Aya::SharedPtr<Aya::Material> mat1 = new Aya::LambertianMaterial(new Aya::ConstantTexture(Aya::Spectrum(1, 0.3, 0.3)));
	Aya::SharedPtr<Aya::Material> mat3 = new Aya::DielectricMaterial(2.417f);
	Aya::SharedPtr<Aya::Material> mat4 = new Aya::DiffuseLight(new Aya::ConstantTexture(Aya::Spectrum(1.0f, 1.0f, 1.0f)));

	std::vector<Aya::SharedPtr<Aya::Primitive> > prims;
	prims.push_back(new Aya::GeometricPrimitive(shape0, mat2));
	//prims.push_back(new Aya::GeometricPrimitive(shape2, mat1));
	prims.push_back(new Aya::GeometricPrimitive(light, mat4));
	//prims.push_back(new Aya::GeometricPrimitive(box, mat2));

	Aya::Accelerator *acc = new Aya::BVHAccel();
	acc->construct(prims);

	Aya::ProjectiveCamera cam(Aya::Vector3(0, -0.25, 0), Aya::Vector3(1, 0, 0), Aya::Vector3(0, 1, 0), 40.f, float(nx) / float(ny), 0.0f, 1.f, 0.f, 0.f);
	Aya::SampleIntegrator integrator;
	Aya::RNG rng;

	Aya::Ray ray = cam.getRay(0.5f, 0.6f);
	integrator.li(ray, acc, 0);

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