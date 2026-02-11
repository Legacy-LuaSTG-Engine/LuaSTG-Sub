#pragma once
#include <cstdint>

namespace d3d11 {
    static constexpr uint32_t vertex_shader_constant_buffer_slot_view_projection_matrix = 0;
    static constexpr uint32_t vertex_shader_constant_buffer_slot_world_matrix = 1;

    static constexpr uint32_t pixel_shader_constant_buffer_slot_user_data = 0;
    static constexpr uint32_t pixel_shader_constant_buffer_slot_camera_position = 0;
    static constexpr uint32_t pixel_shader_constant_buffer_slot_fog_parameter = 1;
}
