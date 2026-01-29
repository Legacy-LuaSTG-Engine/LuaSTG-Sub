#include "core/Graphics/Direct3D11/MeshRenderer.hpp"
#include "core/Graphics/Direct3D11/Constants.hpp"
#include "core/Graphics/Direct3D11/Buffer.hpp"
#include "core/Graphics/Direct3D11/Texture2D.hpp"
#include "core/Graphics/Direct3D11/SamplerState.hpp"
#include "core/Graphics/Direct3D11/Device.hpp"
#include "core/Graphics/Direct3D11/Mesh.hpp"
#include "core/Graphics/Renderer_D3D11.hpp"

namespace core::Graphics::Direct3D11 {
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
	void MeshRenderer::setLegacyBlendState(IRenderer::VertexColorBlendState const vertex_color_blend_state, IRenderer::BlendState const blend_state) {
		m_vertex_color_blend_state = vertex_color_blend_state;
		m_blend_state = blend_state;
	}
	void MeshRenderer::draw(IRenderer* const renderer) {
		assert(renderer);

		renderer->setVertexColorBlendState(m_vertex_color_blend_state);
		renderer->setTexture(m_texture.get());
		renderer->setBlendState(m_blend_state);
		if (!renderer->flush()) {
			return;
		}

		if (!uploadConstantBuffer()) {
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
		mesh->applyNative(ctx, static_cast<Renderer_D3D11*>(renderer)->isFogEnabled());

		// VS stage constant buffer setup by Renderer
		// * constant buffer (view projection matrix)
		// see: Constants::vertex_shader_stage_constant_buffer_slot_view_projection_matrix

		// VS stage constant buffer setup by MeshRenderer
		// * constant buffer (world matrix)

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

		ID3D11ShaderResourceView* const texture[1]{ static_cast<Texture2D*>(m_texture.get())->GetView() };
		ctx->PSSetShaderResources(0, 1, texture);

		ID3D11SamplerState* sampler_state[1]{};
		if (m_texture->getSamplerState()) {
			sampler_state[0] = static_cast<SamplerState*>(m_texture->getSamplerState())->GetState();
		} else {
			auto const ss = renderer->getKnownSamplerState(IRenderer::SamplerState::LinearWrap);
			sampler_state[0] = static_cast<SamplerState*>(ss)->GetState();
		}
		ctx->PSSetSamplers(0, 1, sampler_state);

		static_cast<Renderer_D3D11*>(renderer)->bindTextureAlphaType(m_texture.get());
		//static_cast<Renderer_D3D11*>(renderer)->bindTextureSamplerState(m_texture.get());

		// OM stage setup by Renderer:
		// * depth stencil state
		// * blend state

		// OM stage setup by Renderer/SwapChain/Window:
		// * render target view
		// * depth stencil view

		// Mesh draw

		mesh->drawNative(ctx);

		renderer->endBatch();
		renderer->beginBatch(); // TODO: better batch rendering?
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
		//auto const original = DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4 const*>(&m_transform));
		//auto const transposed = DirectX::XMMatrixTranspose(original);
		//DirectX::XMFLOAT4X4A data;
		//DirectX::XMStoreFloat4x4A(&data, transposed);
		//std::memcpy(ptr, &data, sizeof(data));
		std::memcpy(ptr, &m_transform, sizeof(m_transform));
		if (!m_constant_buffer->unmap()) {
			return false;
		}
		return true;
	}
	bool MeshRenderer::initialize(IDevice* const device) {
		assert(device);
		m_device = device;
		if (!m_device->createConstantBuffer(sizeof(m_transform), m_constant_buffer.put())) {
			return false;
		}
		return true;
	}
}
namespace core::Graphics {
	bool IMeshRenderer::create(IDevice* const device, IMeshRenderer** const output) {
		assert(device);
		assert(output);
		*output = nullptr;
		SmartReference<Direct3D11::MeshRenderer> mesh_renderer;
		mesh_renderer.attach(new Direct3D11::MeshRenderer);
		if (!mesh_renderer->initialize(device)) {
			return false;
		}
		*output = mesh_renderer.detach();
		return true;
	}
}
