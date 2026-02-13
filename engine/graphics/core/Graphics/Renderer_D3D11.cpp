#include "core/Graphics/Renderer_D3D11.hpp"
#include "core/Logger.hpp"
#include "core/Graphics/Model_D3D11.hpp"
#include "core/DepthStencilBuffer.hpp"
#include "d3d11/SlotConstants.hpp"

#define IDX(x) (size_t)static_cast<uint8_t>(x)

namespace {
	ID3D11Buffer* getBuffer(core::IGraphicsBuffer* const buffer) {
		if (buffer != nullptr)
			return static_cast<ID3D11Buffer*>(buffer->getNativeResource());
		return nullptr;
	}
	ID3D11Buffer* getBuffer(const core::SmartReference<core::IGraphicsBuffer>& buffer) {
		if (buffer)
			return static_cast<ID3D11Buffer*>(buffer->getNativeResource());
		return nullptr;
	}

	bool isTextureEquals(const core::SmartReference<core::ITexture2D>& a, core::ITexture2D* const b) {
		if (!a && b == nullptr) {
			return true;
		}
		if (!a || b == nullptr) {
			return false;
		}
		return a->getNativeView() == b->getNativeView();
	}
	core::Graphics::IRenderer::TextureAlphaType getTextureAlphaMode(core::ITexture2D* const b) {
		if (b == nullptr) {
			return core::Graphics::IRenderer::TextureAlphaType::Normal;
		}
		return b->isPremultipliedAlpha()
			? core::Graphics::IRenderer::TextureAlphaType::PremulAlpha
			: core::Graphics::IRenderer::TextureAlphaType::Normal;
	}

	struct RenderTargetHolder {
		win32::com_ptr<ID3D11DeviceContext> ctx;
		win32::com_ptr<ID3D11RenderTargetView> rtv;
		win32::com_ptr<ID3D11DepthStencilView> dsv;
		uint32_t width{};
		uint32_t height{};

		RenderTargetHolder(ID3D11DeviceContext* const context) {
			ctx = context;
			ID3D11RenderTargetView* rtvs[1]{};
			ctx->OMGetRenderTargets(1, rtvs, dsv.put());
			rtv.attach(rtvs[0]);
			if (rtv) {
				win32::com_ptr<ID3D11Resource> resource;
				rtv->GetResource(resource.put());
				D3D11_RESOURCE_DIMENSION type{};
				resource->GetType(&type);
				if (type == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
					win32::com_ptr<ID3D11Texture2D> texture;
					if (SUCCEEDED(resource->QueryInterface(texture.put()))) {
						D3D11_TEXTURE2D_DESC texture_info{};
						texture->GetDesc(&texture_info);
						width = texture_info.Width;
						height = texture_info.Height;
					}
				}
			}
		}

		void bind() {
			ID3D11RenderTargetView* rtvs[1]{ rtv.get() };
			ctx->OMSetRenderTargets(1, rtvs, dsv.get());
		}
	};
}

namespace core::Graphics {
	void PostEffectShader_D3D11::onGraphicsDeviceCreate()
	{
		createResources(true);
	}
	void PostEffectShader_D3D11::onGraphicsDeviceDestroy()
	{
		d3d11_ps.reset();
	}

	bool PostEffectShader_D3D11::findVariable(StringView name, LocalConstantBuffer*& buf, LocalVariable*& val)
	{
		std::string name_s(name);
		for (auto& b : m_buffer_map)
		{
			auto it = b.second.variable.find(name_s);
			if (it != b.second.variable.end())
			{
				buf = &b.second;
				val = &it->second;
				return true;
			}
		}
		return false;
	}
	bool PostEffectShader_D3D11::setFloat(StringView name, float value)
	{
		LocalConstantBuffer* b{};
		LocalVariable* v{};
		if (!findVariable(name, b, v)) { return false; }
		if (v->size != sizeof(value)) { assert(false); return false; }
		memcpy(b->buffer.data() + v->offset, &value, v->size);
		return true;
	}
	bool PostEffectShader_D3D11::setFloat2(StringView name, Vector2F value)
	{
		LocalConstantBuffer* b{};
		LocalVariable* v{};
		if (!findVariable(name, b, v)) { return false; }
		if (v->size != sizeof(value)) { assert(false); return false; }
		memcpy(b->buffer.data() + v->offset, &value, v->size);
		return true;
	}
	bool PostEffectShader_D3D11::setFloat3(StringView name, Vector3F value)
	{
		LocalConstantBuffer* b{};
		LocalVariable* v{};
		if (!findVariable(name, b, v)) { return false; }
		if (v->size != sizeof(value)) { assert(false); return false; }
		memcpy(b->buffer.data() + v->offset, &value, v->size);
		return true;
	}
	bool PostEffectShader_D3D11::setFloat4(StringView name, Vector4F value)
	{
		LocalConstantBuffer* b{};
		LocalVariable* v{};
		if (!findVariable(name, b, v)) { return false; }
		if (v->size != sizeof(value)) { assert(false); return false; }
		memcpy(b->buffer.data() + v->offset, &value, v->size);
		return true;
	}
	bool PostEffectShader_D3D11::setTexture2D(StringView name, ITexture2D* p_texture)
	{
		std::string name_s(name);
		auto it = m_texture2d_map.find(name_s);
		if (it == m_texture2d_map.end()) { return false; }
		it->second.texture = p_texture;
		if (!it->second.texture) { assert(false); return false; }
		return true;
	}
	bool PostEffectShader_D3D11::apply(IRenderer* const p_renderer)
	{
		assert(p_renderer);

		const auto cmd = m_device->getCommandbuffer();
		const auto default_sampler = p_renderer->getKnownSamplerState(IRenderer::SamplerState::LinearClamp);

		for (auto& v : m_buffer_map) {
			if (!v.second.constant_buffer->update(v.second.buffer.data(), static_cast<uint32_t>(v.second.buffer.size()), true)) {
				return false;
			}
			cmd->bindPixelShaderConstantBuffer(v.second.index, v.second.constant_buffer.get());
		}

		for (auto& v : m_texture2d_map) {
			cmd->bindPixelShaderTexture2D(v.second.index, v.second.texture.get());
			const auto sampler = v.second.texture->getSamplerState() != nullptr
				? v.second.texture->getSamplerState()
				: default_sampler;
			cmd->bindPixelShaderSampler(v.second.index, sampler);
		}

		return true;
	}

