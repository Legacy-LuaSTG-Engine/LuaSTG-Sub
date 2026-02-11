#include "d3d11/GraphicsPipeline.hpp"
#include "d3d11/FormatHelper.hpp"

namespace {
    using std::string_view_literals::operator ""sv;

    D3D11_INPUT_CLASSIFICATION toVertexInputRate(const core::GraphicsVertexInputRate value) {
        switch (value) {
        case core::GraphicsVertexInputRate::vertex:   return D3D11_INPUT_PER_VERTEX_DATA;
        case core::GraphicsVertexInputRate::instance: return D3D11_INPUT_PER_INSTANCE_DATA;
        default: assert(false); return D3D11_INPUT_CLASSIFICATION{};
        }
    }
    std::vector<D3D11_INPUT_ELEMENT_DESC> toVertexInputElements(const core::GraphicsVertexInputState& input) {
        std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
        elements.resize(input.element_count);
        for (uint32_t i = 0; i < input.element_count; i += 1) {
            const auto& e = input.elements[i];
            auto& element = elements[i];
            element.SemanticName = e.semantic_name;
            element.SemanticIndex = e.semantic_index;
            element.Format = d3d11::toFormat(e.format);
            element.InputSlot = e.buffer_slot;
            element.AlignedByteOffset = e.offset;
            for (uint32_t j = 0; j < input.buffer_count; j += 1) {
                const auto& b = input.buffers[j];
                if (e.buffer_slot == b.slot) {
                    element.InputSlotClass = toVertexInputRate(b.input_rate);
                    element.InstanceDataStepRate = (element.InputSlotClass == D3D11_INPUT_PER_VERTEX_DATA) ? 0 : b.instance_step_rate;
                    break;
                }
            }
        }
        return elements;
    }

    D3D11_PRIMITIVE_TOPOLOGY toPrimitiveType(const core::GraphicsPrimitiveType value) {
        switch (value) {
        case core::GraphicsPrimitiveType::triangle_list:  return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case core::GraphicsPrimitiveType::triangle_strip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case core::GraphicsPrimitiveType::line_list:      return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        case core::GraphicsPrimitiveType::line_strip:     return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case core::GraphicsPrimitiveType::point_list:     return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        default: assert(false); return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        }
    }
    
    D3D11_CULL_MODE toCullMode(const core::GraphicsCullMode value) {
        switch (value) {
        case core::GraphicsCullMode::none:  return D3D11_CULL_NONE;
        case core::GraphicsCullMode::front: return D3D11_CULL_FRONT;
        case core::GraphicsCullMode::back:  return D3D11_CULL_BACK;
        default: assert(false); return D3D11_CULL_MODE{};
        }
    }
    BOOL toFrontCounterClockwise(const core::GraphicsFrontFace value) {
        return value == core::GraphicsFrontFace::counter_clockwise;
    }
    D3D11_RASTERIZER_DESC toRasterizerState(const core::GraphicsRasterizerState& input) {
        D3D11_RASTERIZER_DESC output{};
        output.FillMode = D3D11_FILL_SOLID; // no wire frame
        output.CullMode = toCullMode(input.cull_mode);
        output.FrontCounterClockwise = toFrontCounterClockwise(input.front_face);
        output.DepthBias = static_cast<INT>(input.depth_bias_constant_factor);
        output.DepthBiasClamp = input.depth_bias_clamp;
        output.SlopeScaledDepthBias = input.depth_bias_slope_factor;
        output.DepthClipEnable = TRUE;
        output.ScissorEnable = TRUE;
        output.MultisampleEnable = FALSE; // no MSAA
        output.AntialiasedLineEnable = FALSE;  // no MSAA
        return output;
    }

