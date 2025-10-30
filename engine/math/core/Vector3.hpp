#pragma once
#include <cstdint>
#include <cmath>
#include <limits>
#include "core/Vector2.hpp"

namespace core {
    template<typename T>
	struct Vector3 {
		T x{};
		T y{};
		T z{};

		Vector3() noexcept : x(0), y(0), z(0) {}
		Vector3(Vector2<T> const& xy, T const z) noexcept : x(xy.x), y(xy.y), z(z) {}
		Vector3(T const x, T const y, T const z) noexcept : x(x), y(y), z(z) {}

		[[nodiscard]] Vector3 operator+(Vector3 const& r) const noexcept { return Vector3(x + r.x, y + r.y, z + r.z); }
		[[nodiscard]] Vector3 operator-(Vector3 const& r) const noexcept { return Vector3(x - r.x, y - r.y, z - r.z); }
		[[nodiscard]] Vector3 operator*(Vector3 const& r) const noexcept { return Vector3(x * r.x, y * r.y, z * r.z); }
		[[nodiscard]] Vector3 operator/(Vector3 const& r) const noexcept { return Vector3(x / r.x, y / r.y, z / r.z); }

		[[nodiscard]] Vector3 operator+(T const r) const noexcept { return Vector3(x + r, y + r, z + r); }
		[[nodiscard]] Vector3 operator-(T const r) const noexcept { return Vector3(x - r, y - r, z - r); }
		[[nodiscard]] Vector3 operator*(T const r) const noexcept { return Vector3(x * r, y * r, z * r); }
		[[nodiscard]] Vector3 operator/(T const r) const noexcept { return Vector3(x / r, y / r, z / r); }

		[[nodiscard]] Vector3 operator-() const noexcept { return Vector3(-x, -y, -z); }

		Vector3& operator+=(Vector3 const& r) noexcept { x += r.x; y += r.y; z += r.z; return *this; }
		Vector3& operator-=(Vector3 const& r) noexcept { x -= r.x; y -= r.y; z -= r.z; return *this; }
		Vector3& operator*=(Vector3 const& r) noexcept { x *= r.x; y *= r.y; z *= r.z; return *this; }
		Vector3& operator/=(Vector3 const& r) noexcept { x /= r.x; y /= r.y; z /= r.z; return *this; }

		Vector3& operator+=(T const r) noexcept { x += r; y += r; z += r; return *this; }
		Vector3& operator-=(T const r) noexcept { x -= r; y -= r; z -= r; return *this; }
		Vector3& operator*=(T const r) noexcept { x *= r; y *= r; z *= r; return *this; }
		Vector3& operator/=(T const r) noexcept { x /= r; y /= r; z /= r; return *this; }

		[[nodiscard]] bool operator==(Vector3 const& r) const noexcept { return x == r.x && y == r.y && z == r.z; }
		[[nodiscard]] bool operator!=(Vector3 const& r) const noexcept { return x != r.x || y != r.y || z != r.z; }

		[[nodiscard]] T& operator[](size_t const i) { return (&x)[i]; }

		[[nodiscard]] Vector3& normalize() noexcept {
			T const l = length();
			if (l >= std::numeric_limits<T>::min()) {
				x /= l; y /= l; z /= l;
			}
			else {
				x = T{}; y = T{}; z = T{};
			}
			return *this;
		}
		[[nodiscard]] Vector3 normalized() const noexcept {
			T const l = length();
			if (l >= std::numeric_limits<T>::min()) {
				return Vector3(x / l, y / l, z / l);
			}
			else {
				return Vector3();
			}
		}
		[[nodiscard]] T length() const noexcept { return std::sqrt(x * x + y * y + z * z); }
		[[nodiscard]] T dot(Vector3 const& r) const noexcept { return x * r.x + y * r.y + z * r.z; }
	};

	using Vector3I = Vector3<int32_t>;
	using Vector3U = Vector3<uint32_t>;
	using Vector3F = Vector3<float>;
}
