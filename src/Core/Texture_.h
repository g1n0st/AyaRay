#ifndef AYA_CORE_TEXTURE___H
#define AYA_CORE_TEXTURE___H

#include "Config.h"
#include "Rng.h"
#include "Spectrum.h"

#include "../Math/Vector3.h"

namespace Aya {
	class Texture {
	public:
#if defined(AYA_USE_SIMD)
		inline void  *operator new(size_t i) {
			return _mm_malloc(i, 16);
		}

		inline void operator delete(void *p) {
			_mm_free(p);
		}
#endif
		virtual Spectrum value(float u, float v, const Point3 &p) const = 0;
	};

	class ConstantTexture : public Texture {
	public:
		Spectrum m_color;

	public:
		ConstantTexture() {}
		ConstantTexture(const Spectrum &color) : m_color(color) {}

		virtual Spectrum value(float u, float v, const Point3 &p) const {
			return m_color;
		}
	};

	class CrossTexture : public Texture {
	public:
		Texture *m_t0, *m_t1;

	public:
		CrossTexture() {}
		CrossTexture(Texture *t0, Texture *t1) : m_t0(t0), m_t1(t1) {}

		virtual Spectrum value(float u, float v, const Point3 &p) const {
			float sines = sinf(0.1f * p.x()) *
				sinf(0.1f * p.y()) *
				sinf(0.1f * p.z());
			return sines < 0 ? m_t0->value(u, v, p) : m_t1->value(u, v, p);
		}
	};

	class NoiseTexture : public Texture {
	public:
		PerlinNoise m_noise;
		float m_scale;

	public:
		NoiseTexture() {}
		NoiseTexture(const float sc) : m_scale(sc) {}

		virtual Spectrum value(float u, float v, const Point3 &p) const {
			float rgb[3] = { .5f, .5f, .5f };
			return Spectrum::fromRGB(rgb) * (1 + sinf(m_scale * p.x() + 5 * m_noise.turb(m_scale * p)));
		}
	};
}

#endif