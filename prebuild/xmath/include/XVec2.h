#pragma once
//#define XMATH_USING_COCOS_VEC2 //If you want to use cocos Vec2 class.

#ifdef XMATH_USING_COCOS_VEC2
#include "math/Vec2.h"
#else
#include <cmath>
#include <algorithm>
#include <limits>
#endif // XMATH_USING_COCOS_VEC2

namespace xmath {
#ifdef XMATH_USING_COCOS_VEC2
	using Vec2 = cocos2d::Vec2;
#else
	struct Vec2 {
		float x;
		float y;
		inline bool isZero() const { return x == 0.0f && y == 0.0f; }
		inline float cross(const Vec2& other) const { return x * other.y - y * other.x; }
		inline float length() const { return std::sqrtf(x * x + y * y); }
		inline float dot(const Vec2& v) const { return (x * v.x + y * v.y); }
		void normalize();
		inline Vec2 rotate(const Vec2& other) const {
			return Vec2(x * other.x - y * other.y, x * other.y + y * other.x);
		}
		void rotate(const Vec2& point, float angle);
		inline float lengthSquared() const { return (x * x + y * y); }
		inline Vec2 getPerp() const { return Vec2(-y, x); }
		inline Vec2 getNormalized() const {
			Vec2 v(*this);
			v.normalize();
			return v;
		}
		inline Vec2 operator+(const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
		inline Vec2 operator-(const Vec2& v) const { return Vec2(x - v.x, y - v.y); }
		inline Vec2 operator-() const { return Vec2(-x, -y); }
		inline bool operator==(const Vec2& v) const { return (x == v.x && y == v.y); }
		inline Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
		inline Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
		Vec2(float xx, float yy) { x = xx; y = yy; }
		Vec2(const Vec2& v) { x = v.x; y = v.y; }
		static const Vec2 ZERO;
	};
	inline Vec2 operator*(float x, const Vec2& v) {
		return Vec2(v.x * x, v.y * x);
	}
#endif // XMATH_USING_COCOS_VEC2
}
