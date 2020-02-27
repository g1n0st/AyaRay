#ifndef AYA_CORE_SCENE_H
#define AYA_CORE_SCENE_H

#include "Camera.h"
#include "Film.h"
#include "Integrator.h"
#include "Memory.h"
#include "Primitive_.h"

#include <ppl.h>
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
		bool occluded(const Ray &ray) const {
			return true;
		}
	};
}
#endif