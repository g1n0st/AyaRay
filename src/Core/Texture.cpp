#include "Texture.h"

#include <cstring>

namespace Aya {
	template<class T>
	inline void Mipmap2D<T>::generate(const Vector2i & dims, const T * raw_tex) {

		Bitmap::save("fuck.bmp", (float*)raw_tex, dims.x, dims.y, RGBA_32);

		m_tex_dims = dims;
		m_levels = ceilLog2(Max(dims.x, dims.y));
		SetMax(m_levels, 1);

		m_leveled_texels = new BlockedArray<T>[m_levels];
		m_leveled_texels[0].init(dims.x, dims.y, raw_tex);

		Vector2i level_dims = m_tex_dims;
		for (auto l = 1; l < m_levels; l++) {
			level_dims.x = Max(level_dims.x >> 1, 1);
			level_dims.y = Max(level_dims.y >> 1, 1);

			m_leveled_texels[l].init(level_dims.x, level_dims.y);
			for (auto x = 0; x < level_dims.x; x++)
				for (auto y = 0; y < level_dims.y; y++) {
					Vector2i idx0, idx1;

					if (level_dims.x < m_leveled_texels[l - 1].x()) {
						idx0.x = x << 1;
						idx1.x = x << 1 | 1;
					}
					else
						idx0.x = idx1.x = x;

					if (level_dims.y < m_leveled_texels[l - 1].y()) {
						idx0.y = y << 1;
						idx1.y = y << 1 | 1;
					}
					else
						idx0.y = idx1.y = y;

					T sum = T(0);
					sum += m_leveled_texels[l - 1](idx0.x, idx0.y);
					//sum += m_leveled_texels[l - 1](idx1.x, idx1.y);
					//sum += m_leveled_texels[l - 1](idx0.x, idx1.y);
					//sum += m_leveled_texels[l - 1](idx1.x, idx0.y);
					//m_leveled_texels[l](x, y) = sum / 4;
					m_leveled_texels[l](x, y) = sum;
				}
			char s[2] = { 'a' + l - 1, 0 };
			Bitmap::save((std::string(s) + std::string(".bmp")).c_str(), (float*)m_leveled_texels[l].data(), level_dims.x, level_dims.y, RGBA_32);
		}
	}

	inline float fast_log2(float val) {
		int * const    exp_ptr = reinterpret_cast <int *> (&val);
		int            x = *exp_ptr;
		const int      log_2 = ((x >> 23) & 255) - 128;
		x &= ~(255 << 23);
		x += 127 << 23;
		*exp_ptr = x;

		val = ((-1.0f / 3) * val + 2) * val - 2.0f / 3;

		return (val + log_2);
	}

	template<class T>
	T Mipmap2D<T>::linearSample(const Vector2f & coord, const Vector2f diffs[2]) const {
		float filter_width = Max(diffs[0].length(), diffs[1].length());
		float lod = m_levels - 1 + fast_log2(Max(filter_width, 1e-8f));
		if (lod < 0)
			return levelLinearSample(coord, 0);
		else
			return nearestSample(coord);
	}

	template<class T>
	T Mipmap2D<T>::triLinearSample(const Vector2f & coord, const Vector2f diffs[2]) const {
		float filter_width = Max(diffs[0].length(), diffs[1].length());
		float lod = m_levels - 1 + fast_log2(Max(filter_width, 1e-8f));
		if (lod < 0)
			return levelLinearSample(coord, 0);
		if (lod >= m_levels - 1)
			return m_leveled_texels[m_levels - 1](0, 0);

		uint32_t lod_base = floorToInt(lod);
		float lin = lod - lod_base;
		if (lin < .2f)
			return levelLinearSample(coord, lod_base);
		if (lin > .8f)
			return levelLinearSample(coord, lod_base + 1);

		return Lerp(
			lin,
			levelLinearSample(coord, lod_base),
			levelLinearSample(coord, lod_base + 1)
		);
	}

