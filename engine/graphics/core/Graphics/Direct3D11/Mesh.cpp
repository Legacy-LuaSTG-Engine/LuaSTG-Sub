// ReSharper disable CppTooWideScopeInitStatement

#include "core/Graphics/Direct3D11/Mesh.hpp"
#include "core/Logger.hpp"
#include "d3d11/GraphicsDevice.hpp"
#include "d3d11/shader/mesh/vertex_shader_xy_normal.h"
#include "d3d11/shader/mesh/vertex_shader_xy_fog.h"
#include "d3d11/shader/mesh/vertex_shader_xyz_normal.h"
#include "d3d11/shader/mesh/vertex_shader_xyz_fog.h"

namespace {
	using std::string_view_literals::operator ""sv;

	void reportReadOnly() {
		core::Logger::error("[core] [Mesh] read-only mesh");
	}
	void reportIndexOutOfBounds() {
		core::Logger::error("[core] [Mesh] index out of bounds");
	}
}

#define REPORT_VERTEX_INDEX_OOB_RETURN(v) if (m_validation && ((v) >= m_options.vertex_count)) { reportIndexOutOfBounds(); assert(false); return; }

#define REPORT_INDEX_INDEX_OOB_RETURN(v) if (m_validation && ((v) >= m_options.index_count)) { reportIndexOutOfBounds(); assert(false); return; }

#define REPORT_READ_ONLY_RETURN if (m_validation && m_read_only) { reportReadOnly(); assert(false); return; }

#define REPORT_READ_ONLY_RETURN_BOOL if (m_validation && m_read_only) { reportReadOnly(); assert(false); return false; }

namespace core::Graphics::Direct3D11 {
	void Mesh::onGraphicsDeviceCreate() {
		if (m_initialized) {
			createResources();
			// TODO: 如何确保回调顺序？我们希望 VertexBuffer 和 IndexBuffer 先重建
			commit();
		}
	}
	void Mesh::onGraphicsDeviceDestroy() {
		m_vertex_shader.reset();
		m_vertex_shader_fog.reset();
		m_input_layout.reset();
	}

	void Mesh::setVertex(uint32_t const vertex_index, Vector2F const& position, Vector2F const& uv, Color4B const color) {
		REPORT_READ_ONLY_RETURN;
		REPORT_VERTEX_INDEX_OOB_RETURN(vertex_index);
		setPosition(vertex_index, position);
		setUv(vertex_index, uv);
		setColor(vertex_index, color);
	}
	void Mesh::setVertex(uint32_t const vertex_index, Vector2F const& position, Vector2F const& uv, Vector4F const& color) {
		REPORT_READ_ONLY_RETURN;
		REPORT_VERTEX_INDEX_OOB_RETURN(vertex_index);
		setPosition(vertex_index, position);
		setUv(vertex_index, uv);
		setColor(vertex_index, color);
	}
	void Mesh::setVertex(uint32_t const vertex_index, Vector3F const& position, Vector2F const& uv, Color4B const color) {
		REPORT_READ_ONLY_RETURN;
		REPORT_VERTEX_INDEX_OOB_RETURN(vertex_index);
		setPosition(vertex_index, position);
		setUv(vertex_index, uv);
		setColor(vertex_index, color);
	}
	void Mesh::setVertex(uint32_t const vertex_index, Vector3F const& position, Vector2F const& uv, Vector4F const& color) {
		REPORT_READ_ONLY_RETURN;
		REPORT_VERTEX_INDEX_OOB_RETURN(vertex_index);
		setPosition(vertex_index, position);
		setUv(vertex_index, uv);
		setColor(vertex_index, color);
	}

