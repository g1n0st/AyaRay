#ifndef AYA_MATH_VECTOR2
#define AYA_MATH_VECTOR2

#include "MathUtility.h"

namespace Aya {
	template<class T>
	class Vector2 {
	public:
		union {
			struct { T u, v; };
			struct { T x, y; };
		};

	public:
		__forceinline Vector2() : x(0), y(0) {}
		__forceinline Vector2(const T &x1, const T &y1) : x(x1), y(y1) {}
		__forceinline Vector2(const Vector2 &rhs) : x(rhs.x), y(rhs.y) {}
		__forceinline Vector2& operator =(const Vector2 &rhs) {
			x = rhs.x;
			y = rhs.y;
			return *this;
		}

		__forceinline void setValue(const T &x1, const T &y1) {
			x = x1;
			y = y1;
		}

		__forceinline bool operator == (const Vector2 &v) const {
			return (x == v.x) && (y == v.y);
		}
		__forceinline bool operator != (const Vector2 &v) const {
			return !((*this) == v);
		}
		__forceinline bool isZero() const {
			return (x == T(0) && y == T(0));
		}

		__forceinline Vector2 operator + (const Vector2 &v) const {
			return Vector2(x + v.x, y + v.y);
		}
		__forceinline Vector2 & operator += (const Vector2 &v) {
			x += v.x;
			y += v.y;
			return *this;
		}
		__forceinline Vector2 operator - (const Vector2 &v) const {
			return Vector2(x - v.x, y - v.y);
		}
		__forceinline Vector2 & operator -= (const Vector2 &v) {
			x -= v.x;
			y -= v.y;
			return *this;
		}
		__forceinline Vector2 operator- () const {
			return Vector2(-x, -y);
		}
		__forceinline Vector2 operator *(const T &s) const {
			return Vector2(x * s, y * s);
		}
		__forceinline Vector2 & operator *= (const T &s) {
			x *= s;
			y *= s;
			return *this;
		}
		__forceinline friend Vector2 operator * (const float &s, const Vector2 &v) {
			return v * s;
		}
		__forceinline Vector2 operator /(const T &s) const {
			assert(s != T(0));
			return Vector2(x / s, y / s);
		}
		__forceinline Vector2 & operator /= (const T &s) {
			assert(s != T(0));
			x /= s;
			y /= s;
			return *this;
		}
		__forceinline Vector2 operator << (const uint32_t &v) const {
			return Vector2(x << v.x, y << v.y);
		}
		__forceinline Vector2 & operator <<= (const uint32_t &v) {
			x <<= v.x;
			y <<= v.y;
			return *this;
		}
		__forceinline Vector2 operator >> (const uint32_t &v) const {
			return Vector2(x >> v.x, y >> v.y);
		}
		__forceinline Vector2 & operator >>= (const uint32_t &v) {
			x >>= v.x;
			y >>= v.y;
			return *this;
		}

		friend __forceinline std::ostream &operator<<(std::ostream &os, const Vector2 &v) {
			os << "[ " << v.x
				<< ", " << v.y
				<< " ]";
			return os;
		}

		__forceinline float length() const {
			return Sqrt(float(x) * float(x) + float(y) * float(y));
		}
	};

	typedef Vector2<int> Vector2i;
	typedef Vector2<float> Vector2f;
}

#endif