    D3D11_DEPTH_WRITE_MASK toDepthWriteMask(const bool depth_write_enable) {
        return depth_write_enable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    }
    D3D11_COMPARISON_FUNC toCompareMethod(const core::GraphicsCompareMethod value) {
        switch (value) {
        case core::GraphicsCompareMethod::never:           return D3D11_COMPARISON_NEVER;
        case core::GraphicsCompareMethod::less:            return D3D11_COMPARISON_LESS;
        case core::GraphicsCompareMethod::equal:           return D3D11_COMPARISON_EQUAL;
        case core::GraphicsCompareMethod::less_or_equal:   return D3D11_COMPARISON_LESS_EQUAL;
        case core::GraphicsCompareMethod::grater:          return D3D11_COMPARISON_GREATER;
        case core::GraphicsCompareMethod::not_equal:       return D3D11_COMPARISON_NOT_EQUAL;
        case core::GraphicsCompareMethod::grater_or_equal: return D3D11_COMPARISON_GREATER_EQUAL;
        case core::GraphicsCompareMethod::always:          return D3D11_COMPARISON_ALWAYS;
        default: assert(false); return D3D11_COMPARISON_FUNC{};
        }
    }
    D3D11_STENCIL_OP toStencilMethod(const core::GraphicsStencilMethod value) {
        switch (value) {
        case core::GraphicsStencilMethod::keep:                return D3D11_STENCIL_OP_KEEP;
        case core::GraphicsStencilMethod::zero:                return D3D11_STENCIL_OP_ZERO;
        case core::GraphicsStencilMethod::replace:             return D3D11_STENCIL_OP_REPLACE;
        case core::GraphicsStencilMethod::increment_and_clamp: return D3D11_STENCIL_OP_INCR_SAT;
        case core::GraphicsStencilMethod::decrement_and_clamp: return D3D11_STENCIL_OP_DECR_SAT;
        case core::GraphicsStencilMethod::invert:              return D3D11_STENCIL_OP_INVERT;
        case core::GraphicsStencilMethod::increment_and_wrap:  return D3D11_STENCIL_OP_INCR;
        case core::GraphicsStencilMethod::decrement_and_wrap:  return D3D11_STENCIL_OP_DECR;
        default: assert(false); return D3D11_STENCIL_OP{};
        }
    }
    D3D11_DEPTH_STENCILOP_DESC toStencilState(const core::GraphicsStencilState& input) {
        D3D11_DEPTH_STENCILOP_DESC output{};
        output.StencilFailOp = toStencilMethod(input.fail_method);
        output.StencilDepthFailOp = toStencilMethod(input.depth_fail_method);
        output.StencilPassOp = toStencilMethod(input.pass_method);
        output.StencilFunc = toCompareMethod(input.compare_method);
        return output;
    }
    D3D11_DEPTH_STENCIL_DESC toDepthStencilState(const core::GraphicsDepthStencilState& input) {
        D3D11_DEPTH_STENCIL_DESC output{};
        output.DepthEnable = input.depth_test_enable ? TRUE : FALSE;
        output.DepthWriteMask = toDepthWriteMask(input.depth_write_enable);
        output.DepthFunc = toCompareMethod(input.depth_compare_method);
        output.StencilEnable = input.stencil_test_enable ? TRUE : FALSE;
        output.StencilReadMask = input.stencil_read_mask;
        output.StencilWriteMask = input.stencil_write_mask;
        output.FrontFace = toStencilState(input.front_face);
        output.BackFace = toStencilState(input.back_face);
        return output;
    }

