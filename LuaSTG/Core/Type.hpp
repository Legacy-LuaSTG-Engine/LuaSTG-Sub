#pragma once
#include <cstdint>
#include <string_view>

namespace LuaSTG::Core
{
	struct Vector3
	{
		float x, y, z;
		bool operator==(Vector3 const& right_) const
		{
			return x == right_.x
				&& y == right_.y
				&& z == right_.z;
		}
		bool operator!=(Vector3 const& right_) const
		{
			return x != right_.x
				|| y != right_.y
				|| z != right_.z;
		}
	};
	
	struct Vector4
	{
		float x, y, z, w;
		bool operator==(Vector4 const& right_) const
		{
			return x == right_.x
				&& y == right_.y
				&& z == right_.z
				&& w == right_.w;
		}
		bool operator!=(Vector4 const& right_) const
		{
			return x != right_.x
				|| y != right_.y
				|| z != right_.z
				|| w != right_.w;
		}
	};

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

	struct Rect
	{
		float left, top;
		float right, bottom;
		bool operator==(Rect const& right_) const
		{
			return left == right_.left
				&& top == right_.top
				&& right == right_.right
				&& bottom == right_.bottom;
		}
		bool operator!=(Rect const& right_) const
		{
			return left != right_.left
				|| top != right_.top
				|| right != right_.right
				|| bottom != right_.bottom;
		}
	};

	struct Box
	{
		float left, top, front;
		float right, bottom, back;
		bool operator==(Box const& right_) const
		{
			return left == right_.left
				&& top == right_.top
				&& front == right_.front
				&& right == right_.right
				&& bottom == right_.bottom
				&& back == right_.back;
		}
		bool operator!=(Box const& right_) const
		{
			return left != right_.left
				|| top != right_.top
				|| front != right_.front
				|| right != right_.right
				|| bottom != right_.bottom
				|| back != right_.back;
		}
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
		operator bool() { return ptr_ != nullptr; }
	public:
		ScopeObject() : ptr_(nullptr) {}
		ScopeObject(ScopeObject& right) : ptr_(right.ptr_) { internal_retain(); }
		ScopeObject(ScopeObject const& right) : ptr_(right.ptr_) { internal_retain(); }
		ScopeObject(ScopeObject&& right) : ptr_(right.ptr_) { right.ptr_ = nullptr; }
		ScopeObject(ScopeObject const&&) = delete;
		~ScopeObject() { internal_release(); }
	};
}
