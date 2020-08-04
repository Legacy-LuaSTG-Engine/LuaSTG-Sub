#include "XEquation.h"
#include "XConstant.h"

using namespace xmath;
using xmath::Complex;

std::array<double, 2> equation::SolveLiner(double a1, double b1, double c1, double a2, double b2, double c2)
{
	const auto den = a2 * b1 - a1 * b2;
	return { (c2*b1 - c1 * b2) / den,(c1*a2 - c2 * a1) / den };
}

std::array<Complex, 2> equation::SolveQuadratic(double a, double b, double c)
{
	if (a == 0.0)
	{
		const auto x = Complex(-c / b);
		return { x,x };
	}
	else
	{
		const auto delta_sq = std::sqrt(Complex(b*b - 4 * a*c));
		const auto den = a * 2;
		return { (-b + delta_sq) / den,(-b - delta_sq) / den };
	}
}

std::array<Complex, 3> equation::SolveCubic(double a, double b, double c, double d)
{
	const auto A = b * b - 3 * a*c;
	const auto B = b * c - 9 * a*d;
	const auto C = c * c - 3 * b*d;
	const auto delta = B * B - 4 * A*C;
	if (A == 0.0&&B == 0.0)
	{
		const auto r = Complex(-c / b);
		return { r,r,r };
	}
	else if (delta > 0.0)
	{
		const auto tmp = std::sqrt(delta);
		const auto y1_cbrt = std::cbrt(A * b + 3 * a*(-B + tmp) / 2);
		const auto y2_cbrt = std::cbrt(A * b + 3 * a*(-B - tmp) / 2);
		const auto tmp2 = y1_cbrt + y2_cbrt;
		const auto a3 = a * 3;
		const auto x1 = Complex((-b - tmp2) / a3);
		const auto real = (-b + 0.5*tmp2) / a3;
		const auto img = (y1_cbrt - y2_cbrt)*sqrt3_2 / a3;
		const auto x2 = Complex(real, img);
		const auto x3 = Complex(real, -img);
		return { x1,x2,x3 };
	}
	else if (delta == 0.0)
	{
		const auto K = B / A;
		return { Complex(-b / a + K),Complex(-K / 2),Complex(-K / 2) };
	}
	else// if (delta < 0.0)
	{
		const auto A_sqrt = std::sqrt(A);
		const auto T = (2 * A*b - 3 * a*B) / (2 * A*A_sqrt);
		const auto theta_3 = std::acos(T) / 3;
		const auto tcos = std::cos(theta_3);
		const auto tsin = std::sin(theta_3);
		const auto a3 = a * 3;
		const auto x1 = (-b - 2 * A_sqrt*tcos) / a3;
		const auto x2 = (-b + A_sqrt * (tcos + tsin * sqrt3)) / a3;
		const auto x3 = (-b + A_sqrt * (tcos - tsin * sqrt3)) / a3;
		return { x1,x2,x3 };
	}
}

std::array<Complex, 3> equation::SolveCubic2(double a, double b, double c, double d)
{
	const auto D = b * b - 3 * a*c;
	const auto Dsq = std::sqrt(std::abs(D));
	const auto r = (9 * a*b*c - 2 * b*b*b - 27 * a*a*d) / (2 * Dsq*Dsq*Dsq);
	const auto den = a * 3;
	if (D < 0.0)
	{
		const auto K = std::cbrt(r + std::sqrt(r*r + 1));
		const auto tmp = Dsq * (K - 1 / K);
		const auto tmp1 = (-b - tmp * 0.5) / den;
		const auto tmp2 = (Dsq * (K + 1 / K)*sqrt3_2) / den;
		return { Complex((-b + tmp) / den),Complex(tmp1,tmp2),Complex(tmp1,-tmp2) };
	}
	else if (D > 0.0)
	{
		const auto r_abs = std::abs(r);
		if (r_abs > 1.0)
		{
			const auto K = std::cbrt(r + std::sqrt(r*r - 1));
			const auto tmp = Dsq * (K + 1 / K);
			const auto tmp1 = (-b - tmp * 0.5) / den;
			const auto tmp2 = (Dsq * (K - 1 / K)*sqrt3_2) / den;
			return { Complex((-b + tmp) / den),Complex(tmp1,tmp2),Complex(tmp1,-tmp2) };
		}
		else if (r_abs < 1.0)
		{
			const auto th = std::acos(r) / 3;
			const auto tmp = Dsq * 2;
			return {
				Complex((-b + tmp * std::cos(th)) / den),
				Complex((-b + tmp * std::cos(th + pix2_3)) / den),
				Complex((-b + tmp * std::cos(th - pix2_3)) / den) };
		}
		else if (r > 0.0)//r=1
		{
			const auto x = Complex((-b - Dsq) / den);
			return { Complex((-b + 2 * Dsq) / den),x,x };
		}
		else//r=-1
		{
			const auto x = Complex((-b + Dsq) / den);
			return { Complex((-b - 2 * Dsq) / den),x,x };
		}
	}
	else
	{
		const auto delta = b*b*b - 27 * a*a*d;
		if (delta == 0.0)
		{
			const auto x = Complex(-b / den);
			return { x,x,x };
		}
		else
		{
			const auto tmp = std::cbrt(delta);
			return {
				Complex((tmp - b) / den),
				(tmp*complex_omiga_3 - b) / den ,
				(tmp*complex_omiga_3c - b) / den };
		}
	}
}

std::array<Complex, 4> equation::SolveQuartic(double a, double b, double c, double d, double e)
{
	const double P = (c*c + 12 * a*e - 3 * b*d) / 9;
	const auto Q = (27 * a*d*d + 2 * c*c*c + 27 * b*b*e - 72 * a*c*e - 9 * b*c*d) / 54;
	const auto D = std::sqrt(Complex(Q*Q - P*P*P, 0.f));
	Complex u;
	if (D.real() * Q > 0.f)
		u = Q + D;
	else
		u = Q - D;
	u = cbrt(u);
	const auto v = (u == complex_zero) ? complex_zero : P / u;
	double m_abs = -1.f;
	Complex m;
	const auto tmp = b * b - 8.0 / 3.0 * a*c;
	Complex tmp2;
	for (auto k = 1; k < 4; k++)
	{
		const auto t = (std::pow(complex_omiga_3, k - 1)*u + std::pow(complex_omiga_3, 4 - k)*v)*4.0*a;
		const auto mm = std::sqrt(t + tmp);
		const auto mm_abs = std::abs(mm);
		if (mm_abs > m_abs)
		{
			m_abs = mm_abs;
			m = mm;
			tmp2 = t;
		}
	}
	Complex S, T;
	if (m_abs == 0.0)
	{
		m = complex_zero;
		S = tmp;
		T = complex_zero;
	}
	else
	{
		S = tmp * 2 - tmp2;
		T = (8 * a*b*c - 16 * a*a*d - 2 * b*b*b) / m;
	}
	const auto sq1 = std::sqrt(S - T);
	const auto sq2 = std::sqrt(S + T);
	const auto b1 = -b - m;
	const auto b2 = -b + m;
	const auto a4 = a * 4;
	return {
		(b1 + sq1) / a4,
		(b1 - sq1) / a4,
		(b2 + sq2) / a4,
		(b2 - sq2) / a4
	};
}
