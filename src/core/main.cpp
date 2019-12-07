#include "camera.h"
#include "ray.h"
#include "rng.h"
#include "config.h"
#include "../math/bbox.h"
#include "../math/vector3.h"
#include "../math/transform.h"
#include "../math/quaternion.h"
#include "../math/matrix3x3.h"

int main() {
	PerspectiveCamera cam(Point3(0, 0, 0), Point3(1, 0, 0), Vector3(0, 0, 1), 22.5f,
		1.5f, 1.0f, 1000.f, 0.f, 1.f);

	Ray r = cam.getRay(0.5f, 0.5f);

	return 0;
}