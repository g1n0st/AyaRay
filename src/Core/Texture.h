#ifndef AYA_CORE_TEXTURE_H
#define AYA_CORE_TEXTURE_H

#include <Core/Config.h>
#include <Math/Vector2.h>
#include <Core/Spectrum.h>
#include <Loaders/Bitmap.h>
#include <Core/Memory.h>

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
		virtual bool alphaTest(const Vector2f &coord) const {
			return true;
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
		inline T sample(const Vector2f &coord, const Vector2f diffs[2]) const override {
			return m_val;
		}
		inline T sample(const Vector2f &coord, const Vector2f diffs[2], TextureFilter filter) const override {
			return m_val;
		}
		bool isConstant() const override {
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
		Vector2i m_texDims;
		int m_levels;

	public:
		BlockedArray<T>* mp_leveled_texels;
		Mipmap2D() {}
		~Mipmap2D() {
			SafeDeleteArray(mp_leveled_texels);
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
		float m_widthInv, m_heightInv;
		bool m_hasAlpha;
		TextureFilter m_filter;
		Mipmap2D<TMem> m_texels;

	public:
		ImageTexture2D(const char *file_name, const float gamma = 1.f);
		ImageTexture2D(const TMem* pixels, const int width, const int height);
		~ImageTexture2D() {}

		TRet sample(const Vector2f &coord, const Vector2f diffs[2]) const override;
		TRet sample(const Vector2f &coord, const Vector2f diffs[2], TextureFilter filter) const override;
		TRet anisotropicSample(const Vector2f &coord, const Vector2f diffs[2], const int max_rate) const;

		inline void setFilter(const TextureFilter filter) override {
			m_filter = filter;
		}
		inline int width() const override {
			return m_width;
		}
		inline int height() const override {
			return m_height;
		}
		inline bool hasAlpha() const override {
			return m_hasAlpha;
		}
		inline bool alphaTest(const Vector2f &coord) const override {
			return alpha(sample(coord, nullptr, TextureFilter::Nearest)) != 0.f;
		}
		const TMem* getLevelData(const int level = 0) const {
			return m_texels.getLevelData(level);
		}

		static RGBSpectrum gammaCorrect(RGBSpectrum s, const float gamma) {
			return s.pow(gamma);
		}
		static SampledSpectrum gammaCorrect(SampledSpectrum s, const float gamma) {
			return SampledSpectrum(RGBSpectrum(s).pow(gamma));
		}
		static byteSpectrum gammaCorrect(byteSpectrum s, const float gamma) {
			return (byteSpectrum)RGBSpectrum(s).pow(gamma);
		}
		static float gammaCorrect(float s, const float gamma) {
			return std::powf(s, gamma);
		}

		static float alpha(const RGBSpectrum &s) {
			return s[3];
		}
		static float alpha(const SampledSpectrum &s) {
			return -1;
		}
		static float alpha(const byteSpectrum &s) {
			return s.a;
		}
		static float alpha(const float s) {
			return -1;
		}
	};
}
#endif