	template<class T>
	T Mipmap2D<T>::levelLinearSample(const Vector2f & coord, const int level) const
	{
		const BlockedArray<T> &texel = m_leveled_texels[level];
		
		float coord_x = Min(coord.x * texel.x(), texel.x() - 1.f);
		float coord_y = Min(coord.y * texel.y(), texel.y() - 1.f);
		int idx0_x = floorToInt(coord_x);
		int idx0_y = floorToInt(coord_y);
		int idx1_x = Min(idx0_x + 1, texel.x() - 1);
		int idx1_y = Min(idx0_y + 1, texel.y() - 1);

		if (coord_x == idx0_x && coord_y == idx0_y) {
			return texel(idx0_x, idx0_y);
		}

		const T &v1 = texel(idx0_x, idx0_y);
		const T &v2 = texel(idx1_x, idx0_y);
		const T &v3 = texel(idx0_x, idx1_y);
		const T &v4 = texel(idx1_x, idx1_y);
		return Lerp(
			coord_y - idx0_y,
			Lerp(coord_x - idx0_x, v1, v2),
			Lerp(coord_x - idx0_x, v3, v4)
		);
	}

	template<class T>
	T Mipmap2D<T>::nearestSample(const Vector2f & coord) const
	{
		const BlockedArray<T> &texel = m_leveled_texels[0];
		int coord_x = Min(int(coord.x * texel.x()), texel.x() - 1);
		int coord_y = Min(int(coord.y * texel.y()), texel.y() - 1);
		return texel(coord_x, coord_y);
	}

	template<class TRet, class TMem>
	ImageTexture2D<TRet, TMem>::ImageTexture2D(const char * file_name, const float gamma) {
		int channel;
		TMem *pixels = Bitmap::read<TMem>(file_name, &m_width, &m_height, &channel);
		std::exception("Texture file load failed.");

		for (int i = 0; i < m_width * m_height; i++) {
			pixels[i] = gammaCorrect(pixels[i], gamma);
		}

		m_has_alpha = (channel == 4);
		m_texels.generate(Vector2i(m_width, m_height), pixels);
		m_width_inv = 1.f / float(m_width);
		m_height_inv = 1.f / float(m_height);

		delete[] pixels;
	}
	template<class TRet, class TMem>
	ImageTexture2D<TRet, TMem>::ImageTexture2D(const TMem * pixels, const int width, const int height) {
		m_width = width;
		m_height = height;
		m_width_inv = 1.f / float(m_width);
		m_height_inv = 1.f / float(m_height);
		m_has_alpha = false;
		m_texels.generate(Vector2i(m_width, m_height), pixels);
	}

	template<class TRet, class TMem>
	TRet ImageTexture2D<TRet, TMem>::sample(const Vector2f & coord, const Vector2f diffs[2]) const {
		Vector2f wrapped_coord(	coord.x - floorToInt(coord.x), 
								coord.y - floorToInt(coord.y));

		switch (m_filter) {
		case TextureFilter::Nearest:
			return m_texels.nearestSample(wrapped_coord);
		case TextureFilter::Linear:
			return m_texels.levelLinearSample(wrapped_coord, 0);
		case TextureFilter::TriLinear:
			return m_texels.triLinearSample(wrapped_coord, diffs);
		case TextureFilter::Anisotropic4x:
			return anisotropicSample(wrapped_coord, diffs, 4);
		case TextureFilter::Anisotropic8x:
			return anisotropicSample(wrapped_coord, diffs, 8);
		case TextureFilter::Anisotropic16x:
			return anisotropicSample(wrapped_coord, diffs, 16);
		}

		return TRet(0);
	}

	template<class TRet, class TMem>
	TRet ImageTexture2D<TRet, TMem>::sample(const Vector2f & coord, const Vector2f diffs[2], TextureFilter filter) const {
		Vector2f wrapped_coord(coord.x - floorToInt(coord.x),
			coord.y - floorToInt(coord.y));

		switch (filter) {
		case TextureFilter::Nearest:
			return m_texels.nearestSample(wrapped_coord);
		case TextureFilter::Linear:
			return m_texels.linearSample(wrapped_coord, diffs);
		case TextureFilter::TriLinear:
			return m_texels.triLinearSample(wrapped_coord, diffs);
		case TextureFilter::Anisotropic4x:
			return anisotropicSample(wrapped_coord, diffs, 4);
		case TextureFilter::Anisotropic8x:
			return anisotropicSample(wrapped_coord, diffs, 8);
		case TextureFilter::Anisotropic16x:
			return anisotropicSample(wrapped_coord, diffs, 16);
		}

		return TRet(0);
	}

