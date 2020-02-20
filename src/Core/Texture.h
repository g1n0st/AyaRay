#ifndef AYA_CORE_TEXTURE_H
#define AYA_CORE_TEXTURE_H

#include "../Core/Spectrum.h"
#include "../Image/Bitmap.h"
#include "../Math/Vector2.h"
#include "../Core/Memory.h"

namespace Aya {
	enum class TextureFilter {
		Nearest = 0,
		Linear = 1,
		TriLinear = 2,
		Anisotropic4x = 3,
		Anisotropic8x = 4,
		Anisotropic16x = 5
	};
	enum class TextureWrapMode {
		Clamp,
		Repeat,
		Mirror
	};

	template<class T>
	class Texture2D {
	public:
		virtual ~Texture2D() {}
		virtual T sample(const Vector2f &coord, const Vector2f diffs[2]) const = 0;
		virtual T sample(const Vector2f &coord, const Vector2f diffs[2], TextureFilter filter) const = 0;
		virtual bool hasAlpha() const {
			return false;
		}
		virtual bool isConstant() const {
			return false;
		}
		virtual int width() const {
			return 0;
		}
		virtual int height() const {
			return 0;
		}
		virtual void setFilter(const TextureFilter filter) {}
		virtual T getValue() const {
			return T();
		}
		virtual void setValue(const T &value) {}
	};

	template<class T>
	class ConstantTexture2D : public Texture2D<T> {
	private:
		T m_val;

	public:
		ConstantTexture2D(const T &val) : m_val(val) {}
		__forceinline T sample(const Vector2f &coord, const Vector2f diffs[2]) const {
			return m_val;
		}
		__forceinline T sample(const Vector2f &coord, const Vector2f diffs[2], TextureFilter filter) const {
			return m_val;
		}
		bool isConstant() const {
			return true;
		}
		T getValue() const {
			return m_val;
		}
		void setValue(const T &value) const {
			this->m_val = value;
		}
	};

	template<class T>
	class Mipmap2D {
	private:
		Vector2i m_tex_dims;
		int m_levels;

	public:
		BlockedArray<T>* mp_leveled_texels;
		Mipmap2D() {}
		~Mipmap2D() {
			delete[] mp_leveled_texels;
		}

		void generate(const Vector2i &dims, const T* raw_tex);

		T linearSample(const Vector2f& coord, const Vector2f diffs[2]) const;
		T triLinearSample(const Vector2f& coord, const Vector2f diffs[2]) const;
		T levelLinearSample(const Vector2f& coord, const int level) const;
		T nearestSample(const Vector2f& coord) const;

		const T* getLevelData(const int level = 0) const {
			assert(level < m_levels);
			return mp_leveled_texels[level].data();
		}
		const int getLevels() const {
			return m_levels;
		}
	};

	template<class TRet, class TMem>
	class ImageTexture2D : public Texture2D<TRet> {
	private:
		int m_width;
		int m_height;
		float m_width_inv, m_height_inv;
		bool m_has_alpha;
		TextureFilter m_filter;
		Mipmap2D<TMem> m_texels;

	public:
		ImageTexture2D(const char *file_name, const float gamma = 1.f / 2.2f);
		ImageTexture2D(const TMem* pixels, const int width, const int height);
		~ImageTexture2D() {}

		TRet sample(const Vector2f &coord, const Vector2f diffs[2]) const;
		TRet sample(const Vector2f &coord, const Vector2f diffs[2], TextureFilter filter) const;
		TRet anisotropicSample(const Vector2f &coord, const Vector2f diffs[2], const int max_rate) const;

		__forceinline void setFilter(const TextureFilter filter) {
			m_filter = filter;
		}
		__forceinline int width() const {
			return m_width;
		}
		__forceinline int height() const {
			return m_height;
		}
		__forceinline bool has_alpha() const {
			return m_has_alpha;
		}
		const TMem* getLevelData(const int level = 0) const {
			return m_texels.getLevelData(level);
		}

		static RGBSpectrum gammaCorrect(RGBSpectrum s, const float gamma) {
			return s.pow(gamma);
		}
		static SampledSpectrum gammaCorrect(SampledSpectrum s, const float gamma) {
			return s.pow(gamma);
		}
		static byteSpectrum gammaCorrect(byteSpectrum s, const float gamma) {
			return (Spectrum)Spectrum(s).pow(gamma);
		}
		static float gammaCorrect(float s, const float gamma) {
			return std::pow(s, gamma);
		}
	};
}
#endif