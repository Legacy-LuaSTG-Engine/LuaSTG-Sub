#include "core/Graphics/Renderer_D3D11.hpp"
#include "core/Logger.hpp"

namespace { namespace shader {
#include "d3d11/shader/renderer/vertex_shader_fog.h"
#include "d3d11/shader/renderer/vertex_shader_normal.h"
#include "d3d11/shader/renderer/pixel_shader_add_exp2_premul.h"
#include "d3d11/shader/renderer/pixel_shader_add_exp2_straight.h"
#include "d3d11/shader/renderer/pixel_shader_add_exp_premul.h"
#include "d3d11/shader/renderer/pixel_shader_add_exp_straight.h"
#include "d3d11/shader/renderer/pixel_shader_add_linear_premul.h"
#include "d3d11/shader/renderer/pixel_shader_add_linear_straight.h"
#include "d3d11/shader/renderer/pixel_shader_add_none_premul.h"
#include "d3d11/shader/renderer/pixel_shader_add_none_straight.h"
#include "d3d11/shader/renderer/pixel_shader_mul_exp2_premul.h"
#include "d3d11/shader/renderer/pixel_shader_mul_exp2_straight.h"
#include "d3d11/shader/renderer/pixel_shader_mul_exp_premul.h"
#include "d3d11/shader/renderer/pixel_shader_mul_exp_straight.h"
#include "d3d11/shader/renderer/pixel_shader_mul_linear_premul.h"
#include "d3d11/shader/renderer/pixel_shader_mul_linear_straight.h"
#include "d3d11/shader/renderer/pixel_shader_mul_none_premul.h"
#include "d3d11/shader/renderer/pixel_shader_mul_none_straight.h"
#include "d3d11/shader/renderer/pixel_shader_one_exp2_premul.h"
#include "d3d11/shader/renderer/pixel_shader_one_exp2_straight.h"
#include "d3d11/shader/renderer/pixel_shader_one_exp_premul.h"
#include "d3d11/shader/renderer/pixel_shader_one_exp_straight.h"
#include "d3d11/shader/renderer/pixel_shader_one_linear_premul.h"
#include "d3d11/shader/renderer/pixel_shader_one_linear_straight.h"
#include "d3d11/shader/renderer/pixel_shader_one_none_premul.h"
#include "d3d11/shader/renderer/pixel_shader_one_none_straight.h"
#include "d3d11/shader/renderer/pixel_shader_zero_exp2_premul.h"
#include "d3d11/shader/renderer/pixel_shader_zero_exp2_straight.h"
#include "d3d11/shader/renderer/pixel_shader_zero_exp_premul.h"
#include "d3d11/shader/renderer/pixel_shader_zero_exp_straight.h"
#include "d3d11/shader/renderer/pixel_shader_zero_linear_premul.h"
#include "d3d11/shader/renderer/pixel_shader_zero_linear_straight.h"
#include "d3d11/shader/renderer/pixel_shader_zero_none_premul.h"
#include "d3d11/shader/renderer/pixel_shader_zero_none_straight.h"
}}

namespace {
#define MAKE_SPAN(NAME) const std::span NAME{shader::NAME, sizeof(shader::NAME)}
    MAKE_SPAN(vertex_shader_fog);
    MAKE_SPAN(vertex_shader_normal);
    MAKE_SPAN(pixel_shader_add_exp2_premul);
    MAKE_SPAN(pixel_shader_add_exp2_straight);
    MAKE_SPAN(pixel_shader_add_exp_premul);
    MAKE_SPAN(pixel_shader_add_exp_straight);
    MAKE_SPAN(pixel_shader_add_linear_premul);
    MAKE_SPAN(pixel_shader_add_linear_straight);
    MAKE_SPAN(pixel_shader_add_none_premul);
    MAKE_SPAN(pixel_shader_add_none_straight);
    MAKE_SPAN(pixel_shader_mul_exp2_premul);
    MAKE_SPAN(pixel_shader_mul_exp2_straight);
    MAKE_SPAN(pixel_shader_mul_exp_premul);
    MAKE_SPAN(pixel_shader_mul_exp_straight);
    MAKE_SPAN(pixel_shader_mul_linear_premul);
    MAKE_SPAN(pixel_shader_mul_linear_straight);
    MAKE_SPAN(pixel_shader_mul_none_premul);
    MAKE_SPAN(pixel_shader_mul_none_straight);
    MAKE_SPAN(pixel_shader_one_exp2_premul);
    MAKE_SPAN(pixel_shader_one_exp2_straight);
    MAKE_SPAN(pixel_shader_one_exp_premul);
    MAKE_SPAN(pixel_shader_one_exp_straight);
    MAKE_SPAN(pixel_shader_one_linear_premul);
    MAKE_SPAN(pixel_shader_one_linear_straight);
    MAKE_SPAN(pixel_shader_one_none_premul);
    MAKE_SPAN(pixel_shader_one_none_straight);
    MAKE_SPAN(pixel_shader_zero_exp2_premul);
    MAKE_SPAN(pixel_shader_zero_exp2_straight);
    MAKE_SPAN(pixel_shader_zero_exp_premul);
    MAKE_SPAN(pixel_shader_zero_exp_straight);
    MAKE_SPAN(pixel_shader_zero_linear_premul);
    MAKE_SPAN(pixel_shader_zero_linear_straight);
    MAKE_SPAN(pixel_shader_zero_none_premul);
    MAKE_SPAN(pixel_shader_zero_none_straight);
#undef MAKE_SPAN
}

