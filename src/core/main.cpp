#include "material.h"
#include "memory.h"
#include "camera.h"
#include "scene.h"
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

void readFromFile(const char *name) {
	std::vector <Aya::Point3> P;
	std::vector <Aya::Normal3> N;
	std::vector<int> V;
	int vs = 0, ts = 0;
	std::ifstream fin(
		name
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
using Aya::Point3;
using Aya::Normal3;
Normal3 nnn[] = {
	Normal3(0.9534f, 0, 0.301709f),
	Normal3(0.9534f, 0, 0.301709f),
	Normal3(0.9534f, 0, 0.301709f),
	Normal3(0.9534f, 0, 0.301709f)
};
Point3 ppp[] = {
	Point3(290.0,   0.0,114.0),
Point3(290.0, 165.0, 114.0),
Point3(240.0, 165.0, 272.0),
Point3(240.0,   0.0, 272.0),

Point3(130.0,165.0 , 65.0),
Point3(82.0, 165.0 ,225.0),
Point3(240.0, 165.0, 272.0),
Point3(290.0,165.0 ,114.0),

Point3(130.0,   0.0 , 65.0),
Point3(130.0, 165.0 , 65.0),
Point3(290.0, 165.0 ,114.0),
Point3(290.0,   0.0 ,114.0),

Point3(82.0 ,  0.0,225.0),
Point3(82.0 ,165.0, 225.0),
Point3(130.0, 165.0,  65.0),
Point3(130.0,  0.0,  65.0),

Point3(240.0,   0.0, 272.0),
Point3(240.0, 165.0 ,272.0),
Point3(82.0, 165.0, 225.0),
Point3(82.0 ,  0.0, 225.0)
};
int vvv[] = {
	0, 1, 2,
	2, 3, 0,

	4, 5, 6,
	6, 7, 4,

	8, 9, 10,
	10, 11, 8,

	12, 13, 14,
	14, 15, 12,

	16, 17, 18,
	18, 19, 16 };

int main() {

	readFromSimFile();

	int nx = 200, ny = 200;
	int sample_times = 500;

	Aya::Transform t0 = Aya::Transform().setTranslate(5, 0, 0);
	//* Aya::Transform().setEulerZYX(-20, 90, 90);
	Aya::Transform _t0 = t0.inverse();

	Aya::Transform t1 = Aya::Transform().setTranslate(2.3, -0.86, 0) * Aya::Transform().setScale(8, 8, 8) * Aya::Transform().setEulerZYX(0, -M_PI_2, 0);
		//* Aya::Transform().setEulerZYX(-20, 90, 90);
	Aya::Transform _t1 = t1.inverse();

	Aya::Transform t11 = Aya::Transform().setTranslate(4, -1, 0);
	//* Aya::Transform().setEulerZYX(-20, 90, 90);
	Aya::Transform _t11 = t11.inverse();


	Aya::Transform t2 = Aya::Transform().setTranslate(7, 3, 0);
	Aya::Transform _t2 = t2.inverse();

	Aya::Transform t3 = Aya::Transform().setTranslate(5, 0, 0);
	Aya::Transform _t3 = t3.inverse();

	Aya::Transform t4 = Aya::Transform().setTranslate(0, -10, 0);
	Aya::Transform _t4 = t4.inverse();

	//Aya::SharedPtr<Aya::Shape> shape0(new Aya::TriangleMesh(&t0, &_t0, 1, 4, vvv, ppp, nnn, NULL));
	Aya::SharedPtr<Aya::Shape> shape0(new Aya::TriangleMesh(&t0, &_t0, TS, VS, VV, PP, NULL, NULL));
	Aya::SharedPtr<Aya::Shape> shape1(new Aya::Rectangle(&t1, &_t1, 1, 1, 1));
	Aya::SharedPtr<Aya::Shape> dub(new Aya::Sphere(&t2, &_t2, 0.7f));
	Aya::SharedPtr<Aya::Shape> light(new Aya::Sphere(&t11, &_t11, 1.f));
	Aya::SharedPtr<Aya::Shape> box(new Aya::Rectangle(&t4, &_t4, 400.0f, 18.5f, 400.0f));

	Aya::SharedPtr<Aya::Material> mental = new Aya::MentalMaterial(Aya::Spectrum(191.0 / 256.0, 173.0 / 256.0, 111.0 / 256.0), 0.0f);
	Aya::SharedPtr<Aya::Material> mat = new Aya::LambertianMaterial(new Aya::ConstantTexture(Aya::Spectrum(0.3, 0.3, 0.3)));
	Aya::SharedPtr<Aya::Material> mat1 = new Aya::LambertianMaterial(new Aya::ConstantTexture(Aya::Spectrum(0.8, 0.8, 0.8)));
	Aya::SharedPtr<Aya::Material> glass = new Aya::DielectricMaterial(1.55f);
	Aya::SharedPtr<Aya::Material> mat4 = new Aya::DiffuseLight(new Aya::ConstantTexture(Aya::Spectrum(1.0f, 1.0f, 1.0f)));

	std::vector<Aya::SharedPtr<Aya::Primitive> > prims;
	prims.push_back(new Aya::GeometricPrimitive(shape0, mat1));
	//prims.push_back(new Aya::GeometricPrimitive(dub, mat2));
	prims.push_back(new Aya::GeometricPrimitive(light, mat4));
	//prims.push_back(new Aya::GeometricPrimitive(box, mat2));

	Aya::Accelerator *acc = new Aya::BVHAccel();

	Aya::ProjectiveCamera *cam = new Aya::ProjectiveCamera(Aya::Vector3(0, -0.25, 0), Aya::Vector3(1, 0, 0), Aya::Vector3(0, 1, 0), 40.f, float(nx) / float(ny), 0.0f, 1e10f, 0.f, 0.f);
	Aya::Integrator *inte = new Aya::SampleIntegrator();
	Aya::Scene scene(500, 500, 200);
	scene.m_acc = acc;
	scene.m_acc->construct(prims);
	scene.m_cam = cam;

	scene.m_int = inte;

	scene.render("output.ppm");
	//Aya::ProjectiveCamera cam(Aya::Point3(278, 273, -800), Aya::Vector3(0, 0, 1), Aya::Vector3(0, 1, 0), 45.f, float(nx) / float(ny), 0.0f, 2.f, 0.f, 0.f);
	

	return 0;
}