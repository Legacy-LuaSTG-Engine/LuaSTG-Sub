#include "E2DMath.hpp"

double Eyes2D::Math::LinearToLog(double v) {
	if (v <= 0.0) {
		return 0.0;
	}
	else {
		v = std::fmin(v, 1.0);
		double dB = Eyes2D::Math::ValueConvertTodB(v);
		double rate = (dB + 100.0) / 100.0;
		return rate;
	}
}

double Eyes2D::Math::LogToLinear(double d) {
	if (d <= 0.0) {
		return 0.0;
	}
	else {
		d = std::fmin(d, 1.0);
		double dB = 100 * d - 100;
		double rate = Eyes2D::Math::dBConvertToValue(dB);
		return rate;
	}
}