	PostEffectShader_D3D11::PostEffectShader_D3D11(GraphicsDevice* p_device, StringView path, bool is_path_)
		: m_device(p_device)
		, source(path)
		, is_path(is_path_)
	{
		if (!createResources(false))
			throw std::runtime_error("PostEffectShader_D3D11::PostEffectShader_D3D11");
		m_device->addEventListener(this);
	}
	PostEffectShader_D3D11::~PostEffectShader_D3D11()
	{
		m_device->removeEventListener(this);
	}
}

namespace core::Graphics {
	bool Renderer_D3D11::createBuffers() {
		if (!m_device->createConstantBuffer(sizeof(DirectX::XMFLOAT4X4), _vp_matrix_buffer.put())) {
			return false;
		}
		if (!m_device->createConstantBuffer(sizeof(DirectX::XMFLOAT4X4), _world_matrix_buffer.put())) {
			return false;
		}
		if (!m_device->createConstantBuffer(2 * sizeof(DirectX::XMFLOAT4), _camera_pos_buffer.put())) {
			return false;
		}
		if (!m_device->createConstantBuffer(2 * sizeof(DirectX::XMFLOAT4), _fog_data_buffer.put())) {
			return false;
		}
		// 用户最多可用 8 个 float4
		if (!m_device->createConstantBuffer(8 * sizeof(DirectX::XMFLOAT4), _user_float_buffer.put())) {
			return false;
		}
		return true;
	}
	bool Renderer_D3D11::createSamplers() {
		GraphicsSamplerInfo sampler_info{};

		// point

		sampler_info.filter = GraphicsFilter::point;
		sampler_info.address_u = GraphicsTextureAddressMode::wrap;
		sampler_info.address_v = GraphicsTextureAddressMode::wrap;
		sampler_info.address_w = GraphicsTextureAddressMode::wrap;
		if (!m_device->createSampler(sampler_info, _sampler_state[IDX(SamplerState::PointWrap)].put()))
			return false;

		sampler_info.filter = GraphicsFilter::point;
		sampler_info.address_u = GraphicsTextureAddressMode::clamp;
		sampler_info.address_v = GraphicsTextureAddressMode::clamp;
		sampler_info.address_w = GraphicsTextureAddressMode::clamp;
		if (!m_device->createSampler(sampler_info, _sampler_state[IDX(SamplerState::PointClamp)].put()))
			return false;

		sampler_info.filter = GraphicsFilter::point;
		sampler_info.address_u = GraphicsTextureAddressMode::border;
		sampler_info.address_v = GraphicsTextureAddressMode::border;
		sampler_info.address_w = GraphicsTextureAddressMode::border;
		if (!m_device->createSampler(sampler_info, _sampler_state[IDX(SamplerState::PointBorderBlack)].put()))
			return false;

		sampler_info.filter = GraphicsFilter::point;
		sampler_info.address_u = GraphicsTextureAddressMode::border;
		sampler_info.address_v = GraphicsTextureAddressMode::border;
		sampler_info.address_w = GraphicsTextureAddressMode::border;
		if (!m_device->createSampler(sampler_info, _sampler_state[IDX(SamplerState::PointBorderWhite)].put())) // TODO: white?
			return false;

		// linear

		sampler_info.filter = GraphicsFilter::linear;
		sampler_info.address_u = GraphicsTextureAddressMode::wrap;
		sampler_info.address_v = GraphicsTextureAddressMode::wrap;
		sampler_info.address_w = GraphicsTextureAddressMode::wrap;
		if (!m_device->createSampler(sampler_info, _sampler_state[IDX(SamplerState::LinearWrap)].put()))
			return false;

		sampler_info.filter = GraphicsFilter::linear;
		sampler_info.address_u = GraphicsTextureAddressMode::clamp;
		sampler_info.address_v = GraphicsTextureAddressMode::clamp;
		sampler_info.address_w = GraphicsTextureAddressMode::clamp;
		if (!m_device->createSampler(sampler_info, _sampler_state[IDX(SamplerState::LinearClamp)].put()))
			return false;

		sampler_info.filter = GraphicsFilter::linear;
		sampler_info.address_u = GraphicsTextureAddressMode::border;
		sampler_info.address_v = GraphicsTextureAddressMode::border;
		sampler_info.address_w = GraphicsTextureAddressMode::border;
		if (!m_device->createSampler(sampler_info, _sampler_state[IDX(SamplerState::LinearBorderBlack)].put()))
			return false;

		sampler_info.filter = GraphicsFilter::linear;
		sampler_info.address_u = GraphicsTextureAddressMode::border;
		sampler_info.address_v = GraphicsTextureAddressMode::border;
		sampler_info.address_w = GraphicsTextureAddressMode::border;
		if (!m_device->createSampler(sampler_info, _sampler_state[IDX(SamplerState::LinearBorderWhite)].put())) // TODO: white?
			return false;
		
		return true;
	}
	void Renderer_D3D11::bindGraphicsPipeline() {
		const auto graphics_pipeline = getKnownGraphicsPipeline(
			_state_set.vertex_color_blend_state,
			_state_set.fog_state,
			_state_set.texture_alpha_type,
			_state_set.depth_state,
			_state_set.blend_state
		);
		m_device->getCommandbuffer()->bindGraphicsPipeline(graphics_pipeline);
	}
	void Renderer_D3D11::bindTextureSamplerState(ITexture2D* const texture) {
		const auto sampler = texture->getSamplerState() != nullptr
			? texture->getSamplerState()
			: _sampler_state[IDX(SamplerState::LinearClamp)].get(); // default sampler: linear+clamp
		m_device->getCommandbuffer()->bindPixelShaderSampler(0, sampler);
	}
	void Renderer_D3D11::bindTextureAlphaType(ITexture2D* const texture) {
		const auto alpha_mode = getTextureAlphaMode(texture);
		// update graphics pipeline
		_state_set.texture_alpha_type = alpha_mode;
		bindGraphicsPipeline();
	}
	bool Renderer_D3D11::batchFlush(const bool discard) {
		tracy_zone_scoped;

		if (discard) {
			m_primitive_batch_renderer.discard();
			m_primitive_batch_renderer.endBatch();
			m_primitive_batch_renderer.setCycleOnNextBatch();
			setTexture(_state_texture.get());
			return true;
		}

		{
			tracy_d3d11_context_zone(m_device->GetTracyContext(), "BatchFlush");
			if (!m_primitive_batch_renderer.consume()) {
				return false;
			}
		}

		//cmd->bindPixelShaderTexture2D(0, cmd_.texture.get());
		//bindTextureSamplerState(cmd_.texture.get());
		//bindTextureAlphaType(cmd_.texture.get());

		// unbound: solve some debug warning
		//cmd->bindPixelShaderTexture2D(0, nullptr);

		return true;
	}

