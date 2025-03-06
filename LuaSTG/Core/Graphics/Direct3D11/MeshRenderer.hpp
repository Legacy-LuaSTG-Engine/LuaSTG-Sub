#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/Mesh.hpp"

namespace Core::Graphics::Direct3D11 {
	class MeshRenderer final
		: public Object<IMeshRenderer>
		, public IDeviceEventListener {
	public:
		// IDeviceEventListener

		void onDeviceCreate() override;
		void onDeviceDestroy() override;

		// IMeshRenderer

		void setTransform(Matrix4F const& transform) override;
		void setTexture(ITexture2D* texture) override;
		void setMesh(IMesh* mesh) override;
		void setLegacyBlendState(IRenderer::VertexColorBlendState vertex_color_blend_state, IRenderer::BlendState blend_state) override;
		void draw(IRenderer* renderer) override;

		// MeshRenderer

		MeshRenderer();
		MeshRenderer(MeshRenderer const&) = delete;
		MeshRenderer(MeshRenderer&&) = delete;
		~MeshRenderer();

		MeshRenderer& operator=(MeshRenderer const&) = delete;
		MeshRenderer& operator=(MeshRenderer&&) = delete;

		bool uploadConstantBuffer();
		bool initialize(IDevice* device);

	private:
		ScopeObject<IDevice> m_device;
		ScopeObject<ITexture2D> m_texture;
		ScopeObject<IMesh> m_mesh;
		ScopeObject<IBuffer> m_constant_buffer;
		Matrix4F m_transform{ Matrix4F::identity() };
		IRenderer::VertexColorBlendState m_vertex_color_blend_state{ IRenderer::VertexColorBlendState::Mul };
		IRenderer::BlendState m_blend_state{ IRenderer::BlendState::Alpha };
		bool m_transform_dirty{ true };
	};
}
