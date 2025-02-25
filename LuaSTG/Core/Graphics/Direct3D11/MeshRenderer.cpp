#include "Core/Graphics/Direct3D11/MeshRenderer.hpp"
#include "Core/Graphics/Direct3D11/Constants.hpp"
#include "Core/Graphics/Direct3D11/Buffer.hpp"
#include "Core/Graphics/Direct3D11/Texture2D.hpp"
#include "Core/Graphics/Direct3D11/SamplerState.hpp"
#include "Core/Graphics/Direct3D11/Device.hpp"
#include "Core/Graphics/Direct3D11/Mesh.hpp"

namespace Core::Graphics::Direct3D11 {
	void MeshRenderer::onDeviceCreate() {
	}
	void MeshRenderer::onDeviceDestroy() {
		m_transform_dirty = true;
	}

	void MeshRenderer::setTransform(Matrix4F const& transform) {
		if (m_transform != transform) {
			m_transform = transform;
			m_transform_dirty = true;
		}
	}
	void MeshRenderer::setTexture(ITexture2D* const texture) {
		m_texture = texture;
	}
	void MeshRenderer::setMesh(IMesh* const mesh) {
		m_mesh = mesh;
	}
	void MeshRenderer::draw(IRenderer* const renderer) {
		assert(renderer);
		if (!renderer->flush()) {
			return;
		}

		auto const ctx = static_cast<Device*>(m_device.get())->GetD3D11DeviceContext();
		assert(ctx);

		// Mesh setup:
		// * IA stage
		//     * vertex buffer
		//     * index buffer
		//     * primitive topology
		//     * input layout
		// * VS stage
		//     * vertex shader

		auto const mesh = static_cast<Mesh*>(m_mesh.get());
		assert(mesh);
		mesh->applyNative(ctx, false); // TODO: FOG

		// VS stage constant buffer setup by Renderer
		// * constant buffer (view projection matrix)
		// see: Constants::vertex_shader_stage_constant_buffer_slot_view_projection_matrix

		// VS stage constant buffer setup by MeshRenderer
		// * constant buffer (world matrix)

		ID3D11Buffer* vs_sb_backup[1]{};
		ctx->VSGetConstantBuffers(Constants::vertex_shader_stage_constant_buffer_slot_world_matrix, 1, vs_sb_backup);
		if (!uploadConstantBuffer()) {
			return;
		}
		ID3D11Buffer* const world_matrix[1]{ static_cast<Buffer*>(m_constant_buffer.get())->getNativeBuffer() };
		ctx->VSSetConstantBuffers(Constants::vertex_shader_stage_constant_buffer_slot_world_matrix, 1, world_matrix);

		// RS stage setup by Renderer:
		// * viewport
		// * scissor rect
		// * rs state

		// PS stage setup by Renderer:
		// * pixel shader
		// * constant buffer (camera position, fog parameter)

		// PS stage setup by MeshRenderer:
		// * shader resource view (texture)
		// * sampler state

		ID3D11ShaderResourceView* ps_srv_backup[1]{};
		ctx->PSGetShaderResources(0, 1, ps_srv_backup);
		ID3D11ShaderResourceView* const texture[1]{ static_cast<Texture2D*>(m_texture.get())->GetView() };
		ctx->PSSetShaderResources(0, 1, texture);

		ID3D11SamplerState* ps_ss_backup[1]{};
		ctx->PSGetSamplers(0, 1, ps_ss_backup);
		ID3D11SamplerState* sampler_state[1]{};
		if (m_texture->getSamplerState()) {
			sampler_state[0] = static_cast<SamplerState*>(m_texture->getSamplerState())->GetState();
		} else {
			auto const ss = renderer->getKnownSamplerState(IRenderer::SamplerState::LinearWrap);
			sampler_state[0] = static_cast<SamplerState*>(ss)->GetState();
		}
		ctx->PSSetSamplers(0, 1, sampler_state);

		// OM stage setup by Renderer:
		// * depth stencil state
		// * blend state

		// OM stage setup by Renderer/SwapChain/Window:
		// * render target view
		// * depth stencil view

		// Mesh draw

		mesh->drawNative(ctx);

		// restore state

		ctx->VSSetConstantBuffers(Constants::vertex_shader_stage_constant_buffer_slot_world_matrix, 1, vs_sb_backup);
		ctx->PSSetShaderResources(0, 1, ps_srv_backup);
		ctx->PSSetSamplers(0, 1, ps_ss_backup);
	}

	MeshRenderer::MeshRenderer() = default;
	MeshRenderer::~MeshRenderer() = default;

	bool MeshRenderer::uploadConstantBuffer() {
		if (!m_transform_dirty) {
			return true;
		}
		void* ptr{};
		if (!m_constant_buffer->map(sizeof(m_transform), true, &ptr)) {
			return false;
		}
		std::memcpy(ptr, &m_transform, sizeof(m_transform));
		if (!m_constant_buffer->unmap()) {
			return false;
		}
		return true;
	}
	bool MeshRenderer::initialize(IDevice* const device) {
		assert(device);
		m_device = device;
		if (!m_device->createConstantBuffer(sizeof(m_transform), ~m_constant_buffer)) {
			return false;
		}
		return true;
	}
}
namespace Core::Graphics {
	bool IMeshRenderer::create(IDevice* const device, IMeshRenderer** const output) {
		assert(device);
		assert(output);
		*output = nullptr;
		ScopeObject<Direct3D11::MeshRenderer> mesh_renderer;
		mesh_renderer.attach(new Direct3D11::MeshRenderer);
		if (!mesh_renderer->initialize(device)) {
			return false;
		}
		*output = mesh_renderer.detach();
		return true;
	}
}