	template<class TRet, class TMem>
	TRet ImageTexture2D<TRet, TMem>::anisotropicSample(const Vector2f & coord, const Vector2f diffs[2], const int max_rate) const
	{
		float dudx = diffs[0].u;
		float dvdx = diffs[0].v;
		float dudy = diffs[1].u;
		float dvdy = diffs[1].v;

		float A = dvdx * dvdx + dvdy * dvdy;
		float B = -2.0f * (dudx * dvdx + dudy * dvdy);
		float C = dudx * dudx + dudy * dudy;
		float F = (dudx * dvdy - dudy * dvdx);
		F *= F;
		float p = A - C;
		float q = A + C;
		float t = sqrt(p * p + B * B);

		dudx *= m_width; dudy *= m_width;
		dvdx *= m_height; dvdy *= m_height;

		float squared_length_x = dudx * dudx + dvdx * dvdx;
		float squared_length_y = dudy * dudy + dvdy * dvdy;
		float determinant = Abs(dudx * dvdy - dvdx * dudy);
		bool is_major_x = squared_length_x > squared_length_y;
		float squared_length_major = is_major_x ? squared_length_x : squared_length_y;
		float length_major = Sqrt(squared_length_major);
		float norm_major = 1.f / length_major;

		Vector2f aniso_dir;
		aniso_dir.x = (is_major_x ? dudx : dudy) * norm_major;
		aniso_dir.y = (is_major_x ? dvdx : dvdy) * norm_major;

		float ratio_of_anisotropy = squared_length_major / determinant;

		// clamp ratio and compute LOD
		float length_minor;
		if (ratio_of_anisotropy > max_rate) { // maxAniso comes from a Sampler state.
			// ratio is clamped - LOD is based on ratio (preserves area)
			ratio_of_anisotropy = (float)max_rate;
			length_minor = length_major / ratio_of_anisotropy;
		}
		else {
			// ratio not clamped - LOD is based on area
			length_minor = determinant / length_major;
		}

		// clamp to top LOD
		if (length_minor < 1.f) {
			ratio_of_anisotropy = Max(1.f, ratio_of_anisotropy * length_minor);
			length_minor = 1.f;
		}

		float LOD = fast_log2(length_minor);
		float inv_rate = 1.f / (int)ratio_of_anisotropy;
		float start_u = coord.u * m_width - length_major * aniso_dir.x * .5f;
		float start_v = coord.v * m_height - length_major * aniso_dir.y * .5f;
		float step_u = length_major * aniso_dir.x * inv_rate;
		float step_v = length_major * aniso_dir.y * inv_rate;
		int lod1, lod2;
		lod1 = Min(floorToInt(LOD), m_texels.getLevels() - 1);
		lod2 = Min(ceilToInt(LOD), m_texels.getLevels() - 1);

		TMem ret;
		Vector2f uv;
		for (int i = 0; i < (int)ratio_of_anisotropy; i++) {
			uv.u = (start_u + step_u * (i + 0.5f)) * m_width_inv;
			uv.v = (start_v + step_v * (i + 0.5f)) * m_height_inv;
			float lin = LOD - lod1;
			if (lin < 0.2f)
				ret += m_texels.levelLinearSample(uv, lod1) * inv_rate;
			else if (lin > 0.8f)
				ret += m_texels.levelLinearSample(uv, lod2) * inv_rate;
			else
				ret += Lerp(LOD - lod1, m_texels.levelLinearSample(uv, lod1), m_texels.levelLinearSample(uv, lod2)) * inv_rate;
		}

		return ret;
	}

	template class Mipmap2D<Spectrum>;
	template class Mipmap2D<byteSpectrum>;
	template class Mipmap2D<float>;
	template class ImageTexture2D<Spectrum, byteSpectrum>;
	template class ImageTexture2D<Spectrum, Spectrum>;
	template class ImageTexture2D<float, float>;
	
}