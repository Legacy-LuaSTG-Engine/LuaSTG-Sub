#include "XIntersect.h"
#include "XDistance.h"
#include "XMath.h"

using namespace xmath;

bool intersect::Point_AABB(const Vec2& p0, const Vec2& p1, float halfW, float halfH)
{
	const auto dx = p0.x - p1.x;
	const auto dy = p0.y - p1.y;
	return -halfW < dx&&dx < halfW&&-halfH < dy&&dy < halfH;
}

bool intersect::Point_OBB(const Vec2& p0, const Vec2& p1, float halfW, float halfH, float rot)
{
	auto p = p0;
	p.rotate(p1, -rot);
	return Point_AABB(p, p1, halfW, halfH);
}

bool intersect::Point_Diamond(const Vec2& p0, const Vec2& p1, float a, float b, float rot)
{
	auto p = p0 - p1;
	p.rotate(Vec2::ZERO, -rot);
	const auto x_ = p.x / a;
	const auto y_ = p.y / b;
	const auto sum = x_ + y_;
	const auto dif = x_ - y_;
	return -1 < sum&&sum < 1 && -1 < dif&&dif < 1;
}

bool intersect::Point_Ellipse(const Vec2& p0, const Vec2& p1, float a, float b, float rot)
{
	if (a == b)
		return Point_Circle(p0, p1, a);
	auto p = p0 - p1;
	p.rotate(Vec2::ZERO, -rot);
	const auto x = p.x;
	const auto y = p.y;
	return x * x / (a * a) + y * y / (b * b) < 1;
}

bool intersect::Point_Triangle(const Vec2& p,
	const Vec2& A, const Vec2& B, const Vec2& C)
{
	return Point_Triangle(p - B, A - B, C - B);
}

bool intersect::Point_Triangle(const Vec2& P, const Vec2& E0, const Vec2& E1)
{
	const auto _den = E0.y*E1.x - E0.x*E1.y;
	const auto s = (P.y*E1.x - P.x*E1.y) / _den;
	const auto t = (P.x*E0.y - P.y*E0.x) / _den;
	return s > 0 && t > 0 && s + t < 1;
}

bool intersect::Point_Parallelogram(const Vec2& p0,
	const Vec2& p1, const Vec2& halfDiagA, const Vec2& halfDiagB)
{
	const auto E0 = halfDiagA + halfDiagB;
	const auto E1 = halfDiagA - halfDiagB;
	const auto P = p0 - p1 + halfDiagA;
	const auto _den = E0.y*E1.x - E0.x*E1.y;
	const auto s = (P.y*E1.x - P.x*E1.y) / _den;
	const auto t = (P.x*E0.y - P.y*E0.x) / _den;
	return 0 < s&&s < 1 && 0 < t&&t < 1;
}

bool intersect::OBB_Circle(const Vec2& p0, float halfW, float halfH, float rot,
	const Vec2& p1, float r)
{
	float tSin, tCos;
	SinCos(rot, tSin, tCos);
	const auto d = p0 - p1;
	const auto dw = std::max(0.f, std::abs(tCos * d.x + tSin * d.y) - halfW);
	const auto dh = std::max(0.f, std::abs(-tSin * d.x + tCos * d.y) - halfH);
	return r * r >= dh * dh + dw * dw;
}

bool intersect::OBB_OBB(const Vec2& p0, float halfW0, float halfH0, float rot0,
	const Vec2& p1, float halfW1, float halfH1, float rot1)
{
	float tSin0, tCos0;
	SinCos(rot0, tSin0, tCos0);
	float tSin1, tCos1;
	SinCos(rot1, tSin1, tCos1);
	Vec2 e[] = {
		{tCos0, tSin0},//e00
		{-tSin0, tCos0},//e01
		{tCos1, tSin1},//e10
		{-tSin1, tCos1}//e11
	};
	float projOther[] = { halfW0,halfH0,halfW1,halfH1 };
	const auto d = p0 - p1;
	for (size_t i = 0; i < 4; i++)
	{
		const auto ii = 2 - size_t(i / 2) * 2;//2200
		const auto v0 = e[ii] * projOther[ii];
		const auto v1 = e[ii + 1] * projOther[ii + 1];
		const auto ex = e[i].x;
		const auto ey = e[i].y;
		const auto projHalfDiag = std::max(
			std::abs(ex*(v0.x + v1.x) + ey * (v0.y + v1.y)),
			std::abs(ex*(v0.x - v1.x) + ey * (v0.y - v1.y))
		);
		if (projHalfDiag + projOther[i] < std::abs(ex * d.x + ey * d.y))
			return false;
	}
	return true;
}

