#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
    enum class GraphicsFilterType : int32_t {
        point = 0,
        linear = 1,
    };

#define FILTER_TYPE_MASK 0x3
#define MIN_FILTER_SHIFT 4
#define MAG_FILTER_SHIFT 2
#define MIP_FILTER_SHIFT 0
#define ANISOTROPIC_FILTER_BIT 0x40

#define ENCODE_FILTER(MIN, MAG, MIP) \
    ((static_cast<int32_t>(MIN) & FILTER_TYPE_MASK) << MIN_FILTER_SHIFT) | \
    ((static_cast<int32_t>(MAG) & FILTER_TYPE_MASK) << MAG_FILTER_SHIFT) | \
    ((static_cast<int32_t>(MIP) & FILTER_TYPE_MASK) << MIP_FILTER_SHIFT)

#define FILTER_VALUE(MIN, MAG, MIP) min_##MIN##_mag_##MAG##_mip_##MIP = ENCODE_FILTER(GraphicsFilterType::##MIN, GraphicsFilterType::##MAG, GraphicsFilterType::##MIP)

    enum class GraphicsFilter : int32_t {
        FILTER_VALUE(point , point , point ),
        FILTER_VALUE(linear, point , point ),
        FILTER_VALUE(point , linear, point ),
        FILTER_VALUE(point , point , linear),
        FILTER_VALUE(linear, linear, point ),
        FILTER_VALUE(linear, point , linear),
        FILTER_VALUE(point , linear, linear),
        FILTER_VALUE(linear, linear, linear),

        point = min_point_mag_point_mip_point,
        linear = min_linear_mag_linear_mip_linear,
        anisotropic = ENCODE_FILTER(GraphicsFilterType::linear, GraphicsFilterType::linear, GraphicsFilterType::linear) | ANISOTROPIC_FILTER_BIT,
    };

#undef FILTER_TYPE_MASK
#undef MIN_FILTER_SHIFT
#undef MAG_FILTER_SHIFT
#undef MIP_FILTER_SHIFT
#undef ANISOTROPIC_FILTER_BIT

#undef ENCODE_FILTER

#undef FILTER_VALUE

    enum class GraphicsTextureAddressMode : int32_t {
        wrap = 1,
        clamp = 3,
        border = 4,
    };

    struct GraphicsSamplerInfo {
        GraphicsFilter filter;
        GraphicsTextureAddressMode address_u;
        GraphicsTextureAddressMode address_v;
        GraphicsTextureAddressMode address_w;
        float mip_lod_bias;
        uint32_t max_anisotropy;
        float min_lod;
        float max_lod;
    };

    CORE_INTERFACE IGraphicsSampler : IReferenceCounted {
        virtual void* getNativeHandle() const noexcept = 0;

        virtual const GraphicsSamplerInfo* getInfo() const noexcept = 0;
    };

    CORE_INTERFACE_ID(IGraphicsSampler, "446fb2a1-0bb3-50c1-bb64-4de769036200");
}
