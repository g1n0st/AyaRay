#ifndef AYA_SCENE_H
#define AYA_SCENE_H

#include "camera.h"
#include "primitive.h"
#include "memory.h"
#include "integrator.h"

#include <fstream>

namespace Aya {
	class Scene {
	private:
		RNG rng;
	public:
		int m_screen_x, m_screen_y;
		int m_sample_times;
		SharedPtr<Accelerator> m_acc;
		SharedPtr<Camera> m_cam;
		SharedPtr<Integrator> m_int;

	public:
		Scene() {}
		Scene(const int &x, const int &y, const int &time);

		void render(const char *output);
	};
}
#endif