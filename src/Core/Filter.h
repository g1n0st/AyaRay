#ifndef AYA_CORE_FILTER_H
#define AYA_CORE_FILTER_H

#include "../Core/Config.h"
#include "../Math/MathUtility.h"

namespace Aya {
	class Filter {
	protected:
		float m_radius;

	public:
		Filter(const float &rad) : m_radius(rad) {}
		virtual ~Filter() {}

		const float getRadius() const;
		virtual const float evaluate(const float dx, const float dy) const = 0;
	};
}
#endif