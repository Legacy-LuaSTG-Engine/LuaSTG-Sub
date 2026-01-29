#pragma once
#include "d3d11/pch.h"

namespace {
    using std::string_view_literals::operator ""sv;

    enum class Semantic {
        boolean,
        integer,
        support,
        answer,
        data_size,
    };

    std::string_view toStringView(const bool v, Semantic semantic = Semantic::boolean) {
        switch (semantic) {
        default:
        case Semantic::boolean: return v ? "true"sv : "false"sv;
        case Semantic::support: return v ? "supported"sv : "not supported"sv;
        case Semantic::answer:  return v ? "yes"sv : "no"sv;
        }
    }

    std::string toString(const uint64_t v, Semantic semantic = Semantic::integer) {
        switch (semantic) {
        default:
        case Semantic::integer: return std::to_string(v);
        case Semantic::data_size:
            {
                constexpr uint64_t KiB{ 1024ull };
                constexpr uint64_t MiB{ 1024ull * 1024ull };
                constexpr uint64_t GiB{ 1024ull * 1024ull * 1024ull };
                constexpr uint64_t TiB{ 1024ull * 1024ull * 1024ull * 1024ull };
                if (v < KiB) {
                    return std::format("{}B"sv, v);
                }
                else if (v < MiB) {
                    return std::format("{:.3f}KiB"sv, static_cast<double>(v) / static_cast<double>(KiB));
                }
                else if (v < GiB) {
                    return std::format("{:.3f}MiB"sv, static_cast<double>(v) / static_cast<double>(MiB));
                }
                else if (v < TiB) {
                    return std::format("{:.3f}GiB"sv, static_cast<double>(v) / static_cast<double>(GiB));
                }
                else {
                    return std::format("{:.3f}TiB"sv, static_cast<double>(v) / static_cast<double>(TiB));
                }
            }
        }
    }

    std::string_view toStringView(const D3D_FEATURE_LEVEL v) {
        switch (v) {
        case D3D_FEATURE_LEVEL_1_0_GENERIC: return "1.0 (generic)"sv;
        case D3D_FEATURE_LEVEL_1_0_CORE:    return "1.0 (core)"sv;
        case D3D_FEATURE_LEVEL_9_1:         return "9.1"sv;
        case D3D_FEATURE_LEVEL_9_2:         return "9.2"sv;
        case D3D_FEATURE_LEVEL_9_3:         return "9.3"sv;
        case D3D_FEATURE_LEVEL_10_0:        return "10.0"sv;
        case D3D_FEATURE_LEVEL_10_1:        return "10.1"sv;
        case D3D_FEATURE_LEVEL_11_0:        return "11.0"sv;
        case D3D_FEATURE_LEVEL_11_1:        return "11.1"sv;
        case D3D_FEATURE_LEVEL_12_0:        return "12.0"sv;
        case D3D_FEATURE_LEVEL_12_1:        return "12.1"sv;
        case D3D_FEATURE_LEVEL_12_2:        return "12.2"sv;
        default:                            return "unknown"sv;
        }
    }

    std::string_view toStringView(const DXGI_GRAPHICS_PREEMPTION_GRANULARITY v) {
        switch (v) {
        case DXGI_GRAPHICS_PREEMPTION_DMA_BUFFER_BOUNDARY:  return "DMA_BUFFER_BOUNDARY"sv;
        case DXGI_GRAPHICS_PREEMPTION_PRIMITIVE_BOUNDARY:   return "PRIMITIVE_BOUNDARY"sv;
        case DXGI_GRAPHICS_PREEMPTION_TRIANGLE_BOUNDARY:    return "TRIANGLE_BOUNDARY"sv;
        case DXGI_GRAPHICS_PREEMPTION_PIXEL_BOUNDARY:       return "PIXEL_BOUNDARY"sv;
        case DXGI_GRAPHICS_PREEMPTION_INSTRUCTION_BOUNDARY: return "INSTRUCTION_BOUNDARY"sv;
        default:                            return "unknown"sv;
        }
    }

    std::string_view toStringView(const DXGI_COMPUTE_PREEMPTION_GRANULARITY v) {
        switch (v) {
        case DXGI_COMPUTE_PREEMPTION_DMA_BUFFER_BOUNDARY:   return "DMA_BUFFER_BOUNDARY"sv;
        case DXGI_COMPUTE_PREEMPTION_DISPATCH_BOUNDARY:     return "DISPATCH_BOUNDARY"sv;
        case DXGI_COMPUTE_PREEMPTION_THREAD_GROUP_BOUNDARY: return "THREAD_GROUP_BOUNDARY"sv;
        case DXGI_COMPUTE_PREEMPTION_THREAD_BOUNDARY:       return "THREAD_BOUNDARY"sv;
        case DXGI_COMPUTE_PREEMPTION_INSTRUCTION_BOUNDARY:  return "INSTRUCTION_BOUNDARY"sv;
        default:                            return "unknown"sv;
        }
    }

    std::string toString(const DXGI_ADAPTER_FLAG3 v) {
        std::string s;
        if (v & DXGI_ADAPTER_FLAG3_REMOTE) s.append("REMOTE "sv);
        if (v & DXGI_ADAPTER_FLAG3_SOFTWARE) s.append("SOFTWARE "sv);
        if (v & DXGI_ADAPTER_FLAG3_ACG_COMPATIBLE) s.append("ACG_COMPATIBLE "sv);
        if (v & DXGI_ADAPTER_FLAG3_SUPPORT_MONITORED_FENCES) s.append("SUPPORT_MONITORED_FENCES "sv);
        if (v & DXGI_ADAPTER_FLAG3_SUPPORT_NON_MONITORED_FENCES) s.append("SUPPORT_NON_MONITORED_FENCES "sv);
        if (v & DXGI_ADAPTER_FLAG3_KEYED_MUTEX_CONFORMANCE) s.append("KEYED_MUTEX_CONFORMANCE "sv);
        if (!s.empty()) if (s.back() == ' ') s.pop_back();
        return s;
    }
}
