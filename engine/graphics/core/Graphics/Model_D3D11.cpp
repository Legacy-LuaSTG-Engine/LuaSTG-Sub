#include "core/Graphics/Model_D3D11.hpp"
#include "core/Logger.hpp"
#include "core/FileSystem.hpp"

#define IDX(x) (size_t)static_cast<uint8_t>(x)

namespace DirectX
{
    inline XMMATRIX XM_CALLCONV XMMatrixInverseTranspose(DirectX::FXMMATRIX M)
    {
        // 世界矩阵的逆的转置仅针对法向量，我们也不需要世界矩阵的平移分量
        // 而且不去掉的话，后续再乘上观察矩阵之类的就会产生错误的变换结果
        XMMATRIX A = M;
        A.r[3] = g_XMIdentityR3;
        return XMMatrixTranspose(XMMatrixInverse(NULL, A));
    }
}

namespace core::Graphics
{
    bool ModelSharedComponent_D3D11::createImage()
    {
        auto* device = m_device->GetD3D11Device();
        assert(device);

        HRESULT hr = S_OK;

        // default: purple & black tile image

        auto RGBA = [](uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) -> uint32_t
        {
            return uint32_t(r_)
                | (uint32_t(g_) << 8)
                | (uint32_t(b_) << 16)
                | (uint32_t(a_) << 24);
        };
        uint32_t black = RGBA(0, 0, 0, 255);
        uint32_t purple = RGBA(255, 0, 255, 255);
        std::vector<uint32_t> pixels(64 * 64);
        uint32_t* ptr = pixels.data();
        for (int i = 0; i < 32; i += 1)
        {
            for (int j = 0; j < 32; j += 1)
            {
                *ptr = black;
                ptr++;
            }
            for (int j = 0; j < 32; j += 1)
            {
                *ptr = purple;
                ptr++;
            }
        }
        for (int i = 0; i < 32; i += 1)
        {
            for (int j = 0; j < 32; j += 1)
            {
                *ptr = purple;
                ptr++;
            }
            for (int j = 0; j < 32; j += 1)
            {
                *ptr = black;
                ptr++;
            }
        }

        // default: create

        D3D11_TEXTURE2D_DESC def_tex_def = {
            .Width = 64,
            .Height = 64,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc = {
                .Count = 1,
                .Quality = 0,
            },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_SHADER_RESOURCE,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
        };
        D3D11_SUBRESOURCE_DATA def_dat_def = {
            .pSysMem = pixels.data(),
            .SysMemPitch = 64 * 4,
            .SysMemSlicePitch = 0,
        };
        win32::com_ptr<ID3D11Texture2D> def_texture2d;
        hr = device->CreateTexture2D(&def_tex_def, &def_dat_def, def_texture2d.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }
        D3D11_SHADER_RESOURCE_VIEW_DESC def_srv_def = {
            .Format = def_tex_def.Format,
            .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
            .Texture2D = {
                .MostDetailedMip = 0,
                .MipLevels = def_tex_def.MipLevels,
            },
        };
        hr = device->CreateShaderResourceView(def_texture2d.get(), &def_srv_def, default_image.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        return true;
    }
    bool ModelSharedComponent_D3D11::createSampler()
    {
        auto* device = m_device->GetD3D11Device();
        assert(device);

        HRESULT hr = S_OK;

        // default: create

        D3D11_SAMPLER_DESC def_samp_def = {
            .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
            .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
            .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
            .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
            .MipLODBias = D3D11_DEFAULT_MIP_LOD_BIAS,
            .MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY,
            .ComparisonFunc = D3D11_COMPARISON_ALWAYS,
            .BorderColor = {},
            .MinLOD = 0.0f,
            .MaxLOD = D3D11_FLOAT32_MAX,
        };
        hr = device->CreateSamplerState(&def_samp_def, default_sampler.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        return true;
    }
    bool ModelSharedComponent_D3D11::createConstantBuffer()
    {
        auto* device = m_device->GetD3D11Device();
        assert(device);

        HRESULT hr = S_OK;

        // built-in: view-proj matrix

        D3D11_BUFFER_DESC cbo_def = {
            .ByteWidth = sizeof(DirectX::XMFLOAT4X4),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
            .StructureByteStride = 0,
        };
        hr = device->CreateBuffer(&cbo_def, NULL, cbo_mvp.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        // built-in: local-world matrix

        cbo_def.ByteWidth = 2 * sizeof(DirectX::XMFLOAT4X4);
        hr = device->CreateBuffer(&cbo_def, NULL, cbo_mlw.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        // built-in: camera info

        cbo_def.ByteWidth = 2 * sizeof(DirectX::XMFLOAT4X4);
        hr = device->CreateBuffer(&cbo_def, NULL, cbo_caminfo.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        // // built-in: alpha mask

        cbo_def.ByteWidth = 2 * sizeof(DirectX::XMFLOAT4);
        hr = device->CreateBuffer(&cbo_def, NULL, cbo_alpha.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        // // built-in: light

        cbo_def.ByteWidth = 4 * sizeof(DirectX::XMFLOAT4);
        hr = device->CreateBuffer(&cbo_def, NULL, cbo_light.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        return true;
    }
    bool ModelSharedComponent_D3D11::createState()
    {
        auto* device = m_device->GetD3D11Device();
        assert(device);

        HRESULT hr = S_OK;

        //// RS \\\\

        // built-in: cull-none

        D3D11_RASTERIZER_DESC rs_def = {
            .FillMode = D3D11_FILL_SOLID,
            .CullMode = D3D11_CULL_NONE,
            .FrontCounterClockwise = FALSE,
            .DepthBias = D3D11_DEFAULT_DEPTH_BIAS,
            .DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
            .SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
            .DepthClipEnable = TRUE,
            .ScissorEnable = TRUE,
            .MultisampleEnable = FALSE,
            .AntialiasedLineEnable = FALSE,
        };
        hr = device->CreateRasterizerState(&rs_def, state_rs_cull_none.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        // built-in: cull-back

        rs_def.CullMode = D3D11_CULL_BACK;
        rs_def.FrontCounterClockwise = TRUE;
        hr = device->CreateRasterizerState(&rs_def, state_rs_cull_back.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        //// DS \\\\

        // built-in: depth-test

        D3D11_DEPTH_STENCIL_DESC ds_def = {
            .DepthEnable = TRUE,
            .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
            .DepthFunc = D3D11_COMPARISON_LESS_EQUAL,
            .StencilEnable = FALSE,
            .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
            .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
            .FrontFace = {
                .StencilFailOp = D3D11_STENCIL_OP_KEEP,
                .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
                .StencilPassOp = D3D11_STENCIL_OP_KEEP,
                .StencilFunc = D3D11_COMPARISON_ALWAYS,
            },
            .BackFace = {
                .StencilFailOp = D3D11_STENCIL_OP_KEEP,
                .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
                .StencilPassOp = D3D11_STENCIL_OP_KEEP,
                .StencilFunc = D3D11_COMPARISON_ALWAYS,
            },
        };
        hr = device->CreateDepthStencilState(&ds_def, state_ds.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        // built-in: depth-test less only

        ds_def.DepthFunc = D3D11_COMPARISON_LESS;

        hr = device->CreateDepthStencilState(&ds_def, state_ds_dl.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        // built-in: depth-test less only, no write

        ds_def.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

        hr = device->CreateDepthStencilState(&ds_def, state_ds_no_write.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        // built-in: depth-test disable

        ds_def.DepthEnable = FALSE;
        ds_def.DepthFunc = D3D11_COMPARISON_ALWAYS;

        hr = device->CreateDepthStencilState(&ds_def, state_ds_disable.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        //// BLEND \\\\

        // built-in: disable

        D3D11_RENDER_TARGET_BLEND_DESC rt_blend_def = {
            .BlendEnable = FALSE,
            .SrcBlend = D3D11_BLEND_ONE,
            .DestBlend = D3D11_BLEND_INV_SRC_ALPHA,
            .BlendOp = D3D11_BLEND_OP_ADD,
            .SrcBlendAlpha = D3D11_BLEND_ONE,
            .DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA,
            .BlendOpAlpha = D3D11_BLEND_OP_ADD,
            .RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL,
        };
        D3D11_BLEND_DESC blend_def = {
            .AlphaToCoverageEnable = FALSE,
            .IndependentBlendEnable = FALSE,
            .RenderTarget = {},
        };
        for (auto& rt_blend : blend_def.RenderTarget)
        {
            rt_blend = rt_blend_def;
        }
        hr = device->CreateBlendState(&blend_def, state_blend.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        // built-in: alpha-blend

        rt_blend_def.BlendEnable = TRUE;
        rt_blend_def.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        for (auto& rt_blend : blend_def.RenderTarget)
        {
            rt_blend = rt_blend_def;
        }
        hr = device->CreateBlendState(&blend_def, state_blend_alpha.put());
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        return true;
    }

    bool ModelSharedComponent_D3D11::createResources()
    {
        // load image to shader resource

        if (!createImage()) return false;

        // create sampler state

        if (!createSampler()) return false;

        // create shader and input layout

        if (!createShader()) return false;

        // create constant buffer

        if (!createConstantBuffer()) return false;

        // create state

        if (!createState()) return false;

        return true;
    }
    void ModelSharedComponent_D3D11::onGraphicsDeviceCreate()
    {
        createResources();
    }
    void ModelSharedComponent_D3D11::onGraphicsDeviceDestroy()
    {
        default_image.reset();
        default_sampler.reset();

        input_layout.reset();
        input_layout_vc.reset();
        shader_vertex.reset();
        shader_vertex_vc.reset();
        for (auto& v : shader_pixel) v.reset();
        for (auto& v : shader_pixel_alpha) v.reset();
        for (auto& v : shader_pixel_nt) v.reset();
        for (auto& v : shader_pixel_alpha_nt) v.reset();
        for (auto& v : shader_pixel_vc) v.reset();
        for (auto& v : shader_pixel_alpha_vc) v.reset();
        for (auto& v : shader_pixel_nt_vc) v.reset();
        for (auto& v : shader_pixel_alpha_nt_vc) v.reset();

        state_rs_cull_none.reset();
        state_rs_cull_back.reset();
        state_ds.reset();
        state_blend.reset();
        state_blend_alpha.reset();

        cbo_mvp.reset();
        cbo_mlw.reset();
        cbo_caminfo.reset();
        cbo_alpha.reset();
        cbo_light.reset();
    }

    ModelSharedComponent_D3D11::ModelSharedComponent_D3D11(GraphicsDevice* p_device)
        : m_device(p_device)
    {
        if (!createResources())
            throw std::runtime_error("ModelSharedComponent_D3D11::ModelSharedComponent_D3D11");
        m_device->addEventListener(this);
    }
    ModelSharedComponent_D3D11::~ModelSharedComponent_D3D11()
    {
        m_device->removeEventListener(this);
    }
}

namespace core::Graphics
{
    static void map_sampler_to_d3d11(tinygltf::Sampler& samp, D3D11_SAMPLER_DESC& desc)
    {
    #define MAKE_FILTER(MIN, MAG_MIP) ((MAG_MIP << 16) | (MIN))
        switch (MAKE_FILTER(samp.minFilter, samp.magFilter))
        {
        default:
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            break;
        case MAKE_FILTER(TINYGLTF_TEXTURE_FILTER_NEAREST, TINYGLTF_TEXTURE_FILTER_NEAREST):
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            desc.MinLOD = 0.0f;
            desc.MaxLOD = 0.0f;
            break;
        case MAKE_FILTER(TINYGLTF_TEXTURE_FILTER_NEAREST, TINYGLTF_TEXTURE_FILTER_LINEAR):
            desc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
            desc.MinLOD = 0.0f;
            desc.MaxLOD = 0.0f;
            break;
        case MAKE_FILTER(TINYGLTF_TEXTURE_FILTER_NEAREST, TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST):
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            break;
        case MAKE_FILTER(TINYGLTF_TEXTURE_FILTER_NEAREST, TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST):
            desc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
            break;
        case MAKE_FILTER(TINYGLTF_TEXTURE_FILTER_NEAREST, TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR):
            desc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
            break;
        case MAKE_FILTER(TINYGLTF_TEXTURE_FILTER_NEAREST, TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR):
            desc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
            break;
        case MAKE_FILTER(TINYGLTF_TEXTURE_FILTER_LINEAR, TINYGLTF_TEXTURE_FILTER_NEAREST):
            desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
            desc.MinLOD = 0.0f;
            desc.MaxLOD = 0.0f;
            break;
        case MAKE_FILTER(TINYGLTF_TEXTURE_FILTER_LINEAR, TINYGLTF_TEXTURE_FILTER_LINEAR):
            desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            desc.MinLOD = 0.0f;
            desc.MaxLOD = 0.0f;
            break;
        case MAKE_FILTER(TINYGLTF_TEXTURE_FILTER_LINEAR, TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST):
            desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
            break;
        case MAKE_FILTER(TINYGLTF_TEXTURE_FILTER_LINEAR, TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST):
            desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            break;
        case MAKE_FILTER(TINYGLTF_TEXTURE_FILTER_LINEAR, TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR):
            desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            break;
        case MAKE_FILTER(TINYGLTF_TEXTURE_FILTER_LINEAR, TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR):
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            break;
        }
    #undef MAKE_FILTER
        switch (samp.wrapS)
        {
        default:
            desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            break;
        case TINYGLTF_TEXTURE_WRAP_REPEAT:
            desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            break;
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
            desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            break;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
            desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
            break;
        }
        switch (samp.wrapT)
        {
        default:
        case -1:
            desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            break;
        case TINYGLTF_TEXTURE_WRAP_REPEAT:
            desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            break;
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
            desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            break;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
            desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
            break;
        }
    }

    static D3D11_PRIMITIVE_TOPOLOGY map_primitive_mode_to_d3d11(const int mode) {
        switch (mode) {
        case TINYGLTF_MODE_POINTS:
            return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        case TINYGLTF_MODE_LINE:
        case TINYGLTF_MODE_LINE_LOOP: // 需要展开
            return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        case TINYGLTF_MODE_LINE_STRIP:
            return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case TINYGLTF_MODE_TRIANGLE_STRIP:
            return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case TINYGLTF_MODE_TRIANGLE_FAN: // 需要展开
        case TINYGLTF_MODE_TRIANGLES:
        default:
            return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        }
    }

    static bool read_gltf_index_accessor(
        tinygltf::Model const& model,
        tinygltf::Primitive const& prim,
        uint32_t vertex_count,
        std::string const& prim_tag,
        std::vector<uint32_t>& out
    ) {
        // accessor

        if (prim.indices < 0 || prim.indices >= static_cast<int>(model.accessors.size())) {
            Logger::error(
                "[core] [Model] {} -- indices accessor index out of bound (value = {}, count = {})",
                prim_tag, prim.indices, model.accessors.size()
            );
            return false;
        }

        auto const& accessor = model.accessors[prim.indices];
        size_t elem_size = 0; // 单索引字节数：spec 要求 SCALAR + 无符号整型

        switch (accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            elem_size = 1;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            elem_size = 2;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            elem_size = 4;
            break;
        default:
            Logger::error(
                "[core] [Model] {} -- indices accessor (index = {}) must be unsigned integer component (value = {})",
                prim_tag, prim.indices, accessor.componentType
            );
            return false;
        }

        if (accessor.type != TINYGLTF_TYPE_SCALAR) {
            Logger::error(
                "[core] [Model] {} -- indices accessor (index = {}) must be SCALAR (type = {})",
                prim_tag, prim.indices, accessor.type
            );
            return false;
        }

        if (accessor.bufferView < 0 || accessor.bufferView >= static_cast<int>(model.bufferViews.size())) {
            Logger::error(
                "[core] [Model] {} -- indices accessor (index = {}) buffer view index out of bound (value = {})",
                prim_tag, prim.indices, accessor.bufferView
            );
            return false;
        }

        // buffer view

        auto const& view = model.bufferViews[accessor.bufferView];

        if (view.buffer < 0 || view.buffer >= static_cast<int>(model.buffers.size())) {
            Logger::error(
                "[core] [Model] {} -- buffer view (index = {}) buffer index out of bound (value = {})",
                prim_tag, accessor.bufferView, view.buffer
            );
            return false;
        }

        // buffer

        auto const& buffer = model.buffers[view.buffer];

        if (view.byteOffset + view.byteLength > buffer.data.size()) {
            Logger::error(
                "[core] [Model] {} -- buffer view (index = {}) range out of bound (offset = {}, length = {}, buffer size = {})",
                prim_tag, view.buffer, view.byteOffset, view.byteLength, buffer.data.size()
            );
            return false;
        }

        size_t const count = accessor.count;
        size_t const stride = view.byteStride > 0 ? view.byteStride : elem_size;
        size_t const span = count > 0 ? accessor.byteOffset + stride * (count - 1) + elem_size : 0;

        if (span > view.byteLength) {
            Logger::error(
                "[core] [Model] {} -- indices accessor (index = {}) data span ({}) out of buffer view byte length ({})",
                prim_tag, prim.indices, span, view.byteLength
            );
            return false;
        }

        auto const* base = buffer.data.data() + view.byteOffset + accessor.byteOffset;
        out.resize(count);

        for (size_t i = 0; i < count; i += 1) {
            uint32_t value = 0;
            std::memcpy(&value, base + stride * i, elem_size); // GLB 统一小端，目标架构同为小端
            if (value >= vertex_count) {
                Logger::error(
                    "[core] [Model] {} -- index (value = {}) out of vertex count ({}) at accessor (index = {}), element {}",
                    prim_tag, value, vertex_count, prim.indices, i
                );
                return false;
            }
            out[i] = value;
        }

        return true;
    }

    static bool expand_to_native_topology(
        int mode,
        std::vector<uint32_t> const& in_indices,
        bool has_indices,
        uint32_t vertex_count,
        std::string const& prim_tag,
        std::vector<uint32_t>& out_indices,
        D3D11_PRIMITIVE_TOPOLOGY& out_topology
    ) {
        size_t const count = has_indices ? in_indices.size() : vertex_count;

        char const* mode_name = "UNKNOWN";
        switch (mode) {
        case TINYGLTF_MODE_POINTS:
            mode_name = "POINTS";
            if (count < 1) {
                Logger::error("[core] [Model] {} -- {} requires at least 1 element, got {}", prim_tag, mode_name, count);
                return false;
            }
            break;
        case TINYGLTF_MODE_LINE:
            mode_name = "LINES";
            if (count < 2 || count % 2 != 0) {
                Logger::error("[core] [Model] {} -- {} requires >= 2 and even element count, got {}", prim_tag, mode_name, count);
                return false;
            }
            break;
        case TINYGLTF_MODE_LINE_LOOP:
            mode_name = "LINE_LOOP";
            if (count < 2) {
                Logger::error("[core] [Model] {} -- {} requires at least 2 elements, got {}", prim_tag, mode_name, count);
                return false;
            }
            break;
        case TINYGLTF_MODE_LINE_STRIP:
            mode_name = "LINE_STRIP";
            if (count < 2) {
                Logger::error("[core] [Model] {} -- {} requires at least 2 elements, got {}", prim_tag, mode_name, count);
                return false;
            }
            break;
        case TINYGLTF_MODE_TRIANGLES:
            mode_name = "TRIANGLES";
            if (count < 3 || count % 3 != 0) {
                Logger::error("[core] [Model] {} -- {} requires >= 3 and multiple-of-3 element count, got {}", prim_tag, mode_name, count);
                return false;
            }
            break;
        case TINYGLTF_MODE_TRIANGLE_STRIP:
            mode_name = "TRIANGLE_STRIP";
            if (count < 3) {
                Logger::error("[core] [Model] {} -- {} requires at least 3 elements, got {}", prim_tag, mode_name, count);
                return false;
            }
            break;
        case TINYGLTF_MODE_TRIANGLE_FAN:
            mode_name = "TRIANGLE_FAN";
            if (count < 3) {
                Logger::error("[core] [Model] {} -- {} requires at least 3 elements, got {}", prim_tag, mode_name, count);
                return false;
            }
            break;
        default:
            Logger::error("[core] [Model] {} -- unknown primitive mode {}", prim_tag, mode);
            return false;
        }

        out_topology = map_primitive_mode_to_d3d11(mode);

        bool const expand_loop = mode == TINYGLTF_MODE_LINE_LOOP;
        bool const expand_fan = mode == TINYGLTF_MODE_TRIANGLE_FAN;
        if (!expand_loop && !expand_fan) {
            out_indices = in_indices;
            return true;
        }

    #define GET_INDEX(i) (has_indices ? in_indices[( i )] : static_cast<uint32_t>( i ))

        if (expand_loop) {
            // N 条边 -> 2N 索引，首尾闭合
            out_indices.clear();
            out_indices.reserve(count * 2);
            for (size_t i = 0; i < count; i += 1) {
                out_indices.push_back(GET_INDEX(i));
                out_indices.push_back(GET_INDEX((i + 1) % count));
            }
        }
        else {
            // 扇心 0，三角形 (0, i, i + 1)，保留绕序
            out_indices.clear();
            out_indices.reserve((count - 2) * 3);
            for (size_t i = 1; i + 1 < count; i += 1) {
                out_indices.push_back(GET_INDEX(0));
                out_indices.push_back(GET_INDEX(i));
                out_indices.push_back(GET_INDEX(i + 1));
            }
        }

    #undef GET_INDEX

        Logger::warn(
            "[core] [Model] {} -- topology {} not native to D3D11, expanded to {} (indices {} -> {})",
            prim_tag,
            mode_name,
            expand_loop ? "LINES" : "TRIANGLES",
            count,
            out_indices.size()
        );

        return true;
    }

    static DirectX::XMMATRIX XM_CALLCONV get_local_transfrom_from_node(tinygltf::Node& node) {
        if (!node.matrix.empty())
        {
        #pragma warning(disable:4244)
            // [Potential Overflow]
            DirectX::XMFLOAT4X4 mM(
                node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3],
                node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7],
                node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11],
                node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]);
        #pragma warning(default:4244)
            return DirectX::XMLoadFloat4x4(&mM);
        }
        else
        {
            DirectX::XMMATRIX mS = DirectX::XMMatrixIdentity();
            DirectX::XMMATRIX mR = DirectX::XMMatrixIdentity();
            DirectX::XMMATRIX mT = DirectX::XMMatrixIdentity();
            if (!node.scale.empty())
            {
            #pragma warning(disable:4244)
                // [Potential Overflow]
                mS = DirectX::XMMatrixScaling(node.scale[0], node.scale[1], node.scale[2]);
            #pragma warning(default:4244)
            }
            if (!node.rotation.empty())
            {
            #pragma warning(disable:4244)
                // [Potential Overflow]
                DirectX::XMFLOAT4 quat(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]);
            #pragma warning(default:4244)
                mR = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&quat));
            }
            if (!node.translation.empty())
            {
            #pragma warning(disable:4244)
                // [Potential Overflow]
                mT = DirectX::XMMatrixTranslation(node.translation[0], node.translation[1], node.translation[2]);
            #pragma warning(default:4244)
            }
            return DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(mS, mR), mT);
        }
    }

    static bool getBufferFromAccessor(
        tinygltf::Model const& model,
        tinygltf::Accessor const& accessor,
        uint8_t*& output,
        size_t& total_size_in_bytes,
        std::vector<uint8_t>& intermediate_buffer
    ) {
        // buffer view
        if (accessor.bufferView < 0 || accessor.bufferView >= model.bufferViews.size()) {
            Logger::error(
                "[core] [Model] gltf 2.0 loader -- accessor (index = {}) buffer view index out of bound (value = {})",
                &accessor - model.accessors.data(),
                accessor.bufferView
            );
        }
        const auto& buffer_view = model.bufferViews[accessor.bufferView];
        // buffer
        if (buffer_view.buffer < 0 || buffer_view.buffer >= model.buffers.size()) {
            Logger::error(
                "[core] [Model] gltf 2.0 loader -- buffer view (index = {}) buffer index out of bound (value = {})",
                &buffer_view - model.bufferViews.data(),
                buffer_view.buffer
            );
        }
        const auto& buffer = model.buffers[buffer_view.buffer];
        // total size
        if (tinygltf::GetComponentSizeInBytes(accessor.componentType) < 0) {
            Logger::error(
                "[core] [Model] gltf 2.0 loader -- unknown accessor (index = {}) component type (value = {})",
                &accessor - model.accessors.data(),
                accessor.componentType
            );
            return false;
        }
        if (tinygltf::GetNumComponentsInType(accessor.type) < 0) {
            Logger::error(
                "[core] [Model] gltf 2.0 loader -- unknown accessor (index = {}) type (value = {})",
                &accessor - model.accessors.data(),
                accessor.type
            );
            return false;
        }
        total_size_in_bytes = static_cast<size_t>(tinygltf::GetComponentSizeInBytes(accessor.componentType))
            * static_cast<size_t>(tinygltf::GetNumComponentsInType(accessor.type))
            * accessor.count;
        // no stride
        if (buffer_view.byteStride == 0) {
            output = const_cast<uint8_t*>(buffer.data.data()) + buffer_view.byteOffset + accessor.byteOffset;
            return true;
        }
        // prepare intermediate buffer
        intermediate_buffer.resize(total_size_in_bytes);
        // copy data
        const auto size = static_cast<size_t>(tinygltf::GetComponentSizeInBytes(accessor.componentType))
            * static_cast<size_t>(tinygltf::GetNumComponentsInType(accessor.type));
        auto source = buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset;
        auto target = intermediate_buffer.data();
        for (size_t i = 0; i < accessor.count; i += 1) {
            std::memcpy(target, source, size);
            source += buffer_view.byteStride;
            target += size;
        }
        // using intermediate buffer
        output = intermediate_buffer.data();
        return true;
    }

    void Model_D3D11::setAmbient(Vector3F const& color, float brightness)
    {
        sunshine.ambient = DirectX::XMFLOAT4(color.x, color.y, color.z, brightness);
    }
    void Model_D3D11::setDirectionalLight(Vector3F const& direction, Vector3F const& color, float brightness)
    {
        sunshine.dir = DirectX::XMFLOAT4(direction.x, direction.y, direction.z, 0.0f);
        sunshine.color = DirectX::XMFLOAT4(color.x, color.y, color.z, brightness);
    }
    void Model_D3D11::setScaling(Vector3F const& scale)
    {
        t_scale_ = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
    }
    void Model_D3D11::setPosition(Vector3F const& pos)
    {
        t_trans_ = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
    }
    void Model_D3D11::setRotationRollPitchYaw(float roll, float pitch, float yaw)
    {
        t_mbrot_ = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
    }
    void Model_D3D11::setRotationQuaternion(Vector4F const& quat)
    {
        DirectX::XMFLOAT4 const xq(quat.x, quat.y, quat.z, quat.w);
        t_mbrot_ = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&xq));
    }

    bool Model_D3D11::createImage(tinygltf::Model& model)
    {
        auto* device = m_device->GetD3D11Device();
        auto* context = m_device->GetD3D11DeviceContext();

        HRESULT hr = S_OK;

        // gltf: create

        image.resize(model.images.size());
        for (size_t idx = 0; idx < model.images.size(); idx += 1)
        {
            tinygltf::Image& img = model.images[idx];

            if (img.width <= 0 || img.height <= 0)
            {
                image[idx] = shared_->default_image; // 兄啊，你这纹理好怪哦
                Logger::error("[core] [Model] load texture '{}' failed", img.name);
                continue;
            }

            bool mipmap = true;
            D3D11_TEXTURE2D_DESC tex_def = {
                .Width = (UINT)img.width,
                .Height = (UINT)img.height,
                .MipLevels = mipmap ? (UINT)0 : (UINT)1,
                .ArraySize = 1,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {
                    .Count = 1,
                    .Quality = 0,
                },
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_SHADER_RESOURCE | (mipmap ? D3D11_BIND_RENDER_TARGET : (UINT)0),
                .CPUAccessFlags = 0,
                .MiscFlags = mipmap ? D3D11_RESOURCE_MISC_GENERATE_MIPS : (UINT)0,
            };
            D3D11_SUBRESOURCE_DATA dat_def = {
                .pSysMem = img.image.data(),
                .SysMemPitch = (UINT)(img.width * img.component * img.bits) / 8,
                .SysMemSlicePitch = (UINT)img.image.size(),
            };
            win32::com_ptr<ID3D11Texture2D> texture2d;
            hr = device->CreateTexture2D(&tex_def, mipmap ? NULL : &dat_def, texture2d.put());
            if (FAILED(hr))
            {
                assert(false);
                return false;
            }
            D3D11_SHADER_RESOURCE_VIEW_DESC srv_def = {
                .Format = tex_def.Format,
                .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
                .Texture2D = {
                    .MostDetailedMip = 0,
                    .MipLevels = mipmap ? UINT(-1) : tex_def.MipLevels,
                },
            };
            hr = device->CreateShaderResourceView(texture2d.get(), &srv_def, image[idx].put());
            if (FAILED(hr))
            {
                assert(false);
                return false;
            }
            if (mipmap)
            {
                context->UpdateSubresource(texture2d.get(), 0, NULL, dat_def.pSysMem, dat_def.SysMemPitch, dat_def.SysMemSlicePitch);
                context->GenerateMips(image[idx].get());
            }
        }

        return true;
    }
    bool Model_D3D11::createSampler(tinygltf::Model& model)
    {
        auto* device = m_device->GetD3D11Device();

        HRESULT hr = S_OK;

        // gltf: create

        sampler.resize(model.samplers.size());
        for (size_t idx = 0; idx < model.samplers.size(); idx += 1)
        {
            tinygltf::Sampler& samp = model.samplers[idx];
            D3D11_SAMPLER_DESC samp_def = {
                .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
                .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
                .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
                .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
                .MipLODBias = D3D11_DEFAULT_MIP_LOD_BIAS,
                .MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY,
                .ComparisonFunc = D3D11_COMPARISON_ALWAYS,
                .BorderColor = {},
                .MinLOD = 0.0f,
                .MaxLOD = D3D11_FLOAT32_MAX,
            };
            map_sampler_to_d3d11(samp, samp_def);
            samp_def.Filter = D3D11_FILTER_ANISOTROPIC; // TODO: better?
            hr = device->CreateSamplerState(&samp_def, sampler[idx].put());
            if (FAILED(hr))
            {
                assert(false);
                return false;
            }
        }

        return true;
    }
    bool Model_D3D11::processNode(tinygltf::Model& model, tinygltf::Node& node)
    {
        auto* device = m_device->GetD3D11Device();

        HRESULT hr = S_OK;

        DirectX::XMMATRIX mTRS = get_local_transfrom_from_node(node);

        if (node.mesh >= 0)
        {
            const auto& mesh = model.meshes[node.mesh];
            for (size_t prim_idx = 0; prim_idx < mesh.primitives.size(); prim_idx += 1) {
                const auto& prim = mesh.primitives[prim_idx];
                const std::string prim_tag{std::format("mesh {} primitive {}", node.mesh, prim_idx)};

                if (!prim.attributes.contains("POSITION")) {
                    Logger::warn("[core] [Model] {} -- no POSITION attribute, primitive skipped", prim_tag);
                    continue;
                }

                const int position_accessor_index = prim.attributes.at("POSITION");
                if (position_accessor_index < 0 || position_accessor_index >= static_cast<int>(model.accessors.size())) {
                    Logger::error(
                        "[core] [Model] {} -- POSITION accessor index out of bound (value = {})",
                        prim_tag, position_accessor_index
                    );
                    return false;
                }
                const uint32_t vertex_count = static_cast<uint32_t>(model.accessors[position_accessor_index].count);

                ModelBlock mblock;
                DirectX::XMMATRIX mTRSw = mTRS;
                for (auto it = mTRS_stack.crbegin(); it != mTRS_stack.crend(); it++)
                {
                    mTRSw = DirectX::XMMatrixMultiply(mTRSw, *it);
                }
                mTRSw = DirectX::XMMatrixMultiply(mTRSw, DirectX::XMMatrixScaling(1.0f, 1.0f, -1.0f)); // to left-hand
                DirectX::XMStoreFloat4x4(&mblock.local_matrix, mTRSw);
                DirectX::XMStoreFloat4x4(&mblock.local_matrix_normal, DirectX::XMMatrixInverseTranspose(mTRSw)); // face normal
                if (prim.attributes.contains("POSITION"))
                {
                    tinygltf::Accessor& accessor = model.accessors[prim.attributes.at("POSITION")];
                    uint8_t* buffer_ptr{};
                    size_t total_size_in_bytes{};
                    std::vector<uint8_t> intermediate_buffer;
                    if (!getBufferFromAccessor(model, accessor, buffer_ptr, total_size_in_bytes, intermediate_buffer))
                        return false;

                    D3D11_BUFFER_DESC vbo_def = {
                        .ByteWidth = static_cast<UINT>(total_size_in_bytes),
                        .Usage = D3D11_USAGE_DEFAULT,
                        .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                        .CPUAccessFlags = 0,
                        .MiscFlags = 0,
                        .StructureByteStride = 0,
                    };
                    D3D11_SUBRESOURCE_DATA dat_def = {
                        .pSysMem = buffer_ptr,
                        .SysMemPitch = 0,
                        .SysMemSlicePitch = 0,
                    };
                    hr = device->CreateBuffer(&vbo_def, &dat_def, mblock.vertex_buffer.put());
                    if (FAILED(hr))
                    {
                        assert(false);
                        return false;
                    }

                    mblock.draw_count = (UINT)accessor.count;
                }
                if (prim.attributes.contains("NORMAL"))
                {
                    tinygltf::Accessor& accessor = model.accessors[prim.attributes.at("NORMAL")];
                    uint8_t* buffer_ptr{};
                    size_t total_size_in_bytes{};
                    std::vector<uint8_t> intermediate_buffer;
                    if (!getBufferFromAccessor(model, accessor, buffer_ptr, total_size_in_bytes, intermediate_buffer))
                        return false;

                    D3D11_BUFFER_DESC vbo_def = {
                        .ByteWidth = static_cast<UINT>(total_size_in_bytes),
                        .Usage = D3D11_USAGE_DEFAULT,
                        .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                        .CPUAccessFlags = 0,
                        .MiscFlags = 0,
                        .StructureByteStride = 0,
                    };
                    D3D11_SUBRESOURCE_DATA dat_def = {
                        .pSysMem = buffer_ptr,
                        .SysMemPitch = 0,
                        .SysMemSlicePitch = 0,
                    };
                    hr = device->CreateBuffer(&vbo_def, &dat_def, mblock.normal_buffer.put());
                    if (FAILED(hr))
                    {
                        assert(false);
                        return false;
                    }
                }
                if (prim.attributes.contains("COLOR_0"))
                {
                    tinygltf::Accessor& accessor = model.accessors[prim.attributes.at("COLOR_0")];
                    uint8_t* buffer_ptr{};
                    size_t total_size_in_bytes{};
                    std::vector<uint8_t> intermediate_buffer;
                    if (!getBufferFromAccessor(model, accessor, buffer_ptr, total_size_in_bytes, intermediate_buffer))
                        return false;

                    D3D11_BUFFER_DESC vbo_def = {
                        .ByteWidth = static_cast<UINT>(total_size_in_bytes),
                        .Usage = D3D11_USAGE_DEFAULT,
                        .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                        .CPUAccessFlags = 0,
                        .MiscFlags = 0,
                        .StructureByteStride = 0,
                    };
                    D3D11_SUBRESOURCE_DATA dat_def = {
                        .pSysMem = buffer_ptr,
                        .SysMemPitch = 0,
                        .SysMemSlicePitch = 0,
                    };
                    hr = device->CreateBuffer(&vbo_def, &dat_def, mblock.color_buffer.put());
                    if (FAILED(hr))
                    {
                        assert(false);
                        return false;
                    }
                }
                if (prim.attributes.contains("TEXCOORD_0"))
                {
                    tinygltf::Accessor& accessor = model.accessors[prim.attributes.at("TEXCOORD_0")];
                    uint8_t* buffer_ptr{};
                    size_t total_size_in_bytes{};
                    std::vector<uint8_t> intermediate_buffer;
                    if (!getBufferFromAccessor(model, accessor, buffer_ptr, total_size_in_bytes, intermediate_buffer))
                        return false;

                    D3D11_BUFFER_DESC vbo_def = {
                        .ByteWidth = static_cast<UINT>(total_size_in_bytes),
                        .Usage = D3D11_USAGE_DEFAULT,
                        .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                        .CPUAccessFlags = 0,
                        .MiscFlags = 0,
                        .StructureByteStride = 0,
                    };
                    D3D11_SUBRESOURCE_DATA dat_def = {
                        .pSysMem = buffer_ptr,
                        .SysMemPitch = 0,
                        .SysMemSlicePitch = 0,
                    };
                    hr = device->CreateBuffer(&vbo_def, &dat_def, mblock.uv_buffer.put());
                    if (FAILED(hr))
                    {
                        assert(false);
                        return false;
                    }
                }

                std::vector<uint32_t> native_indices;
                bool use_indices = prim.indices >= 0;
                {
                    std::vector<uint32_t> decoded;
                    if (use_indices) {
                        if (!read_gltf_index_accessor(model, prim, vertex_count, prim_tag, decoded)) {
                            return false;
                        }
                    }
                    int const mode = prim.mode < 0 ? TINYGLTF_MODE_TRIANGLES : prim.mode; // spec 默认 TRIANGLES
                    if (!expand_to_native_topology(
                            mode,
                            decoded,
                            use_indices,
                            vertex_count,
                            prim_tag,
                            native_indices,
                            mblock.primitive_topology
                    )) {
                        return false;
                    }
                    // 无索引的 LOOP/FAN 合成展开后，改为经索引缓冲按 LINES/TRIANGLES 提交
                    if (!use_indices && !native_indices.empty()) {
                        use_indices = true;
                    }
                }

                if (use_indices) {
                    uint32_t max_index = 0;
                    for (const uint32_t value : native_indices) {
                        if (value > max_index) {
                            max_index = value;
                        }
                    }
                    const bool wide = max_index > 0xFFFE; // 超 16 位容量时用 R32，否则使用 R16，0xFFFF 是特殊值需要避开
                    std::vector<uint16_t> narrow;
                    D3D11_BUFFER_DESC ibo_def = {
                        .ByteWidth = static_cast<UINT>(native_indices.size() * (wide ? 4 : 2)),
                        .Usage = D3D11_USAGE_DEFAULT,
                        .BindFlags = D3D11_BIND_INDEX_BUFFER,
                        .CPUAccessFlags = 0,
                        .MiscFlags = 0,
                        .StructureByteStride = 0,
                    };
                    D3D11_SUBRESOURCE_DATA ibo_dat = {
                        .pSysMem = NULL,
                        .SysMemPitch = 0,
                        .SysMemSlicePitch = 0,
                    };
                    if (wide) {
                        mblock.index_format = DXGI_FORMAT_R32_UINT;
                        ibo_dat.pSysMem = native_indices.data();
                    }
                    else {
                        narrow.resize(native_indices.size());
                        for (size_t i = 0; i < native_indices.size(); i += 1)
                            narrow[i] = (uint16_t)native_indices[i];
                        mblock.index_format = DXGI_FORMAT_R16_UINT;
                        ibo_dat.pSysMem = narrow.data();
                    }
                    hr = device->CreateBuffer(&ibo_def, &ibo_dat, mblock.index_buffer.put());
                    if (FAILED(hr)) {
                        assert(false);
                        return false;
                    }
                    mblock.draw_count = (UINT)native_indices.size();
                }
                else {
                    mblock.draw_count = vertex_count;
                }

                if (prim.material >= 0)
                {
                    tinygltf::Material& material = model.materials[prim.material];
                    auto& bcc = material.pbrMetallicRoughness.baseColorFactor;
                #pragma warning(disable:4244)
                    // [Potential Overflow]
                    mblock.base_color = DirectX::XMFLOAT4(bcc[0], bcc[1], bcc[2], bcc[3]);
                #pragma warning(default:4244)
                    tinygltf::TextureInfo& texture_info = material.pbrMetallicRoughness.baseColorTexture;
                    if (texture_info.index >= 0)
                    {
                        tinygltf::Texture& texture = model.textures[texture_info.index];
                        mblock.image = image[texture.source];
                        if (texture.sampler >= 0)
                        {
                            mblock.sampler = sampler[texture.sampler];
                        }
                        else
                        {
                            mblock.sampler = shared_->default_sampler;
                        }
                    }
                    else
                    {
                        //mblock.image = shared_->default_image;
                        mblock.sampler = shared_->default_sampler;
                    }
                    if (material.alphaMode == "MASK")
                    {
                        mblock.alpha_mask = TRUE;
                    }
                    else if (material.alphaMode == "BLEND")
                    {
                        mblock.alpha_blend = TRUE;
                    }
                    // [Potential Overflow]
                #pragma warning(disable:4244)
                    mblock.alpha = material.alphaCutoff;
                #pragma warning(default:4244)
                    mblock.double_side = material.doubleSided;
                }
                model_block.emplace_back(mblock);
            }
        }

        mTRS_stack.push_back(mTRS);
        if (!node.children.empty())
        {
            for (auto const& child_node_idx : node.children)
            {
                if (!processNode(model, model.nodes[child_node_idx]))
                {
                    return false;
                }
            }
        }
        mTRS_stack.pop_back();

        return true;
    };
    bool Model_D3D11::createModelBlock(tinygltf::Model& model)
    {
        int default_scene = model.defaultScene;
        if (default_scene < 0) default_scene = 0;
        tinygltf::Scene& scene = model.scenes[default_scene];
        for (int const& node_idx : scene.nodes)
        {
            tinygltf::Node& node = model.nodes[node_idx];
            if (!processNode(model, node))
            {
                return false;
            }
        }
        return true;
    }

    bool Model_D3D11::createResources()
    {
        struct FileSystemWrapper
        {
            static bool FileExists(const std::string& abs_filename, void*)
            {
                return FileSystemManager::hasFile(abs_filename);
            }
            static bool ReadWholeFile(std::vector<unsigned char>* out, std::string* err, const std::string& filepath, void*)
            {
                // TODO: no copy x2
                SmartReference<IData> data;
                if (!FileSystemManager::readFile(filepath, data.put())) {
                    if (err) {
                        (*err) += "File load error : " + filepath + "\n";
                    }
                    return false;
                }
                out->resize(data->size());
                std::memcpy(out->data(), data->data(), data->size());
                return true;
            }
            static bool GetFileSizeInBytes(size_t* filesize_out, [[maybe_unused]] std::string* err, const std::string& abs_filename, void*) {
                *filesize_out = FileSystemManager::getFileSize(abs_filename);
                return *filesize_out > 0;
            }
        };
        tinygltf::FsCallbacks fs_cb = {
            .FileExists = &FileSystemWrapper::FileExists,
            .ExpandFilePath = &tinygltf::ExpandFilePath,
            .ReadWholeFile = &FileSystemWrapper::ReadWholeFile,
            .WriteWholeFile = &tinygltf::WriteWholeFile,
            .GetFileSizeInBytes = &FileSystemWrapper::GetFileSizeInBytes,
            .user_data = nullptr,
        };
        tinygltf::TinyGLTF gltf_ctx;
        gltf_ctx.SetStoreOriginalJSONForExtrasAndExtensions(true);
        gltf_ctx.SetFsCallbacks(fs_cb);

        tinygltf::Model model;
        std::string warn;
        std::string err;

        bool ret = false;
        if (gltf_path.ends_with(".gltf"))
        {
            ret = gltf_ctx.LoadASCIIFromFile(&model, &err, &warn, gltf_path.c_str());
        }
        else
        {
            ret = gltf_ctx.LoadBinaryFromFile(&model, &err, &warn, gltf_path.c_str());
        }
        if (!warn.empty())
        {
            Logger::warn("[core] [Model] gltf model warning: {}", warn);
        }
        if (!err.empty())
        {
            Logger::error("[core] [Model] gltf model error: {}", err);
        }
        if (!ret)
        {
            return false;
        }

        // load image to shader resource

        if (!createImage(model)) return false;

        // create sampler state

        if (!createSampler(model)) return false;

        // create model block

        if (!createModelBlock(model)) return false;

        return true;
    }
    void Model_D3D11::onGraphicsDeviceCreate()
    {
        createResources();
    }
    void Model_D3D11::onGraphicsDeviceDestroy()
    {
        image.clear();
        sampler.clear();

        model_block.clear();
    }

    void Model_D3D11::draw(IRenderer::FogState fog)
    {
        auto* context = m_device->GetD3D11DeviceContext();

        // common data

        context->UpdateSubresource(shared_->cbo_light.get(), 0, NULL, &sunshine, 0, 0);
        DirectX::XMMATRIX const t_locwo_ = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(t_scale_, t_mbrot_), t_trans_);

        auto set_state_matrix_from_block = [&](ModelBlock& mblock)
        {
            // IA

            if (mblock.color_buffer)
                context->IASetInputLayout(shared_->input_layout_vc.get());
            else
                context->IASetInputLayout(shared_->input_layout.get());

            // VS

            if (mblock.color_buffer)
                context->VSSetShader(shared_->shader_vertex_vc.get(), NULL, 0);
            else
                context->VSSetShader(shared_->shader_vertex.get(), NULL, 0);

            // PS

            if (!mblock.alpha_mask)
            {
                if (mblock.image)
                {
                    if (mblock.color_buffer)
                        context->PSSetShader(shared_->shader_pixel_vc[IDX(fog)].get(), NULL, 0);
                    else
                        context->PSSetShader(shared_->shader_pixel[IDX(fog)].get(), NULL, 0);
                }
                else
                {
                    if (mblock.color_buffer)
                        context->PSSetShader(shared_->shader_pixel_nt_vc[IDX(fog)].get(), NULL, 0);
                    else
                        context->PSSetShader(shared_->shader_pixel_nt[IDX(fog)].get(), NULL, 0);
                }
            }
            else
            {
                if (mblock.image)
                {
                    if (mblock.color_buffer)
                        context->PSSetShader(shared_->shader_pixel_alpha_vc[IDX(fog)].get(), NULL, 0);
                    else
                        context->PSSetShader(shared_->shader_pixel_alpha[IDX(fog)].get(), NULL, 0);
                }
                else
                {
                    if (mblock.color_buffer)
                        context->PSSetShader(shared_->shader_pixel_alpha_nt_vc[IDX(fog)].get(), NULL, 0);
                    else
                        context->PSSetShader(shared_->shader_pixel_alpha_nt[IDX(fog)].get(), NULL, 0);
                }
            }
        };
        auto upload_local_world_matrix = [&](ModelBlock& mblock)
        {
            struct
            {
                DirectX::XMFLOAT4X4 v1;
                DirectX::XMFLOAT4X4 v2;
            } v{};
            DirectX::XMMATRIX const t_total_ = DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&mblock.local_matrix), t_locwo_);
            DirectX::XMStoreFloat4x4(&v.v1, t_total_);
            DirectX::XMStoreFloat4x4(&v.v2, DirectX::XMMatrixInverseTranspose(t_total_));
            context->UpdateSubresource(shared_->cbo_mlw.get(), 0, NULL, &v, 0, 0);
        };
        auto set_alpha_mode_opaque = [&](ModelBlock& mblock)
        {
            // PS

            FLOAT const alpha[8] = {
                    mblock.base_color.x, mblock.base_color.y, mblock.base_color.z, mblock.base_color.w,
                    0.5f, 0.0f, 0.0f, 0.0f,
            };
            context->UpdateSubresource(shared_->cbo_alpha.get(), 0, NULL, alpha, 0, 0);
            ID3D11Buffer* ps_cbo[2] = {
                // camera position and look to vector are setup by Renderer at register(b0)
                shared_->cbo_alpha.get(),
                shared_->cbo_light.get(),
            };
            context->PSSetConstantBuffers(2, 2, ps_cbo);

            // OM
            FLOAT const blend_factor[4]{};
            context->OMSetBlendState(shared_->state_blend.get(), blend_factor, D3D11_DEFAULT_SAMPLE_MASK);
        };
        auto set_alpha_mode_mask = [&](ModelBlock& mblock)
        {
            // PS

            FLOAT const alpha[8] = {
                    mblock.base_color.x, mblock.base_color.y, mblock.base_color.z, mblock.base_color.w,
                    mblock.alpha, 0.0f, 0.0f, 0.0f,
            };
            context->UpdateSubresource(shared_->cbo_alpha.get(), 0, NULL, alpha, 0, 0);
            ID3D11Buffer* ps_cbo[2] = {
                // camera position and look to vector are setup by Renderer at register(b0)
                shared_->cbo_alpha.get(),
                shared_->cbo_light.get(),
            };
            context->PSSetConstantBuffers(2, 2, ps_cbo);

            // OM
            FLOAT const blend_factor[4]{};
            context->OMSetBlendState(shared_->state_blend.get(), blend_factor, D3D11_DEFAULT_SAMPLE_MASK);
        };
        auto set_alpha_mode_mask_custom = [&](ModelBlock& mblock, float const value)
        {
            // PS

            FLOAT const alpha[8] = {
                    mblock.base_color.x, mblock.base_color.y, mblock.base_color.z, mblock.base_color.w,
                    value, 0.0f, 0.0f, 0.0f,
            };
            context->UpdateSubresource(shared_->cbo_alpha.get(), 0, NULL, alpha, 0, 0);
            ID3D11Buffer* ps_cbo[2] = {
                // camera position and look to vector are setup by Renderer at register(b0)
                shared_->cbo_alpha.get(),
                shared_->cbo_light.get(),
            };
            context->PSSetConstantBuffers(2, 2, ps_cbo);
            if (mblock.image)
            {
                if (mblock.color_buffer)
                    context->PSSetShader(shared_->shader_pixel_alpha_vc[IDX(fog)].get(), NULL, 0);
                else
                    context->PSSetShader(shared_->shader_pixel_alpha[IDX(fog)].get(), NULL, 0);
            }
            else
            {
                if (mblock.color_buffer)
                    context->PSSetShader(shared_->shader_pixel_alpha_nt_vc[IDX(fog)].get(), NULL, 0);
                else
                    context->PSSetShader(shared_->shader_pixel_alpha_nt[IDX(fog)].get(), NULL, 0);
            }

            // OM
            FLOAT const blend_factor[4]{};
            context->OMSetBlendState(shared_->state_blend.get(), blend_factor, D3D11_DEFAULT_SAMPLE_MASK);
        };
        auto set_alpha_mode_blend = [&](ModelBlock& mblock)
        {
            // PS

            FLOAT const alpha[8] = {
                    mblock.base_color.x, mblock.base_color.y, mblock.base_color.z, mblock.base_color.w,
                    0.5f, 0.0f, 0.0f, 0.0f,
            };
            context->UpdateSubresource(shared_->cbo_alpha.get(), 0, NULL, alpha, 0, 0);
            ID3D11Buffer* ps_cbo[2] = {
                // camera position and look to vector are setup by Renderer at register(b0)
                shared_->cbo_alpha.get(),
                shared_->cbo_light.get(),
            };
            context->PSSetConstantBuffers(2, 2, ps_cbo);

            // OM
            FLOAT const blend_factor[4]{};
            context->OMSetBlendState(shared_->state_blend_alpha.get(), blend_factor, D3D11_DEFAULT_SAMPLE_MASK);
        };
        auto set_alpha_mode_blend_overlay = [&](ModelBlock& mblock, float const exclude_value)
        {
            // PS

            FLOAT const alpha[8] = {
                    mblock.base_color.x, mblock.base_color.y, mblock.base_color.z, mblock.base_color.w,
                    exclude_value, 0.0f, 0.0f, 0.0f,
            };
            context->UpdateSubresource(shared_->cbo_alpha.get(), 0, NULL, alpha, 0, 0);
            ID3D11Buffer* ps_cbo[2] = {
                // camera position and look to vector are setup by Renderer at register(b0)
                shared_->cbo_alpha.get(),
                shared_->cbo_light.get(),
            };
            context->PSSetConstantBuffers(2, 2, ps_cbo);
            if (mblock.image)
            {
                if (mblock.color_buffer)
                    context->PSSetShader(shared_->shader_pixel_inv_alpha_vc[IDX(fog)].get(), NULL, 0);
                else
                    context->PSSetShader(shared_->shader_pixel_inv_alpha[IDX(fog)].get(), NULL, 0);
            }
            else
            {
                if (mblock.color_buffer)
                    context->PSSetShader(shared_->shader_pixel_inv_alpha_nt_vc[IDX(fog)].get(), NULL, 0);
                else
                    context->PSSetShader(shared_->shader_pixel_inv_alpha_nt[IDX(fog)].get(), NULL, 0);
            }

            // OM
            context->OMSetDepthStencilState(shared_->state_ds.get(), D3D11_DEFAULT_STENCIL_REFERENCE);
            FLOAT const blend_factor[4]{};
            context->OMSetBlendState(shared_->state_blend_alpha.get(), blend_factor, D3D11_DEFAULT_SAMPLE_MASK);
        };
        auto set_alpha_mode_screen_door = [&](ModelBlock& mblock)
        {
            // PS

            FLOAT const alpha[8] = {
                    mblock.base_color.x, mblock.base_color.y, mblock.base_color.z, mblock.base_color.w,
                    0.5f, 0.0f, 0.0f, 0.0f,
            };
            context->UpdateSubresource(shared_->cbo_alpha.get(), 0, NULL, alpha, 0, 0);
            ID3D11Buffer* ps_cbo[2] = {
                // camera position and look to vector are setup by Renderer at register(b0)
                shared_->cbo_alpha.get(),
                shared_->cbo_light.get(),
            };
            context->PSSetConstantBuffers(2, 2, ps_cbo);
            if (mblock.image)
            {
                if (mblock.color_buffer)
                    context->PSSetShader(shared_->shader_pixel_sd_vc[IDX(fog)].get(), NULL, 0);
                else
                    context->PSSetShader(shared_->shader_pixel_sd[IDX(fog)].get(), NULL, 0);
            }
            else
            {
                if (mblock.color_buffer)
                    context->PSSetShader(shared_->shader_pixel_sd_nt_vc[IDX(fog)].get(), NULL, 0);
                else
                    context->PSSetShader(shared_->shader_pixel_sd_nt[IDX(fog)].get(), NULL, 0);
            }

            // OM
            context->OMSetDepthStencilState(shared_->state_ds.get(), D3D11_DEFAULT_STENCIL_REFERENCE);
            FLOAT const blend_factor[4]{};
            context->OMSetBlendState(shared_->state_blend.get(), blend_factor, D3D11_DEFAULT_SAMPLE_MASK);
        };
        auto set_state_from_block = [&](ModelBlock& mblock)
        {
            set_state_matrix_from_block(mblock);

            // IA

            context->IASetPrimitiveTopology(mblock.primitive_topology);
            ID3D11Buffer* vbo[4] = { mblock.vertex_buffer.get(), mblock.normal_buffer.get(), mblock.uv_buffer.get(), mblock.color_buffer.get() };
            UINT stride[4] = { 3 * sizeof(float), 3 * sizeof(float), 2 * sizeof(float), 3 * sizeof(float) };
            UINT offset[4] = { 0, 0, 0, 0 };
            context->IASetVertexBuffers(0, 4, vbo, stride, offset);
            context->IASetIndexBuffer(mblock.index_buffer.get(), mblock.index_format, 0);

            // VS

            upload_local_world_matrix(mblock);
            ID3D11Buffer* vs_cbo[1] = {
                // view-projection matrix setup by Renderer at register(b0)
                shared_->cbo_mlw.get(),
            };
            context->VSSetConstantBuffers(1, 1, vs_cbo);

            // RS

            if (mblock.double_side)
            {
                context->RSSetState(shared_->state_rs_cull_none.get());
            }
            else
            {
                context->RSSetState(shared_->state_rs_cull_back.get());
            }

            // PS

            ID3D11SamplerState* ps_samp[1] = { mblock.sampler.get() };
            context->PSSetSamplers(0, 1, ps_samp);
            ID3D11ShaderResourceView* ps_srv[1] = { mblock.image.get() };
            context->PSSetShaderResources(0, 1, ps_srv);

            // OM

            context->OMSetDepthStencilState(shared_->state_ds.get(), D3D11_DEFAULT_STENCIL_REFERENCE);

            // other
            if (mblock.alpha_blend) {
                //set_alpha_mode_blend(mblock);
                set_alpha_mode_screen_door(mblock);
            }
            else if (mblock.alpha_mask) {
                set_alpha_mode_mask(mblock);
            }
            else {
                set_alpha_mode_opaque(mblock);
            }
        };
        auto draw_block = [&](ModelBlock& mblock)
        {
            if (mblock.index_buffer)
                m_device->getCommandbuffer()->drawIndexed(mblock.draw_count, 0, 0);
            else
                m_device->getCommandbuffer()->draw(mblock.draw_count, 0);
        };
        auto clear_state = [&]()
        {
            // IA

            ID3D11Buffer* vbo_null[4] = { NULL, NULL, NULL, NULL };
            UINT stride_zero[4] = { 0, 0, 0, 0 };
            UINT offset_zero[4] = { 0, 0, 0, 0 };
            context->IASetVertexBuffers(0, 4, vbo_null, stride_zero, offset_zero);
            context->IASetIndexBuffer(NULL, DXGI_FORMAT_R16_UINT, 0);

            // VS

            ID3D11Buffer* vs_cbo[1] = { NULL };
            context->VSSetConstantBuffers(1, 1, vs_cbo);

            // PS

            ID3D11SamplerState* ps_samp[1] = { NULL };
            context->PSSetSamplers(0, 1, ps_samp);
            ID3D11ShaderResourceView* ps_srv[1] = { NULL };
            context->PSSetShaderResources(0, 1, ps_srv);
            ID3D11Buffer* ps_cbo[2] = { NULL, NULL };
            context->PSSetConstantBuffers(2, 2, ps_cbo);
        };

        // pass 1 opaque object

        for (auto& mblock : model_block)
        {
            if (!mblock.alpha_mask && !mblock.alpha_blend)
            {
                set_state_from_block(mblock);
                draw_block(mblock);
            }
        }

        // pass 2 alpha mask object

        for (auto& mblock : model_block)
        {
            if (mblock.alpha_mask)
            {
                set_state_from_block(mblock);
                draw_block(mblock);
            }
        }

        // pass 3 alpha blend object

        for (auto& mblock : model_block)
        {
            if (mblock.alpha_blend)
            {
                set_state_from_block(mblock);
                draw_block(mblock);
            }
        }
        
        // unbind

        clear_state();
    }

    Model_D3D11::Model_D3D11(GraphicsDevice* p_device, ModelSharedComponent_D3D11* p_model_shared, StringView path)
        : m_device(p_device)
        , shared_(p_model_shared)
        , gltf_path(path)
    {
        t_scale_ = DirectX::XMMatrixIdentity();
        t_trans_ = DirectX::XMMatrixIdentity();
        t_mbrot_ = DirectX::XMMatrixIdentity();
        if (!createResources())
            throw std::runtime_error("Model_D3D11::Model_D3D11");
        m_device->addEventListener(this);
    }
    Model_D3D11::~Model_D3D11()
    {
        m_device->removeEventListener(this);
    }
}