    D3D11_BLEND toBlendFactor(const core::GraphicsBlendFactor value) {
        switch (value) {
        case core::GraphicsBlendFactor::zero:                 return D3D11_BLEND_ZERO;
        case core::GraphicsBlendFactor::one:                  return D3D11_BLEND_ONE;
        case core::GraphicsBlendFactor::src_color:            return D3D11_BLEND_SRC_COLOR;
        case core::GraphicsBlendFactor::one_minus_src_color:  return D3D11_BLEND_INV_SRC_COLOR;
        case core::GraphicsBlendFactor::dest_color:           return D3D11_BLEND_DEST_COLOR;
        case core::GraphicsBlendFactor::one_minus_dest_color: return D3D11_BLEND_INV_DEST_COLOR;
        case core::GraphicsBlendFactor::src_alpha:            return D3D11_BLEND_SRC_ALPHA;
        case core::GraphicsBlendFactor::one_minus_src_alpha:  return D3D11_BLEND_INV_SRC_ALPHA;
        case core::GraphicsBlendFactor::dest_alpha:           return D3D11_BLEND_DEST_ALPHA;
        case core::GraphicsBlendFactor::one_minus_dest_alpha: return D3D11_BLEND_INV_DEST_ALPHA;
        case core::GraphicsBlendFactor::constant:             return D3D11_BLEND_BLEND_FACTOR;
        case core::GraphicsBlendFactor::one_minus_constant:   return D3D11_BLEND_INV_BLEND_FACTOR;
        case core::GraphicsBlendFactor::src_alpha_saturate:   return D3D11_BLEND_SRC_ALPHA_SAT;
        default: assert(false); return D3D11_BLEND{};
        }
    }
    D3D11_BLEND_OP toBlendMethod(const core::GraphicsBlendMethod value) {
        switch (value) {
        case core::GraphicsBlendMethod::add:              return D3D11_BLEND_OP_ADD;
        case core::GraphicsBlendMethod::subtract:         return D3D11_BLEND_OP_SUBTRACT;
        case core::GraphicsBlendMethod::reverse_subtract: return D3D11_BLEND_OP_REV_SUBTRACT;
        case core::GraphicsBlendMethod::min:              return D3D11_BLEND_OP_MIN;
        case core::GraphicsBlendMethod::max:              return D3D11_BLEND_OP_MAX;
        default: assert(false); return D3D11_BLEND_OP{};
        }
    }
    D3D11_RENDER_TARGET_BLEND_DESC toRenderTargetBlendState(const core::GraphicsBlendState& input) {
        D3D11_RENDER_TARGET_BLEND_DESC output{};
        output.BlendEnable = input.blend_enable ? TRUE : FALSE;
        output.SrcBlend = toBlendFactor(input.src_color_blend_factor);
        output.DestBlend = toBlendFactor(input.dest_color_blend_factor);
        output.BlendOp = toBlendMethod(input.color_blend_method);
        output.SrcBlendAlpha = toBlendFactor(input.src_alpha_blend_factor);
        output.DestBlendAlpha = toBlendFactor(input.dest_alpha_blend_factor);
        output.BlendOpAlpha = toBlendMethod(input.alpha_blend_method);
        output.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // always write all
        return output;
    }
    D3D11_BLEND_DESC toBlendState(const core::GraphicsBlendState& input) {
        D3D11_BLEND_DESC output{};
        output.AlphaToCoverageEnable = FALSE; // no MSAA
        output.IndependentBlendEnable = FALSE; // no multi render target
        output.RenderTarget[0] = toRenderTargetBlendState(input);
        for (size_t i = 1; i < 8; i += 1) {
            output.RenderTarget[i] = output.RenderTarget[0];
        }
        return output;
    }
}

namespace core {
    InputLayoutCache::InputLayoutCache() = default;
    InputLayoutCache::InputLayoutCache(const InputLayoutCache& other) {
        this->operator=(other);
    }
    InputLayoutCache::InputLayoutCache(InputLayoutCache&& other) {
        this->operator=(std::move(other));
    }
    InputLayoutCache::~InputLayoutCache() = default;

