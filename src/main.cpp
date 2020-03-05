#include "Core/Scene_.h"
#include "Core/Parser.h"

#include "Core/Medium.h"
#include "Media/Homogeneous.h"
#include "Core\BSDF.h"
#include "BSDFs\LambertianDiffuse.h"
#include "BSDFs\Glass.h"
#include "BSDFs\Mirror.h"

#include "Accelerators\BVH.h"
#include "Core/Scene.h"

#include "Lights\AreaLight.h"
#include "Lights\EnvironmentLight.h"
void ayaInit() {
	Aya::SampledSpectrum::init();
}
void ayaCleanUp() {
}

void ayaMain(int argc, char **argv) {
	if (argc != 3) {
		printf("the argv is error.\n");
		exit(1);
	}
	
	ayaInit();

	Aya::Scene_ *scene = new Aya::Scene_();
	Aya::Parser parser;
	parser.load(argv[1]);
	parser.run(scene);
	scene->render(argv[2]);

	ayaCleanUp();
}

using namespace Aya;

int main(int argc, char **argv) {
	Transform o2w = Transform().setEulerZYX(-1.3, 1.4, -1.8) * Transform().setScale(0.44, 0.44, 0.44);
	Primitive primitive;
	primitive.loadMesh(o2w, "teapot.obj");
	std::vector<Primitive*> prims;
	prims.push_back(&primitive);

	Scene scene;
	scene.addPrimitive(&primitive);
	scene.initAccelerator();

	MitchellNetravaliFilter *filter = new MitchellNetravaliFilter();
	ProjectiveCamera cam(Point3(-5, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 1),
		40, 1, 0, 1, 0, 0);
	int testnum = 1000;
	Film film(testnum, testnum, filter);
	SurfaceIntersection si;
	for (int i = 0; i < testnum; i++)
		for (int j = 0; j < testnum; j++) {
			Ray r = cam.getRay(float(i) / float(testnum), float(j) / float(testnum));
			r.m_dir.normalized();
			if (scene.intersect(r, &si)) film.addSample(i, j, Spectrum::fromRGB(1.f, 1.f, 1.f));
			else film.addSample(i, j, Spectrum::fromRGB(0.f, 0.f, 0.f));
		}
	film.addSampleCount();
	film.updateDisplay();
	const RGBSpectrum * offset = film.getPixelBuffer();
	Bitmap::save("unittest.bmp", (float*)film.getPixelBuffer(), testnum, testnum, RGBA_32);
	return 0;
}