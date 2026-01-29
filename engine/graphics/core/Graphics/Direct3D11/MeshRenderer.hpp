#pragma once
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "core/Graphics/Mesh.hpp"

namespace core::Graphics::Direct3D11 {
	class MeshRenderer final
		: public implement::ReferenceCounted<IMeshRenderer>
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
		SmartReference<IDevice> m_device;
		SmartReference<ITexture2D> m_texture;
		SmartReference<IMesh> m_mesh;
		SmartReference<IBuffer> m_constant_buffer;
		Matrix4F m_transform{ Matrix4F::identity() };
		IRenderer::VertexColorBlendState m_vertex_color_blend_state{ IRenderer::VertexColorBlendState::Mul };
		IRenderer::BlendState m_blend_state{ IRenderer::BlendState::Alpha };
		bool m_transform_dirty{ true };
	};
}
