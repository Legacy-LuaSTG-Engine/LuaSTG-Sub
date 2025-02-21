// ReSharper disable CppTooWideScopeInitStatement

#include "Core/Graphics/Direct3D11/Mesh.hpp"

namespace {
	void reportReadOnly() {
		spdlog::error("[core] Mesh is read-only");
	}
}

#define REPORT_READ_ONLY_RETURN if (m_read_only) { reportReadOnly(); assert(false); return; }

#define REPORT_READ_ONLY_RETURN_BOOL if (m_read_only) { reportReadOnly(); assert(false); return false; }

namespace Core::Graphics::Direct3D11 {
	// 警告：这里使用了一些黑科技，因此仅适配小端序平台
	// 原因：std::memcpy 复制时，从低字节开始复制，高字节可能会被丢弃

	void Mesh::onDeviceCreate() {
		if (m_initialized) {
			// 不需要调用 createResources，依赖的资源已有自动重建机制
			// TODO: 如何确保回调顺序？我们希望 VertexBuffer 和 IndexBuffer 先重建
			commit();
		}
	}
	void Mesh::onDeviceDestroy() {
		// no need to call unmapAll
	}

	void Mesh::setVertex(uint32_t const vertex_index, Vector2F const& position, Vector2F const& uv, Color4B const color) {
		REPORT_READ_ONLY_RETURN;
		setPosition(vertex_index, position);
		setUv(vertex_index, uv);
		setColor(vertex_index, color);
	}
	void Mesh::setVertex(uint32_t const vertex_index, Vector2F const& position, Vector2F const& uv, Vector4F const& color) {
		REPORT_READ_ONLY_RETURN;
		setPosition(vertex_index, position);
		setUv(vertex_index, uv);
		setColor(vertex_index, color);
	}
	void Mesh::setVertex(uint32_t const vertex_index, Vector3F const& position, Vector2F const& uv, Color4B const color) {
		REPORT_READ_ONLY_RETURN;
		setPosition(vertex_index, position);
		setUv(vertex_index, uv);
		setColor(vertex_index, color);
	}
	void Mesh::setVertex(uint32_t const vertex_index, Vector3F const& position, Vector2F const& uv, Vector4F const& color) {
		REPORT_READ_ONLY_RETURN;
		setPosition(vertex_index, position);
		setUv(vertex_index, uv);
		setColor(vertex_index, color);
	}

	void Mesh::setPosition(uint32_t const vertex_index, Vector2F const& position) {
		REPORT_READ_ONLY_RETURN;
		size_t const offset = static_cast<size_t>(vertex_index) * static_cast<size_t>(m_vertex_metadata.stride) + static_cast<size_t>(m_vertex_metadata.position_offset);
		Vector3F const float3(position.x, position.y, 0.0f);
		std::memcpy(
			m_vertex_data.data() + offset,
			&float3,
			m_vertex_metadata.position_size_in_bytes);
	}
	void Mesh::setPosition(uint32_t const vertex_index, Vector3F const& position) {
		REPORT_READ_ONLY_RETURN;
		size_t const offset = static_cast<size_t>(vertex_index) * static_cast<size_t>(m_vertex_metadata.stride) + static_cast<size_t>(m_vertex_metadata.position_offset);
		std::memcpy(
			m_vertex_data.data() + offset,
			&position,
			m_vertex_metadata.position_size_in_bytes);
	}

	void Mesh::setUv(uint32_t const vertex_index, Vector2F const& uv) {
		REPORT_READ_ONLY_RETURN;
		size_t const offset = static_cast<size_t>(vertex_index) * static_cast<size_t>(m_vertex_metadata.stride) + static_cast<size_t>(m_vertex_metadata.uv_offset);
		std::memcpy(
			m_vertex_data.data() + offset,
			&uv,
			m_vertex_metadata.uv_size_in_bytes);
	}

	void Mesh::setColor(uint32_t const vertex_index, Color4B const color) {
		REPORT_READ_ONLY_RETURN;
		size_t const offset = static_cast<size_t>(vertex_index) * static_cast<size_t>(m_vertex_metadata.stride) + static_cast<size_t>(m_vertex_metadata.color_offset);
		Vector4F float4;
		void const* const source = m_options.vertex_color_compression ? &color : &float4;
		if (!m_options.vertex_color_compression) {
			float4.x = static_cast<float>(color.r) / 255.0f;
			float4.y = static_cast<float>(color.g) / 255.0f;
			float4.z = static_cast<float>(color.b) / 255.0f;
			float4.w = static_cast<float>(color.a) / 255.0f;
		}
		std::memcpy(
			m_vertex_data.data() + offset,
			source,
			m_vertex_metadata.color_size_in_bytes);
	}
	void Mesh::setColor(uint32_t const vertex_index, Vector4F const& color) {
		REPORT_READ_ONLY_RETURN;
		size_t const offset = static_cast<size_t>(vertex_index) * static_cast<size_t>(m_vertex_metadata.stride) + static_cast<size_t>(m_vertex_metadata.color_offset);
		Color4B rgba32;
		void const* const source = m_options.vertex_color_compression ? &rgba32 : &color;
		if (m_options.vertex_color_compression) {
			rgba32.r = static_cast<uint8_t>(std::clamp(color.x * 255.0f, 0.0f, 255.0f));
			rgba32.g = static_cast<uint8_t>(std::clamp(color.y * 255.0f, 0.0f, 255.0f));
			rgba32.b = static_cast<uint8_t>(std::clamp(color.z * 255.0f, 0.0f, 255.0f));
			rgba32.a = static_cast<uint8_t>(std::clamp(color.w * 255.0f, 0.0f, 255.0f));
		}
		std::memcpy(
			m_vertex_data.data() + offset,
			source,
			m_vertex_metadata.color_size_in_bytes);
	}

