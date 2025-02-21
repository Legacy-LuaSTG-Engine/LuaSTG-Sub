#pragma once
#include "Core/Graphics/Device.hpp"

namespace Core::Graphics {
	enum class PrimitiveTopology : uint8_t {
		triangle_list = 4,
		triangle_strip = 5,
	};

	struct MeshOptions {
		uint32_t vertex_count{};
		uint32_t index_count{};

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

		virtual IMesh* setVertex(uint32_t vertex_index, Vector2F const& position, Vector2F const& uv, Color4B color);
		virtual IMesh* setVertex(uint32_t vertex_index, Vector2F const& position, Vector2F const& uv, Vector4F const& color);
		virtual IMesh* setVertex(uint32_t vertex_index, Vector3F const& position, Vector2F const& uv, Color4B color);
		virtual IMesh* setVertex(uint32_t vertex_index, Vector3F const& position, Vector2F const& uv, Vector4F const& color);

		virtual IMesh* setPosition(uint32_t vertex_index, Vector2F const& position);
		virtual IMesh* setPosition(uint32_t vertex_index, Vector3F const& position);

		virtual IMesh* setUv(uint32_t vertex_index, Vector2F const& uv);

		virtual IMesh* setColor(uint32_t vertex_index, Color4B color);
		virtual IMesh* setColor(uint32_t vertex_index, Vector4F const& color);

		virtual IMesh* setIndex(uint32_t index_index, uint32_t vertex_index);

		virtual bool commit();
		virtual void setReadOnly();

		virtual bool create(MeshOptions const& options, IMesh** output);
	};
}
