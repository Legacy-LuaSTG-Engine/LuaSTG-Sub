#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"
#include <array>
#include <vector>
#include <memory_resource>

using std::string_view_literals::operator ""sv;

namespace {
	template<typename T, typename F>
	int SliderType(lua_State* const vm, std::string_view const default_format, F const f) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto v = ctx.get_value<T>(2);
		auto const v_min = ctx.get_value<T>(3);
		auto const v_max = ctx.get_value<T>(4);
		auto const format = ctx.get_value<std::string_view>(5, default_format);
		auto const flags = ctx.get_value<ImGuiSliderFlags>(6, 0);
		auto const result = f(label.data(), &v, v_min, v_max, format.data(), flags);
		ctx.push_value(result);
		ctx.push_value(v);
		return 2;
	}
	template<typename T, size_t N, typename F>
	int SliderTypeN(lua_State* const vm, std::string_view const default_format, F const f) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		T v[N]{};
		for (int i = 0; i < static_cast<int>(N); i += 1) {
			v[i] = ctx.get_array_value<T>(2, i + 1);
		}
		auto const v_min = ctx.get_value<T>(3);
		auto const v_max = ctx.get_value<T>(4);
		auto const format = ctx.get_value<std::string_view>(5, default_format);
		auto const flags = ctx.get_value<ImGuiSliderFlags>(6, 0);
		auto const result = f(label.data(), v, v_min, v_max, format.data(), flags);
		for (int i = 0; i < static_cast<int>(N); i += 1) {
			ctx.set_array_value<T>(2, i + 1, v[i]);
		}
		ctx.push_value(result);
		ctx.push_value(lua::stack_index_t(2));
		return 2;
	}

	int SliderFloat(lua_State* const vm) {
		return SliderType<float>(vm, "%.3f"sv, &ImGui::SliderFloat);
	}
	int SliderFloat2(lua_State* const vm) {
		return SliderTypeN<float, 2>(vm, "%.3f"sv, &ImGui::SliderFloat2);
	}
	int SliderFloat3(lua_State* const vm) {
		return SliderTypeN<float, 3>(vm, "%.3f"sv, &ImGui::SliderFloat3);
	}
	int SliderFloat4(lua_State* const vm) {
		return SliderTypeN<float, 4>(vm, "%.3f"sv, &ImGui::SliderFloat4);
	}
	int SliderAngle(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto v_rad = ctx.get_value<float>(2);
		auto const v_degrees_min = ctx.get_value<float>(3, -360.0f);
		auto const v_degrees_max = ctx.get_value<float>(4, +360.0f);
		auto const format = ctx.get_value<std::string_view>(5, "%.0f deg"sv);
		auto const flags = ctx.get_value<ImGuiSliderFlags>(6, 0);
		auto const result = ImGui::SliderAngle(label.data(), &v_rad, v_degrees_min, v_degrees_max, format.data(), flags);
		ctx.push_value(result);
		ctx.push_value(v_rad);
		return 2;
	}

	int SliderInt(lua_State* const vm) {
		return SliderType<int>(vm, "%d"sv, &ImGui::SliderInt);
	}
	int SliderInt2(lua_State* const vm) {
		return SliderTypeN<int, 2>(vm, "%d"sv, &ImGui::SliderInt2);
	}
	int SliderInt3(lua_State* const vm) {
		return SliderTypeN<int, 3>(vm, "%d"sv, &ImGui::SliderInt3);
	}
	int SliderInt4(lua_State* const vm) {
		return SliderTypeN<int, 4>(vm, "%d"sv, &ImGui::SliderInt4);
	}

	template<typename T, ImGuiDataType DataType>
	int SliderScalarT(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		// data_type at 2
		auto p_data = ctx.get_value<T>(3);
		auto const p_min = ctx.get_value<T>(4);
		auto const p_max = ctx.get_value<T>(5);
		auto const format = ctx.get_value<std::string_view>(6, {});
		auto const flags = ctx.get_value<ImGuiSliderFlags>(6, 0);
		auto const result = ImGui::SliderScalar(label.data(), DataType, &p_data, &p_min, &p_max, format.data(), flags);
		ctx.push_value(result);
		ctx.push_value(p_data);
		return 2;
	}
	int SliderScalar(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const data_type = ctx.get_value<ImGuiDataType>(2);
		switch (data_type) {
		case ImGuiDataType_S8:
			return SliderScalarT<int8_t, ImGuiDataType_S8>(vm);
		case ImGuiDataType_U8:
			return SliderScalarT<uint8_t, ImGuiDataType_U8>(vm);
		case ImGuiDataType_S16:
			return SliderScalarT<int16_t, ImGuiDataType_S16>(vm);
		case ImGuiDataType_U16:
			return SliderScalarT<int16_t, ImGuiDataType_U16>(vm);
		case ImGuiDataType_S32:
			return SliderScalarT<int32_t, ImGuiDataType_S32>(vm);
		case ImGuiDataType_U32:
			return SliderScalarT<uint32_t, ImGuiDataType_U32>(vm);
		case ImGuiDataType_S64:
			// WARN: not full supported
			return SliderScalarT<int64_t, ImGuiDataType_S64>(vm);
		case ImGuiDataType_U64:
			// WARN: not full supported
			return SliderScalarT<uint64_t, ImGuiDataType_U64>(vm);
		case ImGuiDataType_Float:
			return SliderScalarT<float, ImGuiDataType_Float>(vm);
		case ImGuiDataType_Double:
			return SliderScalarT<double, ImGuiDataType_Double>(vm);
		case ImGuiDataType_Bool:
		case ImGuiDataType_String:
			return luaL_error(vm, "unsupported data type");
		default:
			return luaL_error(vm, "unknown data type");
		}
	}
	template<typename T, ImGuiDataType DataType>
	int SliderScalarNT(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		// data_type at 2
		auto const components = ctx.get_value<int>(4);
		if (components < 0) {
			return luaL_error(vm, "components is less then 0");
		}
		std::array<size_t, 256 / sizeof(size_t)> buf{};
		std::pmr::monotonic_buffer_resource res(buf.data(), 256, std::pmr::get_default_resource());
		std::pmr::vector<T> data(static_cast<size_t>(components), &res);
		for (int i = 0; i < components; i += 1) {
			data[i] = ctx.get_array_value<T>(3, i + 1);
		}
		auto const p_min = ctx.get_value<T>(5);
		auto const p_max = ctx.get_value<T>(6);
		auto const format = ctx.get_value<std::string_view>(7, {});
		auto const flags = ctx.get_value<ImGuiSliderFlags>(8, 0);
		auto const result = ImGui::SliderScalarN(label.data(), DataType, data.data(), components, &p_min, &p_max, format.data(), flags);
		for (int i = 0; i < components; i += 1) {
			ctx.set_array_value(3, i + 1, data[i]);
		}
		ctx.push_value(result);
		ctx.push_value(lua::stack_index_t(3));
		return 2;
	}
	int SliderScalarN(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const data_type = ctx.get_value<ImGuiDataType>(2);
		switch (data_type) {
		case ImGuiDataType_S8:
			return SliderScalarNT<int8_t, ImGuiDataType_S8>(vm);
		case ImGuiDataType_U8:
			return SliderScalarNT<uint8_t, ImGuiDataType_U8>(vm);
		case ImGuiDataType_S16:
			return SliderScalarNT<int16_t, ImGuiDataType_S16>(vm);
		case ImGuiDataType_U16:
			return SliderScalarNT<int16_t, ImGuiDataType_U16>(vm);
		case ImGuiDataType_S32:
			return SliderScalarNT<int32_t, ImGuiDataType_S32>(vm);
		case ImGuiDataType_U32:
			return SliderScalarNT<uint32_t, ImGuiDataType_U32>(vm);
		case ImGuiDataType_S64:
			// WARN: not full supported
			return SliderScalarNT<int64_t, ImGuiDataType_S64>(vm);
		case ImGuiDataType_U64:
			// WARN: not full supported
			return SliderScalarNT<uint64_t, ImGuiDataType_U64>(vm);
		case ImGuiDataType_Float:
			return SliderScalarNT<float, ImGuiDataType_Float>(vm);
		case ImGuiDataType_Double:
			return SliderScalarNT<double, ImGuiDataType_Double>(vm);
		case ImGuiDataType_Bool:
		case ImGuiDataType_String:
			return luaL_error(vm, "unsupported data type");
		default:
			return luaL_error(vm, "unknown data type");
		}
	}

	template<typename T, typename F>
	int VSliderType(lua_State* const vm, std::string_view const default_format, F const f) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const size = imgui::binding::ImVec2Binding::as(vm, 2);
		auto v = ctx.get_value<T>(3);
		auto const v_min = ctx.get_value<T>(4);
		auto const v_max = ctx.get_value<T>(5);
		auto const format = ctx.get_value<std::string_view>(6, default_format);
		auto const flags = ctx.get_value<ImGuiSliderFlags>(7, 0);
		auto const result = f(label.data(), *size, &v, v_min, v_max, format.data(), flags);
		ctx.push_value(result);
		ctx.push_value(v);
		return 2;
	}
	int VSliderFloat(lua_State* const vm) {
		return VSliderType<float>(vm, "%.3f"sv, &ImGui::VSliderFloat);
	}
	int VSliderInt(lua_State* const vm) {
		return VSliderType<int>(vm, "%d"sv, &ImGui::VSliderInt);
	}

	template<typename T, ImGuiDataType DataType>
	int VSliderScalarT(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const size = imgui::binding::ImVec2Binding::as(vm, 2);
		// data_type at 3
		auto p_data = ctx.get_value<T>(4);
		auto const p_min = ctx.get_value<T>(5);
		auto const p_max = ctx.get_value<T>(6);
		auto const format = ctx.get_value<std::string_view>(7, {});
		auto const flags = ctx.get_value<ImGuiSliderFlags>(8, 0);
		auto const result = ImGui::VSliderScalar(label.data(), *size, DataType, &p_data, &p_min, &p_max, format.data(), flags);
		ctx.push_value(result);
		ctx.push_value(p_data);
		return 2;
	}
	int VSliderScalar(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const data_type = ctx.get_value<ImGuiDataType>(2);
		switch (data_type) {
		case ImGuiDataType_S8:
			return VSliderScalarT<int8_t, ImGuiDataType_S8>(vm);
		case ImGuiDataType_U8:
			return VSliderScalarT<uint8_t, ImGuiDataType_U8>(vm);
		case ImGuiDataType_S16:
			return VSliderScalarT<int16_t, ImGuiDataType_S16>(vm);
		case ImGuiDataType_U16:
			return VSliderScalarT<int16_t, ImGuiDataType_U16>(vm);
		case ImGuiDataType_S32:
			return VSliderScalarT<int32_t, ImGuiDataType_S32>(vm);
		case ImGuiDataType_U32:
			return VSliderScalarT<uint32_t, ImGuiDataType_U32>(vm);
		case ImGuiDataType_S64:
			// WARN: not full supported
			return VSliderScalarT<int64_t, ImGuiDataType_S64>(vm);
		case ImGuiDataType_U64:
			// WARN: not full supported
			return VSliderScalarT<uint64_t, ImGuiDataType_U64>(vm);
		case ImGuiDataType_Float:
			return VSliderScalarT<float, ImGuiDataType_Float>(vm);
		case ImGuiDataType_Double:
			return VSliderScalarT<double, ImGuiDataType_Double>(vm);
		case ImGuiDataType_Bool:
		case ImGuiDataType_String:
			return luaL_error(vm, "unsupported data type");
		default:
			return luaL_error(vm, "unknown data type");
		}
	}
}

namespace imgui::binding {
	void registerImGuiWidgetsRegularSliders(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);

		ctx.set_map_value(m, "SliderFloat"sv, &SliderFloat);
		ctx.set_map_value(m, "SliderFloat2"sv, &SliderFloat2);
		ctx.set_map_value(m, "SliderFloat3"sv, &SliderFloat3);
		ctx.set_map_value(m, "SliderFloat4"sv, &SliderFloat4);

		ctx.set_map_value(m, "SliderAngle"sv, &SliderAngle);

		ctx.set_map_value(m, "SliderInt"sv, &SliderInt);
		ctx.set_map_value(m, "SliderInt2"sv, &SliderInt2);
		ctx.set_map_value(m, "SliderInt3"sv, &SliderInt3);
		ctx.set_map_value(m, "SliderInt4"sv, &SliderInt4);

		ctx.set_map_value(m, "SliderScalar"sv, &SliderScalar);
		ctx.set_map_value(m, "SliderScalarN"sv, &SliderScalarN);

		ctx.set_map_value(m, "VSliderFloat"sv, &VSliderFloat);
		ctx.set_map_value(m, "VSliderInt"sv, &VSliderInt);

		ctx.set_map_value(m, "VSliderScalar"sv, &VSliderScalar);
	}
}