bool intersect::OBB_Line(const Vec2& p0, float halfW0, float halfH0, float rot0,
	const Vec2& p1, float rot1)
{
	float tSin0, tCos0;
	SinCos(rot0, tSin0, tCos0);
	float tSin1, tCos1;
	SinCos(rot1, tSin1, tCos1);
	const Vec2 e00(tCos0, tSin0);
	const Vec2 e01(-tSin0, tCos0);
	const Vec2 halfDiag0 = e00 * halfW0 + e01 * halfH0;
	const Vec2 halfDiag1 = e00 * halfW0 - e01 * halfH0;
	Vec2 eProj(-tSin1, tCos1);
	const auto halfProj = std::max(
		std::abs(eProj.dot(halfDiag0)),
		std::abs(eProj.dot(halfDiag1)));
	const auto d = distance::Point_Line(p0, p1, tCos1, tSin1);
	return d <= halfProj;
}

bool intersect::OBB_Triangle(
	const Vec2& p, float halfW, float halfH, float rot,
	const Vec2& A, const Vec2& B, const Vec2& C)
{
	float tSin, tCos;
	SinCos(rot, tSin, tCos);
	const Vec2 hw(tCos*halfW, tSin*halfW);
	const Vec2 hh(-tSin * halfH, tCos*halfH);
	const auto v0 = p + hw + hh;
	const auto v1 = p + hw - hh;
	const auto v2 = p - hw - hh;
	const auto v3 = p - hw + hh;
	return Triangle_Triangle(A, B, C, v0, v1, v2) || Triangle_Triangle(A, B, C, v0, v3, v2);
}

bool intersect::OBB_Diamond(const Vec2& p0, float halfW, float halfH, float rot0,
	const Vec2& p1, float a, float b, float rot1)
{
	float tSin0, tCos0, tSin1, tCos1;
	SinCos(rot0, tSin0, tCos0);
	SinCos(rot1, tSin1, tCos1);
	const Vec2 hw(tCos0*halfW, tSin0*halfW);
	const Vec2 hh(-tSin0*halfH, tCos0*halfH);
	return Parallelogram_Parallelogram(
		p0, hw + hh, hw - hh,
		p1, Vec2(tCos1*a, tSin1*a), Vec2(-tSin1 * b, tCos1*b));
}

bool intersect::OBB_Ellipse(const Vec2& p0, float halfW, float halfH, float rot0,
	const Vec2& p1, float a, float b, float rot1)
{
	if (a == b)
		return OBB_Circle(p0, halfW, halfH, rot0, p1, a);
	float tSin0, tCos0;
	SinCos(rot0, tSin0, tCos0);
	float tSin1, tCos1;
	SinCos(rot1, tSin1, tCos1);
	const auto e00 = Vec2(tCos0, tSin0);
	const auto e01 = Vec2(-tSin0, tCos0);
	const auto e11 = Vec2(-tSin1, tCos1);
	const auto f = e11 * (a / b - 1);
	const auto p0_ = p0 + distance::Point_Line_signed(p0, p1, e11) * f;
	const auto tmp = e00 * halfW + p0;
	const auto vDiag0 = tmp + e01 * halfH;
	const auto vDiag1 = tmp - e01 * halfH;
	const auto vDiag0_ = vDiag0 + distance::Point_Line_signed(vDiag0, p1, e11) * f;
	const auto vDiag1_ = vDiag1 + distance::Point_Line_signed(vDiag1, p1, e11) * f;
	const auto halfDiag0_ = vDiag0_ - p0_;
	const auto halfDiag1_ = vDiag1_ - p0_;
	const auto d = distance::Point_Parallelogram(p1, p0_, halfDiag0_, halfDiag1_);
	return d <= a;
}

