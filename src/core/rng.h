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

	Point3 randomUnitDisk() {
		Point3 p;
		do {
			p = (Point3(drand48(), drand48(), 0.f) - Point3(1.f, 1.f, 0.f)) * 2.f;
		} while (p.length2() >= 1.f);
		return p;
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

// Hash lookup table as defined by Ken Perlin.  This is a randomly
// arranged array of all numbers from 0-255 inclusive.
const int PERLIN_PERM[] = { 151,160,137,91,90,15,                 
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23, 190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166, 77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196, 135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42, 223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228, 251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254, 138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23, 190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166, 77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196, 135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42, 223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228, 251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254, 138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

// https://www.cnblogs.com/leoin2012/p/7218033.html
class PerlinNoise {
public:
	float m_repeat;

private:
// Fade function as defined by Ken Perlin.  This eases coordinate values
// so that they will ease towards integral values.  This ends up smoothing
// the final output.
	inline float fade(const float &t) const {
		return t * t * t * (t * (t * 6 - 15) + 10);         // 6t^5 - 15t^4 + 10t^3
	}
	inline int inc(const int &x) const {
		if (m_repeat > 0) {
			return (int)fmodf((float)(x + 1), m_repeat);
		}
		return x + 1;
	}
	inline float grad(const int &hash, const float &x, const float &y, const float &z) const {
		int h = hash & 15;
		float u = h < 8 /* b1000 */ ? x : y;

		float v;

		if (h < 4 /* 0b1000 */) {
			v = y;
		}
		else if (h == 12 /* 0b1100 */ || h == 14 /* 0b1110 */) {
			v = x;
		}
		else {
			v = z;
		}

		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}

public:
	PerlinNoise() { m_repeat = 0; }
	PerlinNoise(const float &rep) : m_repeat(rep) {}

	virtual float turb(const Point3 &p) const {
		float x = p.m_val[0];
		float y = p.m_val[1];
		float z = p.m_val[2];
		if (m_repeat > 0) {
			x = fmodf(x, m_repeat);
			y = fmodf(y, m_repeat);
			z = fmodf(z, m_repeat);
		}

		int xi = (int)x & 255;
		int yi = (int)y & 255;
		int zi = (int)z & 255;
		float xf = x - (int)x;
		float yf = y - (int)y;
		float zf = z - (int)z;

		float u = fade(xf);
		float v = fade(yf);
		float w = fade(zf);

		int aaa = PERLIN_PERM[PERLIN_PERM[PERLIN_PERM[xi] + yi] + zi];
		int aba = PERLIN_PERM[PERLIN_PERM[PERLIN_PERM[xi] + inc(yi)] + zi];
		int aab = PERLIN_PERM[PERLIN_PERM[PERLIN_PERM[xi] + yi] + inc(zi)];
		int abb = PERLIN_PERM[PERLIN_PERM[PERLIN_PERM[xi] + inc(yi)] + inc(zi)];
		int baa = PERLIN_PERM[PERLIN_PERM[PERLIN_PERM[inc(xi)] + yi] + zi];
		int bba = PERLIN_PERM[PERLIN_PERM[PERLIN_PERM[inc(xi)] + inc(yi)] + zi];
		int bab = PERLIN_PERM[PERLIN_PERM[PERLIN_PERM[inc(xi)] + yi] + inc(zi)];
		int bbb = PERLIN_PERM[PERLIN_PERM[PERLIN_PERM[inc(xi)] + inc(yi)] + inc(zi)];

		float x1, x2, y1, y2;
		x1 = Lerp(u,
			grad(aaa, xf, yf, zf),
			grad(baa, xf - 1.f, yf, zf));
		x2 = Lerp(u,
			grad(aba, xf, yf - 1.f, zf),
			grad(bba, xf - 1.f, yf - 1.f, zf));
		y1 = Lerp(v, x1, x2);

		x1 = Lerp(u,
			grad(aab, xf, yf, zf - 1.f),
			grad(bab, xf - 1.f, yf, zf - 1.f));
		x2 = Lerp(u,
			grad(abb, xf, yf - 1.f, zf - 1.f),
			grad(bbb, xf - 1.f, yf - 1.f, zf - 1.f));
		y2 = Lerp(v, x1, x2);

		float r = (Lerp(w, y1, y2) + 1.f) / 2.f;
		return (Lerp(w, y1, y2) + 1.f) / 2.f;
	}
};

#endif