	bool Renderer_D3D11::createResources(IGraphicsDevice* const device) {
		Logger::info("[core] [Renderer] initializing...");
		if (device == nullptr) {
			assert(false); return false;
		}
		m_device = static_cast<GraphicsDevice*>(device);

		if (!createBuffers()) {
			Logger::error("[core] [Renderer] create Buffers failed");
			return false;
		}
		if (!createSamplers()) {
			Logger::error("[core] [Renderer] create Samplers failed");
			return false;
		}
		if (!createGraphicsPipelines()) {
			Logger::error("[core] [Renderer] create GraphicsPipelines failed");
			return false;
		}
		if (!m_primitive_batch_renderer.createResources(m_device.get())) {
			Logger::error("[core] [Renderer] create PrimitiveBatchRenderer failed");
			return false;
		}

		Logger::info("[core] [Renderer] initialization complete");
		m_device->addEventListener(this);
		m_initialized = true;
		return true;
	}
	void Renderer_D3D11::onGraphicsDeviceCreate() {}
	void Renderer_D3D11::onGraphicsDeviceDestroy() {
		batchFlush(true);
	}

	bool Renderer_D3D11::beginBatch() {
		if (_batch_scope) {
			assert(false); return false;
		}
		const auto cmd = m_device->getCommandbuffer();

		// IA Stage
		// - vertex buffer
		// - index buffer

		if (!m_primitive_batch_renderer.beginBatch(true)) {
			return false;
		}

		// VS Stage
		// - constant buffer (view projection matrix @0)
		// - constant buffer (world/model matrix @1)

		cmd->bindVertexShaderConstantBuffer(d3d11::vertex_shader_constant_buffer_slot_view_projection_matrix, _vp_matrix_buffer.get());
		cmd->bindVertexShaderConstantBuffer(d3d11::vertex_shader_constant_buffer_slot_world_matrix, _world_matrix_buffer.get());

		// RS Stage
		// - viewport
		// - scissor rect

		applyViewportAndScissorRect();

		// PS Stage
		// - constant buffer (camera info @0)
		// - constant buffer (fog parameters @1)
		// - shader resource view (texture @0)
		// - sampler state (sampler @0)

		cmd->bindPixelShaderConstantBuffer(d3d11::pixel_shader_constant_buffer_slot_camera_position, _camera_pos_buffer.get());
		cmd->bindPixelShaderConstantBuffer(d3d11::pixel_shader_constant_buffer_slot_fog_parameter, _fog_data_buffer.get());
		if (_state_texture) {
			_state_set.texture_alpha_type = getTextureAlphaMode(_state_texture.get());
			cmd->bindPixelShaderTexture2D(0, _state_texture.get());
			bindTextureSamplerState(_state_texture.get());
		}

		// Camera
		// - VS Stage
		//     - constant buffer (view projection matrix @0)
		// - PS Stage
		//     - constant buffer (camera info @0)
		//     - constant buffer (fog parameters @1)

		if (!uploadCameraState()) {
			return false;
		}
		if (!uploadFogState()) {
			return false;
		}

		// Pipeline
		// - IA Stage
		//     - input layout
		//     - primitive type
		// - VS Stage
		//     - vertex shader
		// - RS Stage
		//     - rasterizer state
		// - PS Stage
		//     - pixel shader
		// - OM Stage
		//     - depth stencil state
		//     - blend state

		bindGraphicsPipeline();

		_batch_scope = true;
		return true;
	}
	bool Renderer_D3D11::endBatch() {
		if (!_batch_scope) {
			assert(false); return false;
		}
		_batch_scope = false;
		if (!m_primitive_batch_renderer.endBatch()) {
			return false;
		}
		_state_texture.reset();
		return true;
	}
	bool Renderer_D3D11::flush() {
		return batchFlush();
	}

