#pragma once
#include "XVec2.h"

namespace xmath
{
	namespace spline
	{
		float Cardinal(float t, float p1, float p2, float p3, float p4, float alpha);
		float CatmullRom(float t, float p1, float p2, float p3, float p4);
		Vec2 CentripetalCatmullRom(float t,
			const Vec2& p1, const Vec2& p2, const Vec2&, const Vec2& p4);
	}
}
