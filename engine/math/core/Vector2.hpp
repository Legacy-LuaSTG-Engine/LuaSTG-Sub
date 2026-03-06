#pragma once
#include <cstdint>
#include <cmath>
#include <limits>

namespace core {
	template<typename T>
	struct Vector2 {
		T x{};
		T y{};

		Vector2() noexcept : x(0), y(0) {}
		Vector2(T const x, T const y) noexcept : x(x), y(y) {}

		[[nodiscard]] Vector2 operator+(Vector2 const& r) const noexcept { return Vector2(x + r.x, y + r.y); }
		[[nodiscard]] Vector2 operator-(Vector2 const& r) const noexcept { return Vector2(x - r.x, y - r.y); }
		[[nodiscard]] Vector2 operator*(Vector2 const& r) const noexcept { return Vector2(x * r.x, y * r.y); }
		[[nodiscard]] Vector2 operator/(Vector2 const& r) const noexcept { return Vector2(x / r.x, y / r.y); }

		[[nodiscard]] Vector2 operator+(T const r) const noexcept { return Vector2(x + r, y + r); }
		[[nodiscard]] Vector2 operator-(T const r) const noexcept { return Vector2(x - r, y - r); }
		[[nodiscard]] Vector2 operator*(T const r) const noexcept { return Vector2(x * r, y * r); }
		[[nodiscard]] Vector2 operator/(T const r) const noexcept { return Vector2(x / r, y / r); }

		[[nodiscard]] Vector2 operator-() const noexcept { return Vector2(-x, -y); }

		Vector2& operator+=(Vector2 const& r) noexcept { x += r.x; y += r.y; return *this; }
		Vector2& operator-=(Vector2 const& r) noexcept { x -= r.x; y -= r.y; return *this; }
		Vector2& operator*=(Vector2 const& r) noexcept { x *= r.x; y *= r.y; return *this; }
		Vector2& operator/=(Vector2 const& r) noexcept { x /= r.x; y /= r.y; return *this; }

		Vector2& operator+=(T const r) noexcept { x += r; y += r; return *this; }
		Vector2& operator-=(T const r) noexcept { x -= r; y -= r; return *this; }
		Vector2& operator*=(T const r) noexcept { x *= r; y *= r; return *this; }
		Vector2& operator/=(T const r) noexcept { x /= r; y /= r; return *this; }

		[[nodiscard]] bool operator==(Vector2 const& r) const noexcept { return x == r.x && y == r.y; }
		[[nodiscard]] bool operator!=(Vector2 const& r) const noexcept { return x != r.x || y != r.y; }

		Vector2& normalize() noexcept {
			T const l = length();
			if (l >= std::numeric_limits<T>::min()) {
				x /= l; y /= l;
			}
			else {
				x = T{}; y = T{};
			}
			return *this;
		}
		[[nodiscard]] Vector2 normalized() const noexcept {
			T const l = length();
			if (l >= std::numeric_limits<T>::min()) {
				return Vector2(x / l, y / l);
			}
			else {
				return Vector2();
			}
		}
		[[nodiscard]] T length() const noexcept { return std::sqrt(x * x + y * y); }
		[[nodiscard]] T angle() const noexcept { return std::atan2(y, x); }
		[[nodiscard]] T dot(Vector2 const& r) const noexcept { return x * r.x + y * r.y; }

		[[nodiscard]] T& operator[](size_t const i) { return (&x)[i]; }
	};

	using Vector2I = Vector2<int32_t>;
	using Vector2U = Vector2<uint32_t>;
	using Vector2F = Vector2<float>;
}
