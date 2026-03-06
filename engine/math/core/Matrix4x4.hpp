#pragma once
#include <cstdint>
#include <cmath>
#include <limits>
#include "core/Vector4.hpp"

namespace core {
    template<typename T>
	struct Matrix4 {
		union {
			struct {
				T m11, m12, m13, m14;
				T m21, m22, m23, m24;
				T m31, m32, m33, m34;
				T m41, m42, m43, m44;
			} s;
			Vector4<T> rows[4];
			T m[4][4];
		} u;

		[[nodiscard]] bool operator==(Matrix4 const& other) const noexcept {
			return u.s.m11 == other.u.s.m11
				&& u.s.m12 == other.u.s.m12
				&& u.s.m13 == other.u.s.m13
				&& u.s.m14 == other.u.s.m14
				&& u.s.m21 == other.u.s.m21
				&& u.s.m22 == other.u.s.m22
				&& u.s.m23 == other.u.s.m23
				&& u.s.m24 == other.u.s.m24
				&& u.s.m31 == other.u.s.m31
				&& u.s.m32 == other.u.s.m32
				&& u.s.m33 == other.u.s.m33
				&& u.s.m34 == other.u.s.m34
				&& u.s.m41 == other.u.s.m41
				&& u.s.m42 == other.u.s.m42
				&& u.s.m43 == other.u.s.m43
				&& u.s.m44 == other.u.s.m44;
		}
		[[nodiscard]] bool operator!=(Matrix4 const& other) const noexcept {
			return u.s.m11 != other.u.s.m11
				|| u.s.m12 != other.u.s.m12
				|| u.s.m13 != other.u.s.m13
				|| u.s.m14 != other.u.s.m14
				|| u.s.m21 != other.u.s.m21
				|| u.s.m22 != other.u.s.m22
				|| u.s.m23 != other.u.s.m23
				|| u.s.m24 != other.u.s.m24
				|| u.s.m31 != other.u.s.m31
				|| u.s.m32 != other.u.s.m32
				|| u.s.m33 != other.u.s.m33
				|| u.s.m34 != other.u.s.m34
				|| u.s.m41 != other.u.s.m41
				|| u.s.m42 != other.u.s.m42
				|| u.s.m43 != other.u.s.m43
				|| u.s.m44 != other.u.s.m44;
		}

		[[nodiscard]] static Matrix4 identity() noexcept {
			Matrix4 m{};
			m.u.s.m11 = static_cast<T>(1);
			m.u.s.m22 = static_cast<T>(1);
			m.u.s.m33 = static_cast<T>(1);
			m.u.s.m44 = static_cast<T>(1);
			return m;
		}
	};

	using Matrix4F = Matrix4<float>;
}
