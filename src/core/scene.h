#ifndef AYA_SCENE_H
#define AYA_SCENE_H

#include "camera.h"
#include "primitive.h"
#include "memory.h"
#include "integrator.h"

#include <fstream>

namespace Aya {
	/**@brief Scene class defines everything needed for a renderable scene */
	class Scene {
	private:
		RNG rng;
	public:
		/**@brief Render the picture's length and width pixels */
		int m_screen_x, m_screen_y;
		/**@brief Number of samples per pixel */
		int m_sample_times;
		/**@brief Accelerator used by the scene */
		SharedPtr<Accelerator> m_acc;
		/**@brief Camera used by the scene */
		SharedPtr<Camera> m_cam;
		/**@brief Integrator used by the scene */
		SharedPtr<Integrator> m_int;

	public:
		Scene() {}
		Scene(const int &x, const int &y, const int &time);

		/**@brief start rendering
		* @param output output file name */
		void render(const char *output);
	};
}
#endif