bool intersect::Circle_Ellipse(const Vec2& p0, float r, const Vec2& p1, float a, float b, float rot)
{
	return distance::Point_Ellipse2(p0, p1, a, b, rot) <= r;
}

bool intersect::Circle_Diamond(const Vec2& p0, float r, const Vec2& p1, float a, float b, float rot)
{
	return distance::Point_Diamond(p0, p1, a, b, rot) <= r;
}

bool intersect::Circle_Triangle(const Vec2& p, float r,
	const Vec2& A, const Vec2& B, const Vec2& C)
{
	return distance::Point_Triangle(p, A, B, C) <= r;
}

bool intersect::Ellipse_Ellipse(
	const Vec2& p0, float a0, float b0, float rot0,
	const Vec2& p1, float a1, float b1, float rot1)
{
	if (a0 == b0)
		return Circle_Ellipse(p0, a0, p1, a1, b1, rot1);
	if (a1 == b1)
		return Circle_Ellipse(p1, a1, p0, a0, b0, rot0);
	float s, c;
	SinCos(rot1 - rot0, s, c);
	const auto c2 = c * c;
	const auto s2 = s * s;
	const auto sc = s * c;
	const auto a_ = 1 / (a1*a1);
	const auto b_ = 1 / (b1*b1);
	const auto m00 = (a_ * c2 + b_ * s2) * (a0*a0);
	const auto m11 = (b_ * c2 + a_ * s2) * (b0*b0);
	const auto m01 = (a_ - b_)*sc * (a0*b0);
	const auto sum = m00 + m11;
	const auto tmp = m00 - m11;
	const auto dif = std::sqrt(tmp*tmp + 4 * m01*m01);
	const auto tanv = 2 * m01 / (dif + m00 - m11);
	float s0, c0;
	SinCos(-rot0, s0, c0);
	const auto d = p1 - p0;
	auto d_ = Vec2(d.x*c0 - d.y*s0, d.y*c0 + d.x*s0);
	d_.x /= a0;
	d_.y /= b0;
	return distance::Point_Ellipse2(
		Vec2::ZERO, d_,
		std::sqrt(2 / (sum + dif)), std::sqrt(2 / (sum - dif)), std::atan(tanv)) <= 1;
}

bool intersect::Ellipse_Diamond(const Vec2& p0, float a0, float b0, float rot0,
	const Vec2& p1, float a1, float b1, float rot1)
{
	if (a0 == b0)
		return Circle_Diamond(p0, a0, p1, a1, b1, rot1);
	float s, c;
	SinCos(rot1 - rot0, s, c);
	const auto fac = a0 / b0;
	auto p = p1 - p0;
	p.rotate(Vec2::ZERO, -rot0);
	p.y *= fac;
	return distance::Point_Parallelogram(
		Vec2::ZERO, p, Vec2(c*a1, s*a1*fac), Vec2(-s * b1, c*b1*fac)) <= a0;
}

bool intersect::Ellipse_Triangle(const Vec2& p, float a, float b, float rot,
	const Vec2& A, const Vec2& B, const Vec2& C)
{
	if (a == b)
		return Circle_Triangle(p, a, A, B, C);
	float s, c;
	SinCos(-rot, s, c);
	const auto fac = a / b;
	const auto PA = A - p;
	const auto PB = B - p;
	const auto PC = C - p;
	const Vec2 A_(PA.x*c - PA.y*s, (PA.y*c + PA.x*s)*fac);
	const Vec2 B_(PB.x*c - PB.y*s, (PB.y*c + PB.x*s)*fac);
	const Vec2 C_(PC.x*c - PC.y*s, (PC.y*c + PC.x*s)*fac);
	return Point_Triangle(Vec2::ZERO, A_, B_, C_);
}

