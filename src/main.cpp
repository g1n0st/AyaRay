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
#include "Lights\PointLight.h"

#include "Integrators\DirectLighting.h"
#include "Integrators\PathTracing.h"
#include "Samplers\RandomSampler.h"
#include "Samplers\SobolSampler.h"

#include "Filters\BoxFilter.h"
#include "Filters\GaussianFilter.h"
#include "Filters\MitchellNetravaliFilter.h"
#include "Filters\TriangleFilter.h"

void ayaInit() {
	Aya::SampledSpectrum::init();
}

using namespace Aya;
using namespace std;
int main(int argc, char **argv) {
	ayaInit();
	Transform cb = Transform().setScale(1, 1, 1);
	Transform bunnyc = Transform().setScale(0.2, 0.2, 0.2) * Transform().setTranslate(0, 4, 0);
	Transform idt = Transform() * Transform().setScale(0.004, 0.004, 0.004) * Transform().setTranslate(60, -420, 0);
	Transform o2w = Transform().setTranslate(3, -2.4, 0) * Transform().setScale(0.44, 0.44, 0.44);
	Transform o2w1 = Transform().setTranslate(3, 1.4, 0);
	int st = clock();
	Primitive * primitive = new Primitive();
	Primitive * light_p = new Primitive();
	Primitive *bunny = new Primitive();
	//primitive->loadMesh(o2w, "teapot.obj", true);
	//primitive->loadPlane(o2w, 3, MakeUnique<LambertianDiffuse>(Spectrum(1.f, 1.f, 1.f)));
	light_p->loadMesh(cb, "./cornell-box/light.obj");

	bunny->loadMesh(bunnyc, "bunny.obj", true, MakeUnique<Glass>(Spectrum::fromRGB(1.f, 1.f, 1.f), 1.f, 1.6f));
	//primitive->loadPlane(o2w, 2, MakeUnique<LambertianDiffuse>("cnm777.bmp"));
	//primitive->loadPlane(o2w, 2, MakeUnique<Glass>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//primitive->loadMesh(o2w, "teapot.obj", true, MakeUnique<Glass>(Spectrum::fromRGB(1.f, 0.7529f, 0.796f), 1.f, 1.5f));
	//primitive->loadMesh(o2w, "teapot.obj", true, MakeUnique<LambertianDiffuse>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//primitive->loadMesh(idt, "Alucy.obj", true, MakeUnique<Mirror>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//primitive->loadMesh(idt, "Alucy.obj", true, MakeUnique<LambertianDiffuse>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//primitive->loadMesh(cb, "./cornell-box/CornellBox-Water.obj");
	primitive->loadMesh(cb, "./cornell-box/CornellBox-Water.obj");
	//primitive->loadMesh(o2w, "teapot.obj", true, MakeUnique<Glass>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//primitive->loadMesh(o2w, "teapot.obj", true, MakeUnique<Mirror>(Spectrum::fromRGB(191.f / 255.f, 173.f / 255.f, 111.f / 255.f)));
	//primitive->loadSphere(o2w, 3, MakeUnique<Glass>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	Scene *scene = new Scene();
	scene->addPrimitive(primitive);
	scene->addPrimitive(bunny);
	//scene->addPrimitive(light_p);
	//scene->addLight(new EnvironmentLight("uffizi-large.hdr", scene));
	//scene->addLight(new EnvironmentLight("forest.jpg", scene));

	//scene->addLight(new PointLight(Point3(3, 1.4, -1), Spectrum(1.f, 0.f, 0.f)));
	//scene->addLight(new PointLight(Point3(3, 1.4, 1), Spectrum(0.f, 1.f, 0.f)));
	scene->addLight(new AreaLight(light_p, Spectrum::fromRGB(8.f, 8.f, 8.f)));
	//scene->addLight(new EnvironmentLight("uffizi-large.hdr", scene));

	scene->initAccelerator();

	GaussianFilter *filter = new GaussianFilter();
	//ProjectiveCamera *cam = new ProjectiveCamera(Point3(-5, 0, 0), Vector3(0, 0, 0), Vector3(0, 1, 0), 40, 1, 0, 1, 0, 0);
	ProjectiveCamera *cam = new ProjectiveCamera(Point3(0, 0.75, 3), Vector3(0, 0.75, -1), Vector3(0, 1, 0), 40, 1, 0, 100000, 0, 0);

	int testnumx = 600;
	int testnumy = 600;
	RandomSampler *random_sampler = new RandomSampler();
	SobolSampler *sobol_sampler = new SobolSampler(testnumx, testnumy);

	Film *film = new Film(testnumx, testnumy, filter);

	TaskSynchronizer task(testnumx, testnumy);
	int spp = 10;
	DirectLightingIntegrator *integrator = new DirectLightingIntegrator(task, spp, 5);
	PathTracingIntegrator *pt = new PathTracingIntegrator(task, spp, 16);
	RNG rng;
	MemoryPool memory;
	//integrator->li(cam->getRay(0.5, 0.5), scene, sobol_sampler, rng, memory);


	pt->render(scene, cam, sobol_sampler, film);
	//integrator->render(scene, cam, random_sampler, film);
	cout << clock() - st << endl;
	const RGBSpectrum * offset = film->getPixelBuffer();
	Bitmap::save("unittest.bmp", (float*)film->getPixelBuffer(), testnumx, testnumy, RGBA_32);
		return 0;
}