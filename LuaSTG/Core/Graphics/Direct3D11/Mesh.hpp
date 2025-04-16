#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/Mesh.hpp"

namespace core::Graphics::Direct3D11 {
	class Mesh final
		: public Object<IMesh>
		, public IDeviceEventListener
	{
	public:
		// IDeviceEventListener

		void onDeviceCreate() override;
		void onDeviceDestroy() override;

		// IMesh

		[[nodiscard]] uint32_t getVertexCount() const noexcept override { return m_options.vertex_count; }
		[[nodiscard]] uint32_t getIndexCount() const noexcept override { return m_options.index_count; }
		[[nodiscard]] PrimitiveTopology getPrimitiveTopology() const noexcept override { return m_options.primitive_topology; }
		[[nodiscard]] bool isReadOnly() const noexcept override { return m_read_only; }

		void setValidationEnable(bool const enable) override { m_validation = enable; }

		void setVertex(uint32_t vertex_index, Vector2F const& position, Vector2F const& uv, Color4B color) override;
		void setVertex(uint32_t vertex_index, Vector2F const& position, Vector2F const& uv, Vector4F const& color) override;
		void setVertex(uint32_t vertex_index, Vector3F const& position, Vector2F const& uv, Color4B color) override;
		void setVertex(uint32_t vertex_index, Vector3F const& position, Vector2F const& uv, Vector4F const& color) override;

		void setPosition(uint32_t vertex_index, Vector2F const& position) override;
		void setPosition(uint32_t vertex_index, Vector3F const& position) override;

		void setUv(uint32_t vertex_index, Vector2F const& uv) override;

		void setColor(uint32_t vertex_index, Color4B color) override;
		void setColor(uint32_t vertex_index, Vector4F const& color) override;

		void setIndex(uint32_t index_index, uint32_t vertex_index) override;

		bool commit() override;
		void setReadOnly() override;

		// Mesh

		Mesh();
		Mesh(Mesh const&) = delete;
		Mesh(Mesh&&) = delete;
		Mesh& operator=(Mesh const&) = delete;
		Mesh& operator=(Mesh&&) = delete;
		~Mesh();

		[[nodiscard]] ID3D11VertexShader* getNativeVertexShader(bool const fog = false) const noexcept { return fog ? m_vertex_shader_fog.get() : m_vertex_shader.get(); }
		[[nodiscard]] ID3D11InputLayout* getNativeInputLayout() const noexcept { return m_input_layout.get(); }
		void applyNative(ID3D11DeviceContext* ctx, bool fog = false);
		void drawNative(ID3D11DeviceContext* ctx) const;

		bool initialize(IDevice* device, MeshOptions const& options);
		bool createResources();

	private:
		ScopeObject<IDevice> m_device;
		ScopeObject<IBuffer> m_vertex_buffer;
		ScopeObject<IBuffer> m_index_buffer;
		wil::com_ptr_nothrow<ID3DBlob> m_vertex_shader_byte_code;
		wil::com_ptr_nothrow<ID3DBlob> m_vertex_shader_byte_code_fog;
		wil::com_ptr_nothrow<ID3D11VertexShader> m_vertex_shader;
		wil::com_ptr_nothrow<ID3D11VertexShader> m_vertex_shader_fog;
		wil::com_ptr_nothrow<ID3D11InputLayout> m_input_layout;
		std::vector<uint8_t> m_vertex_data;
		std::vector<uint8_t> m_index_data;
		struct VertexMetadata {
			uint32_t position_offset{};
			uint32_t position_size_in_bytes{};
			uint32_t uv_offset{};
			uint32_t uv_size_in_bytes{};
			uint32_t color_offset{};
			uint32_t color_size_in_bytes{};
			uint32_t stride{};
		} m_vertex_metadata;
		struct IndexMetadata {
			uint32_t size_in_bytes{};
			uint32_t stride{};
		} m_index_metadata;
		MeshOptions m_options;
		bool m_read_only{ false };
		bool m_validation{ true };
		bool m_initialized{ false };
	};
}