	void Mesh::setPosition(uint32_t const vertex_index, Vector2F const& position) {
		REPORT_READ_ONLY_RETURN;
		REPORT_VERTEX_INDEX_OOB_RETURN(vertex_index);
		size_t const offset = static_cast<size_t>(vertex_index) * static_cast<size_t>(m_vertex_metadata.stride) + static_cast<size_t>(m_vertex_metadata.position_offset);
		Vector3F const float3(position.x, position.y, 0.0f);
		std::memcpy(
			m_vertex_data.data() + offset,
			&float3,
			m_vertex_metadata.position_size_in_bytes);
	}
	void Mesh::setPosition(uint32_t const vertex_index, Vector3F const& position) {
		REPORT_READ_ONLY_RETURN;
		REPORT_VERTEX_INDEX_OOB_RETURN(vertex_index);
		size_t const offset = static_cast<size_t>(vertex_index) * static_cast<size_t>(m_vertex_metadata.stride) + static_cast<size_t>(m_vertex_metadata.position_offset);
		std::memcpy(
			m_vertex_data.data() + offset,
			&position,
			m_vertex_metadata.position_size_in_bytes);
	}

	void Mesh::setUv(uint32_t const vertex_index, Vector2F const& uv) {
		REPORT_READ_ONLY_RETURN;
		REPORT_VERTEX_INDEX_OOB_RETURN(vertex_index);
		size_t const offset = static_cast<size_t>(vertex_index) * static_cast<size_t>(m_vertex_metadata.stride) + static_cast<size_t>(m_vertex_metadata.uv_offset);
		std::memcpy(
			m_vertex_data.data() + offset,
			&uv,
			m_vertex_metadata.uv_size_in_bytes);
	}

