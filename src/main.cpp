#include "Core/integrator.h"
#include "Core/Medium.h"
#include <array>
#include "Media/Homogeneous.h"
#include "Core\BSDF.h"
#include "BSDFs\LambertianDiffuse.h"
#include "BSDFs\Glass.h"
#include "Core/Memory.h"
#include "BSDFs\Mirror.h"
#include "BSDFs\Disney.h"

#include "Accelerators\BVH.h"
#include "Core\Scene.h"
#include "Core\Camera.h"
#include "Lights\AreaLight.h"
#include "Lights\EnvironmentLight.h"
#include "Lights\PointLight.h"
#include "Lights\SpotLight.h"
#include "Lights\DirectionalLight.h"

#include "Integrators\DirectLighting.h"
#include "Integrators\PathTracing.h"
#include "Samplers\RandomSampler.h"
#include "Samplers\SobolSampler.h"

#include "Filters\BoxFilter.h"
#include "Filters\GaussianFilter.h"
#include "Filters\MitchellNetravaliFilter.h"
#include "Filters\TriangleFilter.h"

#include "Math\Matrix4x4.h"

using namespace Aya;
using namespace std;

void ayaInit() {
	Aya::SampledSpectrum::init();
}

int main(void) {
	ayaInit();

	int testnumx = 600;
	int testnumy = 600;

	RNG rng;
	RandomSampler *random_sampler = new RandomSampler();
	SobolSampler *sobol_sampler = new SobolSampler(testnumx, testnumy);
	GaussianFilter *filter = new GaussianFilter();
	//ProjectiveCamera *cam = new ProjectiveCamera(Point3(0, 2.5, 2.5), Vector3(0, 2.5, 0), Vector3(0, 1, 0), 40, 1, 0, 1, 0, 0);
	Camera *cam = new Camera(Point3(-5, 0, 0), Vector3(0, 0, 0), Vector3(0, -1, 0), testnumx, testnumy, 40.f, 0.1f, 1000.f, 0.1f, 5, 2.f);
	//ProjectiveCamera *cam = new ProjectiveCamera(Point3(0, 0.75, 3), Vector3(0, 0.75, -1), Vector3(0, 1, 0), 40, 1, 0, 100000, 0, 0);
	Film *film = new Film(testnumx, testnumy, filter);
	
	/// debug
	sobol_sampler->startPixel(300, 300);
	CameraSample cam_sample;
	sobol_sampler->generateSamples(300, 300, &cam_sample, rng);
	cam_sample.image_x += 300;
	cam_sample.image_y += 300;

	RayDifferential ray;
	cam->generateRayDifferential(cam_sample, &ray);
	/// 

	Transform murb = Transform().setScale(0.04f, 0.04f, 0.04f) * Transform().setEulerZYX(0, 15, 0);
	AffineTransform cb = AffineTransform().setScale(2, 2, 2) * AffineTransform().setEulerZYX(0, 90, 0);
	AffineTransform cbb = AffineTransform().setScale(1.3, 1.3, 1.3);
	Transform bunnyc = Transform().setScale(0.4, 0.4, 0.4) * Transform().setTranslate(0, -2, 0) * AffineTransform().setEulerZYX(0, -90, 0);
	Transform idt = Transform() * Transform().setScale(0.004, 0.004, 0.004) * Transform().setTranslate(60, -420, 0);
	Transform o2w = Transform().setTranslate(3, -1.4, 0) * Transform().setScale(0.9, 0.9, 0.9) * Transform().setEulerZYX(-10, 90, 10);
	Transform o2w1 = Transform().setTranslate(3, 1.4, 0);
	int st = clock();
	Primitive * primitive = new Primitive();
	Primitive * light_p = new Primitive();
	Primitive *bunny = new Primitive();
	Primitive *mur = new Primitive();
	//primitive->loadMesh(o2w, "teapot.obj", true);
	//primitive->loadPlane(o2w, 3, MakeUnique<LambertianDiffuse>(Spectrum(1.f, 1.f, 1.f)));
	//light_p->loadMesh(cb, "./cornell-box/light.obj");
	//mur->loadMesh(murb, "mur.obj", true);
	RNG dr;
	dr.srand(time(0));
	bunny->loadMesh(bunnyc, "bunny.obj", true, true, MakeUnique<Disney>(Spectrum::fromRGB(100.f / 255.f, 149.f / 255.f, 225.0f / 255.0f), 0.75, 0.0, 0
		, 0, 0, 0, 0, 1, 1));
	//primitive->loadPlane(o2w, 2, MakeUnique<LambertianDiffuse>("cnm777.bmp"));
	//primitive->loadPlane(o2w, 2, MakeUnique<Glass>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//primitive->loadMesh(o2w, "teapot.obj", true, MakeUnique<Glass>(Spectrum::fromRGB(1.f, 0.7529f, 0.796f), 1.f, 1.5f));
	//primitive->loadMesh(o2w, "teapot.obj", true);
	//primitive->loadMesh(idt, "Alucy.obj", true, MakeUnique<Mirror>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//primitive->loadMesh(idt, "Alucy.obj", true, MakeUnique<LambertianDiffuse>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//primitive->loadMesh(cb, "./cornell-box/CornellBox-Water.obj");
	//primitive->loadMesh(cb, "./cornell-box/CornellBox-Water.obj");
	//primitive->loadMesh(o2w, "teapot.obj", true, MakeUnique<Glass>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//primitive->loadMesh(o2w, "teapot.obj", true, true, MakeUnique<Disney>(Spectrum::fromRGB(65.f / 255.f, 105.f / 255.f, 225.f / 255.f), 0.0f, 1.0f, 0.0f, 0, 0, 0.5f, 0.f));
	//primitive->loadMesh(cb, "shaderball.obj", true, true);
	//primitive->loadSphere(cbb, 1, MakeUnique<Disney>(MakeUnique<ConstantTexture2D<Spectrum>>(Spectrum::fromRGB(65.f / 255.f, 105.f / 255.f, 225.f / 255.f)),
		//MakeUnique<ImageTexture2D<RGBSpectrum, RGBSpectrum>>("norm5.bmp"), 0.1f, 0.8f));
	Scene *scene = new Scene();
	//scene->addPrimitive(mur);
	//scene->addPrimitive(primitive);
	scene->addPrimitive(bunny);
	//scene->addPrimitive(light_p);
	//scene->addLight(new EnvironmentLight("uffizi-large.hdr", scene));
	//scene->addLight(new EnvironmentLight("forest.jpg", scene));

	//scene->addLight(new PointLight(Point3(3, 1, 0), Spectrum(2.f, 2.f, 2.f)));
	//scene->addLight(new SpotLight(Point3(3, 1, 0), Spectrum(1.f, 1.f, 1.f), Vector3(0, -1, 0), 20, 0));
	////scene->addLight(new DirectionalLight(Vector3(1.3, -1, 0), Spectrum(2.f, 2.f, 2.f), scene, 30));
	Primitive * l1 = new Primitive();
	//Primitive * l2 = new Primitive();
	//l1->loadSphere(Transform().setTranslate(3, 1, 0), 0.5, MakeUnique<LambertianDiffuse>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//l2->loadSphere(Transform().setTranslate(-0.6, 2.7, 1.3), 0.2, MakeUnique<LambertianDiffuse>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//scene->addLight(new AreaLight(l1, Spectrum(10.f, 10.f, 10.f)));
	//scene->addLight(new AreaLight(l2, Spectrum(0.f, 0, 1.f)));
	//scene->addLight(new PointLight(Point3(0.6, 3.0, 1.2), Spectrum(1.f, 55.0 / 255.0 * 1, 0.f)));
	//scene->addLight(new PointLight(Point3(-0.6, 3.0, 1.2), Spectrum(0.f, 0, 1.f)));
	//scene->addLight(new AreaLight(light_p, Spectrum::fromRGB(8.f, 8.f, 8.f)));
	scene->addLight(new EnvironmentLight("uffizi-large.hdr", scene));

	scene->initAccelerator();

	TaskSynchronizer task(testnumx, testnumy);
	int spp = 50;
	DirectLightingIntegrator *integrator = new DirectLightingIntegrator(task, spp, 5);
	PathTracingIntegrator *pt = new PathTracingIntegrator(task, spp, 16);
	
	MemoryPool memory;
	//integrator->li(cam->getRay(0.5, 0.5), scene, sobol_sampler, rng, memory);


	pt->render(scene, cam, sobol_sampler , film);
	//integrator->render(scene, cam, sobol_sampler, film);
	cout << clock() - st << endl;
	const RGBSpectrum * offset = film->getPixelBuffer();
	Bitmap::save("test.bmp", (float*)film->getPixelBuffer(), testnumx, testnumy, RGBA_32);
		return 0;
}