#pragma once

namespace core::Graphics::Direct3D11 {
	class Constants {
	public:
		static constexpr uint32_t vertex_shader_stage_constant_buffer_slot_view_projection_matrix = 0;
		static constexpr uint32_t vertex_shader_stage_constant_buffer_slot_world_matrix = 1;

		static_assert(vertex_shader_stage_constant_buffer_slot_view_projection_matrix < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
		static_assert(vertex_shader_stage_constant_buffer_slot_world_matrix < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);

		static constexpr uint32_t pixel_shader_stage_constant_buffer_slot_user_data = 0; // TODO: native! too young too simple!
		static constexpr uint32_t pixel_shader_stage_constant_buffer_slot_camera_position = 0;
		static constexpr uint32_t pixel_shader_stage_constant_buffer_slot_fog_parameter = 1;

		static_assert(pixel_shader_stage_constant_buffer_slot_user_data < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
		static_assert(pixel_shader_stage_constant_buffer_slot_camera_position < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
		static_assert(pixel_shader_stage_constant_buffer_slot_fog_parameter < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
	};
}
