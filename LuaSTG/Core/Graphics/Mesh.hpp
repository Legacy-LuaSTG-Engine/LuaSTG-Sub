#pragma once
#include "Core/Graphics/Device.hpp"
#include "Core/Graphics/Renderer.hpp"

namespace Core::Graphics {
	enum class PrimitiveTopology : uint8_t {
		triangle_list = 4,
		triangle_strip = 5,
	};

	struct MeshOptions {
		uint32_t vertex_count{};
		uint32_t index_count{};

		// false: position (x, y, z)
		// true : position (x, y)
		bool vertex_position_no_z{ false };

		// false: uint32
		// true : uint16
		bool vertex_index_compression{ true };

		// false: float4 R32G32B32A32 FLOAT
		// true : uint32 B8G8R8A8 U NORM
		bool vertex_color_compression{ true };

		PrimitiveTopology primitive_topology{ PrimitiveTopology::triangle_list };
	};

	struct IMesh : IObject {
		[[nodiscard]] virtual uint32_t getVertexCount() const noexcept = 0;
		[[nodiscard]] virtual uint32_t getIndexCount() const noexcept = 0;
		[[nodiscard]] virtual PrimitiveTopology getPrimitiveTopology() const noexcept = 0;
		[[nodiscard]] virtual bool isReadOnly() const noexcept = 0;

		virtual void setValidationEnable(bool enable) = 0;

		virtual void setVertex(uint32_t vertex_index, Vector2F const& position, Vector2F const& uv, Color4B color) = 0;
		virtual void setVertex(uint32_t vertex_index, Vector2F const& position, Vector2F const& uv, Vector4F const& color) = 0;
		virtual void setVertex(uint32_t vertex_index, Vector3F const& position, Vector2F const& uv, Color4B color) = 0;
		virtual void setVertex(uint32_t vertex_index, Vector3F const& position, Vector2F const& uv, Vector4F const& color) = 0;

		virtual void setPosition(uint32_t vertex_index, Vector2F const& position) = 0;
		virtual void setPosition(uint32_t vertex_index, Vector3F const& position) = 0;

		virtual void setUv(uint32_t vertex_index, Vector2F const& uv) = 0;

		virtual void setColor(uint32_t vertex_index, Color4B color) = 0;
		virtual void setColor(uint32_t vertex_index, Vector4F const& color) = 0;

		virtual void setIndex(uint32_t index_index, uint32_t vertex_index) = 0;

		virtual bool commit() = 0;
		virtual void setReadOnly() = 0;

		static bool create(IDevice* device, MeshOptions const& options, IMesh** output);
	};

	struct IMeshRenderer : IObject {
		virtual void setTransform(Matrix4F const& transform) = 0;
		virtual void setTexture(ITexture2D* texture) = 0;
		virtual void setMesh(IMesh* mesh) = 0;
		virtual void setLegacyBlendState(IRenderer::VertexColorBlendState vertex_color_blend_state, IRenderer::BlendState blend_state) = 0;
		virtual void draw(IRenderer* renderer) = 0;

		static bool create(IDevice* device, IMeshRenderer** output);
	};
}