bool intersect::Segment_Segment(const Vec2& A0, const Vec2& B0,
	const Vec2& A1, const Vec2& B1)
{
	const auto A0B0 = B0 - A0;
	const auto A0A1 = A1 - A0;
	const auto A0B1 = B1 - A0;
	const auto c1 = A0B0.cross(A0A1);
	const auto c2 = A0B0.cross(A0B1);
	if (c1 == 0.f&&c2 == 0.f)
	{
		if (A0B0.lengthSquared() == 0.f)
			return false;
		const auto t1 = (A0B0.x == 0.f) ? A0A1.y / A0B0.y : A0A1.x / A0B0.x;
		const auto t2 = (A0B0.x == 0.f) ? A0B1.y / A0B0.y : A0B1.x / A0B0.x;
		return (0 < t1&&t1 < 1) ||
			(0 < t2&&t2 < 1) ||
			(t1 < 0 && 1 < t2) ||
			(t2 < 0 && 1 < t1);
	}
	if (c1*c2 > 0)
		return false;
	const auto A1B1 = B1 - A1;
	//if (A1B1.lengthSquared() == 0.f)
	//	return false;
	const auto c3 = -A1B1.cross(A0A1);
	const auto c4 = A1B1.cross(B0 - A1);
	return c3 * c4 < 0;
}

bool intersect::Triangle_Triangle(const Vec2& A0, const Vec2& B0, const Vec2& C0,
	const Vec2& A1, const Vec2& B1, const Vec2& C1)
{
	const auto E00 = A0 - B0;
	const auto E01 = C0 - B0;
	for (auto& p : { A1,B1,C1 })
	{
		if (Point_Triangle(p - B0, E00, E01))
			return true;
	}
	const auto E10 = A1 - B1;
	const auto E11 = C1 - B1;
	for (auto& p : { A0,B0,C0 })
	{
		if (Point_Triangle(p - B1, E10, E11))
			return true;
	}
	for (auto& p0 : { A0,C0 })
	{
		for (auto& p1 : { A1,C1 })
		{
			if (Segment_Segment(B0, p0, B1, p1))
				return true;
		}
	}
	return false;
}

bool intersect::Parallelogram_Parallelogram(
	const Vec2& p0, const Vec2& halfDiagA0, const Vec2& halfDiagB0,
	const Vec2& p1, const Vec2& halfDiagA1, const Vec2& halfDiagB1)
{
	const auto d01 = p1 - p0;
	for (auto& e : {
		(halfDiagA0 + halfDiagB0),
		(halfDiagA0 - halfDiagB0),
		(halfDiagA1 + halfDiagB1),
		(halfDiagA1 - halfDiagB1)})
	{
		const auto ep = e.getPerp().getNormalized();
		const auto proj0 = std::max(std::abs(ep.dot(halfDiagA0)), std::abs(ep.dot(halfDiagB0)));
		const auto proj1 = std::max(std::abs(ep.dot(halfDiagA1)), std::abs(ep.dot(halfDiagB1)));
		if (proj0 + proj1 < std::abs(ep.dot(d01)))
			return false;
	}
	return true;
}

bool intersect::Diamond_Diamond(
	const Vec2& p0, float a0, float b0, float rot0,
	const Vec2& p1, float a1, float b1, float rot1)
{
	float tSin0, tCos0, tSin1, tCos1;
	SinCos(rot0, tSin0, tCos0);
	SinCos(rot1, tSin1, tCos1);
	return Parallelogram_Parallelogram(
		p0, Vec2(tCos0*a0, tSin0*a0), Vec2(-tSin0 * b0, tCos0*b0),
		p1, Vec2(tCos1*a1, tSin1*a1), Vec2(-tSin1 * b1, tCos1*b1));
}

bool intersect::Diamond_Triangle(const Vec2& p, float a, float b, float rot,
	const Vec2& A, const Vec2& B, const Vec2& C)
{
	float tSin, tCos;
	SinCos(rot, tSin, tCos);
	const Vec2 hd0(tCos*a, tSin*a);
	const Vec2 hd1(-tSin*b, tCos*b);
	const auto v0 = p + hd0;
	const auto v1 = p + hd1;
	const auto v2 = p - hd0;
	const auto v3 = p - hd1;
	return Triangle_Triangle(A, B, C, v0, v1, v2) || Triangle_Triangle(A, B, C, v0, v3, v2);
}

bool intersect::Line_Circle(const Vec2& p0, float r, const Vec2& p1, float rot)
{
	float tSin0, tCos0;
	SinCos(rot, tSin0, tCos0);
	return distance::Point_Line(p0, p1, tCos0, tSin0) <= r;
}

