#include "d3d11/FormatHelper.hpp"

namespace d3d11 {
    DXGI_FORMAT toFormat(const core::GraphicsFormat format) {
        switch (format) {
        case core::GraphicsFormat::unknown: return DXGI_FORMAT_UNKNOWN;

        // float

        case core::GraphicsFormat::r32_g32_b32_a32_float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case core::GraphicsFormat::r32_g32_b32_float: return DXGI_FORMAT_R32G32B32_FLOAT;
        case core::GraphicsFormat::r32_g32_float: return DXGI_FORMAT_R32G32_FLOAT;
        case core::GraphicsFormat::r32_float: return DXGI_FORMAT_R32_FLOAT;

        case core::GraphicsFormat::r16_g16_b16_a16_float: return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case core::GraphicsFormat::r16_g16_float: return DXGI_FORMAT_R16G16_FLOAT;
        case core::GraphicsFormat::r16_float: return DXGI_FORMAT_R16_FLOAT;

        // unsigned float

        case core::GraphicsFormat::r11_g11_b10_ufloat: return DXGI_FORMAT_R11G11B10_FLOAT;

        // int

        case core::GraphicsFormat::r32_g32_b32_a32_int: return DXGI_FORMAT_R32G32B32A32_SINT;
        case core::GraphicsFormat::r32_g32_b32_int: return DXGI_FORMAT_R32G32B32_SINT;
        case core::GraphicsFormat::r32_g32_int: return DXGI_FORMAT_R32G32_SINT;
        case core::GraphicsFormat::r32_int: return DXGI_FORMAT_R32_SINT;

        case core::GraphicsFormat::r16_g16_b16_a16_int: return DXGI_FORMAT_R16G16B16A16_SINT;
        case core::GraphicsFormat::r16_g16_int: return DXGI_FORMAT_R16G16_SINT;
        case core::GraphicsFormat::r16_int: return DXGI_FORMAT_R16_SINT;

        case core::GraphicsFormat::r8_g8_b8_a8_int: return DXGI_FORMAT_R8G8B8A8_SINT;
        case core::GraphicsFormat::r8_g8_int: return DXGI_FORMAT_R8G8_SINT;
        case core::GraphicsFormat::r8_int: return DXGI_FORMAT_R8_SINT;

        // uint

        case core::GraphicsFormat::r32_g32_b32_a32_uint: return DXGI_FORMAT_R32G32B32A32_UINT;
        case core::GraphicsFormat::r32_g32_b32_uint: return DXGI_FORMAT_R32G32B32_UINT;
        case core::GraphicsFormat::r32_g32_uint: return DXGI_FORMAT_R32G32_UINT;
        case core::GraphicsFormat::r32_uint: return DXGI_FORMAT_R32_UINT;

        case core::GraphicsFormat::r16_g16_b16_a16_uint: return DXGI_FORMAT_R16G16B16A16_UINT;
        case core::GraphicsFormat::r16_g16_uint: return DXGI_FORMAT_R16G16_UINT;
        case core::GraphicsFormat::r16_uint: return DXGI_FORMAT_R16_UINT;

        case core::GraphicsFormat::r10_g10_b10_a2_uint: return DXGI_FORMAT_R10G10B10A2_UINT;

        case core::GraphicsFormat::r8_g8_b8_a8_uint: return DXGI_FORMAT_R8G8B8A8_UINT;
        case core::GraphicsFormat::r8_g8_uint: return DXGI_FORMAT_R8G8_UINT;
        case core::GraphicsFormat::r8_uint: return DXGI_FORMAT_R8_UINT;

        // signed normalized

        case core::GraphicsFormat::r16_g16_b16_a16_snorm: return DXGI_FORMAT_R16G16B16A16_SNORM;
        case core::GraphicsFormat::r16_g16_snorm: return DXGI_FORMAT_R16G16_SNORM;
        case core::GraphicsFormat::r16_snorm: return DXGI_FORMAT_R16_SNORM;

        case core::GraphicsFormat::r8_g8_b8_a8_snorm: return DXGI_FORMAT_R8G8B8A8_SNORM;
        case core::GraphicsFormat::r8_g8_snorm: return DXGI_FORMAT_R8G8_SNORM;
        case core::GraphicsFormat::r8_snorm: return DXGI_FORMAT_R8_SNORM;

        // unsigned normalized

        case core::GraphicsFormat::r16_g16_b16_a16_unorm: return DXGI_FORMAT_R16G16B16A16_UNORM;
        case core::GraphicsFormat::r16_g16_unorm: return DXGI_FORMAT_R16G16_UNORM;
        case core::GraphicsFormat::r16_unorm: return DXGI_FORMAT_R16_UNORM;

        case core::GraphicsFormat::r10_g10_b10_a2_unorm: return DXGI_FORMAT_R10G10B10A2_UNORM;

        case core::GraphicsFormat::r8_g8_b8_a8_unorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case core::GraphicsFormat::r8_g8_unorm: return DXGI_FORMAT_R8G8_UNORM;
        case core::GraphicsFormat::r8_unorm: return DXGI_FORMAT_R8_UNORM;
        case core::GraphicsFormat::a8_unorm: return DXGI_FORMAT_A8_UNORM;

        case core::GraphicsFormat::b8_g8_r8_a8_unorm: return DXGI_FORMAT_B8G8R8A8_UNORM;

        case core::GraphicsFormat::b5_g6_r5_unorm: return DXGI_FORMAT_B5G6R5_UNORM;
        case core::GraphicsFormat::b5_g5_r5_a1_unorm: return DXGI_FORMAT_B5G5R5A1_UNORM;

        // sRGB

        case core::GraphicsFormat::r8_g8_b8_a8_unorm_srgb: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case core::GraphicsFormat::b8_g8_r8_a8_unorm_srgb: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

        // depth stencil

        case core::GraphicsFormat::d32_float: return DXGI_FORMAT_D32_FLOAT;
        case core::GraphicsFormat::d32_float_s8_x24_uint: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

        case core::GraphicsFormat::d24_unorm_s8_uint: return DXGI_FORMAT_D24_UNORM_S8_UINT;

        case core::GraphicsFormat::d16_unorm: return DXGI_FORMAT_D16_UNORM;

        // bc

        case core::GraphicsFormat::bc1_unorm: return DXGI_FORMAT_BC1_UNORM;
        case core::GraphicsFormat::bc1_unorm_srgb: return DXGI_FORMAT_BC1_UNORM_SRGB;

        case core::GraphicsFormat::bc2_unorm: return DXGI_FORMAT_BC2_UNORM;
        case core::GraphicsFormat::bc2_unorm_srgb: return DXGI_FORMAT_BC2_UNORM_SRGB;

        case core::GraphicsFormat::bc3_unorm: return DXGI_FORMAT_BC3_UNORM;
        case core::GraphicsFormat::bc3_unorm_srgb: return DXGI_FORMAT_BC3_UNORM_SRGB;

        case core::GraphicsFormat::bc4_unorm: return DXGI_FORMAT_BC4_UNORM;
        case core::GraphicsFormat::bc4_snorm: return DXGI_FORMAT_BC4_SNORM;

        case core::GraphicsFormat::bc5_unorm: return DXGI_FORMAT_BC5_UNORM;
        case core::GraphicsFormat::bc5_snorm: return DXGI_FORMAT_BC5_SNORM;

        case core::GraphicsFormat::bc6h_uf16: return DXGI_FORMAT_BC6H_UF16;
        case core::GraphicsFormat::bc6h_sf16: return DXGI_FORMAT_BC6H_SF16;

        case core::GraphicsFormat::bc7_unorm: return DXGI_FORMAT_BC7_UNORM;
        case core::GraphicsFormat::bc7_unorm_srgb: return DXGI_FORMAT_BC7_UNORM_SRGB;

        // other

        case core::GraphicsFormat::r32_g32_b32_a32_typeless: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
        case core::GraphicsFormat::r32_g32_b32_typeless: return DXGI_FORMAT_R32G32B32_TYPELESS;
        case core::GraphicsFormat::r32_g32_typeless: return DXGI_FORMAT_R32G32_TYPELESS;
        case core::GraphicsFormat::r32_g8_x24_typeless: return DXGI_FORMAT_R32G8X24_TYPELESS;
        case core::GraphicsFormat::r32_float_x8_x24_typeless: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        case core::GraphicsFormat::x32_typeless_g8_x24_uint: return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
        case core::GraphicsFormat::r32_typeless: return DXGI_FORMAT_R32_TYPELESS;

        case core::GraphicsFormat::r24_g8_typeless: return DXGI_FORMAT_R24G8_TYPELESS;
        case core::GraphicsFormat::r24_unorm_x8_typeless: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        case core::GraphicsFormat::x24_typeless_g8_uint: return DXGI_FORMAT_X24_TYPELESS_G8_UINT;

        case core::GraphicsFormat::r16_g16_b16_a16_typeless: return DXGI_FORMAT_R16G16B16A16_TYPELESS;
        case core::GraphicsFormat::r16_g16_typeless: return DXGI_FORMAT_R16G16_TYPELESS;
        case core::GraphicsFormat::r16_typeless: return DXGI_FORMAT_R16_TYPELESS;

        case core::GraphicsFormat::r10_g10_b10_a2_typeless: return DXGI_FORMAT_R10G10B10A2_TYPELESS;
        case core::GraphicsFormat::r10_g10_b10_xr_bias_a2_unorm: return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

        case core::GraphicsFormat::r8_g8_b8_a8_typeless: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
        case core::GraphicsFormat::r8_g8_typeless: return DXGI_FORMAT_R8G8_TYPELESS;
        case core::GraphicsFormat::r8_typeless: return DXGI_FORMAT_R8_TYPELESS;

        case core::GraphicsFormat::b8_g8_r8_a8_typeless: return DXGI_FORMAT_B8G8R8A8_TYPELESS;
        case core::GraphicsFormat::b8_g8_r8_x8_typeless: return DXGI_FORMAT_B8G8R8X8_TYPELESS;

        case core::GraphicsFormat::bc1_typeless: return DXGI_FORMAT_BC1_TYPELESS;
        case core::GraphicsFormat::bc2_typeless: return DXGI_FORMAT_BC2_TYPELESS;
        case core::GraphicsFormat::bc3_typeless: return DXGI_FORMAT_BC3_TYPELESS;
        case core::GraphicsFormat::bc4_typeless: return DXGI_FORMAT_BC4_TYPELESS;
        case core::GraphicsFormat::bc5_typeless: return DXGI_FORMAT_BC5_TYPELESS;
        case core::GraphicsFormat::bc6h_typeless: return DXGI_FORMAT_BC6H_TYPELESS;
        case core::GraphicsFormat::bc7_typeless: return DXGI_FORMAT_BC7_TYPELESS;

        // fallback

        default: assert(false); return DXGI_FORMAT_UNKNOWN;
        };
    }
}

