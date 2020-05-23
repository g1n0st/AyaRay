#ifndef AYA_FILTERS_BOXFILTER_H
#define AYA_FILTERS_BOXFILTER_H

#include <Core/Filter.h>

namespace Aya {
	class BoxFilter : public Filter {
	public:
		BoxFilter() : Filter(0.25f) {}
		const float evaluate(const float dx, const float dy) const {
			return 1.0f;
		}
	};
}

#endif