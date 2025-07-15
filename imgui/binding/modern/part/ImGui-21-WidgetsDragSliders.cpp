#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"
#include <array>
#include <vector>
#include <memory_resource>

using std::string_view_literals::operator ""sv;

namespace {
	int DragFloat(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto v = ctx.get_value<float>(2);
		auto const v_speed = ctx.get_value<float>(3, 1.0f);
		auto const v_min = ctx.get_value<float>(4, 0.0f);
		auto const v_max = ctx.get_value<float>(5, 0.0f);
		auto const format = ctx.get_value<std::string_view>(6, "%.3f"sv);
		auto const flags = ctx.get_value<ImGuiSliderFlags>(7, 0);
		auto const result = ImGui::DragFloat(label.data(), &v, v_speed, v_min, v_max, format.data(), flags);
		ctx.push_value(result);
		ctx.push_value(v);
		return 2;
	}
	template<size_t N, typename F>
	int DragFloatN(lua_State* const vm, F const f) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		float v[N]{};
		for (int i = 0; i < static_cast<int>(N); i += 1) {
			v[i] = ctx.get_array_value<float>(2, i + 1);
		}
		auto const v_speed = ctx.get_value<float>(3, 1.0f);
		auto const v_min = ctx.get_value<float>(4, 0.0f);
		auto const v_max = ctx.get_value<float>(5, 0.0f);
		auto const format = ctx.get_value<std::string_view>(6, "%.3f"sv);
		auto const flags = ctx.get_value<ImGuiSliderFlags>(7, 0);
		auto const result = f(label.data(), v, v_speed, v_min, v_max, format.data(), flags);
		for (int i = 0; i < static_cast<int>(N); i += 1) {
			ctx.set_array_value(2, i + 1, v[i]);
		}
		ctx.push_value(result);
		ctx.push_value(lua::stack_index_t(2));
		return 2;
	}
	int DragFloat2(lua_State* const vm) {
		return DragFloatN<2>(vm, &ImGui::DragFloat2);
	}
	int DragFloat3(lua_State* const vm) {
		return DragFloatN<3>(vm, &ImGui::DragFloat3);
	}
	int DragFloat4(lua_State* const vm) {
		return DragFloatN<4>(vm, &ImGui::DragFloat4);
	}
	int DragFloatRange2(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto v_current_min = ctx.get_value<float>(2);
		auto v_current_max = ctx.get_value<float>(3);
		auto const v_speed = ctx.get_value<float>(4, 1.0f);
		auto const v_min = ctx.get_value<float>(5, 0.0f);
		auto const v_max = ctx.get_value<float>(6, 0.0f);
		auto const format = ctx.get_value<std::string_view>(7, "%.3f");
		auto const format_max = ctx.get_value<std::string_view>(8, {});
		auto const flags = ctx.get_value<ImGuiSliderFlags>(9, 0);
		auto const result = ImGui::DragFloatRange2(label.data(), &v_current_min, &v_current_max, v_speed, v_min, v_max, format.data(), format_max.data(), flags);
		ctx.push_value(result);
		ctx.push_value(v_current_min);
		ctx.push_value(v_current_max);
		return 3;
	}
	int DragInt(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto v = ctx.get_value<int>(2);
		auto const v_speed = ctx.get_value<float>(3, 1.0f);
		auto const v_min = ctx.get_value<int>(4, 0);
		auto const v_max = ctx.get_value<int>(5, 0);
		auto const format = ctx.get_value<std::string_view>(6, "%d");
		auto const flags = ctx.get_value<ImGuiSliderFlags>(7, 0);
		auto const result = ImGui::DragInt(label.data(), &v, v_speed, v_min, v_max, format.data(), flags);
		ctx.push_value(result);
		ctx.push_value(v);
		return 2;
	}
	template<size_t N, typename F>
	int DragIntN(lua_State* const vm, F const f) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		int v[N]{};
		for (int i = 0; i < static_cast<int>(N); i += 1) {
			v[i] = ctx.get_array_value<int>(2, i + 1);
		}
		auto const v_speed = ctx.get_value<float>(3, 1.0f);
		auto const v_min = ctx.get_value<int>(4, 0);
		auto const v_max = ctx.get_value<int>(5, 0);
		auto const format = ctx.get_value<std::string_view>(6, "%d"sv);
		auto const flags = ctx.get_value<ImGuiSliderFlags>(7, 0);
		auto const result = f(label.data(), v, v_speed, v_min, v_max, format.data(), flags);
		for (int i = 0; i < static_cast<int>(N); i += 1) {
			ctx.set_array_value(2, i + 1, v[i]);
		}
		ctx.push_value(result);
		ctx.push_value(lua::stack_index_t(2));
		return 2;
	}
	int DragInt2(lua_State* const vm) {
		return DragIntN<2>(vm, &ImGui::DragInt2);
	}
	int DragInt3(lua_State* const vm) {
		return DragIntN<3>(vm, &ImGui::DragInt3);
	}
	int DragInt4(lua_State* const vm) {
		return DragIntN<4>(vm, &ImGui::DragInt4);
	}
	int DragIntRange2(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto v_current_min = ctx.get_value<int>(2);
		auto v_current_max = ctx.get_value<int>(3);
		auto const v_speed = ctx.get_value<float>(4, 1.0f);
		auto const v_min = ctx.get_value<int>(5, 0);
		auto const v_max = ctx.get_value<int>(6, 0);
		auto const format = ctx.get_value<std::string_view>(7, "%d"sv);
		auto const format_max = ctx.get_value<std::string_view>(8, {});
		auto const flags = ctx.get_value<ImGuiSliderFlags>(9, 0);
		auto const result = ImGui::DragIntRange2(label.data(), &v_current_min, &v_current_max, v_speed, v_min, v_max, format.data(), format_max.data(), flags);
		ctx.push_value(result);
		ctx.push_value(v_current_min);
		ctx.push_value(v_current_max);
		return 3;
	}
	template<typename T, ImGuiDataType DataType>
	int DragScalarT(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		// data_type from template
		auto p_data = ctx.get_value<T>(3);
		auto const v_speed = ctx.get_value<float>(4, 1.0f);
		auto const p_min = ctx.get_value<T>(5, {});
		auto const p_max = ctx.get_value<T>(6, {});
		auto const format = ctx.get_value<std::string_view>(7, {});
		auto const flags = ctx.get_value<ImGuiSliderFlags>(8, 0);
		auto const result = ImGui::DragScalar(label.data(), DataType, &p_data, v_speed,
											  ctx.has_value(5) ? &p_min : nullptr,
											  ctx.has_value(6) ? &p_max : nullptr,
											  format.data(), flags);
		ctx.push_value(result);
		ctx.push_value(p_data);
		return 2;
	}
	int DragScalar(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const data_type = ctx.get_value<ImGuiDataType>(2);
		switch (data_type) {
		case ImGuiDataType_S8:
			return DragScalarT<int8_t, ImGuiDataType_S8>(vm);
		case ImGuiDataType_U8:
			return DragScalarT<uint8_t, ImGuiDataType_U8>(vm);
		case ImGuiDataType_S16:
			return DragScalarT<int16_t, ImGuiDataType_S16>(vm);
		case ImGuiDataType_U16:
			return DragScalarT<int16_t, ImGuiDataType_U16>(vm);
		case ImGuiDataType_S32:
			return DragScalarT<int32_t, ImGuiDataType_S32>(vm);
		case ImGuiDataType_U32:
			return DragScalarT<uint32_t, ImGuiDataType_U32>(vm);
		case ImGuiDataType_S64:
			// WARN: not full supported
			return DragScalarT<int64_t, ImGuiDataType_S64>(vm);
		case ImGuiDataType_U64:
			// WARN: not full supported
			return DragScalarT<uint64_t, ImGuiDataType_U64>(vm);
		case ImGuiDataType_Float:
			return DragScalarT<float, ImGuiDataType_Float>(vm);
		case ImGuiDataType_Double:
			return DragScalarT<double, ImGuiDataType_Double>(vm);
		case ImGuiDataType_Bool:
		case ImGuiDataType_String:
			return luaL_error(vm, "unsupported data type");
		default:
			return luaL_error(vm, "unknown data type");
		}
	}
	template<typename T, ImGuiDataType DataType>
	int DragScalarNT(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		// data_type from template
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
		auto const v_speed = ctx.get_value<float>(5, 1.0f);
		auto const p_min = ctx.get_value<T>(6, {});
		auto const p_max = ctx.get_value<T>(7, {});
		auto const format = ctx.get_value<std::string_view>(8, {});
		auto const flags = ctx.get_value<ImGuiSliderFlags>(9, 0);
		auto const result = ImGui::DragScalarN(label.data(), DataType, data.data(), components, v_speed,
											   ctx.has_value(6) ? &p_min : nullptr,
											   ctx.has_value(7) ? &p_max : nullptr,
											   format.data(), flags);
		for (int i = 0; i < components; i += 1) {
			ctx.set_array_value(3, i + 1, data[i]);
		}
		ctx.push_value(result);
		ctx.push_value(lua::stack_index_t(3));
		return 2;
	}
	int DragScalarN(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const data_type = ctx.get_value<ImGuiDataType>(2);
		switch (data_type) {
		case ImGuiDataType_S8:
			return DragScalarNT<int8_t, ImGuiDataType_S8>(vm);
		case ImGuiDataType_U8:
			return DragScalarNT<uint8_t, ImGuiDataType_U8>(vm);
		case ImGuiDataType_S16:
			return DragScalarNT<int16_t, ImGuiDataType_S16>(vm);
		case ImGuiDataType_U16:
			return DragScalarNT<int16_t, ImGuiDataType_U16>(vm);
		case ImGuiDataType_S32:
			return DragScalarNT<int32_t, ImGuiDataType_S32>(vm);
		case ImGuiDataType_U32:
			return DragScalarNT<uint32_t, ImGuiDataType_U32>(vm);
		case ImGuiDataType_S64:
			// WARN: not full supported
			return DragScalarNT<int64_t, ImGuiDataType_S64>(vm);
		case ImGuiDataType_U64:
			// WARN: not full supported
			return DragScalarNT<uint64_t, ImGuiDataType_U64>(vm);
		case ImGuiDataType_Float:
			return DragScalarNT<float, ImGuiDataType_Float>(vm);
		case ImGuiDataType_Double:
			return DragScalarNT<double, ImGuiDataType_Double>(vm);
		case ImGuiDataType_Bool:
		case ImGuiDataType_String:
			return luaL_error(vm, "unsupported data type");
		default:
			return luaL_error(vm, "unknown data type");
		}
	}
}

namespace imgui::binding {
	void registerImGuiWidgetsDragSliders(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "DragFloat"sv, &DragFloat);
		ctx.set_map_value(m, "DragFloat2"sv, &DragFloat2);
		ctx.set_map_value(m, "DragFloat3"sv, &DragFloat3);
		ctx.set_map_value(m, "DragFloat4"sv, &DragFloat4);
		ctx.set_map_value(m, "DragFloatRange2"sv, &DragFloatRange2);
		ctx.set_map_value(m, "DragInt"sv, &DragInt);
		ctx.set_map_value(m, "DragInt2"sv, &DragInt2);
		ctx.set_map_value(m, "DragInt3"sv, &DragInt3);
		ctx.set_map_value(m, "DragInt4"sv, &DragInt4);
		ctx.set_map_value(m, "DragIntRange2"sv, &DragIntRange2);
		ctx.set_map_value(m, "DragScalar"sv, &DragScalar);
		ctx.set_map_value(m, "DragScalarN"sv, &DragScalarN);
	}
}