	void Mesh::setColor(uint32_t const vertex_index, Color4B const color) {
		REPORT_READ_ONLY_RETURN;
		REPORT_VERTEX_INDEX_OOB_RETURN(vertex_index);
		size_t const offset = static_cast<size_t>(vertex_index) * static_cast<size_t>(m_vertex_metadata.stride) + static_cast<size_t>(m_vertex_metadata.color_offset);
		Vector4F float4;
		void const* const source = m_options.vertex_color_compression ? static_cast<void const*>(&color) : static_cast<void const*>(&float4);
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
		REPORT_VERTEX_INDEX_OOB_RETURN(vertex_index);
		size_t const offset = static_cast<size_t>(vertex_index) * static_cast<size_t>(m_vertex_metadata.stride) + static_cast<size_t>(m_vertex_metadata.color_offset);
		Color4B rgba32;
		void const* const source = m_options.vertex_color_compression ? static_cast<void const*>(&rgba32) : static_cast<void const*>(&color);
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
		REPORT_INDEX_INDEX_OOB_RETURN(index_index);
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
			if (!m_vertex_buffer->map(&pointer, true)) {
				return false;
			}
			std::memcpy(pointer, m_vertex_data.data(), m_vertex_data.size());
			if (!m_vertex_buffer->unmap()) {
				return false;
			}
		}
		if (m_options.index_count > 0) {
			void* pointer{};
			if (!m_index_buffer->map(&pointer, true)) {
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

	Mesh::Mesh() = default;
	Mesh::~Mesh() {
		if (m_initialized && m_device) {
			m_device->removeEventListener(this);
		}
	}

	void Mesh::applyNative(ID3D11DeviceContext* const ctx, bool const fog) {
		assert(ctx);

		// Stage: IA

		m_device->getCommandbuffer()->bindVertexBuffer(0, m_vertex_buffer.get());
		if (m_index_buffer) {
			m_device->getCommandbuffer()->bindIndexBuffer(m_index_buffer.get());
		}
		ctx->IASetPrimitiveTopology(m_options.primitive_topology == PrimitiveTopology::triangle_list
									? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
									: D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		ctx->IASetInputLayout(m_input_layout.get());

		// Stage: VS

		ctx->VSSetShader(getNativeVertexShader(fog), nullptr, 0);
	}
	void Mesh::drawNative(ID3D11DeviceContext* const ctx) const {
		assert(ctx);
		if (m_options.index_count > 0) {
			m_device->getCommandbuffer()->drawIndexed(m_options.index_count, 0, 0);
		}
		else {
			m_device->getCommandbuffer()->draw(m_options.vertex_count, 0);
		}
	}

	bool Mesh::initialize(IGraphicsDevice* const device, MeshOptions const& options) {
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
		Logger::info(
			"[core] [Mesh] metadata:\n"
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

		if (m_options.vertex_count > 0) {
			if (!m_device->createVertexBuffer(static_cast<uint32_t>(m_vertex_data.size()), m_vertex_metadata.stride, m_vertex_buffer.put())) {
				return false;
			}
		}
		if (m_options.index_count > 0) {
			const auto format = m_options.vertex_index_compression ? GraphicsFormat::r16_uint : GraphicsFormat::r32_uint;
			if (!m_device->createIndexBuffer(static_cast<uint32_t>(m_index_data.size()), format, m_index_buffer.put())) {
				return false;
			}
		}
		if (!createResources()) {
			return false;
		}

		m_device->addEventListener(this);
		m_initialized = true;
		return true;
	}
	bool Mesh::createResources() {
		const auto device = static_cast<ID3D11Device*>(m_device->getNativeHandle());
		assert(device);

		D3D11_INPUT_ELEMENT_DESC elements[3]{};

		elements[0].SemanticName = "POSITION";
		elements[0].Format = m_options.vertex_position_no_z ? DXGI_FORMAT_R32G32_FLOAT : DXGI_FORMAT_R32G32B32_FLOAT;
		elements[0].AlignedByteOffset = m_vertex_metadata.position_offset;
		elements[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		// ReSharper disable once StringLiteralTypo
		elements[1].SemanticName = "TEXCOORD";
		elements[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		elements[1].AlignedByteOffset = m_vertex_metadata.uv_offset;
		elements[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		elements[2].SemanticName = "COLOR";
		elements[2].Format = m_options.vertex_color_compression ? DXGI_FORMAT_B8G8R8A8_UNORM : DXGI_FORMAT_R32G32B32A32_FLOAT;
		elements[2].AlignedByteOffset = m_vertex_metadata.color_offset;
		elements[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		// TODO: share these objects
		if (m_options.vertex_position_no_z) {
			if (!win32::check_hresult_as_boolean(
				device->CreateVertexShader(vertex_shader_xy_normal, sizeof(vertex_shader_xy_normal), nullptr, m_vertex_shader.put()),
				"ID3D11Device::CreateVertexShader"sv
			)) {
				return false;
			}
			if (!win32::check_hresult_as_boolean(
				device->CreateVertexShader(vertex_shader_xy_fog, sizeof(vertex_shader_xy_fog), nullptr, m_vertex_shader_fog.put()),
				"ID3D11Device::CreateVertexShader"sv
			)) {
				return false;
			}
			// vertex shader input elements are compatible
			if (!win32::check_hresult_as_boolean(
				device->CreateInputLayout(elements, 3, vertex_shader_xy_normal, sizeof(vertex_shader_xy_normal), m_input_layout.put()),
				"ID3D11Device::CreateInputLayout"sv
			)) {
				return false;
			}
		}
		else {
			if (!win32::check_hresult_as_boolean(
				device->CreateVertexShader(vertex_shader_xyz_normal, sizeof(vertex_shader_xyz_normal), nullptr, m_vertex_shader.put()),
				"ID3D11Device::CreateVertexShader"sv
			)) {
				return false;
			}
			if (!win32::check_hresult_as_boolean(
				device->CreateVertexShader(vertex_shader_xyz_fog, sizeof(vertex_shader_xyz_fog), nullptr, m_vertex_shader_fog.put()),
				"ID3D11Device::CreateVertexShader"sv
			)) {
				return false;
			}
			// vertex shader input elements are compatible
			if (!win32::check_hresult_as_boolean(
				device->CreateInputLayout(elements, 3, vertex_shader_xyz_normal, sizeof(vertex_shader_xyz_normal), m_input_layout.put()),
				"ID3D11Device::CreateInputLayout"sv
			)) {
				return false;
			}
		}

		return true;
	}
}
namespace core::Graphics {
	bool IMesh::create(IGraphicsDevice* device, MeshOptions const& options, IMesh** output) {
		*output = nullptr;
		SmartReference<Direct3D11::Mesh> buffer;
		buffer.attach(new Direct3D11::Mesh);
		if (!buffer->initialize(device, options)) {
			return false;
		}
		*output = buffer.detach();
		return true;
	}
}
