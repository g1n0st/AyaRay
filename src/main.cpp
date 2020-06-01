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
#include "Integrators\BidirectionalPathTracing.h"

#include "Samplers\RandomSampler.h"
#include "Samplers\SobolSampler.h"

#include "Filters\BoxFilter.h"
#include "Filters\GaussianFilter.h"
#include "Filters\MitchellNetravaliFilter.h"
#include "Filters\TriangleFilter.h"
#include "Integrators\VertexCM.h"

#include "Math\Matrix4x4.h"

using namespace Aya;
using namespace std;

void ayaInit() {
	//Aya::SampledSpectrum::init();
}

std::unique_ptr<BSDF> scene_parser_lambertian(const ObjMaterial &mtl) {
	std::unique_ptr<BSDF> bsdf;
	if (mtl.Tf != Spectrum(0.f)) {
		bsdf = std::make_unique<Glass>(mtl.Tf, 1.f, mtl.Ni);
	}
	else if (mtl.Ks[0] > 0.4f) {
		bsdf = std::make_unique<Glass>(Spectrum(1.f, 1.f, 1.f), 1.f, 1.5f);
	}
	else {
		/*std::unique_ptr<Texture2D<float>> specular;
		if (mtl.map_Ks[0])
			specular = std::make_unique<ImageTexture2D<float, float>>(mtl.map_Ks);
		else
			specular = std::make_unique<ConstantTexture2D<float>>((mtl.Ks[0] +
				mtl.Ks[1] +
				mtl.Ks[2]) / 3.f);

		std::unique_ptr<Texture2D<float>> roughness = std::make_unique<ConstantTexture2D<float>>((512.f - mtl.Ns) / 512.f);
		float metallic = mtl.Ns / 512.f; */

		if (mtl.map_Kd[0])
			bsdf = std::make_unique<LambertianDiffuse>(mtl.map_Kd);
		else
			bsdf = std::make_unique<LambertianDiffuse>(Spectrum(mtl.Kd));
	}
	//if (mtl.map_Bump[0])
	//	bsdf->setNormalMap(mtl.map_Bump);

	return bsdf;
}
RNG rr;
std::unique_ptr<BSDF> hahaha(const ObjMaterial &mtl) {
	return std::make_unique<Disney>(Spectrum::fromRGB(rr.drand48(), rr.drand48(), rr.drand48()),
		std::make_unique<ConstantTexture2D<float>>(0.1f),
		std::make_unique<ConstantTexture2D<float>>(0.9f));
}
int main(void) {
	ayaInit();

	int testnumx = 600;
	int testnumy = 600;

	RNG rng;
	RandomSampler *random_sampler = new RandomSampler();
	SobolSampler *sobol_sampler = new SobolSampler(testnumx, testnumy);
	GaussianFilter *filter = new GaussianFilter();
	MitchellNetravaliFilter *mfilter = new MitchellNetravaliFilter();
	//Camera *cam = new Camera(Point3(-5, 0, 0), Vector3(0, 0, 0), Vector3(0, -1, 0), testnumx, testnumy, 40.f, 0.1f, 1000.f, 0.1f, 5, 2.f);
	Camera *cam = new Camera(Point3(6, 1.5, 0), Vector3(0, 1.5, 0), Vector3(0, -1, 0), testnumx, testnumy, 40.f);
	//Camera *cam = new Camera(Point3(-7.73, 1.47, 7.50), Point3(-8.49, 0.94, 7.12), Vector3(0.47f, -0.85, 0.24), testnumx, testnumy, 75);
	//Camera *cam = new Camera(Point3(-12.38, 2.03, -0.39), Point3(-13.23, 1.67, -0.76), Vector3(0.34, -0.93, 0.15), testnumx, testnumy, 40);
	//Camera *cam = new Camera(Point3(-24.79, 7.58, -1.87), Point3(-23.80, 7.43, -1.88), Vector3(-0.15, -0.99, -0.00), testnumx, testnumy, 40);
	//Camera *cam = new Camera(Point3(-15.12f, 3.73f, 9.96f), Point3(-14.58f, 3.06f, 9.45f), Vector3(-0.48f, -0.75f, 0.46f), testnumx, testnumy, 30);
	Film *film = new Film(testnumx, testnumy, mfilter);

	// Tomoko Tracker
	//for (int i = 82; i <= 85; i++)
	//	for (int j = 531; j <= 534; j++)
	//film->splat(i, 900 - j, Spectrum(10.f, 10.f, 10.f));
	//film->addSampleCount();
	//film->updateDisplay();
	//Bitmap::save("test.bmp", (float*)film->getPixelBuffer(), testnumx, testnumy, RGBA_32);

	Transform murb = Transform().setScale(0.04f, 0.04f, 0.04f) * Transform().setEulerZYX(0, 15, 0);
	AffineTransform cb = AffineTransform().setScale(2, 2, 2) * AffineTransform().setEulerZYX(0, 90, 0);
	AffineTransform cbb = AffineTransform().setScale(1.3f, 1.3f, 1.3f);
	Transform bunnyc = Transform().setScale(0.4f, 0.4f, 0.4f) * Transform().setTranslate(-0.2f, 0, -1.5f) * AffineTransform().setEulerZYX(0, 75, 0);
	Transform bunnyb = Transform().setScale(0.5f, 0.5f, 0.5f) * Transform().setTranslate(0.6f, 2.1f, 1.4f) * AffineTransform().setEulerZYX(0, 130, 0);
	Transform idt = Transform() * Transform().setScale(0.004f, 0.004f, 0.004f) * Transform().setTranslate(60, -420, 0);
	Transform o2w = Transform().setTranslate(-2.04f, 1.56f, 0) * Transform().setScale(3, 3.25f, 4) * Transform().setEulerZYX(0, 0, -90);
	Transform o2w1 = Transform().setTranslate(3, 1.4f, 0);
	int st = clock();
	Primitive * primitive = new Primitive();
	Primitive * light_p = new Primitive();
	Primitive *bunny = new Primitive();
	Primitive *bunny0 = new Primitive();
	Primitive *mur = new Primitive();
	Primitive *teapot = new Primitive();
	Primitive *plane = new Primitive();
	//primitive->loadMesh(o2w, "teapot.obj", true);
	//primitive->loadPlane(o2w, 3, std::make_unique<LambertianDiffuse>(Spectrum(1.f, 1.f, 1.f)));
	//light_p->loadMesh(cb, "./cornell-box/light.obj");
	//mur->loadMesh(murb, "mur.obj", true);
	RNG dr;
	dr.srand(time(0));
	printf("Reading models...\n");
	bunny0->loadMesh(bunnyb, "bunny.obj",
		[](const ObjMaterial &mtl) { return std::make_unique<Glass>(Spectrum::fromRGB(1.f, 1.f, 1.f), 1.f, 2.f); }
	, true, true);
	bunny->loadMesh(bunnyc, "bunny.obj",
		[](const ObjMaterial &mtl) { return std::make_unique<LambertianDiffuse>(Spectrum::fromRGB(100.f / 255.f, 149.f / 255.f, 225.0f / 255.0f)); }
	, true, true);
	//bunny->loadMesh(bunnyc, "bunny.obj", true, true, std::make_unique<Disney>(Spectrum::fromRGB(100.f / 255.f, 149.f / 255.f, 225.0f / 255.0f), 0.1f, 0.9f));
	//plane->loadPlane(o2w, 1, std::make_unique<Disney>("background.jpg", 0.0f, 1.0f));
	//plane->loadPlane(o2w, 1, std::make_unique<LambertianDiffuse>(Spectrum(0.5, 0.5, 0.5)));
	//primitive->loadPlane(o2w, 2, std::make_unique<Glass>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//primitive->loadMesh(o2w, "teapot.obj", true, std::make_unique<Glass>(Spectrum::fromRGB(1.f, 0.7529f, 0.796f), 1.f, 1.5f));
	//teapot->loadMesh(bunnyc, "teapot.obj", false, true, std::make_unique<Glass>(Spectrum::fromRGB(1.f, 1.f, 1.f), 1.f, 1.5f));
	//primitive->loadMesh(idt, "Alucy.obj", true, std::make_unique<Mirror>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//primitive->loadMesh(idt, "Alucy.obj", true, std::make_unique<LambertianDiffuse>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//primitive->loadMesh(cb, "./cornell-box/CornellBox-Water.obj");
	primitive->loadMesh(cb, "./cornell-box/CornellBox-Empty-Squashed.obj", &scene_parser_lambertian);
	//primitive->loadMesh(Transform(), "san-miguel.obj", &scene_parser_lambertian, true, true);
	//primitive->loadMesh(o2w, "teapot.obj", true, std::make_unique<Glass>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//primitive->loadMesh(o2w, "teapot.obj", true, true, std::make_unique<Disney>(Spectrum::fromRGB(65.f / 255.f, 105.f / 255.f, 225.f / 255.f), 0.0f, 1.0f, 0.0f, 0, 0, 0.5f, 0.f));
	//primitive->loadMesh(cb, "shaderball.obj", true, true);
	//primitive->loadSphere(cbb, 1, std::make_unique<Disney>(Spectrum::fromRGB(0.05f, 0.05f, 0.05f),
	//	std::make_unique<ImageTexture2D<float, float>> ("piso_rustico_Spec.png"), std::make_unique<ImageTexture2D<float, float>> ("piso_rustico_Spec.png")));
	Scene *scene = new Scene();
	//scene->addPrimitive(plane);
	scene->addPrimitive(primitive);
	//scene->addPrimitive(teapot);
	scene->addPrimitive(bunny0);
	scene->addPrimitive(bunny);
	//scene->addPrimitive(light_p);
	//scene->addLight(new EnvironmentLight("uffizi-large.hdr", scene));
	//scene->addLight(new EnvironmentLight("forest.jpg", scene));

	//scene->addLight(new PointLight(Point3(3, 1, 0), Spectrum(2.f, 2.f, 2.f)));
	//scene->addLight(new SpotLight(Point3(3, 1, 0), Spectrum(1.f, 1.f, 1.f), Vector3(0, -1, 0), 20, 0));
	////scene->addLight(new DirectionalLight(Vector3(1.3, -1, 0), Spectrum(2.f, 2.f, 2.f), scene, 30));
	Primitive * l1 = new Primitive();
	//Primitive * l2 = new Primitive();
	//l1->loadSphere(Transform().setTranslate(3, 1, 0), 0.5, std::make_unique<LambertianDiffuse>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//l2->loadSphere(Transform().setTranslate(-0.6, 2.7, 1.3), 0.2, std::make_unique<LambertianDiffuse>(Spectrum::fromRGB(1.f, 1.f, 1.f)));
	//scene->addLight(new AreaLight(l1, Spectrum(10.f, 10.f, 10.f)));
	//scene->addLight(new AreaLight(l2, Spectrum(0.f, 0, 1.f)));
	//scene->addLight(new PointLight(Point3(0.6, 3.0, 1.2), Spectrum(1.f, 55.0 / 255.0 * 1, 0.f)));
	scene->addLight(new PointLight(Point3(0, 3.10f, 0), Spectrum(8.f, 8.f, 8.f)));
	//scene->addLight(new AreaLight(light_p, Spectrum::fromRGB(10.f, 10.f, 10.f)));
	//scene->addLight(new EnvironmentLight("uffizi-large.hdr", scene, 1.f));
	//scene->addLight(new EnvironmentLight(Spectrum::fromRGB(8.f, 8.f, 8.f), scene));
	//scene->addLight(new DirectionalLight(Vector3(0.21, -0.7, -0.67), Spectrum(50000, 50000, 50000), scene, 1.f));
	//scene->addLight(new DirectionalLight(Vector3(0.38f, -0.9f, 0.24f), Spectrum(25000, 25000, 25000), scene, 1.f));
	printf("Constructing accelerator...\n");
	scene->initAccelerator();

	TaskSynchronizer task(testnumx, testnumy);
	int spp = 200;
	DirectLightingIntegrator *dl = new DirectLightingIntegrator(task, spp, 5);
	PathTracingIntegrator *pt = new PathTracingIntegrator(task, spp, 16);
	BidirectionalPathTracingIntegrator *bdpt = new BidirectionalPathTracingIntegrator(task, spp, 32, cam, film);
	VertexCMIntegrator *vcm = new VertexCMIntegrator(task, spp, 0, 16, cam, film, VertexCMIntegrator::AlgorithmType::kBpt, 0.003f, 0.75f);
	MemoryPool memory;
	bdpt->render(scene, cam, random_sampler, film);
	//vcm->render(scene, cam, random_sampler, film);
	//pt->render(scene, cam, sobol_sampler , film);
	//dl->render(scene, cam, sobol_sampler, film);
	cout << clock() - st << endl;
	Bitmap::save("test.bmp", (float*)film->getPixelBuffer(), testnumx, testnumy, RGBA_32);
	return 0;
}