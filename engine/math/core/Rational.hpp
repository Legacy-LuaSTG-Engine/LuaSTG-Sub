#pragma once
#include <cstdint>

namespace core {
    struct Rational {
		uint32_t numerator; // 分子
		uint32_t denominator; // 分母

		Rational() : numerator(0), denominator(0) {}
		Rational(uint32_t const numerator_) : numerator(numerator_), denominator(1) {}
		Rational(uint32_t const numerator_, uint32_t const denominator_) : numerator(numerator_), denominator(denominator_) {}
	};
}
