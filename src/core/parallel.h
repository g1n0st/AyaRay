#ifndef AYA_PARALLEL_H
#define AYA_PARALLEL_H

#ifdef _WIN32
#include <Windows.h>
#endif
#include <stdint.h>

#include "config.h"

namespace Aya {
	/*
		Modified Exclusive Shared Or Invalid Protocol (MESI)
		https://www.cnblogs.com/z00377750/p/9180644.html
	*/
#if defined(_WIN32)
	typedef volatile LONG atom32_t;
	typedef volatile LONGLONG atom64_t;
#else
	typedef volatile int32_t atom32_t;
	typedef volatile int64_t atom64_t;
#endif

	/** @brief Performs an atomic compare-and-exchange operation on the specified values. */
	/** The function compares two specified values and exchanges with another value based on the outcome of the comparison. */
	inline int32_t atomicCompareAndSwap(atom32_t *v, int32_t new_value, int32_t old_value) {
		return InterlockedCompareExchange(v, new_value, old_value);
	}
	template<typename T>
	inline T* atomicCompareAndSwapPointer(T **v, T *new_value, T *old_value) {
		return InterlockedCompareExchange(v, new_value, old_value);
	}
	inline int64_t atomicCompareAndSwap(atom64_t *v, int64_t new_value, int64_t old_value) {
		return InterlockedCompareExchange64(v, new_value, old_value);
	}

	/** @brief Sets an integer variable to a given value as an atomic operation. */
	inline int32_t atomicSwap(atom32_t *v, int32_t new_value) {
		return InterlockedExchange(v, new_value);
	}
	template<typename T>
	inline T* atomicSwapPointer(T **v, T *new_value) {
		return InterlockedExchange(v, new_value);
	}
	inline int64_t atomicSwap(atom64_t *v, int64_t new_value) {
		return InterlockedExchange64(v, new_value);
	}

	/** @brief Support for multi-threaded atomic addition operations */
	inline int32_t atomicAdd(atom32_t *v, int32_t delta) {
#if (AYA_POINTER_SIZE == 8)
		return InterlockedAdd(v, delta);
#else
		int32_t result;
		__asm {
			__asm mov edx, v
			__asm mov eax, delta
			__asm lock xadd[edx], eax
			__asm mov result, eax
		}
		return result + delta;
#endif
	}
	inline int64_t atomicAdd(atom64_t *v, int64_t delta) {
#ifdef _WIN32
		return InterlockedAdd64(v, delta);
#endif
	}
	inline float atomicAdd(volatile float *v, float delta) {
		union bits { float f; int32_t i; };
		bits old_val, new_val;
		do {
			old_val.f = *v;
			new_val.f = old_val.f + delta;
		} while (atomicCompareAndSwap(((atom32_t *)v), new_val.i, old_val.i) != old_val.i);

		return new_val.f;
	}
}

#endif