namespace {
    void applyVertexShader(core::GraphicsPipelineState& info, const core::Graphics::IRenderer::FogState state) {
        auto& s = info.vertex_shader;

        using FogState = core::Graphics::IRenderer::FogState;

        switch (state) {
        case FogState::Disable:
            s.data = vertex_shader_normal.data();
            s.size = vertex_shader_normal.size();
            break;
        case FogState::Linear:
        case FogState::Exp:
        case FogState::Exp2:
            s.data = vertex_shader_fog.data();
            s.size = vertex_shader_fog.size();
            break;
        }
    }
    void applyPixelShader(
        core::GraphicsPipelineState& info,
        const core::Graphics::IRenderer::VertexColorBlendState vertex_color_blend_state,
        const core::Graphics::IRenderer::FogState fog_state,
        const core::Graphics::IRenderer::TextureAlphaType texture_alpha_mode
    ) {
        auto& s = info.pixel_shader;

        using VertexColorBlendState = core::Graphics::IRenderer::VertexColorBlendState;
        using FogState = core::Graphics::IRenderer::FogState;
        using TextureAlphaType = core::Graphics::IRenderer::TextureAlphaType;

    #define LOAD(A, B, C, NAME) if (A == vertex_color_blend_state && B == fog_state && C == texture_alpha_mode) { s.data = pixel_shader_##NAME.data(); s.size = pixel_shader_##NAME.size(); }

        LOAD(VertexColorBlendState::Zero, FogState::Disable, TextureAlphaType::Normal, zero_none_straight);
        LOAD(VertexColorBlendState::One, FogState::Disable, TextureAlphaType::Normal, one_none_straight);
        LOAD(VertexColorBlendState::Add, FogState::Disable, TextureAlphaType::Normal, add_none_straight);
        LOAD(VertexColorBlendState::Mul, FogState::Disable, TextureAlphaType::Normal, mul_none_straight);

        LOAD(VertexColorBlendState::Zero, FogState::Linear, TextureAlphaType::Normal, zero_linear_straight);
        LOAD(VertexColorBlendState::One, FogState::Linear, TextureAlphaType::Normal, one_linear_straight);
        LOAD(VertexColorBlendState::Add, FogState::Linear, TextureAlphaType::Normal, add_linear_straight);
        LOAD(VertexColorBlendState::Mul, FogState::Linear, TextureAlphaType::Normal, mul_linear_straight);

        LOAD(VertexColorBlendState::Zero, FogState::Exp, TextureAlphaType::Normal, zero_exp_straight);
        LOAD(VertexColorBlendState::One, FogState::Exp, TextureAlphaType::Normal, one_exp_straight);
        LOAD(VertexColorBlendState::Add, FogState::Exp, TextureAlphaType::Normal, add_exp_straight);
        LOAD(VertexColorBlendState::Mul, FogState::Exp, TextureAlphaType::Normal, mul_exp_straight);

        LOAD(VertexColorBlendState::Zero, FogState::Exp2, TextureAlphaType::Normal, zero_exp2_straight);
        LOAD(VertexColorBlendState::One, FogState::Exp2, TextureAlphaType::Normal, one_exp2_straight);
        LOAD(VertexColorBlendState::Add, FogState::Exp2, TextureAlphaType::Normal, add_exp2_straight);
        LOAD(VertexColorBlendState::Mul, FogState::Exp2, TextureAlphaType::Normal, mul_exp2_straight);

        LOAD(VertexColorBlendState::Zero, FogState::Disable, TextureAlphaType::PremulAlpha, zero_none_premul);
        LOAD(VertexColorBlendState::One, FogState::Disable, TextureAlphaType::PremulAlpha, one_none_premul);
        LOAD(VertexColorBlendState::Add, FogState::Disable, TextureAlphaType::PremulAlpha, add_none_premul);
        LOAD(VertexColorBlendState::Mul, FogState::Disable, TextureAlphaType::PremulAlpha, mul_none_premul);

        LOAD(VertexColorBlendState::Zero, FogState::Linear, TextureAlphaType::PremulAlpha, zero_linear_premul);
        LOAD(VertexColorBlendState::One, FogState::Linear, TextureAlphaType::PremulAlpha, one_linear_premul);
        LOAD(VertexColorBlendState::Add, FogState::Linear, TextureAlphaType::PremulAlpha, add_linear_premul);
        LOAD(VertexColorBlendState::Mul, FogState::Linear, TextureAlphaType::PremulAlpha, mul_linear_premul);

        LOAD(VertexColorBlendState::Zero, FogState::Exp, TextureAlphaType::PremulAlpha, zero_exp_premul);
        LOAD(VertexColorBlendState::One, FogState::Exp, TextureAlphaType::PremulAlpha, one_exp_premul);
        LOAD(VertexColorBlendState::Add, FogState::Exp, TextureAlphaType::PremulAlpha, add_exp_premul);
        LOAD(VertexColorBlendState::Mul, FogState::Exp, TextureAlphaType::PremulAlpha, mul_exp_premul);

        LOAD(VertexColorBlendState::Zero, FogState::Exp2, TextureAlphaType::PremulAlpha, zero_exp2_premul);
        LOAD(VertexColorBlendState::One, FogState::Exp2, TextureAlphaType::PremulAlpha, one_exp2_premul);
        LOAD(VertexColorBlendState::Add, FogState::Exp2, TextureAlphaType::PremulAlpha, add_exp2_premul);
        LOAD(VertexColorBlendState::Mul, FogState::Exp2, TextureAlphaType::PremulAlpha, mul_exp2_premul);

    #undef LOAD
    }
    void applyDepthState(core::GraphicsPipelineState& info, const core::Graphics::IRenderer::DepthState state) {
        auto& s = info.depth_stencil_state;

        s.depth_test_enable = (state == core::Graphics::IRenderer::DepthState::Enable);
        s.depth_write_enable = info.depth_stencil_state.depth_test_enable;
        s.depth_compare_method = core::GraphicsCompareMethod::less_or_equal;

        s.stencil_test_enable = false;
        s.stencil_read_mask = 0xff;
        s.stencil_write_mask = 0xff;
        s.front_face.fail_method = core::GraphicsStencilMethod::keep;
        s.front_face.depth_fail_method = core::GraphicsStencilMethod::keep;
        s.front_face.pass_method = core::GraphicsStencilMethod::keep;
        s.front_face.compare_method = core::GraphicsCompareMethod::always;
        s.back_face.fail_method = core::GraphicsStencilMethod::keep;
        s.back_face.depth_fail_method = core::GraphicsStencilMethod::keep;
        s.back_face.pass_method = core::GraphicsStencilMethod::keep;
        s.back_face.compare_method = core::GraphicsCompareMethod::always;
    }
    void applyBlendState(core::GraphicsPipelineState& info, const core::Graphics::IRenderer::BlendState state) {
        auto& s = info.blend_state;

        using BlendState = core::Graphics::IRenderer::BlendState;

        switch (state) {
        case BlendState::Disable:
            s.blend_enable = false;
            s.src_color_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_color_blend_factor = core::GraphicsBlendFactor::zero;
            s.color_blend_method = core::GraphicsBlendMethod::add;
            s.src_alpha_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_alpha_blend_factor = core::GraphicsBlendFactor::zero;
            s.alpha_blend_method = core::GraphicsBlendMethod::add;
            break;
        case BlendState::Alpha:
            s.blend_enable = true;
            s.src_color_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_color_blend_factor = core::GraphicsBlendFactor::one_minus_src_alpha;
            s.color_blend_method = core::GraphicsBlendMethod::add;
            s.src_alpha_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_alpha_blend_factor = core::GraphicsBlendFactor::one_minus_src_alpha;
            s.alpha_blend_method = core::GraphicsBlendMethod::add;
            break;
        case BlendState::One:
            s.blend_enable = true;
            s.src_color_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_color_blend_factor = core::GraphicsBlendFactor::zero;
            s.color_blend_method = core::GraphicsBlendMethod::add;
            s.src_alpha_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_alpha_blend_factor = core::GraphicsBlendFactor::zero;
            s.alpha_blend_method = core::GraphicsBlendMethod::add;
            break;
        case BlendState::Min:
            s.blend_enable = true;
            s.src_color_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_color_blend_factor = core::GraphicsBlendFactor::one;
            s.color_blend_method = core::GraphicsBlendMethod::min;
            s.src_alpha_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_alpha_blend_factor = core::GraphicsBlendFactor::one;
            s.alpha_blend_method = core::GraphicsBlendMethod::min;
            break;
        case BlendState::Max:
            s.blend_enable = true;
            s.src_color_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_color_blend_factor = core::GraphicsBlendFactor::one;
            s.color_blend_method = core::GraphicsBlendMethod::max;
            s.src_alpha_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_alpha_blend_factor = core::GraphicsBlendFactor::one;
            s.alpha_blend_method = core::GraphicsBlendMethod::max;
            break;
        case BlendState::Mul:
            s.blend_enable = true;
            s.src_color_blend_factor = core::GraphicsBlendFactor::dest_color;
            s.dest_color_blend_factor = core::GraphicsBlendFactor::zero;
            s.color_blend_method = core::GraphicsBlendMethod::add;
            s.src_alpha_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_alpha_blend_factor = core::GraphicsBlendFactor::one_minus_src_alpha;
            s.alpha_blend_method = core::GraphicsBlendMethod::add;
            break;
        case BlendState::Screen:
            s.blend_enable = true;
            s.src_color_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_color_blend_factor = core::GraphicsBlendFactor::one_minus_src_color;
            s.color_blend_method = core::GraphicsBlendMethod::add;
            s.src_alpha_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_alpha_blend_factor = core::GraphicsBlendFactor::one_minus_src_alpha;
            s.alpha_blend_method = core::GraphicsBlendMethod::add;
            break;
        case BlendState::Add:
            s.blend_enable = true;
            s.src_color_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_color_blend_factor = core::GraphicsBlendFactor::one;
            s.color_blend_method = core::GraphicsBlendMethod::add;
            s.src_alpha_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_alpha_blend_factor = core::GraphicsBlendFactor::one_minus_src_alpha;
            s.alpha_blend_method = core::GraphicsBlendMethod::add;
            break;
        case BlendState::Sub:
            s.blend_enable = true;
            s.src_color_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_color_blend_factor = core::GraphicsBlendFactor::one;
            s.color_blend_method = core::GraphicsBlendMethod::subtract;
            s.src_alpha_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_alpha_blend_factor = core::GraphicsBlendFactor::one_minus_src_alpha;
            s.alpha_blend_method = core::GraphicsBlendMethod::add;
            break;
        case BlendState::RevSub:
            s.blend_enable = true;
            s.src_color_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_color_blend_factor = core::GraphicsBlendFactor::one;
            s.color_blend_method = core::GraphicsBlendMethod::reverse_subtract;
            s.src_alpha_blend_factor = core::GraphicsBlendFactor::one;
            s.dest_alpha_blend_factor = core::GraphicsBlendFactor::one_minus_src_alpha;
            s.alpha_blend_method = core::GraphicsBlendMethod::add;
            break;
        case BlendState::Inv:
            s.blend_enable = true;
            s.src_color_blend_factor = core::GraphicsBlendFactor::one_minus_dest_color;
            s.dest_color_blend_factor = core::GraphicsBlendFactor::one_minus_src_color;
            s.color_blend_method = core::GraphicsBlendMethod::add;
            s.src_alpha_blend_factor = core::GraphicsBlendFactor::zero;
            s.dest_alpha_blend_factor = core::GraphicsBlendFactor::one;
            s.alpha_blend_method = core::GraphicsBlendMethod::add;
            break;
        }
    }
}

