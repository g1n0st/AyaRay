#ifndef AYA_TEXTURE_H
#define AYA_TEXTURE_H

#include "config.h"
#include "rng.h"
#include "spectrum.h"
#include "../math/vector3.h"

namespace Aya {
	/**@brief The Texture base class specifies the methods that textures must implement */
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

		/**@brief Return the color sampled by UV coordinates and p */
		virtual Spectrum value(float u, float v, const Point3 &p) const = 0;
	};

	/**@brief Return the constant color */
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

	/**@brief Return the cross texture defined by other two textures */
	class CrossTexture : public Texture {
	public:
		Texture *m_t0, *m_t1;

	public:
		CrossTexture() {}
		CrossTexture(Texture *t0, Texture *t1) : m_t0(t0), m_t1(t1) {}

		virtual Spectrum value(float u, float v, const Point3 &p) const {
			float sines = sinf(10 * p.x()) *
				sinf(10 * p.y()) *
				sinf(10 * p.z());
			return sines < 0 ? m_t0->value(u, v, p) : m_t1->value(u, v, p);
		}
	};

	/**@brief Return the perlin noise texture */
	class NoiseTexture : public Texture {
	public:
		/**@brief perlin noise generator */
		PerlinNoise m_noise;
		float m_scale;

	public:
		NoiseTexture() {}
		NoiseTexture(const float sc) : m_scale(sc) {}

		virtual Spectrum value(float u, float v, const Point3 &p) const {
			return Spectrum(.5f, .5f, .5f) * (1 + sinf(m_scale * p.x() + 5 * m_noise.turb(m_scale * p)));
		}
	};
}

#endif