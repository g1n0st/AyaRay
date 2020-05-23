#include <Core/Texture.h>

#include <cstring>

namespace Aya {
	template<class T>
	inline void Mipmap2D<T>::generate(const Vector2i &dims, const T *raw_tex) {
		m_texDims = dims;
		m_levels = CeilLog2(Max(dims.x, dims.y));
		SetMax(m_levels, 1);

		mp_leveled_texels = new BlockedArray<T>[m_levels];
		mp_leveled_texels[0].init(dims.y, dims.x, raw_tex);

		Vector2i level_dims = m_texDims;
		for (auto l = 1; l < m_levels; l++) {
			level_dims.x = Max(level_dims.x >> 1, 1);
			level_dims.y = Max(level_dims.y >> 1, 1);

			mp_leveled_texels[l].init(level_dims.y, level_dims.x);
			for (auto x = 0; x < level_dims.x; x++)
				for (auto y = 0; y < level_dims.y; y++) {
					Vector2i idx0, idx1;

					if (level_dims.x < mp_leveled_texels[l - 1].v()) {
						idx0.x = x << 1;
						idx1.x = x << 1 | 1;
					}
					else
						idx0.x = idx1.x = x;

					if (level_dims.y < mp_leveled_texels[l - 1].u()) {
						idx0.y = y << 1;
						idx1.y = y << 1 | 1;
					}
					else
						idx0.y = idx1.y = y;

					T sum = T(0);
					sum += mp_leveled_texels[l - 1](idx0.y, idx0.x);
					sum += mp_leveled_texels[l - 1](idx1.y, idx1.x);
					sum += mp_leveled_texels[l - 1](idx0.y, idx1.x);
					sum += mp_leveled_texels[l - 1](idx1.y, idx0.x);
					mp_leveled_texels[l](y, x) = sum / 4;
				}
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
	T Mipmap2D<T>::linearSample(const Vector2f &coord, const Vector2f diffs[2]) const {
		float filter_width = Max(diffs[0].length(), diffs[1].length());
		float lod = m_levels - 1 + fast_log2(Max(filter_width, 1e-8f));
		if (lod < 0)
			return levelLinearSample(coord, 0);
		else
			return nearestSample(coord);
	}

	template<class T>
	T Mipmap2D<T>::triLinearSample(const Vector2f &coord, const Vector2f diffs[2]) const {
		float filter_width = Max(diffs[0].length(), diffs[1].length());
		float lod = m_levels - 1 + fast_log2(Max(filter_width, 1e-8f));
		if (lod < 0)
			return levelLinearSample(coord, 0);
		if (lod >= m_levels - 1)
			return mp_leveled_texels[m_levels - 1](0, 0);

		uint32_t lod_base = FloorToInt(lod);
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
	T Mipmap2D<T>::levelLinearSample(const Vector2f &coord, const int level) const
	{
		const BlockedArray<T> &texel = mp_leveled_texels[level];
		
		float coord_x = Clamp(coord.x * texel.v(), 0, texel.v() - 1.f);
		float coord_y = Clamp(coord.y * texel.u(), 0, texel.u() - 1.f);
		int idx0_x = Max(FloorToInt(coord_x), 0);
		int idx0_y = Max(FloorToInt(coord_y), 0);
		int idx1_x = Min(idx0_x + 1, texel.v() - 1);
		int idx1_y = Min(idx0_y + 1, texel.u() - 1);

		if (coord_x == idx0_x && coord_y == idx0_y) {
			return texel(idx0_y, idx0_x);
		}

		const T &v1 = texel(idx0_y, idx0_x);
		const T &v2 = texel(idx1_y, idx0_x);
		const T &v3 = texel(idx0_y, idx1_x);
		const T &v4 = texel(idx1_y, idx1_x);
		return Lerp(
			coord_x - idx0_x,
			Lerp(coord_y - idx0_y, v1, v2),
			Lerp(coord_y - idx0_y, v3, v4)
		);
	}

	template<class T>
	T Mipmap2D<T>::nearestSample(const Vector2f &coord) const
	{
		const BlockedArray<T> &texel = mp_leveled_texels[0];
		int coord_x = Clamp(int(coord.x * texel.v()), 0, texel.v() - 1);
		int coord_y = Clamp(int(coord.y * texel.u()), 0, texel.u() - 1);
		return texel(coord_y, coord_x);
	}

	template<class TRet, class TMem>
	ImageTexture2D<TRet, TMem>::ImageTexture2D(const char *file_name, const float gamma) {
		int channel;
		TMem *pixels = Bitmap::read<TMem>(file_name, &m_width, &m_height, &channel);
		std::exception("Texture file load failed.");

		if (gamma != 1.f) {
			for (int i = 0; i < m_width * m_height; i++) {
				pixels[i] = gammaCorrect(pixels[i], gamma);
			}
		}

		m_hasAlpha = (channel == 4);
		m_texels.generate(Vector2i(m_width, m_height), pixels);
		m_widthInv = 1.f / float(m_width);
		m_heightInv = 1.f / float(m_height);

		SafeDeleteArray(pixels);
	}
	template<class TRet, class TMem>
	ImageTexture2D<TRet, TMem>::ImageTexture2D(const TMem *pixels, const int width, const int height) {
		m_width = width;
		m_height = height;
		m_widthInv = 1.f / float(m_width);
		m_heightInv = 1.f / float(m_height);
		m_hasAlpha = false;
		m_texels.generate(Vector2i(m_width, m_height), pixels);
	}

	template<class TRet, class TMem>
	TRet ImageTexture2D<TRet, TMem>::sample(const Vector2f &coord, const Vector2f diffs[2]) const {
		Vector2f wrapped_coord(	coord.x - FloorToInt(coord.x), 
								coord.y - FloorToInt(coord.y));

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
	TRet ImageTexture2D<TRet, TMem>::sample(const Vector2f &coord, const Vector2f diffs[2], TextureFilter filter) const {
		Vector2f wrapped_coord(coord.x - FloorToInt(coord.x),
			coord.y - FloorToInt(coord.y));

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
	TRet ImageTexture2D<TRet, TMem>::anisotropicSample(const Vector2f &coord, const Vector2f diffs[2], const int max_rate) const {
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
		lod1 = Min(FloorToInt(LOD), m_texels.getLevels() - 1);
		lod2 = Min(CeilToInt(LOD), m_texels.getLevels() - 1);

		TMem ret;
		Vector2f uv;
		for (int i = 0; i < (int)ratio_of_anisotropy; i++) {
			uv.u = (start_u + step_u * (i + 0.5f)) * m_widthInv;
			uv.v = (start_v + step_v * (i + 0.5f)) * m_heightInv;
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
	template class ImageTexture2D<RGBSpectrum, byteSpectrum>;
	template class ImageTexture2D<RGBSpectrum, RGBSpectrum>;
	template class ImageTexture2D<SampledSpectrum, byteSpectrum>;
	template class ImageTexture2D<SampledSpectrum, SampledSpectrum>;
	template class ImageTexture2D<float, float>;
	
}