#ifndef AYA_LOADERS_BITMAP_H
#define AYA_LOADERS_BITMAP_H

#include "stb_image.h"

#include "../Math/MathUtility.h"
#include "../Core/Spectrum.h"

#include <Windows.h>
#include <winGDI.h>

namespace Aya {
	enum ImageFormat {
		RGB_24 = 3,
		RGBA_32 = 4
	};

	class Bitmap {
	public:
		static void save(const char *name, const float* data, int width, int height, ImageFormat format = RGBA_32);
		static void save(const char *name, const Byte* data, int width, int height);

		template<typename T>
		static T* read(const char *name, int* width, int* height, int* channel);
		template<typename T>
		static T* read(const char *name, int* width, int* height, int* channel, int required_channel);
	};
}

#endif