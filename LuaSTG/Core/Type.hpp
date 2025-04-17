#pragma once
#include <cstdint>
#include <cmath>
#include <limits>
#include <string_view>
#include <string>
#include "core/ReferenceCounted.hpp"
#include "core/SmartReference.hpp"

namespace core {
	// 二维向量

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

	// 表示一个长方形区域

	template<typename T>
	struct Rect {
		Vector2<T> a;
		Vector2<T> b;

		Rect() noexcept = default;
		Rect(Vector2<T> const& a, Vector2<T> const& b) noexcept : a(a), b(b) {}
		Rect(T const left, T const top, T const right, T const bottom) noexcept : a(left, top), b(right, bottom) {}

		[[nodiscard]] bool operator==(Rect const& r) const noexcept { return a == r.a && b == r.b; }
		[[nodiscard]] bool operator!=(Rect const& r) const noexcept { return a != r.a || b != r.b; }

		[[nodiscard]] Rect operator+(Vector2<T> const& r) const noexcept { return Rect(a + r, b + r); }
		[[nodiscard]] Rect operator-(Vector2<T> const& r) const noexcept { return Rect(a - r, b - r); }

		//inline Rect operator*(T const r) const noexcept { return Rect(a * r, b * r); }
	};

	using RectI = Rect<int32_t>;
	using RectU = Rect<uint32_t>;
	using RectF = Rect<float>;

	// 三维向量

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

	// 表示一个长方体区域

	template<typename T>
	struct Box {
		Vector3<T> a;
		Vector3<T> b;

		Box() noexcept = default;
		Box(Vector3<T> const& a, Vector3<T> const& b) noexcept : a(a), b(b) {}
		Box(T const left, T const top, T const front, T const right, T const bottom, T const back) noexcept : a(left, top, front), b(right, bottom, back) {}

		[[nodiscard]] bool operator==(Box const& r) const noexcept { return a == r.a && b == r.b; }
		[[nodiscard]] bool operator!=(Box const& r) const noexcept { return a != r.a || b != r.b; }
	};

	using BoxI = Box<int32_t>;
	using BoxU = Box<uint32_t>;
	using BoxF = Box<float>;

	// 四维向量

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

	// 四阶矩阵，行主序

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

	// 颜色（有黑魔法）

	struct alignas(uint32_t) Color4B {
		uint8_t b{};
		uint8_t g{};
		uint8_t r{};
		uint8_t a{};

		Color4B() noexcept = default;
		explicit Color4B(uint32_t const argb) noexcept { color(argb); }
		Color4B(uint8_t const r, uint8_t const g, uint8_t const b) : b(b), g(g), r(r), a(255) {}
		Color4B(uint8_t const r, uint8_t const g, uint8_t const b, uint8_t const a) : b(b), g(g), r(r), a(a) {}

		void color(uint32_t const argb) noexcept { *reinterpret_cast<uint32_t*>(this) = argb; }
		[[nodiscard]] uint32_t color() const noexcept { return *reinterpret_cast<uint32_t const*>(this); }

		Color4B& operator=(uint32_t const argb) noexcept { color(argb); return *this; }

		[[nodiscard]] bool operator==(Color4B const& right) const noexcept { return color() == right.color(); }
		[[nodiscard]] bool operator!=(Color4B const& right) const noexcept { return color() != right.color(); }
	};

	static_assert(sizeof(Color4B) == sizeof(uint32_t));

	// 分数

	struct Rational {
		uint32_t numerator; // 分子
		uint32_t denominator; // 分母

		Rational() : numerator(0), denominator(0) {}
		Rational(uint32_t const numerator_) : numerator(numerator_), denominator(1) {}
		Rational(uint32_t const numerator_, uint32_t const denominator_) : numerator(numerator_), denominator(denominator_) {}
	};

	// 数据对象

	struct IData : public IReferenceCounted {
		virtual void* data() = 0;
		virtual size_t size() = 0;

		static bool create(size_t size, IData** pp_data);
		static bool create(size_t size, size_t align, IData** pp_data);
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IData
	template<> constexpr InterfaceId getInterfaceId<IData>() { return UUID::parse("acc69b53-02e3-5d58-a6c6-6c30c936ac98"); }

	// 字符串视图

	using StringView = std::string_view; // pointer | size

	// 不可变的空终止字符串

	struct IImmutableString : IReferenceCounted {
		[[nodiscard]] virtual bool empty() const noexcept = 0;
		[[nodiscard]] virtual char const* data() const noexcept = 0;
		[[nodiscard]] virtual size_t size() const noexcept = 0;
		[[nodiscard]] virtual char const* c_str() const noexcept = 0;
		[[nodiscard]] virtual size_t length() const noexcept = 0;
		[[nodiscard]] virtual StringView view() const noexcept = 0;

		static void create(StringView const& view, IImmutableString** output);
		static void create(char const* data, size_t size, IImmutableString** output);
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IImmutableString
	template<> constexpr InterfaceId getInterfaceId<IImmutableString>() { return UUID::parse("1ef36173-0c4d-5bd8-af47-fa362a5e4805"); }

}