	void Renderer_D3D11::clearRenderTarget(const Color4B& color) {
		if (_batch_scope) {
			m_primitive_batch_renderer.consume();
		}
		auto* ctx = m_device->GetD3D11DeviceContext();
		assert(ctx);
		ID3D11RenderTargetView* rtv = NULL;
		ctx->OMGetRenderTargets(1, &rtv, NULL);
		if (rtv)
		{
			FLOAT const clear_color[4] = {
				(float)color.r / 255.0f,
				(float)color.g / 255.0f,
				(float)color.b / 255.0f,
				(float)color.a / 255.0f,
			};
			ctx->ClearRenderTargetView(rtv, clear_color);
			rtv->Release();
		}
	}
	void Renderer_D3D11::clearDepthBuffer(const float zvalue) {
		if (_batch_scope) {
			m_primitive_batch_renderer.consume();
		}
		auto* ctx = m_device->GetD3D11DeviceContext();
		assert(ctx);
		ID3D11DepthStencilView* dsv = NULL;
		ctx->OMGetRenderTargets(1, NULL, &dsv);
		if (dsv)
		{
			ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, zvalue, D3D11_DEFAULT_STENCIL_REFERENCE);
			dsv->Release();
		}
	}
	void Renderer_D3D11::setRenderAttachment(IRenderTarget* p_rt, IDepthStencilBuffer* p_ds) {
		if (_batch_scope) {
			m_primitive_batch_renderer.consume();
		}
		m_device->getCommandbuffer()->bindRenderTarget(p_rt, p_ds);
	}

	void Renderer_D3D11::setOrtho(const BoxF& box) {
		if (_camera_state_set.isEqual(box)) {
			return;
		}
		if (_batch_scope) {
			m_primitive_batch_renderer.consume();
		}
		// update mvp matrix constant buffer
		_camera_state_set.ortho = box;
		_camera_state_set.is_3D = false;
		uploadOrtho();
	}
	void Renderer_D3D11::setPerspective(
		const Vector3F& eye, const Vector3F& lookat, const Vector3F& headup,
		float fov, float aspect, float znear, float zfar
	) {
		if (_camera_state_set.isEqual(eye, lookat, headup, fov, aspect, znear, zfar)) {
			return;
		}
		if (_batch_scope) {
			m_primitive_batch_renderer.consume();
		}
		// update mvp matrix constant buffer
		_camera_state_set.eye = eye;
		_camera_state_set.lookat = lookat;
		_camera_state_set.headup = headup;
		_camera_state_set.fov = fov;
		_camera_state_set.aspect = aspect;
		_camera_state_set.znear = znear;
		_camera_state_set.zfar = zfar;
		_camera_state_set.is_3D = true;
		uploadPerspective();
	}

	void Renderer_D3D11::setViewport(const BoxF& box) {
		if (_state_set.viewport == box) {
			return;
		}
		if (_batch_scope) {
			m_primitive_batch_renderer.consume();
		}
		// update viewport
		_state_set.viewport = box;
		applyViewport();
	}
	void Renderer_D3D11::setScissorRect(const RectF& rect) {
		if (_state_set.scissor_rect == rect) {
			return;
		}
		if (_batch_scope) {
			m_primitive_batch_renderer.consume();
		}
		// update scissor rect
		_state_set.scissor_rect = rect;
		applyScissorRect();
	}
	void Renderer_D3D11::setViewportAndScissorRect() {
		applyViewportAndScissorRect();
	}

	void Renderer_D3D11::setVertexColorBlendState(const VertexColorBlendState state) {
		if (_state_set.vertex_color_blend_state == state) {
			return;
		}
		if (_batch_scope) {
			m_primitive_batch_renderer.consume();
		}
		// update graphics pipeline
		_state_set.vertex_color_blend_state = state;
		bindGraphicsPipeline();
	}
	void Renderer_D3D11::setFogState(const FogState state, const Color4B& color, const float density_or_znear, const float zfar) {
		if (_state_set.fog_state == state && _state_set.fog_color == color && _state_set.fog_near_or_density == density_or_znear && _state_set.fog_far == zfar) {
			return;
		}
		if (_batch_scope) {
			m_primitive_batch_renderer.consume();
		}
		// update graphics pipeline
		_state_set.fog_state = state;
		_state_set.fog_color = color;
		_state_set.fog_near_or_density = density_or_znear;
		_state_set.fog_far = zfar;
		uploadFogState();
		bindGraphicsPipeline();
	}
	void Renderer_D3D11::setDepthState(const DepthState state) {
		if (_state_set.depth_state == state) {
			return;
		}
		if (_batch_scope) {
			m_primitive_batch_renderer.consume();
		}
		// update graphics pipeline
		_state_set.depth_state = state;
		bindGraphicsPipeline();
	}
	void Renderer_D3D11::setBlendState(const BlendState state) {
		if (_state_set.blend_state == state) {
			return;
		}
		if (_batch_scope) {
			m_primitive_batch_renderer.consume();
		}
		// update graphics pipeline
		_state_set.blend_state = state;
		bindGraphicsPipeline();
	}

	void Renderer_D3D11::setTexture(ITexture2D* const texture) {
		const auto alpha_mode = getTextureAlphaMode(texture);
		if (isTextureEquals(_state_texture, texture) && _state_set.texture_alpha_type == alpha_mode) {
			return;
		}
		if (_batch_scope) {
			m_primitive_batch_renderer.consume();
		}
		// update graphics pipeline
		_state_texture = texture;
		const auto cmd = m_device->getCommandbuffer();
		cmd->bindPixelShaderTexture2D(0, _state_texture.get());
		bindTextureSamplerState(_state_texture.get());
		if (_state_set.texture_alpha_type != alpha_mode) {
			_state_set.texture_alpha_type = alpha_mode;
			bindGraphicsPipeline();
		}
	}

	bool Renderer_D3D11::drawTriangle(const DrawVertex& v1, const DrawVertex& v2, const DrawVertex& v3) {
		return m_primitive_batch_renderer.addTriangle(v1, v2, v3);
	}
	bool Renderer_D3D11::drawTriangle(const DrawVertex vertices[3]) {
		return m_primitive_batch_renderer.addTriangle(vertices);
	}
	bool Renderer_D3D11::drawQuad(const DrawVertex& v1, const DrawVertex& v2, const DrawVertex& v3, const DrawVertex& v4) {
		return m_primitive_batch_renderer.addQuad(v1, v2, v3, v4);
	}
	bool Renderer_D3D11::drawQuad(const DrawVertex vertices[4]) {
		return m_primitive_batch_renderer.addQuad(vertices);
	}
	bool Renderer_D3D11::drawRaw(const DrawVertex* const vertices, const uint16_t vertex_count, const DrawIndex* const indices, const uint16_t index_count) {
		return m_primitive_batch_renderer.addRaw(vertices, vertex_count, indices, index_count);
	}
	bool Renderer_D3D11::drawRequest(const uint16_t vertex_count, const uint16_t index_count, DrawVertex** const out_vertices, DrawIndex** const out_indices, uint16_t* const out_index_offset) {
		size_t index_offset_sz{};
		if (!m_primitive_batch_renderer.addRequest(vertex_count, index_count, out_vertices, out_indices, &index_offset_sz)) {
			return false;
		}
		*out_index_offset = static_cast<uint16_t>(index_offset_sz);
		return true;
	}

	bool Renderer_D3D11::createPostEffectShader(const StringView path, IPostEffectShader** const pp_effect) {
		try {
			*pp_effect = new PostEffectShader_D3D11(m_device.get(), path, true);
			return true;
		}
		catch (...) {
			*pp_effect = nullptr;
			return false;
		}
	}
	bool Renderer_D3D11::drawPostEffect(
		IPostEffectShader* p_effect,
		BlendState blend,
		ITexture2D* p_tex, SamplerState rtsv,
		Vector4F const* cv, size_t cv_n,
		ITexture2D* const* p_tex_arr, SamplerState const* sv, size_t tv_sv_n
	) {
		assert(p_effect);
		assert((cv_n == 0) || (cv_n > 0 && cv));
		assert((tv_sv_n == 0) || (tv_sv_n > 0 && p_tex_arr && sv));

		// FLUSH

		if (!endBatch()) {
			return false;
		}

		// PREPARE

		const auto cmd = m_device->getCommandbuffer();
		const auto ctx = static_cast<ID3D11DeviceContext*>(cmd->getNativeHandle());
		assert(ctx != nullptr);

		RenderTargetHolder rtv(ctx);
		const auto rtv_w = static_cast<float>(rtv.width);
		const auto rtv_h = static_cast<float>(rtv.height);

		if (!rtv.rtv || rtv.width == 0 || rtv.height == 0) {
			Logger::error("[core] [Renderer] postEffect failed: no RenderTarget bound");
			return false;
		}

		ctx->ClearState();

		// [Stage IA] upload vertex data

		if (!m_primitive_batch_renderer.beginBatch(false)) {
			return false;
		}
		const DrawVertex vertex_data[4]{
			DrawVertex(0.00f, rtv_h, 0.0f, 0.0f),
			DrawVertex(rtv_w, rtv_h, 1.0f, 0.0f),
			DrawVertex(rtv_w, 0.00f, 1.0f, 1.0f),
			DrawVertex(0.00f, 0.00f, 0.0f, 1.0f),
		};
		if (!m_primitive_batch_renderer.addQuad(vertex_data)) {
			return false;
		}
		if (!m_primitive_batch_renderer.endBatch()) {
			return false;
		}

		// [Stage VS]

		DirectX::XMFLOAT4X4 vp_matrix;
		DirectX::XMStoreFloat4x4(&vp_matrix, DirectX::XMMatrixOrthographicOffCenterLH(
			0.0f, rtv_w, 0.0f, rtv_h, 0.0f, 1.0f
		));
		if (!_vp_matrix_buffer->update(&vp_matrix, sizeof(vp_matrix), true)) {
			Logger::error("[core] [Renderer] upload constant buffer failed (vp_matrix_buffer)");
		}
		cmd->bindVertexShaderConstantBuffer(d3d11::vertex_shader_constant_buffer_slot_view_projection_matrix, _vp_matrix_buffer.get());

		// [Stage RS]

		cmd->setViewport(0.0f, 0.0f, rtv_w, rtv_h);
		cmd->setScissorRect(0, 0, rtv.width, rtv.height);

		// [Stage PS]

		const auto cv_sz = static_cast<uint32_t>(std::min<size_t>(cv_n, 8) * sizeof(Vector4F));
		if (!_user_float_buffer->update(cv, cv_sz, true)) {
			Logger::error("[core] [Renderer] upload constant buffer failed (user_float_buffer)");
		}
		const float size_viewport[8] = {
			rtv_w, rtv_h, 0.0f, 0.0f,
			_state_set.viewport.a.x, _state_set.viewport.a.y, _state_set.viewport.b.x, _state_set.viewport.b.y,
		};
		if (!_fog_data_buffer->update(size_viewport, sizeof(size_viewport), true)) {
			Logger::error("[core] [Renderer] upload constant buffer failed (fog_data_buffer/size_viewport_buffer)");
		}
		cmd->bindPixelShaderConstantBuffer(d3d11::pixel_shader_constant_buffer_slot_user_data, _user_float_buffer.get());
		cmd->bindPixelShaderConstantBuffer(d3d11::pixel_shader_constant_buffer_slot_fog_parameter, _fog_data_buffer.get());

		ITexture2D* textures[5]{};
		IGraphicsSampler* samplers[5]{};
		for (uint32_t slot = 0; slot < std::min<uint32_t>(static_cast<uint32_t>(tv_sv_n), 4); slot += 1) {
			textures[slot] = p_tex_arr[slot];
			samplers[slot] = _sampler_state[IDX(sv[slot])].get();
		}
		textures[4] = p_tex;
		samplers[4] = _sampler_state[IDX(rtsv)].get();
		cmd->bindPixelShaderTexture2D(0, textures, 5);
		cmd->bindPixelShaderSampler(0, samplers, 5);

		// Pipeline

		const auto graphics_pipeline = getKnownGraphicsPipeline(
			VertexColorBlendState::Mul, // will overwrite
			FogState::Disable,
			TextureAlphaType::PremulAlpha, // will overwrite
			DepthState::Disable,
			blend
		);
		cmd->bindGraphicsPipeline(graphics_pipeline);

		// [Stage PS] Overwrite

		ctx->PSSetShader(static_cast<PostEffectShader_D3D11*>(p_effect)->GetPS(), NULL, 0);

		// [Stage OM]

		rtv.bind();

		// DRAW

		m_primitive_batch_renderer.draw();

		// CLEAR

		ctx->ClearState();
		rtv.bind();

		return beginBatch();
	}
	bool Renderer_D3D11::drawPostEffect(IPostEffectShader* p_effect, BlendState blend) {
		assert(p_effect);

		// FLUSH

		if (!endBatch()) {
			return false;
		}

		// PREPARE

		const auto cmd = m_device->getCommandbuffer();
		const auto ctx = static_cast<ID3D11DeviceContext*>(cmd->getNativeHandle());
		assert(ctx != nullptr);

		RenderTargetHolder rtv(ctx);
		const auto rtv_w = static_cast<float>(rtv.width);
		const auto rtv_h = static_cast<float>(rtv.height);

		if (!rtv.rtv || rtv.width == 0 || rtv.height == 0) {
			Logger::error("[core] [Renderer] postEffect failed: no RenderTarget bound");
			return false;
		}

		ctx->ClearState();

		// [Stage IA] upload vertex data

		if (!m_primitive_batch_renderer.beginBatch(false)) {
			return false;
		}
		const DrawVertex vertex_data[4]{
			DrawVertex(0.00f, rtv_h, 0.0f, 0.0f),
			DrawVertex(rtv_w, rtv_h, 1.0f, 0.0f),
			DrawVertex(rtv_w, 0.00f, 1.0f, 1.0f),
			DrawVertex(0.00f, 0.00f, 0.0f, 1.0f),
		};
		if (!m_primitive_batch_renderer.addQuad(vertex_data)) {
			return false;
		}
		if (!m_primitive_batch_renderer.endBatch()) {
			return false;
		}

		// [Stage VS]

		DirectX::XMFLOAT4X4 vp_matrix;
		DirectX::XMStoreFloat4x4(&vp_matrix, DirectX::XMMatrixOrthographicOffCenterLH(
			0.0f, rtv_w, 0.0f, rtv_h, 0.0f, 1.0f
		));
		if (!_vp_matrix_buffer->update(&vp_matrix, sizeof(vp_matrix), true)) {
			Logger::error("[core] [Renderer] upload constant buffer failed (vp_matrix_buffer)");
		}
		cmd->bindVertexShaderConstantBuffer(d3d11::vertex_shader_constant_buffer_slot_view_projection_matrix, _vp_matrix_buffer.get());

		// [Stage RS]

		cmd->setViewport(0.0f, 0.0f, rtv_w, rtv_h);
		cmd->setScissorRect(0, 0, rtv.width, rtv.height);

		// Pipeline

		const auto graphics_pipeline = getKnownGraphicsPipeline(
			VertexColorBlendState::Mul, // will overwrite
			FogState::Disable,
			TextureAlphaType::PremulAlpha, // will overwrite
			DepthState::Disable,
			blend
		);
		cmd->bindGraphicsPipeline(graphics_pipeline);

		// [Stage PS] Overwrite

		if (!p_effect->apply(this)) {
			Logger::error("[core] [Renderer] PostEffectShader apply failed");
			return false;
		}
		ctx->PSSetShader(static_cast<PostEffectShader_D3D11*>(p_effect)->GetPS(), NULL, 0);

		// [Stage OM]

		rtv.bind();

		// DRAW

		m_primitive_batch_renderer.draw();

		// CLEAR

		ctx->ClearState();
		rtv.bind();

		return beginBatch();
	}

	bool Renderer_D3D11::createModel(StringView path, IModel** pp_model) {
		if (!m_model_shared) {
			Logger::info("[core] [Model] creating shared resources...");
			try {
				*(m_model_shared.put()) = new ModelSharedComponent_D3D11(m_device.get());
				Logger::info("[core] [Model] shared resources created");
			}
			catch (...) {
				Logger::error("[core] [Model] create shared resources failed");
				return false;
			}
		}

		try {
			*pp_model = new Model_D3D11(m_device.get(), m_model_shared.get(), path);
			return true;
		}
		catch (const std::exception&) {
			*pp_model = nullptr;
			Logger::error("[core] [Model] create failed");
			return false;
		}
	}
	bool Renderer_D3D11::drawModel(IModel* p_model)
	{
		if (!p_model)
		{
			assert(false);
			return false;
		}

		if (!endBatch())
		{
			return false;
		}

		static_cast<Model_D3D11*>(p_model)->draw(_state_set.fog_state);

		if (!beginBatch())
		{
			return false;
		}

		return true;
	}

	IGraphicsSampler* Renderer_D3D11::getKnownSamplerState(const SamplerState state) {
		return _sampler_state[IDX(state)].get();
	}
	IGraphicsPipeline* Renderer_D3D11::getKnownGraphicsPipeline(
		const VertexColorBlendState vertex_color_blend_state,
		const FogState fog_state,
		const TextureAlphaType texture_alpha_mode,
		const DepthState depth_state,
		const BlendState blend_state
	) {
		return _graphics_pipeline
			[IDX(vertex_color_blend_state)]
			[IDX(fog_state)]
			[IDX(texture_alpha_mode)]
			[IDX(depth_state)]
			[IDX(blend_state)]
			.get();
	}

	Renderer_D3D11::Renderer_D3D11() = default;
	Renderer_D3D11::~Renderer_D3D11() {
		if (m_initialized) {
			m_device->removeEventListener(this);
		}
	}

	bool Renderer_D3D11::uploadOrtho(const BoxF& box) {
		DirectX::XMFLOAT4X4 mat4;
		DirectX::XMStoreFloat4x4(&mat4, DirectX::XMMatrixOrthographicOffCenterLH(box.a.x, box.b.x, box.b.y, box.a.y, box.a.z, box.b.z));
		if (!_vp_matrix_buffer->update(&mat4, sizeof(mat4), true)) {
			Logger::error("[core] [Renderer] upload constant buffer failed (vp_matrix_buffer)");
			return false;
		}
		return true;
	}
	bool Renderer_D3D11::uploadOrtho() {
		if (_camera_state_set.is_3D) {
			assert(false); return false;
		}
		return uploadOrtho(_camera_state_set.ortho);
	}
	bool Renderer_D3D11::uploadPerspective(
		const Vector3F& position, const Vector3F& look_at, const Vector3F& head_up,
		const float fov, const float aspect_ratio, const float z_near, const float z_far
	) {
		const DirectX::XMFLOAT3 xm_position(position.x, position.y, position.z);
		const DirectX::XMFLOAT3 xm_look_at(look_at.x, look_at.y, look_at.z);
		const DirectX::XMFLOAT3 xm_head_up(head_up.x, head_up.y, head_up.z);
		DirectX::XMFLOAT4X4 mat4;
		DirectX::XMStoreFloat4x4(&mat4,
			DirectX::XMMatrixMultiply(
				DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&xm_position), DirectX::XMLoadFloat3(&xm_look_at), DirectX::XMLoadFloat3(&xm_head_up)),
				DirectX::XMMatrixPerspectiveFovLH(fov, aspect_ratio, z_near, z_far)
			)
		);
		if (!_vp_matrix_buffer->update(&mat4, sizeof(mat4), true)) {
			Logger::error("[core] [Renderer] upload constant buffer failed (vp_matrix_buffer)");
			return false;
		}

		const float camera_pos[8] = {
			position.x, position.y, position.z, 0.0f,
			look_at.x - position.x, look_at.y - position.y, look_at.z - position.z, 0.0f,
		};
		if (!_camera_pos_buffer->update(camera_pos, sizeof(camera_pos), true)) {
			Logger::error("[core] [Renderer] upload constant buffer failed (camera_pos_buffer)");
			return false;
		}

		return true;
	}
	bool Renderer_D3D11::uploadPerspective() {
		if (!_camera_state_set.is_3D) {
			assert(false); return false;
		}
		const auto& s = _camera_state_set;
		return uploadPerspective(s.eye, s.lookat, s.headup, s.fov, s.aspect, s.znear, s.zfar);
	}
	bool Renderer_D3D11::uploadCameraState() {
		if (_camera_state_set.is_3D) {
			return uploadPerspective();
		}
		else {
			return uploadOrtho();
		}
	}
	void Renderer_D3D11::applyViewport(const BoxF& box) {
		m_device->getCommandbuffer()->setViewport(box.a.x, box.a.y, box.b.x - box.a.x, box.b.y - box.a.y, box.a.z, box.b.z);
	}
	void Renderer_D3D11::applyViewport() {
		applyViewport(_state_set.viewport);
	}
	void Renderer_D3D11::applyScissorRect(const RectF& rect) {
		m_device->getCommandbuffer()->setScissorRect(
			static_cast<int32_t>(rect.a.x), static_cast<int32_t>(rect.a.y),
			static_cast<uint32_t>(rect.b.x - rect.a.x), static_cast<uint32_t>(rect.b.y - rect.a.y)
		);
	}
	void Renderer_D3D11::applyScissorRect() {
		applyScissorRect(_state_set.scissor_rect);
	}
	void Renderer_D3D11::applyViewportAndScissorRect() {
		applyViewport();
		applyScissorRect();
	}
	bool Renderer_D3D11::uploadFogState(FogState, const Color4B& color, const float density_or_znear, const float zfar) {
		constexpr float factor = 1.0f / 255.0f;
		const float fog_color_and_range[8] = {
			static_cast<float>(color.r) * factor,
			static_cast<float>(color.g) * factor,
			static_cast<float>(color.b) * factor,
			static_cast<float>(color.a) * factor,
			density_or_znear, zfar, 0.0f, zfar - density_or_znear,
		};
		if (!_fog_data_buffer->update(fog_color_and_range, sizeof(fog_color_and_range), true)) {
			Logger::error("[core] [Renderer] upload constant buffer failed (fog_data_buffer)");
			return false;
		}
		return true;
	}
	bool Renderer_D3D11::uploadFogState() {
		const auto& s = _state_set;
		return uploadFogState(s.fog_state, s.fog_color, s.fog_near_or_density, s.fog_far);
	}

	bool IRenderer::create(IGraphicsDevice* const p_device, IRenderer** const pp_renderer) {
		if (p_device == nullptr) {
			assert(false); return false;
		}
		if (pp_renderer == nullptr) {
			assert(false); return false;
		}
		SmartReference<Renderer_D3D11> renderer;
		renderer.attach(new Renderer_D3D11());
		if (!renderer->createResources(p_device)) {
			return false;
		}
		*pp_renderer = renderer.detach();
		return true;
	}
}