// [x] DXGI_FORMAT_R32G32B32A32_TYPELESS
// [x] DXGI_FORMAT_R32G32B32A32_FLOAT
// [x] DXGI_FORMAT_R32G32B32A32_UINT
// [x] DXGI_FORMAT_R32G32B32A32_SINT
// [x] DXGI_FORMAT_R32G32B32_TYPELESS
// [x] DXGI_FORMAT_R32G32B32_FLOAT
// [x] DXGI_FORMAT_R32G32B32_UINT
// [x] DXGI_FORMAT_R32G32B32_SINT
// [x] DXGI_FORMAT_R16G16B16A16_TYPELESS
// [x] DXGI_FORMAT_R16G16B16A16_FLOAT
// [x] DXGI_FORMAT_R16G16B16A16_UNORM
// [x] DXGI_FORMAT_R16G16B16A16_UINT
// [x] DXGI_FORMAT_R16G16B16A16_SNORM
// [x] DXGI_FORMAT_R16G16B16A16_SINT
// [x] DXGI_FORMAT_R32G32_TYPELESS
// [x] DXGI_FORMAT_R32G32_FLOAT
// [x] DXGI_FORMAT_R32G32_UINT
// [x] DXGI_FORMAT_R32G32_SINT
// [x] DXGI_FORMAT_R32G8X24_TYPELESS
// [x] DXGI_FORMAT_D32_FLOAT_S8X24_UINT
// [x] DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS
// [x] DXGI_FORMAT_X32_TYPELESS_G8X24_UINT
// [x] DXGI_FORMAT_R10G10B10A2_TYPELESS
// [x] DXGI_FORMAT_R10G10B10A2_UNORM
// [x] DXGI_FORMAT_R10G10B10A2_UINT
// [x] DXGI_FORMAT_R11G11B10_FLOAT
// [x] DXGI_FORMAT_R8G8B8A8_TYPELESS
// [x] DXGI_FORMAT_R8G8B8A8_UNORM
// [x] DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
// [x] DXGI_FORMAT_R8G8B8A8_UINT
// [x] DXGI_FORMAT_R8G8B8A8_SNORM
// [x] DXGI_FORMAT_R8G8B8A8_SINT
// [x] DXGI_FORMAT_R16G16_TYPELESS
// [x] DXGI_FORMAT_R16G16_FLOAT
// [x] DXGI_FORMAT_R16G16_UNORM
// [x] DXGI_FORMAT_R16G16_UINT
// [x] DXGI_FORMAT_R16G16_SNORM
// [x] DXGI_FORMAT_R16G16_SINT
// [x] DXGI_FORMAT_R32_TYPELESS
// [x] DXGI_FORMAT_D32_FLOAT
// [x] DXGI_FORMAT_R32_FLOAT
// [x] DXGI_FORMAT_R32_UINT
// [x] DXGI_FORMAT_R32_SINT
// [x] DXGI_FORMAT_R24G8_TYPELESS
// [x] DXGI_FORMAT_D24_UNORM_S8_UINT
// [x] DXGI_FORMAT_R24_UNORM_X8_TYPELESS
// [x] DXGI_FORMAT_X24_TYPELESS_G8_UINT
// [x] DXGI_FORMAT_R8G8_TYPELESS
// [x] DXGI_FORMAT_R8G8_UNORM
// [x] DXGI_FORMAT_R8G8_UINT
// [x] DXGI_FORMAT_R8G8_SNORM
// [x] DXGI_FORMAT_R8G8_SINT
// [x] DXGI_FORMAT_R16_TYPELESS
// [x] DXGI_FORMAT_R16_FLOAT
// [x] DXGI_FORMAT_D16_UNORM
// [x] DXGI_FORMAT_R16_UNORM
// [x] DXGI_FORMAT_R16_UINT
// [x] DXGI_FORMAT_R16_SNORM
// [x] DXGI_FORMAT_R16_SINT
// [x] DXGI_FORMAT_R8_TYPELESS
// [x] DXGI_FORMAT_R8_UNORM
// [x] DXGI_FORMAT_R8_UINT
// [x] DXGI_FORMAT_R8_SNORM
// [x] DXGI_FORMAT_R8_SINT
// [x] DXGI_FORMAT_A8_UNORM
// [ ] DXGI_FORMAT_R1_UNORM
// [ ] DXGI_FORMAT_R9G9B9E5_SHAREDEXP
// [ ] DXGI_FORMAT_R8G8_B8G8_UNORM
// [ ] DXGI_FORMAT_G8R8_G8B8_UNORM
// [x] DXGI_FORMAT_BC1_TYPELESS
// [x] DXGI_FORMAT_BC1_UNORM
// [x] DXGI_FORMAT_BC1_UNORM_SRGB
// [x] DXGI_FORMAT_BC2_TYPELESS
// [x] DXGI_FORMAT_BC2_UNORM
// [x] DXGI_FORMAT_BC2_UNORM_SRGB
// [x] DXGI_FORMAT_BC3_TYPELESS
// [x] DXGI_FORMAT_BC3_UNORM
// [x] DXGI_FORMAT_BC3_UNORM_SRGB
// [x] DXGI_FORMAT_BC4_TYPELESS
// [x] DXGI_FORMAT_BC4_UNORM
// [x] DXGI_FORMAT_BC4_SNORM
// [x] DXGI_FORMAT_BC5_TYPELESS
// [x] DXGI_FORMAT_BC5_UNORM
// [x] DXGI_FORMAT_BC5_SNORM
// [x] DXGI_FORMAT_B5G6R5_UNORM
// [x] DXGI_FORMAT_B5G5R5A1_UNORM
// [x] DXGI_FORMAT_B8G8R8A8_UNORM
// [ ] DXGI_FORMAT_B8G8R8X8_UNORM
// [x] DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM
// [x] DXGI_FORMAT_B8G8R8A8_TYPELESS
// [x] DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
// [x] DXGI_FORMAT_B8G8R8X8_TYPELESS
// [ ] DXGI_FORMAT_B8G8R8X8_UNORM_SRGB
// [x] DXGI_FORMAT_BC6H_TYPELESS
// [x] DXGI_FORMAT_BC6H_UF16
// [x] DXGI_FORMAT_BC6H_SF16
// [x] DXGI_FORMAT_BC7_TYPELESS
// [x] DXGI_FORMAT_BC7_UNORM
// [x] DXGI_FORMAT_BC7_UNORM_SRGB
// 
// [ ] DXGI_FORMAT_AYUV
// [ ] DXGI_FORMAT_Y410
// [ ] DXGI_FORMAT_Y416
// [ ] DXGI_FORMAT_NV12
// [ ] DXGI_FORMAT_P010
// [ ] DXGI_FORMAT_P016
// [ ] DXGI_FORMAT_420_OPAQUE
// [ ] DXGI_FORMAT_YUY2
// [ ] DXGI_FORMAT_Y210
// [ ] DXGI_FORMAT_Y216
// [ ] DXGI_FORMAT_NV11
// [ ] DXGI_FORMAT_AI44
// [ ] DXGI_FORMAT_IA44
// [ ] DXGI_FORMAT_P8
// [ ] DXGI_FORMAT_A8P8
// [ ] DXGI_FORMAT_B4G4R4A4_UNORM
// 
// [ ] DXGI_FORMAT_P208
// [ ] DXGI_FORMAT_V208
// [ ] DXGI_FORMAT_V408
// 
// [ ] DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE
// [ ] DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE
// 
// [ ] DXGI_FORMAT_A4B4G4R4_UNORM
