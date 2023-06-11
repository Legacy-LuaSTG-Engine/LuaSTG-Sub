#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/Renderer.hpp"
#include "Core/Graphics/Device_D3D11.hpp"
#include "Core/Graphics/Model_D3D11.hpp"

#define IDX(x) (size_t)static_cast<uint8_t>(x)

namespace Core::Graphics
{
	struct RendererStateSet
	{
		BoxF viewport = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
		RectF scissor_rect = { 0.0f, 0.0f, 1.0f, 1.0f };
		float fog_near_or_density = 0.0f;
		float fog_far = 0.0f;
		Color4B fog_color;
		IRenderer::VertexColorBlendState vertex_color_blend_state = IRenderer::VertexColorBlendState::Mul;
		IRenderer::SamplerState sampler_state = IRenderer::SamplerState::LinearClamp;
		IRenderer::FogState fog_state = IRenderer::FogState::Disable;
		IRenderer::TextureAlphaType texture_alpha_type = IRenderer::TextureAlphaType::Normal;
		IRenderer::DepthState depth_state = IRenderer::DepthState::Disable;
		IRenderer::BlendState blend_state = IRenderer::BlendState::Alpha;
	};

	struct CameraStateSet
	{
		BoxF ortho = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
		Vector3F eye = { 0.0f, 0.0f, 0.0f };
		Vector3F lookat = { 0.0f, 0.0f, 1.0f };
		Vector3F headup = { 0.0f, 1.0f, 0.0f };
		float fov = 0.0f;
		float aspect = 0.0f;
		float znear = 0.0f;
		float zfar = 0.0f;
		bool is_3D = false;

		bool isEqual(BoxF const& box)
		{
			return !is_3D && ortho == box;
		}
		bool isEqual(Vector3F const& eye_, Vector3F const& lookat_, Vector3F const& headup_, float fov_, float aspect_, float znear_, float zfar_)
		{
			return is_3D
				&& eye == eye_
				&& lookat == lookat_
				&& headup == headup_
				&& fov == fov_
				&& aspect == aspect_
				&& znear == znear_
				&& zfar == zfar_;
		}
	};

