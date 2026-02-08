#pragma once
#include "core/GraphicsPipeline.hpp"
#include "core/GraphicsDevice.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "xxhash.h"
#include "d3d11/pch.h"

namespace core {
    struct InputLayoutCache {
        std::vector<std::string> semantic_names;
        std::vector<D3D11_INPUT_ELEMENT_DESC> input_elements;
        std::vector<uint8_t> shader_byte_code;
        XXH64_hash_t hash_value{};

        InputLayoutCache();
        InputLayoutCache(const InputLayoutCache& other);
        InputLayoutCache(InputLayoutCache&& other);
        ~InputLayoutCache();

        InputLayoutCache& operator=(const InputLayoutCache& other);
        InputLayoutCache& operator=(InputLayoutCache&& other);

        void initialize(
            const D3D11_INPUT_ELEMENT_DESC* p_input_elements, UINT input_element_count,
            const void* p_shader_byte_code, SIZE_T shader_byte_code_length
        );
    };

    struct InputLayoutCacheOp {
        size_t operator()(const InputLayoutCache& s) const noexcept; // std::hash<InputLayoutCache>()
        bool operator()(const InputLayoutCache& a, const InputLayoutCache& b) const noexcept; // std::equal_to<InputLayoutCache>
    };

    class GraphicsPipelineCache {
    private:
        void hash(InputLayoutCache* input_layout_cache);

        XXH3_state_t* m_hash_state{};
        std::pmr::unsynchronized_pool_resource m_memory_resource;
        std::pmr::unordered_set<InputLayoutCache, InputLayoutCacheOp, InputLayoutCacheOp> m_input_layout_cache;

    };

    class GraphicsPipeline final : public implement::ReferenceCounted<IGraphicsPipeline>, public IGraphicsDeviceEventListener {
    public:
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
        bool createResources(const GraphicsPipelineState& create_info);

        SmartReference<IGraphicsDevice> m_device;
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
