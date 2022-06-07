#pragma once
#include <cstdint>
#include <string_view>

namespace LuaSTG::Core
{
	// 二维向量

	template<typename T>
	struct Vector2
	{
		T x{};
		T y{};

		Vector2() noexcept : x(0), y(0) {}
		Vector2(T const x_, T const y_) noexcept : x(x_), y(y_) {}

		inline Vector2 operator+(Vector2 const& r) const noexcept { return Vector2(x + r.x, y + r.y); }
		inline Vector2 operator-(Vector2 const& r) const noexcept { return Vector2(x - r.x, y - r.y); }
		inline Vector2 operator*(T const r) const noexcept { return Vector2(x * r, y * r); }
		inline Vector2 operator/(T const r) const noexcept { return Vector2(x / r, y / r); }

		inline Vector2& operator+=(Vector2 const& r) noexcept { x += r.x; y += r.y; return *this; }
		inline Vector2& operator-=(Vector2 const& r) noexcept { x -= r.x; y -= r.y; return *this; }
		inline Vector2& operator*=(T const r) noexcept { x *= r; y *= r; return *this; }
		inline Vector2& operator/=(T const r) noexcept { x /= r; y /= r; return *this; }

		inline bool operator==(Vector2 const& r) const noexcept { return x == r.x && y == r.y; }
		inline bool operator!=(Vector2 const& r) const noexcept { return x != r.x || y != r.y; }

		inline T& operator[](size_t const i) { return (&x)[i]; }
	};

	using Vector2I = Vector2<int32_t>;
	using Vector2U = Vector2<uint32_t>;
	using Vector2F = Vector2<float>;

	// 表示一个长方形区域

	template<typename T>
	struct Rect
	{
		Vector2<T> a;
		Vector2<T> b;

		Rect() noexcept {}
		Rect(Vector2<T> const& a_, Vector2<T> const& b_) noexcept : a(a_), b(b_) {}
		Rect(T const left, T const top, T const right, T const bottom) noexcept : a(left, top), b(right, bottom) {}

		inline bool operator==(Rect const& r) const noexcept { return a == r.a && b == r.b; }
		inline bool operator!=(Rect const& r) const noexcept { return a != r.a || b != r.b; }

		inline Rect operator+(Vector2<T> const& r) const noexcept { return Rect(a + r, b + r); }
		inline Rect operator-(Vector2<T> const& r) const noexcept { return Rect(a - r, b - r); }

		//inline Rect operator*(T const r) const noexcept { return Rect(a * r, b * r); }
	};

	using RectI = Rect<int32_t>;
	using RectU = Rect<uint32_t>;
	using RectF = Rect<float>;

	// 三维向量

	template<typename T>
	struct Vector3
	{
		T x{};
		T y{};
		T z{};

		Vector3() noexcept : x(0), y(0), z(0) {}
		Vector3(T const x_, T const y_, T const z_) noexcept : x(x_), y(y_), z(z_) {}

		inline Vector3 operator+(Vector3 const& r) const noexcept { return Vector3(x + r.x, y + r.y, z + r.z); }
		inline Vector3 operator-(Vector3 const& r) const noexcept { return Vector3(x - r.x, y - r.y, z - r.z); }
		inline Vector3 operator*(T const r) const noexcept { return Vector3(x * r, y * r, z * r); }
		inline Vector3 operator/(T const r) const noexcept { return Vector3(x / r, y / r, z / r); }

		inline Vector3& operator+=(Vector3 const& r) noexcept { x += r.x; y += r.y; z += r.z; return *this; }
		inline Vector3& operator-=(Vector3 const& r) noexcept { x -= r.x; y -= r.y; z -= r.z; return *this; }
		inline Vector3& operator*=(T const r) noexcept { x *= r; y *= r; z *= r; return *this; }
		inline Vector3& operator/=(T const r) noexcept { x /= r; y /= r; z /= r; return *this; }

		inline bool operator==(Vector3 const& r) const noexcept { return x == r.x && y == r.y && z == r.z; }
		inline bool operator!=(Vector3 const& r) const noexcept { return x != r.x || y != r.y || z != r.z; }

		inline T& operator[](size_t const i) { return (&x)[i]; }
	};

	using Vector3I = Vector3<int32_t>;
	using Vector3U = Vector3<uint32_t>;
	using Vector3F = Vector3<float>;

	// 表示一个长方体区域

	template<typename T>
	struct Box
	{
		Vector3<T> a;
		Vector3<T> b;

		Box() noexcept {}
		Box(Vector3<T> const& a_, Vector3<T> const& b_) noexcept : a(a_), b(b_) {}
		Box(T const left, T const top, T const front, T const right, T const bottom, T const back) noexcept : a(left, top, front), b(right, bottom, back) {}

		inline bool operator==(Box const& r) const noexcept { return a == r.a && b == r.b; }
		inline bool operator!=(Box const& r) const noexcept { return a != r.a || b != r.b; }
	};