	struct VertexIndexBuffer
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;
		INT vertex_offset = 0;
		UINT index_offset = 0;
	};

	struct DrawCommand
	{
		ScopeObject<Texture2D_D3D11> texture;
		uint16_t vertex_count = 0;
		uint16_t index_count = 0;
	};

	struct DrawList
	{
		struct VertexBuffer
		{
			const size_t capacity = 32768;
			size_t size = 0;
			IRenderer::DrawVertex data[32768] = {};
		} vertex;
		struct IndexBuffer
		{
			const size_t capacity = 32768;
			size_t size = 0;
			IRenderer::DrawIndex data[32768] = {};
		} index;
		struct DrawCommandBuffer
		{
			const size_t capacity = 2048;
			size_t size = 0;
			DrawCommand data[2048] = {};
		} command;
	};

	class PostEffectShader_D3D11
		: public Object<IPostEffectShader>
		, IDeviceEventListener
	{
	private:
		struct LocalVariable
		{
			UINT offset{};
			UINT size{};
		};
		struct LocalConstantBuffer
		{
			UINT index{};
			std::vector<uint8_t> buffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> d3d11_buffer;
			std::unordered_map<std::string, LocalVariable> variable;
		};
		struct LocalTexture2D
		{
			UINT index{};
			ScopeObject<Texture2D_D3D11> texture;
		};
	private:
		ScopeObject<Device_D3D11> m_device;
		Microsoft::WRL::ComPtr<ID3DBlob> d3d_ps_blob;
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> d3d11_ps_reflect;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> d3d11_ps;
		std::unordered_map<std::string, LocalConstantBuffer> m_buffer_map;
		std::unordered_map<std::string, LocalTexture2D> m_texture2d_map;
		std::string source;
		bool is_path{ false };

		bool createResources();
		void onDeviceCreate();
		void onDeviceDestroy();
		bool findVariable(StringView name, LocalConstantBuffer*& buf, LocalVariable*& val);

	public:
		ID3D11PixelShader* GetPS() const noexcept { return d3d11_ps.Get(); }

	public:
		bool setFloat(StringView name, float value);
		bool setFloat2(StringView name, Vector2F value);
		bool setFloat3(StringView name, Vector3F value);
		bool setFloat4(StringView name, Vector4F value);
		bool setTexture2D(StringView name, ITexture2D* p_texture);
		bool apply(IRenderer* p_renderer);

	public:
		PostEffectShader_D3D11(Device_D3D11* p_device, StringView path, bool is_path_);
		~PostEffectShader_D3D11();
	};

	class Renderer_D3D11
		: public Object<IRenderer>
		, IDeviceEventListener
	{
	private:
		ScopeObject<Device_D3D11> m_device;
		ScopeObject<ModelSharedComponent_D3D11> m_model_shared;

		Microsoft::WRL::ComPtr<ID3D11Buffer> _fx_vbuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> _fx_ibuffer;
		VertexIndexBuffer _vi_buffer[1];
		size_t _vi_buffer_index = 0;
		const size_t _vi_buffer_count = 1;
		DrawList _draw_list;

		void setVertexIndexBuffer(size_t index = 0xFFFFFFFFu);
		bool uploadVertexIndexBuffer(bool discard);
		void clearDrawList();

		Microsoft::WRL::ComPtr<ID3D11Buffer> _vp_matrix_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> _world_matrix_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> _camera_pos_buffer; // 在 postEffect 的时候被替换了
		Microsoft::WRL::ComPtr<ID3D11Buffer> _fog_data_buffer; // 同时也用于储存 postEffect 的 纹理大小和视口范围
		Microsoft::WRL::ComPtr<ID3D11Buffer> _user_float_buffer; // 在 postEffect 的时候用这个

		Microsoft::WRL::ComPtr<ID3D11InputLayout> _input_layout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertex_shader[IDX(FogState::MAX_COUNT)]; // FogState
		Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixel_shader[IDX(VertexColorBlendState::MAX_COUNT)][IDX(FogState::MAX_COUNT)][IDX(TextureAlphaType::MAX_COUNT)]; // VertexColorBlendState, FogState, TextureAlphaType
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> _raster_state;
		ScopeObject<ISamplerState> _sampler_state[IDX(SamplerState::MAX_COUNT)];
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> _depth_state[IDX(DepthState::MAX_COUNT)];
		Microsoft::WRL::ComPtr<ID3D11BlendState> _blend_state[IDX(BlendState::MAX_COUNT)];
		
		ScopeObject<Texture2D_D3D11> _state_texture;
		CameraStateSet _camera_state_set;
		RendererStateSet _state_set;
		bool _state_dirty = false;
		bool _batch_scope = false;

		bool createBuffers();
		bool createStates();
		bool createShaders();
		void initState();
		void setSamplerState(SamplerState state, UINT index);
		bool uploadVertexIndexBufferFromDrawList();
		void bindTextureSamplerState(ITexture2D* texture);
		void bindTextureAlphaType(ITexture2D* texture);
		bool batchFlush(bool discard = false);

		bool createResources();
		void onDeviceCreate();
		void onDeviceDestroy();

	public:
		bool beginBatch();
		bool endBatch();
		bool isBatchScope() { return _batch_scope; }
		bool flush();

		void clearRenderTarget(Color4B const& color);
		void clearDepthBuffer(float zvalue);
		void setRenderAttachment(IRenderTarget* p_rt, IDepthStencilBuffer* p_ds);

		void setOrtho(BoxF const& box);
		void setPerspective(Vector3F const& eye, Vector3F const& lookat, Vector3F const& headup, float fov, float aspect, float znear, float zfar);

		inline BoxF getViewport() { return _state_set.viewport; }
		void setViewport(BoxF const& box);
		void setScissorRect(RectF const& rect);
		void setViewportAndScissorRect();

		void setVertexColorBlendState(VertexColorBlendState state);
		void setFogState(FogState state, Color4B const& color, float density_or_znear, float zfar);
		void setDepthState(DepthState state);
		void setBlendState(BlendState state);
		void setTexture(ITexture2D* texture);

		bool drawTriangle(DrawVertex const& v1, DrawVertex const& v2, DrawVertex const& v3);
		bool drawTriangle(DrawVertex const* pvert);
		bool drawQuad(DrawVertex const& v1, DrawVertex const& v2, DrawVertex const& v3, DrawVertex const& v4);
		bool drawQuad(DrawVertex const* pvert);
		bool drawRaw(DrawVertex const* pvert, uint16_t nvert, DrawIndex const* pidx, uint16_t nidx);
		bool drawRequest(uint16_t nvert, uint16_t nidx, DrawVertex** ppvert, DrawIndex** ppidx, uint16_t* idxoffset);

		bool createPostEffectShader(StringView path, IPostEffectShader** pp_effect);
		bool drawPostEffect(
			IPostEffectShader* p_effect,
			BlendState blend,
			ITexture2D* p_tex, SamplerState rtsv,
			Vector4F const* cv, size_t cv_n,
			ITexture2D* const* p_tex_arr, SamplerState const* sv, size_t tv_sv_n);
		bool drawPostEffect(IPostEffectShader* p_effect, BlendState blend);

		bool createModel(StringView path, IModel** pp_model);
		bool drawModel(IModel* p_model);

		ISamplerState* getKnownSamplerState(SamplerState state);

	public:
		Renderer_D3D11(Device_D3D11* p_device);
		~Renderer_D3D11();

	public:
		static bool create(Device_D3D11* p_device, Renderer_D3D11** pp_renderer);
	};
}

#undef IDX
