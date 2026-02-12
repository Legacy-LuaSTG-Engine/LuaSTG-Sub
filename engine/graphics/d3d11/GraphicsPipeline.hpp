#pragma once
#include "core/GraphicsPipeline.hpp"
#include "core/GraphicsDevice.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "xxhash.h"
#include "d3d11/pch.h"

namespace core {
    struct GraphicsPipelineStateHelper : GraphicsPipelineState {
        std::vector<GraphicsVertexInputBuffer> buffers_data;
        std::vector<std::string> semantic_names_data;
        std::vector<GraphicsVertexInputElement> elements_data;
        std::vector<uint8_t> vertex_shader_data;
        std::vector<uint8_t> pixel_shader_data;

        void save(const GraphicsPipelineState& state);
    };

    class GraphicsPipeline final : public implement::ReferenceCounted<IGraphicsPipeline>, public IGraphicsDeviceEventListener {
    public:
        // IGraphicsPipeline

        const GraphicsPipelineState* getInfo() const noexcept override;

        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        // GraphicsPipeline

        GraphicsPipeline();
        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline(GraphicsPipeline&&) = delete;
        ~GraphicsPipeline();

        GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(GraphicsPipeline&&) = delete;

        bool createResources(IGraphicsDevice* device, const GraphicsPipelineState& create_info);
        void apply();

    private:
        bool createResources();

        SmartReference<IGraphicsDevice> m_device;
        GraphicsPipelineStateHelper m_graphics_pipeline_state_helper;
        win32::com_ptr<ID3D11InputLayout> m_input_layout;
        D3D11_PRIMITIVE_TOPOLOGY m_primitive_topology;
        win32::com_ptr<ID3D11VertexShader> m_vertex_shader;
        win32::com_ptr<ID3D11RasterizerState> m_rasterizer_state;
        win32::com_ptr<ID3D11PixelShader> m_pixel_shader;
        win32::com_ptr<ID3D11DepthStencilState> m_depth_stencil_state;
        win32::com_ptr<ID3D11BlendState> m_blend_state;
        bool m_initialized{};
    };
}