	using BoxI = Box<int32_t>;
	using BoxU = Box<uint32_t>;
	using BoxF = Box<float>;

	// 四维向量

	template<typename T>
	struct Vector4
	{
		T x{};
		T y{};
		T z{};
		T w{};

		Vector4() noexcept : x(0), y(0), z(0), w(0) {}
		Vector4(T const x_, T const y_, T const z_, T const w_) noexcept : x(x_), y(y_), z(z_), w(w_) {}

		inline Vector4 operator+(Vector4 const& r) const noexcept { return Vector4(x + r.x, y + r.y, z + r.z, w + r.w); }
		inline Vector4 operator-(Vector4 const& r) const noexcept { return Vector4(x - r.x, y - r.y, z - r.z, w - r.w); }
		inline Vector4 operator*(T const r) const noexcept { return Vector4(x * r, y * r, z * r, w * r); }
		inline Vector4 operator/(T const r) const noexcept { return Vector4(x / r, y / r, z / r, w / r); }

		inline Vector4 operator+=(Vector4 const& r) noexcept { x += r.x; y += r.y; z += r.z; w += r.w; }
		inline Vector4 operator-=(Vector4 const& r) noexcept { x -= r.x; y -= r.y; z -= r.z; w -= r.w; }
		inline Vector4 operator*=(T const r) noexcept { x *= r; y *= r; z *= z; w *= r; }
		inline Vector4 operator/=(T const r) noexcept { x /= r; y /= r; z /= z; w /= r; }

		inline bool operator==(Vector4 const& r) const noexcept { return x == r.x && y == r.y && z == r.z && w == r.w; }
		inline bool operator!=(Vector4 const& r) const noexcept { return x != r.x || y != r.y || z != r.z || w != r.w; }

		inline T& operator[](size_t const i) { return (&x)[i]; }
	};

	using Vector4I = Vector4<int32_t>;
	using Vector4U = Vector4<uint32_t>;
	using Vector4F = Vector4<float>;

	// 颜色

	struct Color4B
	{
		union Color4B_byte4_uint_union
		{
			struct Color4B_byte4
			{
				uint8_t b;
				uint8_t g;
				uint8_t r;
				uint8_t a;
			} s;
			uint32_t color;
		} u;
		bool operator==(Color4B const& right) const
		{
			return u.color == right.u.color;
		}
		bool operator!=(Color4B const& right) const
		{
			return u.color != right.u.color;
		}
		Color4B()
		{
			u.color = 0x00000000u;
		}
		Color4B(uint32_t color_ARGB)
		{
			u.color = color_ARGB;
		}
		Color4B(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		{
			u.s.b = b;
			u.s.g = g;
			u.s.r = r;
			u.s.a = a;
		}
	};

	struct Rational
	{
		uint32_t numerator{ 0 }; // 分子
		uint32_t denominator{ 0 }; // 分母
	};

	using StringView = std::string_view;

	struct IObject
	{
		virtual intptr_t retain() = 0;
		virtual intptr_t release() = 0;
		virtual ~IObject() {};
	};
	
	template<typename T = IObject>
	class ScopeObject
	{
	private:
		T* ptr_;
	private:
		inline void internal_retain() { if (ptr_) ptr_->retain(); }
		inline void internal_release() { if (ptr_) ptr_->release(); ptr_ = nullptr; }
	public:
		T* operator->() { return ptr_; }
		T* operator*() { return ptr_; }
		T** operator~() { internal_release(); return &ptr_; }
		ScopeObject& operator=(std::nullptr_t) { internal_release(); return *this; }
		ScopeObject& operator=(T* ptr) { if (ptr_ != ptr) { internal_release(); ptr_ = ptr; internal_retain(); } return *this; }
		ScopeObject& operator=(ScopeObject& right) { if (ptr_ != right.ptr_) { internal_release(); ptr_ = right.ptr_; internal_retain(); } return *this; }
		ScopeObject& operator=(ScopeObject const& right) { if (ptr_ != right.ptr_) { internal_release(); ptr_ = right.ptr_; internal_retain(); } return *this; }
		operator bool() { return ptr_ != nullptr; }
		ScopeObject& attach(T* ptr) { internal_release(); ptr_ = ptr; return *this; }
		T* detach()  { T* tmp_ = ptr_; ptr_ = nullptr; return tmp_; }
		ScopeObject& reset() { internal_release(); return *this; }
		T* get() const { return ptr_; }
	public:
		ScopeObject() : ptr_(nullptr) {}
		ScopeObject(T* ptr) : ptr_(ptr) { internal_retain(); }
		ScopeObject(ScopeObject& right) : ptr_(right.ptr_) { internal_retain(); }
		ScopeObject(ScopeObject const& right) : ptr_(right.ptr_) { internal_retain(); }
		ScopeObject(ScopeObject&& right) : ptr_(right.ptr_) { right.ptr_ = nullptr; }
		ScopeObject(ScopeObject const&&) = delete;
		~ScopeObject() { internal_release(); }
	};
}
