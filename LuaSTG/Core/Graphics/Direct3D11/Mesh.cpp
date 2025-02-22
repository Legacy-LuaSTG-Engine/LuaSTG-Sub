// ReSharper disable CppTooWideScopeInitStatement

#include "Core/Graphics/Direct3D11/Mesh.hpp"
#include "Core/Graphics/Direct3D11/Buffer.hpp"
#include "Core/Graphics/Direct3D11/Device.hpp"
#include "Platform/RuntimeLoader/Direct3DCompiler.hpp"
#include "Core/i18n.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	Platform::RuntimeLoader::Direct3DCompiler d3d_compiler;
	void reportReadOnly() {
		spdlog::error("[core] Mesh: read-only");
	}
	void reportIndexOutOfBounds() {
		spdlog::error("[core] Mesh: index out of bounds");
	}
	std::string generateVertexShader(Core::Graphics::MeshOptions const& options, bool const fog) {
		// ReSharper disable StringLiteralTypo
		std::string generated_vertex_shader;
		generated_vertex_shader.reserve(1024);

		generated_vertex_shader.append("cbuffer view_proj_buffer : register(b0) {\n"sv);
		generated_vertex_shader.append("    float4x4 view_proj;\n"sv);
		generated_vertex_shader.append("};\n"sv);

		generated_vertex_shader.append("struct VS_Input {\n"sv);
		if (options.vertex_position_no_z) {
			generated_vertex_shader.append("    float2 position: POSITION0;\n"sv);
		}
		else {
			generated_vertex_shader.append("    float3 position: POSITION0;\n"sv);
		}
		generated_vertex_shader.append("    float2 uv: TEXCOORD0;\n"sv);
		generated_vertex_shader.append("    float4 color: COLOR0;\n"sv);
		generated_vertex_shader.append("};\n"sv);

		generated_vertex_shader.append("struct VS_Output {\n"sv);
		generated_vertex_shader.append("    float4 xy: SV_POSITION;\n"sv);
		if (fog) {
			generated_vertex_shader.append("    float4 position: POSITION0;\n"sv);
		}
		generated_vertex_shader.append("    float2 uv: TEXCOORD0;\n"sv);
		generated_vertex_shader.append("    float4 color: COLOR0;\n"sv);
		generated_vertex_shader.append("};\n"sv);

		generated_vertex_shader.append("VS_Output main(VS_Input input) {\n"sv);

		if (options.vertex_position_no_z) {
			generated_vertex_shader.append("    float4 position_world = float4(input.position, 0.0f, 1.0f);\n"sv);
		}
		else {
			generated_vertex_shader.append("    float4 position_world = float4(input.position, 1.0f);\n"sv);
		}

		generated_vertex_shader.append("    VS_Output output;\n"sv);
		generated_vertex_shader.append("    output.xy = mul(view_proj, position_world);\n"sv);
		if (fog) {
			generated_vertex_shader.append("    output.position = position_world;\n"sv);
		}
		generated_vertex_shader.append("    output.uv = input.uv;\n"sv);
		generated_vertex_shader.append("    output.color = input.color;\n"sv);

		generated_vertex_shader.append("    return output;\n"sv);
		generated_vertex_shader.append("}\n"sv);
		// ReSharper restore StringLiteralTypo
		return generated_vertex_shader;
	}
	bool compileVertexShader(std::string const& source, ID3DBlob** const output) {
		*output = nullptr;
		HRESULT hr{};
		UINT flags{ D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS };
	#ifdef NDEBUG
		flags |= D3DCOMPILE_AVOID_FLOW_CONTROL;
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
	#else
		flags |= D3DCOMPILE_DEBUG;
		flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL0;
	#endif
		wil::com_ptr<ID3DBlob> vertex_shader_blob;
		wil::com_ptr<ID3DBlob> error_message_blob;
		hr = gHR = d3d_compiler.Compile(
			source.data(), source.size(), "Mesh generated vertex shader",
			nullptr, nullptr, "main", "vs_4_0", flags, 0,
			vertex_shader_blob.put(), error_message_blob.put());
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("D3DCompile");
			if (error_message_blob) {
				std::string_view const error_message(static_cast<std::string_view::const_pointer>(error_message_blob->GetBufferPointer()));
				spdlog::error("compiler error message:\n{}", error_message);
			}
			spdlog::error("generated vertex shader:\n{}", source);
			return false;
		}
		*output = vertex_shader_blob.detach();
		return true;
	}
}

