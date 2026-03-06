#pragma once
#include <cstdint>
#include <cmath>
#include <limits>
#include "core/Vector2.hpp"
#include "core/Vector3.hpp"

namespace core {
    template<typename T>
	struct Vector4 {
		T x{};
		T y{};
		T z{};
		T w{};

		Vector4() noexcept : x(0), y(0), z(0), w(0) {}
		Vector4(Vector2<T> const& xy, T const z, T const w) noexcept : x(xy.x), y(xy.y), z(z), w(w) {}
		Vector4(Vector3<T> const& xyz, T const w) noexcept : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
		Vector4(T const x, T const y, T const z, T const w) noexcept : x(x), y(y), z(z), w(w) {}

		[[nodiscard]] Vector4 operator+(Vector4 const& r) const noexcept { return Vector4(x + r.x, y + r.y, z + r.z, w + r.w); }
		[[nodiscard]] Vector4 operator-(Vector4 const& r) const noexcept { return Vector4(x - r.x, y - r.y, z - r.z, w - r.w); }
		[[nodiscard]] Vector4 operator*(Vector4 const& r) const noexcept { return Vector4(x * r.x, y * r.y, z * r.z, w * r.w); }
		[[nodiscard]] Vector4 operator/(Vector4 const& r) const noexcept { return Vector4(x / r.x, y / r.y, z / r.z, w / r.w); }

		[[nodiscard]] Vector4 operator+(T const r) const noexcept { return Vector4(x + r, y + r, z + r, w + r); }
		[[nodiscard]] Vector4 operator-(T const r) const noexcept { return Vector4(x - r, y - r, z - r, w - r); }
		[[nodiscard]] Vector4 operator*(T const r) const noexcept { return Vector4(x * r, y * r, z * r, w * r); }
		[[nodiscard]] Vector4 operator/(T const r) const noexcept { return Vector4(x / r, y / r, z / r, w / r); }

		[[nodiscard]] Vector4 operator-() const noexcept { return Vector4(-x, -y, -z, -w); }

		Vector4& operator+=(Vector4 const& r) noexcept { x += r.x; y += r.y; z += r.z; w += r.w; return *this; }
		Vector4& operator-=(Vector4 const& r) noexcept { x -= r.x; y -= r.y; z -= r.z; w -= r.w; return *this; }
		Vector4& operator*=(Vector4 const& r) noexcept { x *= r.x; y *= r.y; z *= r.z; w *= r.w; return *this; }
		Vector4& operator/=(Vector4 const& r) noexcept { x /= r.x; y /= r.y; z /= r.z; w /= r.w; return *this; }

		Vector4& operator+=(T const r) noexcept { x += r; y += r; z += z; w += r; return *this; }
		Vector4& operator-=(T const r) noexcept { x -= r; y -= r; z -= z; w -= r; return *this; }
		Vector4& operator*=(T const r) noexcept { x *= r; y *= r; z *= z; w *= r; return *this; }
		Vector4& operator/=(T const r) noexcept { x /= r; y /= r; z /= z; w /= r; return *this; }

		[[nodiscard]] bool operator==(Vector4 const& r) const noexcept { return x == r.x && y == r.y && z == r.z && w == r.w; }
		[[nodiscard]] bool operator!=(Vector4 const& r) const noexcept { return x != r.x || y != r.y || z != r.z || w != r.w; }

		[[nodiscard]] T& operator[](size_t const i) { return (&x)[i]; }

		Vector4& normalize() noexcept {
			T const l = length();
			if (l >= std::numeric_limits<T>::min()) {
				x /= l; y /= l; z /= l; w /= l;
			}
			else {
				x = T{}; y = T{}; z = T{}; w = T{};
			}
			return *this;
		}
		[[nodiscard]] Vector4 normalized() const noexcept {
			T const l = length();
			if (l >= std::numeric_limits<T>::min()) {
				return Vector4(x / l, y / l, z / l, w / l);
			}
			else {
				return Vector4();
			}
		}
		[[nodiscard]] T length() const noexcept { return std::sqrt(x * x + y * y + z * z + w * w); }
		[[nodiscard]] T dot(Vector4 const& r) const noexcept { return x * r.x + y * r.y + z * r.z + w * r.w; }
	};

	using Vector4I = Vector4<int32_t>;
	using Vector4U = Vector4<uint32_t>;
	using Vector4F = Vector4<float>;
}