	void Mesh::setIndex(uint32_t const index_index, uint32_t const vertex_index) {
		REPORT_READ_ONLY_RETURN;
		size_t const offset = static_cast<size_t>(index_index) * static_cast<size_t>(m_index_metadata.stride);
		std::memcpy(
			m_index_data.data() + offset,
			&vertex_index,
			m_index_metadata.size_in_bytes);
	}

	bool Mesh::commit() {
		REPORT_READ_ONLY_RETURN_BOOL;
		if (m_options.vertex_count > 0) {
			void* pointer{};
			if (!m_vertex_buffer->map(static_cast<uint32_t>(m_vertex_data.size()), true, &pointer)) {
				return false;
			}
			std::memcpy(pointer, m_vertex_data.data(), m_vertex_data.size());
			if (!m_vertex_buffer->unmap()) {
				return false;
			}
		}
		if (m_options.index_count > 0) {
			void* pointer{};
			if (!m_index_buffer->map(static_cast<uint32_t>(m_index_data.size()), true, &pointer)) {
				return false;
			}
			std::memcpy(pointer, m_index_data.data(), m_index_data.size());
			if (!m_index_buffer->unmap()) {
				return false;
			}
		}
		return true;
	}
	void Mesh::setReadOnly() {
		m_read_only = true;
	}

	bool Mesh::initialize(IDevice* const device, MeshOptions const& options) {
		assert(device);
		m_device = device;
		m_options = options;

		m_vertex_metadata.position_offset = 0;
		m_vertex_metadata.position_size_in_bytes = m_options.vertex_position_no_z ? sizeof(Vector2F) : sizeof(Vector3F);
		m_vertex_metadata.uv_offset = m_vertex_metadata.position_offset + m_vertex_metadata.position_size_in_bytes;
		m_vertex_metadata.uv_size_in_bytes = sizeof(Vector2F);
		m_vertex_metadata.color_offset = m_vertex_metadata.uv_offset + m_vertex_metadata.uv_size_in_bytes;
		m_vertex_metadata.color_size_in_bytes = m_options.vertex_color_compression ? sizeof(Color4B) : sizeof(Vector4F);
		m_vertex_metadata.stride = m_vertex_metadata.color_offset + m_vertex_metadata.color_size_in_bytes;

		m_index_metadata.size_in_bytes = m_options.vertex_index_compression ? sizeof(uint16_t) : sizeof(uint32_t);
		m_index_metadata.stride = m_options.vertex_index_compression ? sizeof(uint16_t) : sizeof(uint32_t);

	#ifndef NDEBUG
		spdlog::info(
			"[core] Mesh metadata:\n"
			"    vertex metadata:\n"
			"        position offset: {} (bytes)\n"
			"        position size  : {} (bytes)\n"
			"        uv       offset: {} (bytes)\n"
			"        uv       size  : {} (bytes)\n"
			"        color    offset: {} (bytes)\n"
			"        color    size  : {} (bytes)\n"
			"        stride: {} (bytes)\n"
			"    index metadata:\n"
			"        size  : {} (bytes)\n"
			"        stride: {} (bytes)\n"
			, m_vertex_metadata.position_offset
			, m_vertex_metadata.position_size_in_bytes
			, m_vertex_metadata.uv_offset
			, m_vertex_metadata.uv_size_in_bytes
			, m_vertex_metadata.color_offset
			, m_vertex_metadata.color_size_in_bytes
			, m_vertex_metadata.stride
			, m_index_metadata.size_in_bytes
			, m_index_metadata.stride
		);
	#endif

		m_vertex_data.resize(m_options.vertex_count * m_vertex_metadata.stride);
		m_index_data.resize(m_options.index_count * m_index_metadata.stride);

		if (!createResources()) {
			return false;
		}

		m_initialized = true;
		return true;
	}
	bool Mesh::createResources() {
		if (m_options.vertex_count > 0) {
			if (!m_device->createVertexBuffer(static_cast<uint32_t>(m_vertex_data.size()), ~m_vertex_buffer)) {
				return false;
			}
		}
		if (m_options.index_count > 0) {
			if (!m_device->createIndexBuffer(static_cast<uint32_t>(m_index_data.size()), ~m_index_buffer)) {
				return false;
			}
		}
		return mapAll();
	}
}
