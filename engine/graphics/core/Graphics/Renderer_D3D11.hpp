#pragma once
#include "core/implement/ReferenceCounted.hpp"
#include "core/Graphics/Renderer.hpp"
#include "common/PrimitiveBatchRenderer.hpp"
#include "d3d11/GraphicsDevice.hpp"
#include "core/Graphics/Model_D3D11.hpp"

#define IDX(x) (size_t)static_cast<uint8_t>(x)

namespace core::Graphics {
	struct RendererStateSet
	{
		BoxF viewport = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
		RectF scissor_rect = { 0.0f, 0.0f, 1.0f, 1.0f };
		float fog_near_or_density = 0.0f;
		float fog_far = 0.0f;
		Color4B fog_color;
		IRenderer::VertexColorBlendState vertex_color_blend_state = IRenderer::VertexColorBlendState::Mul;
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

	class PostEffectShader_D3D11
		: public implement::ReferenceCounted<IPostEffectShader>
		, IGraphicsDeviceEventListener
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
			SmartReference<IGraphicsBuffer> constant_buffer;
			std::unordered_map<std::string, LocalVariable> variable;
		};
		struct LocalTexture2D
		{
			UINT index{};
			SmartReference<ITexture2D> texture;
		};
	private:
		SmartReference<IGraphicsDevice> m_device;
		win32::com_ptr<ID3DBlob> d3d_ps_blob;
		win32::com_ptr<ID3D11ShaderReflection> d3d11_ps_reflect;
		win32::com_ptr<ID3D11PixelShader> d3d11_ps;
		std::unordered_map<std::string, LocalConstantBuffer> m_buffer_map;
		std::unordered_map<std::string, LocalTexture2D> m_texture2d_map;
		std::string source;
		bool is_path{ false };

		bool createResources(bool is_recreating);
		void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;
		bool findVariable(StringView name, LocalConstantBuffer*& buf, LocalVariable*& val);

	public:
		ID3D11PixelShader* GetPS() const noexcept { return d3d11_ps.get(); }

	public:
		bool setFloat(StringView name, float value);
		bool setFloat2(StringView name, Vector2F value);
		bool setFloat3(StringView name, Vector3F value);
		bool setFloat4(StringView name, Vector4F value);
		bool setTexture2D(StringView name, ITexture2D* p_texture);
		bool apply(IRenderer* p_renderer);

	public:
		PostEffectShader_D3D11(GraphicsDevice* p_device, StringView path, bool is_path_);
		~PostEffectShader_D3D11();
	};

	class Renderer_D3D11
		: public implement::ReferenceCounted<IRenderer>
		, IGraphicsDeviceEventListener
	{
	private:
		SmartReference<GraphicsDevice> m_device;
		SmartReference<ModelSharedComponent_D3D11> m_model_shared;

		PrimitiveBatchRenderer m_primitive_batch_renderer;

		SmartReference<IGraphicsBuffer> _vp_matrix_buffer;
		SmartReference<IGraphicsBuffer> _world_matrix_buffer;
		SmartReference<IGraphicsBuffer> _camera_pos_buffer; // 在 postEffect 的时候被替换了
		SmartReference<IGraphicsBuffer> _fog_data_buffer; // 同时也用于储存 postEffect 的 纹理大小和视口范围
		SmartReference<IGraphicsBuffer> _user_float_buffer; // 在 postEffect 的时候用这个

		SmartReference<IGraphicsPipeline> _graphics_pipeline
			[IDX(VertexColorBlendState::MAX_COUNT)]
			[IDX(FogState::MAX_COUNT)]
			[IDX(TextureAlphaType::MAX_COUNT)]
			[IDX(DepthState::MAX_COUNT)]
			[IDX(BlendState::MAX_COUNT)];
		SmartReference<IGraphicsSampler> _sampler_state[IDX(SamplerState::MAX_COUNT)];

		SmartReference<ITexture2D> _state_texture;
		CameraStateSet _camera_state_set;
		RendererStateSet _state_set;
		bool _batch_scope{};

		bool createBuffers();
		bool createSamplers();
		bool createGraphicsPipelines();
		void bindGraphicsPipeline();
		bool batchFlush(bool discard = false);

		void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

	public:
		// public to MeshRenderer
		[[nodiscard]] bool isFogEnabled() const noexcept { return _state_set.fog_state != FogState::Disable; }
		// public to MeshRenderer
		void bindTextureSamplerState(ITexture2D* texture);
		// public to MeshRenderer
		void bindTextureAlphaType(ITexture2D* texture);

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

		IGraphicsSampler* getKnownSamplerState(SamplerState state);
		IGraphicsPipeline* getKnownGraphicsPipeline(
			VertexColorBlendState vertex_color_blend_state,
			FogState fog_state,
			TextureAlphaType texture_alpha_mode,
			DepthState depth_state,
			BlendState blend_state
		);

	public:
		Renderer_D3D11();
		Renderer_D3D11(const Renderer_D3D11&) = delete;
		Renderer_D3D11(Renderer_D3D11&&) = delete;
		~Renderer_D3D11();

		Renderer_D3D11& operator=(const Renderer_D3D11&) = delete;
		Renderer_D3D11& operator=(Renderer_D3D11&&) = delete;

		bool createResources(IGraphicsDevice* device);

	private:
		bool uploadOrtho(const BoxF& box);
		bool uploadOrtho();
		bool uploadPerspective(const Vector3F& position, const Vector3F& look_at, const Vector3F& head_up, float fov, float aspect_ratio, float z_near, float z_far);
		bool uploadPerspective();
		bool uploadCameraState();
		void applyViewport(const BoxF& box);
		void applyViewport();
		void applyScissorRect(const RectF& rect);
		void applyScissorRect();
		void applyViewportAndScissorRect();
		bool uploadFogState(FogState state, const Color4B& color, float density_or_znear, float zfar);
		bool uploadFogState();

		bool m_initialized{};
	};
}

#undef IDX
