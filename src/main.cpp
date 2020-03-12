#include "Core/Scene_.h"
#include "Core/Parser.h"
#include "Core/integrator.h"
#include "Core/Medium.h"
#include <array>
#include "Media/Homogeneous.h"
#include "Core\BSDF.h"
#include "BSDFs\LambertianDiffuse.h"
#include "BSDFs\Glass.h"
#include "Core/Memory.h"
#include "BSDFs\Mirror.h"

#include "Accelerators\BVH.h"
#include "Core\Scene.h"

#include "Lights\AreaLight.h"
#include "Lights\EnvironmentLight.h"
#include "Integrators\DirectLighting.h"

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
	Transform o2w = Transform().setEulerZYX(-0.3, 1.1, -1.1) * Transform().setScale(0.44, 0.44, 0.44);
	Primitive * primitive = new Primitive();
	//primitive->loadMesh(o2w, "teapot.obj", true);
	primitive->loadMesh(o2w, "teapot.obj", true, MakeUnique<Glass>(Spectrum::fromRGB(1.f, 0.7529f, 0.796f), 1.f, 1.5f));
	//primitive->loadMesh(o2w, "teapot.obj", true, MakeUnique<Mirror>(Spectrum::fromRGB(0.95f, 0.95f, 0.95f)));
	//primitive->loadSphere(o2w, 3, MakeUnique<Glass>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	Scene *scene = new Scene();
	scene->addPrimitive(primitive);
	scene->initAccelerator();

	scene->addLight(new EnvironmentLight("uffizi-large.hdr", scene));

	MitchellNetravaliFilter *filter = new MitchellNetravaliFilter();
	ProjectiveCamera *cam = new ProjectiveCamera(Point3(-5, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 1), 40, 1, 0, 1000000, 0, 0);

	int testnumx = 600;
	int testnumy = 600;
	RandomSampler *random_sampler = new RandomSampler();
	SobolSampler *sobol_sampler = new SobolSampler(testnumx, testnumy);

	Film *film = new Film(testnumx, testnumy, filter);

	TaskSynchronizer task(testnumx, testnumy);
	int spp = 40;
	DirectLightingIntegrator *integrator = new DirectLightingIntegrator(task, spp, 5);

	integrator->render(scene, cam, sobol_sampler, film);

	const RGBSpectrum * offset = film->getPixelBuffer();
	Bitmap::save("unittest.bmp", (float*)film->getPixelBuffer(), testnumx, testnumy, RGBA_32);
		return 0;
}