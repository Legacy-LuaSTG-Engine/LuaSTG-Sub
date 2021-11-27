#pragma once
#include <cstdint>

// basic type primitives
namespace slow
{
	using u8 = std::uint8_t;
	using u16 = std::uint16_t;
	using u32 = std::uint32_t;
	using u64 = std::uint64_t;
	using i8 = std::int8_t;
	using i16 = std::int16_t;
	using i32 = std::int32_t;
	using i64 = std::int64_t;
	using f32 = float;
	using f64 = double;
	
	static_assert(sizeof(u8) == 1);
	static_assert(sizeof(u16) == 2);
	static_assert(sizeof(u32) == 4);
	static_assert(sizeof(u64) == 8);
	static_assert(sizeof(i8) == 1);
	static_assert(sizeof(i16) == 2);
	static_assert(sizeof(i32) == 4);
	static_assert(sizeof(i64) == 8);
	static_assert(sizeof(f32) == 4);
	static_assert(sizeof(f64) == 8);
}

// basic types
namespace slow
{
	struct f32x2
	{
		union
		{
			struct
			{
				f32 x;
				f32 y;
			};
			f32 data[2];
		};
		f32x2() : x(0.0f), y(0.0f) {}
		f32x2(f32 x_, f32 y_) : x(x_), y(y_) {}
	};
	struct f32x3
	{
		union
		{
			struct
			{
				f32 x;
				f32 y;
				f32 z;
			};
			struct
			{
				f32 r;
				f32 g;
				f32 b;
			};
			f32 data[3];
		};
		f32x3() : x(0.0f), y(0.0f), z(0.0f) {}
		f32x3(f32 x_, f32 y_, f32 z_) : x(x_), y(y_), z(z_) {}
	};
	struct f32x4
	{
		union
		{
			struct
			{
				f32 x;
				f32 y;
				f32 z;
				f32 w;
			};
			struct
			{
				f32 r;
				f32 g;
				f32 b;
				f32 a;
			};
			struct
			{
				f32 left;
				f32 top;
				f32 right;
				f32 bottom;
			};
			f32 data[4];
		};
		f32x4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
		f32x4(f32 x_, f32 y_, f32 z_, f32 w_) : x(x_), y(y_), z(z_), w(w_) {}
	};
}