    InputLayoutCache& InputLayoutCache::operator=(const InputLayoutCache& other) {
        if (this == &other) {
            return *this;
        }
        assert(other.semantic_names.size() == other.input_elements.size());
        semantic_names = other.semantic_names;
        input_elements = other.input_elements;
        for (size_t i = 0; i < input_elements.size(); i += 1) {
            input_elements[i].SemanticName = semantic_names[i].c_str();
        }
        shader_byte_code = other.shader_byte_code;
        hash_value = other.hash_value;
        return *this;
    }
    InputLayoutCache& InputLayoutCache::operator=(InputLayoutCache&& other) {
        if (this == &other) {
            return *this;
        }
        assert(other.semantic_names.size() == other.input_elements.size());
        semantic_names = std::move(other.semantic_names);
        input_elements = std::move(other.input_elements);
        shader_byte_code = std::move(other.shader_byte_code);
        hash_value = other.hash_value;
        return *this;
    }

    void InputLayoutCache::initialize(
        const D3D11_INPUT_ELEMENT_DESC* const p_input_elements, const UINT input_element_count,
        const void* const p_shader_byte_code, const SIZE_T shader_byte_code_length
    ) {
        assert(p_input_elements != nullptr);
        assert(input_element_count != 0);
        assert(p_shader_byte_code != nullptr);
        assert(shader_byte_code_length != 0);
        semantic_names.resize(input_element_count);
        input_elements.resize(input_element_count);
        for (uint32_t i = 0; i < input_element_count; i += 1) {
            assert(p_input_elements[i].SemanticName != nullptr);
            input_elements[i] = p_input_elements[i];
            semantic_names[i].assign(p_input_elements[i].SemanticName);
            input_elements[i].SemanticName = semantic_names[i].c_str();
        }
        shader_byte_code.resize(shader_byte_code_length);
        std::memcpy(shader_byte_code.data(), p_shader_byte_code, shader_byte_code_length);
    }

    size_t InputLayoutCacheOp::operator()(const InputLayoutCache& s) const noexcept {
        return s.hash_value;
    }

    bool InputLayoutCacheOp::operator()(const InputLayoutCache& a, const InputLayoutCache& b) const noexcept {
        if (a.hash_value != b.hash_value) {
            return false;
        }
        if (a.input_elements.size() != b.input_elements.size()) {
            return false;
        }
        for (size_t i = 0; i < a.input_elements.size(); i += 1) {
            const auto& a_semantic_name = a.semantic_names[i];
            const auto& a_input_element = a.input_elements[i];
            const auto& b_semantic_name = b.semantic_names[i];
            const auto& b_input_element = b.input_elements[i];
            if (a_semantic_name != b_semantic_name) {
                return false;
            }
            if (std::memcmp(&a_input_element.SemanticIndex, &b_input_element.SemanticIndex, sizeof(a_input_element) - sizeof(a_input_element.SemanticName)) != 0) {
                return false;
            }
        }
        if (a.shader_byte_code.size() != b.shader_byte_code.size()) {
            return false;
        }
        if (std::memcmp(a.shader_byte_code.data(), b.shader_byte_code.data(), a.shader_byte_code.size()) != 0) {
            return false;
        }
        return true;
    }

    void GraphicsPipelineCache::hash(InputLayoutCache* const input_layout_cache) {
        XXH3_64bits_reset(m_hash_state);
        const auto& semantic_names = input_layout_cache->semantic_names;
        const auto& input_elements = input_layout_cache->input_elements;
        assert(semantic_names.size() == input_elements.size());
        for (size_t i = 0; i < input_elements.size(); i += 1) {
            const auto& semantic_name = semantic_names[i];
            const auto& input_element = input_elements[i];
            XXH3_64bits_update(m_hash_state, semantic_name.c_str(), semantic_name.size());
            XXH3_64bits_update(m_hash_state, &input_element.SemanticIndex, sizeof(input_element) - sizeof(input_element.SemanticName));
        }
        const auto& shader_byte_code = input_layout_cache->shader_byte_code;
        XXH3_64bits_update(m_hash_state, shader_byte_code.data(), shader_byte_code.size());
        input_layout_cache->hash_value = XXH3_64bits_digest(m_hash_state);
    }
}

namespace core {
    // GraphicsPipelineStateHelper

