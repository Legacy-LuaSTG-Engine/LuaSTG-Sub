#pragma once

#if (defined(TRACY_ENABLE) && defined(__TRACY_HPP__))

#include <source_location>

#define tracy_connect_part_(x, y) x##y
#define tracy_connect_part(x, y) tracy_connect_part_(x, y)

#define tracy_name_with_line(name) tracy_connect_part(name, __LINE__)

#define tracy_source_location_data_with_name(var, name) static constexpr tracy::SourceLocationData tracy_name_with_line(var) { name, std::source_location::current().function_name(),  std::source_location::current().file_name(), (uint32_t)std::source_location::current().line(), 0 }

#define tracy_zone_scoped                               tracy_source_location_data_with_name(__tracy_source_location, nullptr); tracy::ScopedZone tracy_name_with_line(___tracy_scoped_zone) ( &tracy_name_with_line(__tracy_source_location), true)
#define tracy_zone_scoped_with_name(name)               tracy_source_location_data_with_name(__tracy_source_location, name);    tracy::ScopedZone tracy_name_with_line(___tracy_scoped_zone) ( &tracy_name_with_line(__tracy_source_location), true)

#define tracy_d3d11_context_zone(ctx, name)             tracy_source_location_data_with_name(__tracy_gpu_d3d11_source_location, name); tracy::D3D11ZoneScope tracy_name_with_line(___tracy_gpu_d3d11_zone) ( ctx, &tracy_name_with_line(__tracy_gpu_d3d11_source_location), true )

#else

#define tracy_zone_scoped
#define tracy_zone_scoped_with_name(name)

#define tracy_d3d11_context_zone(ctx, name)

#endif

#if (defined(TRACY_ENABLE) && defined(__TRACYD3D11_HPP__))

using tracy_d3d11_context_t = TracyD3D11Ctx;
inline tracy_d3d11_context_t tracy_d3d11_context_create(ID3D11Device* device, ID3D11DeviceContext* device_context) { return TracyD3D11Context(device, device_context); }
inline void tracy_d3d11_context_destroy(tracy_d3d11_context_t ctx) { TracyD3D11Destroy(ctx); }

#else

using tracy_d3d11_context_t = void*;
inline tracy_d3d11_context_t tracy_d3d11_context_create(void* device, void* device_context) { return nullptr; }
inline void tracy_d3d11_context_destroy(tracy_d3d11_context_t ctx) {}

#endif
