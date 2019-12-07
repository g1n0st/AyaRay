#ifndef AYA_RNG_H
#define AYA_RNG_H

#include <stdint.h>
#include <time.h>

#include "config.h"
#include "..\math\vector3.h"

// Random Number Declarations
#define FLOAT_ONE_MINUS_EPSILON 0.99999994f

// Pbrt Mersenne twister Number
#define PCG32_DEFAULT_STATE 0x853c49e6748fea9bULL
#define PCG32_DEFAULT_STREAM 0xda3e39cb94b95bdbULL
#define PCG32_MULT 0x5851f42d4c957f2dULL

// Mersenne Twister Number
#define MT19937_BUFF_LENGTH 624

class  rng {
public:
	rng() {}
	inline rng(const uint64_t &index) {
		srand(index);
	}

	virtual inline void srand(const uint64_t &index) = 0;
	virtual inline uint32_t rand32() = 0;
	virtual inline float drand48() = 0 {
		return Min(FLOAT_ONE_MINUS_EPSILON, rand32() * 2.3283064365386963e-10f);
	}
};

// PbrtRNG
// debug(1e8): 3261
// release(1e8): 482ms
// https://github.com/mmp/pbrt-v3/blob/master/src/core/rng.h
class PbrtRNG : public rng {
private:
	uint64_t state, inc;

public:
	inline PbrtRNG() : state(PCG32_DEFAULT_STATE), inc(PCG32_DEFAULT_STREAM) {
		srand((uint64_t)time(0));
	}

	inline void srand(const uint64_t &index) {
		state = 0u;
		inc = (index << 1u) | 1u;
		rand32();
		state += PCG32_DEFAULT_STATE;
		rand32();
	}
	inline uint32_t rand32() {
		uint64_t o = state;
		state = o * PCG32_MULT + inc;
		uint32_t x_s = (uint32_t)(((o >> 18u) ^ o) >> 27u);
		uint32_t rot = (uint32_t)(o >> 59u);
		return (x_s >> rot) | (x_s << ((~rot + 1u) & 31));
	}
	inline float drand48() {
		return Min(FLOAT_ONE_MINUS_EPSILON, rand32() * 2.3283064365386963e-10f);
	}
};

// MT19937RNG 
// debug(1e8): 4727ms
// release(1e8): 2364ms
// https://en.wikipedia.org/wiki/Mersenne_Twister
class MT19937RNG : public rng {
private:
	uint32_t MT[MT19937_BUFF_LENGTH];
	uint32_t idx;

	// Generate an array of 624 untempered numbers
	inline void generateNumber() {
		for (uint32_t i = 0; i < MT19937_BUFF_LENGTH; i++) {
			int y = (MT[i] & 0x80000000)				// bit 31 (32nd bit) of MT[i]
				+ (MT[(i + 1) % 624] & 0x7fffffff);	// bits 0-30 (first 31 bits) of MT[...]
			MT[i] = MT[(i + 397) % 624] ^ (y >> 1);
			if (!(y & 1)) {
				MT[i] = MT[i] ^ 2567483615u; // 2567483615 == 0x9908b0df
			}
		}
	}

public:
	inline MT19937RNG() : idx(0) {
		srand((uint64_t)time(0));
	}
	inline void srand(const uint64_t &seed) {
		MT[0] = (uint32_t)seed;
		idx = 0;
		for (uint32_t i = 1; i < MT19937_BUFF_LENGTH; i++) {
			MT[i] = 1812433253u * (MT[i - 1] ^ (MT[i - 1] >> 30)) + i;	// 1812433253 == 0x6c078965
		}
	}
	inline uint32_t rand32() {
		if (!idx) {
			generateNumber();
		}

		int y = MT[idx];
		y = y ^ (y >> 11);
		y = y ^ ((y << 7) & 2636928640u); // 2636928640 == 0x9d2c5680
		y = y ^ ((y << 15) & 4022730752u); // 4022730752 == 0xefc60000
		y = y ^ (y >> 18);

		idx = (idx + 1) % 624;

		return y;
	}
	inline float drand48() {
		return Min(FLOAT_ONE_MINUS_EPSILON, rand32() * 2.3283064365386963e-10f * 2.f);
	}
};

typedef PbrtRNG RNG;

Point3 randomUnitDisk(RNG &rng) {
	Point3 p;
	do {
		p = (Point3(rng.drand48(), rng.drand48(), 0.f) - Point3(1.f, 1.f, 0.f)) * 2.f;
	} while (p.length2() >= 1.f);
	return p;
}
#endif