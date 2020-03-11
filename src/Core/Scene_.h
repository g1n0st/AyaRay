#ifndef AYA_CORE_SCENE__H
#define AYA_CORE_SCENE__H

#include "Camera.h"
#include "Film.h"
#include "Integrator_.h"
#include "Memory.h"
#include "Primitive_.h"

#include <ppl.h>
#include <fstream>

namespace Aya {
	class Scene_ {
	private:
		RNG rng;
	public:
		int m_screen_x, m_screen_y;
		int m_sample_times;
		SharedPtr<Accelerator> m_acc;
		SharedPtr<Camera> m_cam;
		SharedPtr<Integrator_> m_int;

	public:
		Scene_() {}
		Scene_(const int &x, const int &y, const int &time);

		void render(const char *output);
		bool occluded(const Ray &ray) const {
			return true;
		}
	};
}
#endif