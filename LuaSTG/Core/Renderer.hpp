#pragma once
#include <cstdint>

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
	struct Color
	{
		union
		{
			struct
			{
				uint8_t b;
				uint8_t g;
				uint8_t r;
				uint8_t a;
			};
			uint32_t color;
		};
		bool operator==(Color const& right) const
		{
			return color == right.color;
		}
		bool operator!=(Color const& right) const
		{
			return color != right.color;
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

	struct TextureID
	{
		union
		{
			uint64_t address; // 64bit device (GPU) address
			void* handle; // host handle
		};
		bool operator==(TextureID const& right) const { return address == right.address; }
		bool operator!=(TextureID const& right) const { return address != right.address; }
		TextureID() : address(0) {}
		template<typename U>
		TextureID(U* handle_) : address(0) { handle = static_cast<void*>(handle_); }
		TextureID(uint64_t address_) : address(address_) {}
	};
	struct ShaderID
	{
		union
		{
			uint64_t address; // 64bit device (GPU) address
			void* handle; // host handle
		};
		bool operator==(ShaderID const& right) const { return address == right.address; }
		bool operator!=(ShaderID const& right) const { return address != right.address; }
		ShaderID() : address(0) {}
		template<typename U>
		ShaderID(U* handle_) : address(0) { handle = static_cast<void*>(handle_); }
		ShaderID(uint64_t address_) : address(address_) {}
	};
	enum class TextureAlphaType : uint8_t
	{
		MIN_INDEX = 0,

		Normal = MIN_INDEX,
		PremulAlpha,

		MAX_INDEX = PremulAlpha,
		MAX_COUNT,
	};

	enum class VertexColorBlendState : uint8_t
	{
		MIN_INDEX = 0,

		Zero = MIN_INDEX,
		One,
		Add,
		Mul,

		MAX_INDEX = Mul,
		MAX_COUNT,
	};
	enum class FogState : uint8_t
	{
		MIN_INDEX = 0,

		Disable = MIN_INDEX,
		Linear,
		Exp,
		Exp2,

		MAX_INDEX = Exp2,
		MAX_COUNT,
	};
	enum class DepthState : uint8_t
	{
		MIN_INDEX = 0,

		Disable = MIN_INDEX,
		Enable,

		MAX_INDEX = Enable,
		MAX_COUNT,
	};
	enum class BlendState : uint8_t
	{
		MIN_INDEX = 0,

		Disable = MIN_INDEX,
		Alpha,
		One,
		Min,
		Max,
		Mul,
		Screen,
		Add,
		Sub,
		RevSub,
		Inv,

		MAX_INDEX = Inv,
		MAX_COUNT,
	};
	enum class SamplerState : uint8_t
	{
		MIN_INDEX = 0,

		PointWrap = MIN_INDEX,
		PointClamp,
		PointBorderBlack,
		PointBorderWhite,
		LinearWrap,
		LinearClamp,
		LinearBorderBlack,
		LinearBorderWhite,

		MAX_INDEX = LinearBorderWhite,
		MAX_COUNT,
	};

	struct DrawVertex2D
	{
		float x, y, z;
		uint32_t color;
		float u, v;
	};
	using DrawIndex2D = uint16_t;

	struct DrawCommand
	{
		TextureID texture;
		uint16_t vertex_count = 0;
		uint16_t index_count = 0;
	};
	struct DrawList
	{
		struct VertexBuffer
		{
			const size_t capacity = 32768;
			size_t size = 0;
			DrawVertex2D data[32768] = {};
		} vertex;
		struct IndexBuffer
		{
			const size_t capacity = 32768;
			size_t size = 0;
			DrawIndex2D data[32768] = {};
		} index;
		struct DrawCommandBuffer
		{
			const size_t capacity = 2048;
			size_t size = 0;
			DrawCommand data[2048] = {};
		} command;
	};

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
	
	struct IModel : public IObject
	{
		virtual void setAmbient(Vector3 const& color, float brightness) = 0;
		virtual void setDirectionalLight(Vector3 const& direction, Vector3 const& color, float brightness) = 0;

		virtual void setScaling(Vector3 const& scale) = 0;
		virtual void setPosition(Vector3 const& pos) = 0;
		virtual void setRotationRollPitchYaw(float roll, float pitch, float yaw) = 0;
		virtual void setRotationQuaternion(Vector4 const& quat) = 0;
	};

	class Renderer
	{
	private:
		class Renderer11;
		void* _pImpl;
	public:
		bool attachDevice(void* dev);
		void detachDevice();

		bool beginScene();
		bool endScene();

		void clearRenderTarget(Color const& color);
		void clearDepthBuffer(float zvalue);
		
		void setOrtho(Box const& box);
		void setPerspective(Vector3 const& eye, Vector3 const& lookat, Vector3 const& headup, float fov, float aspect, float znear, float zfar);

		void setViewport(Box const& box);
		void setScissorRect(Rect const& rect);
		void setViewportAndScissorRect();
		
		void setVertexColorBlendState(VertexColorBlendState state);
		void setSamplerState(SamplerState state);
		void setFogState(FogState state, Color const& color, float density_or_znear, float zfar);
		void setDepthState(DepthState state);
		void setBlendState(BlendState state);
		void setTexture(TextureID texture);
		void setTextureAlphaType(TextureAlphaType state);

		bool flush();
		void drawTriangle(DrawVertex2D const& v1, DrawVertex2D const& v2, DrawVertex2D const& v3);
		void drawTriangle(DrawVertex2D const* pvert);
		void drawQuad(DrawVertex2D const& v1, DrawVertex2D const& v2, DrawVertex2D const& v3, DrawVertex2D const& v4);
		void drawQuad(DrawVertex2D const* pvert);
		void drawRaw(DrawVertex2D const* pvert, uint16_t nvert, DrawIndex2D const* pidx, uint16_t nidx);
		void drawRequest(uint16_t nvert, uint16_t nidx, DrawVertex2D** ppvert, DrawIndex2D** ppidx, uint16_t* idxoffset);

		ShaderID createPostEffectShader(char const* name, void const* data, size_t size);
		void destroyPostEffectShader(ShaderID& ps);
		void postEffect(ShaderID const& ps, TextureID const& rt, SamplerState rtsv, Vector4 const* cv, size_t cv_n, TextureID const* tv, SamplerState const* sv, size_t tv_sv_n, BlendState blend);
	
		bool createModel(char const* gltf_path, IModel** model);
		bool drawModel(IModel* model);
	public:
		Renderer();
		Renderer(Renderer&) = delete;
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&& right) noexcept;
		Renderer(const Renderer&&) = delete;
		~Renderer();
	};
}
