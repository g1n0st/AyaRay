#include <Core/Memory.h>

namespace Aya {
	void FreeAligned(void *ptr) {
		if (!ptr) return;
		_aligned_free(ptr);
	}
}