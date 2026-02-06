#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/GraphicsFormat.hpp"

// BEGIN: FUCK MICROSOFT

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

// END: FUCK MICROSOFT

namespace core {
    enum class GraphicsVertexInputRate : uint32_t {
        vertex,
        instance,
    };

    struct GraphicsVertexInputBuffer {
        uint32_t slot;
        uint32_t stride;
        GraphicsVertexInputRate input_rate;
        uint32_t instance_step_rate;
    };

    struct GraphicsVertexInputElement {
        const char* semantic_name;
        uint32_t semantic_index;
        GraphicsFormat format;
        uint32_t buffer_slot;
        uint32_t offset;
    };

    struct GraphicsVertexInputState {
        const GraphicsVertexInputBuffer* buffers;
        const GraphicsVertexInputElement* elements;
        uint32_t buffer_count;
        uint32_t element_count;
    };

    enum class GraphicsCullMode : uint32_t {
        none,
        front,
        back,
    };

    enum class GraphicsFrontFace : uint32_t {
        clockwise,
        counter_clockwise,
    };

    struct GraphicsRasterizerState {
        float depth_bias_constant_factor{};
        float depth_bias_clamp{};
        float depth_bias_slope_factor{};
        GraphicsCullMode cull_mode{};
        GraphicsFrontFace front_face{};
    };

    enum class GraphicsCompareMethod : uint8_t {
        never,
        less,
        equal,
        less_or_equal,
        grater,
        not_equal,
        grater_or_equal,
        always,
    };

    enum class GraphicsStencilMethod : uint8_t {
        keep,
        zero,
        replace,
        increment_and_clamp,
        decrement_and_clamp,
        invert,
        increment_and_wrap,
        decrement_and_wrap,
    };

    struct GraphicsStencilState {
        GraphicsStencilMethod fail_method;
        GraphicsStencilMethod depth_fail_method;
        GraphicsStencilMethod pass_method;
        GraphicsCompareMethod compare_method;
    };

    struct GraphicsDepthStencilState {
        bool depth_test_enable;
        bool depth_write_enable;
        GraphicsCompareMethod depth_compare_method;
        bool stencil_test_enable;
        uint8_t stencil_read_mask;
        uint8_t stencil_write_mask;
        GraphicsStencilState front_face;
        GraphicsStencilState back_face;
    };

    enum class GraphicsBlendFactor : uint8_t {
        zero,
        one,

        src_color,
        one_minus_src_color,
        dest_color,
        one_minus_dest_color,

        src_alpha,
        one_minus_src_alpha,
        dest_alpha,
        one_minus_dest_alpha,

        constant,
        one_minus_constant,

        src_alpha_saturate,
    };

    enum class GraphicsBlendMethod : uint8_t {
        add,
        subtract,
        reverse_subtract,
        min,
        max,
    };

    struct GraphicsBlendState {
        bool blend_enable;
        GraphicsBlendFactor src_color_blend_factor;
        GraphicsBlendFactor dest_color_blend_factor;
        GraphicsBlendMethod color_blend_method;
        GraphicsBlendFactor src_alpha_blend_factor;
        GraphicsBlendFactor dest_alpha_blend_factor;
        GraphicsBlendMethod alpha_blend_method;
    };

    struct GraphicsShaderByteCode {
        const void* data;
        size_t size;
    };

    enum class GraphicsPrimitiveType : uint32_t {
        triangle_list,
        triangle_strip,
        line_list,
        line_strip,
        point_list,
    };

    struct GraphicsPipelineState {
        GraphicsVertexInputState vertex_input_state;
        GraphicsPrimitiveType primitive_type;
        GraphicsShaderByteCode vertex_shader;
        GraphicsRasterizerState rasterizer_state;
        GraphicsDepthStencilState depth_stencil_state;
        GraphicsBlendState blend_state;
    };

    CORE_INTERFACE IGraphicsPipeline : IReferenceCounted {
    };
}