#define REPORT_VERTEX_INDEX_OOB_RETURN(v) if (m_validation && ((v) >= m_options.vertex_count)) { reportIndexOutOfBounds(); assert(false); return; }

#define REPORT_INDEX_INDEX_OOB_RETURN(v) if (m_validation && ((v) >= m_options.index_count)) { reportIndexOutOfBounds(); assert(false); return; }

#define REPORT_READ_ONLY_RETURN if (m_validation && m_read_only) { reportReadOnly(); assert(false); return; }

#define REPORT_READ_ONLY_RETURN_BOOL if (m_validation && m_read_only) { reportReadOnly(); assert(false); return false; }

namespace Core::Graphics::Direct3D11 {
	void Mesh::onDeviceCreate() {
		if (m_initialized) {
			createResources();
			// TODO: 如何确保回调顺序？我们希望 VertexBuffer 和 IndexBuffer 先重建
			commit();
		}
	}
	void Mesh::onDeviceDestroy() {
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

	Mesh::Mesh() = default;
	Mesh::~Mesh() {
		if (m_initialized && m_device) {
			m_device->removeEventListener(this);
		}
	}

	void Mesh::applyNative(ID3D11DeviceContext* const ctx, bool const fog) {
		assert(ctx);

		// Stage: IA

		ID3D11Buffer* const vbs[]{ static_cast<Buffer*>(m_vertex_buffer.get())->getNativeBuffer() };
		constexpr UINT offset{};
		ctx->IASetVertexBuffers(0, 1, vbs, &m_vertex_metadata.stride, &offset);
		if (m_index_buffer) {
			ID3D11Buffer* const ib{ static_cast<Buffer*>(m_index_buffer.get())->getNativeBuffer() };
			ctx->IASetIndexBuffer(ib, m_options.vertex_index_compression ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
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
			ctx->DrawIndexed(m_options.index_count, 0, 0);
		}
		else {
			ctx->Draw(m_options.vertex_count, 0);
		}
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
		if (!createResources()) {
			return false;
		}

		m_device->addEventListener(this);
		m_initialized = true;
		return true;
	}
	bool Mesh::createResources() {
		HRESULT hr{};
		auto const device = static_cast<Device*>(m_device.get())->GetD3D11Device();
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

		auto const vertex_shader_source = generateVertexShader(m_options, false);
		if (!compileVertexShader(vertex_shader_source, m_vertex_shader_byte_code.put())) {
			return false;
		}
		auto const vertex_shader_source_fog = generateVertexShader(m_options, true);
		if (!compileVertexShader(vertex_shader_source_fog, m_vertex_shader_byte_code_fog.put())) {
			return false;
		}

		hr = gHR = device->CreateVertexShader(m_vertex_shader_byte_code->GetBufferPointer(), m_vertex_shader_byte_code->GetBufferSize(), nullptr, m_vertex_shader.put());
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("ID3D11Device::CreateVertexShader");
			return false;
		}
		hr = gHR = device->CreateVertexShader(m_vertex_shader_byte_code_fog->GetBufferPointer(), m_vertex_shader_byte_code_fog->GetBufferSize(), nullptr, m_vertex_shader_fog.put());
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("ID3D11Device::CreateVertexShader");
			return false;
		}

		// vertex shader input elements are compatible
		hr = gHR = device->CreateInputLayout(elements, 3, m_vertex_shader_byte_code->GetBufferPointer(), m_vertex_shader_byte_code->GetBufferSize(), m_input_layout.put());
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("ID3D11Device::CreateInputLayout");
			return false;
		}

		return true;
	}
}
namespace Core::Graphics {
	bool IMesh::create(IDevice* device, MeshOptions const& options, IMesh** output) {
		*output = nullptr;
		ScopeObject<Direct3D11::Mesh> buffer;
		buffer.attach(new Direct3D11::Mesh);
		if (!buffer->initialize(device, options)) {
			return false;
		}
		*output = buffer.detach();
		return true;
	}
}