namespace core::Graphics {
    bool Renderer_D3D11::createGraphicsPipelines() {
        GraphicsPipelineState info{};

        #pragma region vertex input state

        GraphicsVertexInputBuffer vertex_buffer{};
        vertex_buffer.slot = 0;
        vertex_buffer.stride = sizeof(DrawVertex);
        vertex_buffer.input_rate = GraphicsVertexInputRate::vertex;
        vertex_buffer.instance_step_rate = 0;

        info.vertex_input_state.buffers = &vertex_buffer;
        info.vertex_input_state.buffer_count = 1;

        GraphicsVertexInputElement vertex_elements[3]{};

        vertex_elements[0].semantic_name = "POSITION";
        vertex_elements[0].semantic_index = 0;
        vertex_elements[0].format = GraphicsFormat::r32_g32_b32_float;
        vertex_elements[0].buffer_slot = 0;
        vertex_elements[0].offset = offsetof(DrawVertex, x);

        vertex_elements[1].semantic_name = "COLOR";
        vertex_elements[1].semantic_index = 0;
        vertex_elements[1].format = GraphicsFormat::b8_g8_r8_a8_unorm;
        vertex_elements[1].buffer_slot = 0;
        vertex_elements[1].offset = offsetof(DrawVertex, color);

        vertex_elements[2].semantic_name = "TEXCOORD";
        vertex_elements[2].semantic_index = 0;
        vertex_elements[2].format = GraphicsFormat::r32_g32_float;
        vertex_elements[2].buffer_slot = 0;
        vertex_elements[2].offset = offsetof(DrawVertex, u);

        info.vertex_input_state.elements = vertex_elements;
        info.vertex_input_state.element_count = 3;

        #pragma endregion

        info.primitive_type = core::GraphicsPrimitiveType::triangle_list;

        #pragma region rasterizer state

        info.rasterizer_state.cull_mode = GraphicsCullMode::none; // 2D 图片精灵可能有负缩放
        info.rasterizer_state.front_face = GraphicsFrontFace::clockwise;

        #pragma endregion

        constexpr VertexColorBlendState vertex_color_blend_state_list[]{
            VertexColorBlendState::Zero,
            VertexColorBlendState::One,
            VertexColorBlendState::Add,
            VertexColorBlendState::Mul,
        };

        constexpr FogState fog_state_list[]{
            FogState::Disable,
            FogState::Linear,
            FogState::Exp,
            FogState::Exp2,
        };

        constexpr TextureAlphaType texture_alpha_mode_list[]{
            TextureAlphaType::Normal,
            TextureAlphaType::PremulAlpha,
        };

        constexpr DepthState depth_state_list[]{
            DepthState::Disable,
            DepthState::Enable,
        };

        constexpr BlendState blend_state_list[]{
            BlendState::Disable,
            BlendState::Alpha,
            BlendState::One,
            BlendState::Min,
            BlendState::Max,
            BlendState::Mul,
            BlendState::Screen,
            BlendState::Add,
            BlendState::Sub,
            BlendState::RevSub,
            BlendState::Inv,
        };

        for (const auto vertex_color_blend_state : vertex_color_blend_state_list) {
            for (const auto fog_state : fog_state_list) {
                for (const auto texture_alpha_mode : texture_alpha_mode_list) {
                    for (const auto depth_state : depth_state_list) {
                        for (const auto blend_state : blend_state_list) {
                            applyVertexShader(info, fog_state);
                            applyPixelShader(info, vertex_color_blend_state, fog_state, texture_alpha_mode);
                            applyDepthState(info, depth_state);
                            applyBlendState(info, blend_state);
                            auto& graphics_pipeline = _graphics_pipeline
                                [static_cast<size_t>(vertex_color_blend_state)]
                                [static_cast<size_t>(fog_state)]
                                [static_cast<size_t>(texture_alpha_mode)]
                                [static_cast<size_t>(depth_state)]
                                [static_cast<size_t>(blend_state)];
                            if (!m_device->createGraphicsPipeline(&info, graphics_pipeline.put())) {
                                return false;
                            }
                        }
                    }
                }
            }
        }

        return true;
    }
}
