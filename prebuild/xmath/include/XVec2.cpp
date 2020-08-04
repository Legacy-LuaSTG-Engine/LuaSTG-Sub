#include "XVec2.h"

namespace xmath {
	#ifndef XMATH_USING_COCOS_VEC2
	void Vec2::normalize() {
			float n = x * x + y * y;
			// Already normalized.
			if (n == 1.0f)
				return;
			n = std::sqrt(n);
			// Too close to zero.
			if (n < std::numeric_limits<float>::min())
				return;
			n = 1.0f / n;
			x *= n;
			y *= n;
	}
	void Vec2::rotate(const Vec2& point, float angle) {
		float sinAngle = std::sin(angle);
		float cosAngle = std::cos(angle);
		if (point.isZero()) {
			float tempX = x * cosAngle - y * sinAngle;
			y = y * cosAngle + x * sinAngle;
			x = tempX;
		}
		else {
			float tempX = x - point.x;
			float tempY = y - point.y;
			x = tempX * cosAngle - tempY * sinAngle + point.x;
			y = tempY * cosAngle + tempX * sinAngle + point.y;
		}
	}
	const Vec2 Vec2::ZERO(0.0f, 0.0f);
	#endif // XMATH_USING_COCOS_VEC2
}