    void GraphicsPipelineStateHelper::save(const GraphicsPipelineState& state) {
        // copy struct

        *static_cast<GraphicsPipelineState*>(this) = state;

        // copy buffers

        buffers_data.resize(state.vertex_input_state.buffer_count);
        for (size_t i = 0; i < buffers_data.size(); i += 1) {
            buffers_data[i] = state.vertex_input_state.buffers[i];
        }
        vertex_input_state.buffers = buffers_data.data();

        // copy elements

        semantic_names_data.resize(state.vertex_input_state.element_count);
        elements_data.resize(state.vertex_input_state.element_count);
        for (size_t i = 0; i < elements_data.size(); i += 1) {
            elements_data[i] = state.vertex_input_state.elements[i];
            semantic_names_data[i].assign(state.vertex_input_state.elements[i].semantic_name);
            elements_data[i].semantic_name = semantic_names_data[i].c_str();
        }
        vertex_input_state.elements = elements_data.data();

        // copy vertex shader

        vertex_shader_data.resize(state.vertex_shader.size);
        std::memcpy(vertex_shader_data.data(), state.vertex_shader.data, state.vertex_shader.size);
        vertex_shader.data = vertex_shader_data.data();

        // copy pixel shader

        pixel_shader_data.resize(state.pixel_shader.size);
        std::memcpy(pixel_shader_data.data(), state.pixel_shader.data, state.pixel_shader.size);
        pixel_shader.data = pixel_shader_data.data();
    }
}

namespace core {
    // IGraphicsDeviceEventListener

    void GraphicsPipeline::onGraphicsDeviceCreate() {
        if (m_initialized) {
            createResources();
        }
    }
    void GraphicsPipeline::onGraphicsDeviceDestroy() {
        m_input_layout.reset();
        m_vertex_shader.reset();
        m_rasterizer_state.reset();
        m_pixel_shader.reset();
        m_depth_stencil_state.reset();
        m_blend_state.reset();
    }

    // GraphicsPipeline

    GraphicsPipeline::GraphicsPipeline() = default;
    GraphicsPipeline::~GraphicsPipeline() {
        if (m_initialized) {
            m_device->removeEventListener(this);
        }
    }

    bool GraphicsPipeline::createResources(IGraphicsDevice* const device, const GraphicsPipelineState& create_info) {
        if (device == nullptr) {
            assert(false); return false;
        }
        m_device = device;
        m_graphics_pipeline_state_helper.save(create_info);
        return createResources();
    }
    void GraphicsPipeline::apply() {
        const auto ctx = static_cast<ID3D11DeviceContext*>(m_device->getCommandbuffer()->getNativeHandle());
        assert(ctx != nullptr);

        // Stage: IA

        assert(m_input_layout);
        ctx->IASetInputLayout(m_input_layout.get());
        ctx->IASetPrimitiveTopology(m_primitive_topology);

        // Stage: VS

        assert(m_vertex_shader);
        ctx->VSSetShader(m_vertex_shader.get(), nullptr, 0);

        // Stage: RS

        assert(m_rasterizer_state);
        ctx->RSSetState(m_rasterizer_state.get());

        // Stage: PS

        assert(m_pixel_shader);
        ctx->PSSetShader(m_pixel_shader.get(), nullptr, 0);

        // Stage: OM

        assert(m_depth_stencil_state);
        ctx->OMSetDepthStencilState(m_depth_stencil_state.get(), D3D11_DEFAULT_STENCIL_REFERENCE); // TODO: stencil ref

        assert(m_blend_state);
        constexpr float blend_constant[4]{
            D3D11_DEFAULT_BLEND_FACTOR_RED,
            D3D11_DEFAULT_BLEND_FACTOR_GREEN,
            D3D11_DEFAULT_BLEND_FACTOR_BLUE,
            D3D11_DEFAULT_BLEND_FACTOR_ALPHA,
        };
        ctx->OMSetBlendState(m_blend_state.get(), blend_constant, D3D11_DEFAULT_SAMPLE_MASK); // TODO: blend constant, sample mask
    }

