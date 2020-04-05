#include "Bitmap.h"

namespace Aya {
	void Bitmap::save(const char *name, const float* data, int width, int height, ImageFormat format) {
		int pixel_bytes = int(format);
		int size = width * height * pixel_bytes;

		const float inv_gamma = 1.f / 2.2f; // Gamma

		unsigned char *pixels = new unsigned char[size];
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				int idx = pixel_bytes * (i * width + j);
				int idx1 = pixel_bytes * ((height - 1 - i) * width + j);
				pixels[idx1 + 0] = Clamp(int(255. * data[idx + 2]), 0, 255);
				pixels[idx1 + 1] = Clamp(int(255. * data[idx + 1]), 0, 255);
				pixels[idx1 + 2] = Clamp(int(255. * data[idx + 0]), 0, 255);

				// set alpha
				if (format == RGBA_32)
					pixels[idx + 3] = 255;
			}
		}

		// bmp first part, file information
		BITMAPFILEHEADER bmp_header;
		bmp_header.bfType = 0x4d42; //Bmp
		bmp_header.bfSize = size // data size
			+ sizeof(BITMAPFILEHEADER) // first section size
			+ sizeof(BITMAPINFOHEADER); // second section size

		bmp_header.bfReserved1 = 0; // reserved 
		bmp_header.bfReserved2 = 0; // reserved
		bmp_header.bfOffBits = bmp_header.bfSize - size;

		// bmp second part, data information
		BITMAPINFOHEADER bmp_info;
		bmp_info.biSize = sizeof(BITMAPINFOHEADER);
		bmp_info.biWidth = width;
		bmp_info.biHeight = height;
		bmp_info.biPlanes = 1;
		bmp_info.biBitCount = 8 * pixel_bytes;
		bmp_info.biCompression = 0;
		bmp_info.biSizeImage = size;
		bmp_info.biXPelsPerMeter = 0;
		bmp_info.biYPelsPerMeter = 0;
		bmp_info.biClrUsed = 0;
		bmp_info.biClrImportant = 0;

		FILE* fp = NULL;
		fopen_s(&fp, name, "wb");
		assert(fp);

		fwrite(&bmp_header, 1, sizeof(BITMAPFILEHEADER), fp);
		fwrite(&bmp_info, 1, sizeof(BITMAPINFOHEADER), fp);
		fwrite(pixels, 1, size, fp);
		fclose(fp);

		delete pixels;
		pixels = nullptr;
	}

	void Bitmap::save(const char *name, const Byte* data, int width, int height) {
		int pixel_bytes = 4;
		int size = width * height * pixel_bytes;

		const float inv_gamma = 1.f / 2.2f; // Gamma

		short *pixels = new short[size];
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				int idx = pixel_bytes * (i * width + j);
				int idx1 = pixel_bytes * ((height - 1 - i) * width + j);
				pixels[idx1 + 0] = data[idx + 2];
				pixels[idx1 + 1] = data[idx + 1];
				pixels[idx1 + 2] = data[idx + 0];
				pixels[idx1 + 3] = data[idx + 3];
			}
		}

		// bmp first part, file information
		BITMAPFILEHEADER bmp_header;
		bmp_header.bfType = 0x4d42; //Bmp
		bmp_header.bfSize = size // data size
			+ sizeof(BITMAPFILEHEADER) // first section size
			+ sizeof(BITMAPINFOHEADER); // second section size

		bmp_header.bfReserved1 = 0; // reserved 
		bmp_header.bfReserved2 = 0; // reserved
		bmp_header.bfOffBits = bmp_header.bfSize - size;

		// bmp second part, data information
		BITMAPINFOHEADER bmp_info;
		bmp_info.biSize = sizeof(BITMAPINFOHEADER);
		bmp_info.biWidth = width;
		bmp_info.biHeight = height;
		bmp_info.biPlanes = 1;
		bmp_info.biBitCount = 8 * pixel_bytes;
		bmp_info.biCompression = 0;
		bmp_info.biSizeImage = size;
		bmp_info.biXPelsPerMeter = 0;
		bmp_info.biYPelsPerMeter = 0;
		bmp_info.biClrUsed = 0;
		bmp_info.biClrImportant = 0;

		FILE* fp = NULL;
		fopen_s(&fp, name, "wb");
		assert(fp);

		fwrite(&bmp_header, 1, sizeof(BITMAPFILEHEADER), fp);
		fwrite(&bmp_info, 1, sizeof(BITMAPINFOHEADER), fp);
		fwrite(pixels, 1, size, fp);
		fclose(fp);

		delete pixels;
		pixels = nullptr;
	}

	template<>
	float* Bitmap::read(const char *name, int* width, int* height, int* channel) {
		return (float*)stbi_loadf(name, width, height, channel, 4);
	}
	template<>
	RGBSpectrum* Bitmap::read(const char *name, int* width, int* height, int* channel) {
		return (RGBSpectrum*)stbi_loadf(name, width, height, channel, 4);
	}
	template<>
	SampledSpectrum* Bitmap::read(const char *name, int* width, int* height, int* channel) {
		float *rgbs = (float*)stbi_loadf(name, width, height, channel, 4);

		int size = (*width) * (*height);
		SampledSpectrum *pixels = new SampledSpectrum[size];

		for (int i = 0; i < size; i++) {
			pixels[i] = SampledSpectrum::fromRGB(rgbs + i * 4, SpectrumType::Reflectance);
		}
		return pixels;
	}
	template<>
	byteSpectrum* Bitmap::read(const char *name, int *width, int *height, int *channel) {
		printf("Read texture: %s\n", name);
		float *rgbs = (float*)stbi_loadf(name, width, height, channel, 4);

		int size = (*width) * (*height);
		byteSpectrum *pixels = new byteSpectrum[size];

		for (int i = 0; i < size; i++) {
			pixels[i].r = (Byte)(rgbs[i * 4 + 0] * 255);
			pixels[i].g = (Byte)(rgbs[i * 4 + 1] * 255);
			pixels[i].b = (Byte)(rgbs[i * 4 + 2] * 255);
			pixels[i].a = (*channel == 4) ? (Byte)(rgbs[i * 4 + 3] * 255) : 255;

		}
		return pixels;
	}

	template<>
	float* Bitmap::read(const char *name, int* width, int* height, int* channel, int required_channel) {
		return (float*)stbi_loadf(name, width, height, channel, required_channel);
	}
	template<>
	uint8_t* Bitmap::read(const char *name, int* width, int* height, int* channel, int required_channel) {
		return (uint8_t*)stbi_loadf(name, width, height, channel, required_channel);
	}
}