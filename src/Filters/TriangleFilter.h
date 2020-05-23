#ifndef AYA_FILTERS_TRIANGLEFILTER_H
#define AYA_FILTERS_TRIANGLEFILTER_H

#include <Core/Filter.h>

namespace Aya {
	class TriangleFilter : public Filter {
		TriangleFilter() : Filter(0.25f) {}
		const float evaluate(const float dx, const float dy) const {
			return Max(0.f, m_radius - dx) * Max(0.f, m_radius - dy);
		}
	};
}

#endif