    bool GraphicsPipeline::createResources() {
        const auto device = static_cast<ID3D11Device*>(m_device->getNativeHandle());
        assert(device != nullptr);
        const auto& create_info = m_graphics_pipeline_state_helper;

        // ID3D11InputLayout

        const std::vector<D3D11_INPUT_ELEMENT_DESC> elements = std::move(toVertexInputElements(create_info.vertex_input_state));
        if (!win32::check_hresult_as_boolean(
            device->CreateInputLayout(
                elements.data(), static_cast<UINT>(elements.size()),
                create_info.vertex_shader.data, create_info.vertex_shader.size,
                m_input_layout.put()
            ),
            "ID3D11Device::CreateInputLayout"sv
        )) {
            return false;
        }

        // D3D11_PRIMITIVE_TOPOLOGY

        m_primitive_topology = toPrimitiveType(create_info.primitive_type);

        // ID3D11VertexShader

        if (!win32::check_hresult_as_boolean(
            device->CreateVertexShader(create_info.vertex_shader.data, create_info.vertex_shader.size, nullptr, m_vertex_shader.put()),
            "ID3D11Device::CreateVertexShader"sv
        )) {
            return false;
        }

        // ID3D11RasterizerState

        const D3D11_RASTERIZER_DESC rasterizer_state = toRasterizerState(create_info.rasterizer_state);
        if (!win32::check_hresult_as_boolean(
            device->CreateRasterizerState(&rasterizer_state, m_rasterizer_state.put()),
            "ID3D11Device::CreateRasterizerState"sv
        )) {
            return false;
        }
    
        // ID3D11PixelShader

        if (!win32::check_hresult_as_boolean(
            device->CreatePixelShader(create_info.pixel_shader.data, create_info.pixel_shader.size, nullptr, m_pixel_shader.put()),
            "ID3D11Device::CreatePixelShader"sv
        )) {
            return false;
        }

        // ID3D11DepthStencilState

        const D3D11_DEPTH_STENCIL_DESC depth_stencil_state = toDepthStencilState(create_info.depth_stencil_state);
        if (!win32::check_hresult_as_boolean(
            device->CreateDepthStencilState(&depth_stencil_state, m_depth_stencil_state.put()),
            "ID3D11Device::CreateDepthStencilState"sv
        )) {
            return false;
        }

        // ID3D11BlendState

        const D3D11_BLEND_DESC blend_state = toBlendState(create_info.blend_state);
        if (!win32::check_hresult_as_boolean(
            device->CreateBlendState(&blend_state, m_blend_state.put()),
            "ID3D11Device::CreateBlendState"sv
        )) {
            return false;
        }

        m_device->addEventListener(this);
        m_initialized = true;
        return true;
    }
}

#include "d3d11/GraphicsDevice.hpp"

namespace core {
    // IGraphicsCommandBuffer

    void GraphicsDevice::bindGraphicsPipeline(IGraphicsPipeline* const graphics_pipeline) {
        assert(graphics_pipeline != nullptr);
        const auto impl = static_cast<GraphicsPipeline*>(graphics_pipeline);
        impl->apply();
        // TODO: ref it
    }

    // IGraphicsDevice

    bool GraphicsDevice::createGraphicsPipeline(const GraphicsPipelineState* const graphics_pipeline_state, IGraphicsPipeline** const out_graphics_pipeline) {
        if (graphics_pipeline_state == nullptr) {
            assert(false); return false;
        }
        if (out_graphics_pipeline == nullptr) {
            assert(false); return false;
        }
        SmartReference<GraphicsPipeline> graphics_pipeline;
        graphics_pipeline.attach(new GraphicsPipeline());
        if (!graphics_pipeline->createResources(this, *graphics_pipeline_state)) {
            return false;
        }
        *out_graphics_pipeline = graphics_pipeline.detach();
        return true;
    }
}
