#include "Texture.h"

namespace Aya {
	template<class T>
	inline void Mipmap2D<T>::generate(const Vector2i & dims, const T * raw_tex) {
		m_tex_dims = dims;
		m_levels = Max(dims.x, dims.y)
	}
}