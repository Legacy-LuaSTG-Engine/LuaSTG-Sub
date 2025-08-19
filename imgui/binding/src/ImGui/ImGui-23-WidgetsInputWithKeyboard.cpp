#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"
#include <array>
#include <vector>
#include <memory_resource>

using std::string_view_literals::operator ""sv;

namespace {
	int InputFloat(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto v = ctx.get_value<float>(2);
		auto const step = ctx.get_value<float>(3, 0.0f);
		auto const step_fast = ctx.get_value<float>(4, 0.0f);
		auto const format = ctx.get_value<std::string_view>(5, "%.3f"sv);
		auto const flags = ctx.get_value<ImGuiInputTextFlags>(6, 0);
		auto const result = ImGui::InputFloat(label.data(), &v, step, step_fast, format.data(), flags);
		ctx.push_value(result);
		ctx.push_value(v);
		return 2;
	}

	template<size_t N, typename F>
	int InputFloatN(lua_State* const vm, F const f) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		float v[N]{};
		for (int i = 0; i < static_cast<int>(N); i += 1) {
			v[i] = ctx.get_array_value<float>(2, i + 1);
		}
		auto const format = ctx.get_value<std::string_view>(3, "%.3f"sv);
		auto const flags = ctx.get_value<ImGuiInputTextFlags>(4, 0);
		auto const result = f(label.data(), v, format.data(), flags);
		for (int i = 0; i < static_cast<int>(N); i += 1) {
			ctx.set_array_value<float>(2, i + 1, v[i]);
		}
		ctx.push_value(result);
		ctx.push_value(lua::stack_index_t(2));
		return 2;
	}
	int InputFloat2(lua_State* const vm) {
		return InputFloatN<2>(vm, &ImGui::InputFloat2);
	}
	int InputFloat3(lua_State* const vm) {
		return InputFloatN<3>(vm, &ImGui::InputFloat3);
	}
	int InputFloat4(lua_State* const vm) {
		return InputFloatN<4>(vm, &ImGui::InputFloat4);
	}

	int InputInt(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto v = ctx.get_value<int>(2);
		auto const step = ctx.get_value<int>(3, 1);
		auto const step_fast = ctx.get_value<int>(4, 100);
		auto const flags = ctx.get_value<ImGuiInputTextFlags>(5, 0);
		auto const result = ImGui::InputInt(label.data(), &v, step, step_fast, flags);
		ctx.push_value(result);
		ctx.push_value(v);
		return 2;
	}

	template<size_t N, typename F>
	int InputIntN(lua_State* const vm, F const f) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		int v[N]{};
		for (int i = 0; i < static_cast<int>(N); i += 1) {
			v[i] = ctx.get_array_value<int>(2, i + 1);
		}
		auto const flags = ctx.get_value<ImGuiInputTextFlags>(3, 0);
		auto const result = f(label.data(), v, flags);
		for (int i = 0; i < static_cast<int>(N); i += 1) {
			ctx.set_array_value<int>(2, i + 1, v[i]);
		}
		ctx.push_value(result);
		ctx.push_value(lua::stack_index_t(2));
		return 2;
	}
	int InputInt2(lua_State* const vm) {
		return InputIntN<2>(vm, &ImGui::InputInt2);
	}
	int InputInt3(lua_State* const vm) {
		return InputIntN<3>(vm, &ImGui::InputInt3);
	}
	int InputInt4(lua_State* const vm) {
		return InputIntN<4>(vm, &ImGui::InputInt4);
	}

	int InputDouble(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto v = ctx.get_value<double>(2);
		auto const step = ctx.get_value<double>(3, 0.0);
		auto const step_fast = ctx.get_value<double>(4, 0.0);
		auto const format = ctx.get_value<std::string_view>(5, "%.6f"sv);
		auto const flags = ctx.get_value<ImGuiInputTextFlags>(6, 0);
		auto const result = ImGui::InputDouble(label.data(), &v, step, step_fast, format.data(), flags);
		ctx.push_value(result);
		ctx.push_value(v);
		return 2;
	}

	template<typename T, ImGuiDataType DataType>
	int InputScalarT(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		// data_type at 2
		auto p_data = ctx.get_value<T>(3);
		auto const p_step = ctx.get_value<T>(4, {});
		auto const p_step_fast = ctx.get_value<T>(5, {});
		auto const format = ctx.get_value<std::string_view>(6, {});
		auto const flags = ctx.get_value<ImGuiInputTextFlags>(7, 0);
		auto const result = ImGui::InputScalar(label.data(), DataType, &p_data,
											   ctx.is_non_or_nil(4) ? nullptr : &p_step,
											   ctx.is_non_or_nil(5) ? nullptr : &p_step_fast,
											   format.data(), flags);
		ctx.push_value(result);
		ctx.push_value(p_data);
		return 2;
	}
	int InputScalar(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const data_type = ctx.get_value<ImGuiDataType>(2);
		switch (data_type) {
		case ImGuiDataType_S8:
			return InputScalarT<int8_t, ImGuiDataType_S8>(vm);
		case ImGuiDataType_U8:
			return InputScalarT<uint8_t, ImGuiDataType_U8>(vm);
		case ImGuiDataType_S16:
			return InputScalarT<int16_t, ImGuiDataType_S16>(vm);
		case ImGuiDataType_U16:
			return InputScalarT<int16_t, ImGuiDataType_U16>(vm);
		case ImGuiDataType_S32:
			return InputScalarT<int32_t, ImGuiDataType_S32>(vm);
		case ImGuiDataType_U32:
			return InputScalarT<uint32_t, ImGuiDataType_U32>(vm);
		case ImGuiDataType_S64:
			// WARN: not full supported
			return InputScalarT<int64_t, ImGuiDataType_S64>(vm);
		case ImGuiDataType_U64:
			// WARN: not full supported
			return InputScalarT<uint64_t, ImGuiDataType_U64>(vm);
		case ImGuiDataType_Float:
			return InputScalarT<float, ImGuiDataType_Float>(vm);
		case ImGuiDataType_Double:
			return InputScalarT<double, ImGuiDataType_Double>(vm);
		case ImGuiDataType_Bool:
		case ImGuiDataType_String:
			return luaL_error(vm, "unsupported data type");
		default:
			return luaL_error(vm, "unknown data type");
		}
	}

	template<typename T, ImGuiDataType DataType>
	int InputScalarNT(lua_State* const vm) {
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
		auto const p_step = ctx.get_value<T>(5, {});
		auto const p_step_fast = ctx.get_value<T>(6, {});
		auto const format = ctx.get_value<std::string_view>(7, {});
		auto const flags = ctx.get_value<ImGuiInputTextFlags>(8, 0);
		auto const result = ImGui::InputScalarN(label.data(), DataType, data.data(), components,
												ctx.is_non_or_nil(4) ? nullptr : &p_step,
												ctx.is_non_or_nil(5) ? nullptr : &p_step_fast,
												format.data(), flags);
		for (int i = 0; i < components; i += 1) {
			ctx.set_array_value(3, i + 1, data[i]);
		}
		ctx.push_value(result);
		ctx.push_value(lua::stack_index_t(3));
		return 2;
	}
	int InputScalarN(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const data_type = ctx.get_value<ImGuiDataType>(2);
		switch (data_type) {
		case ImGuiDataType_S8:
			return InputScalarNT<int8_t, ImGuiDataType_S8>(vm);
		case ImGuiDataType_U8:
			return InputScalarNT<uint8_t, ImGuiDataType_U8>(vm);
		case ImGuiDataType_S16:
			return InputScalarNT<int16_t, ImGuiDataType_S16>(vm);
		case ImGuiDataType_U16:
			return InputScalarNT<int16_t, ImGuiDataType_U16>(vm);
		case ImGuiDataType_S32:
			return InputScalarNT<int32_t, ImGuiDataType_S32>(vm);
		case ImGuiDataType_U32:
			return InputScalarNT<uint32_t, ImGuiDataType_U32>(vm);
		case ImGuiDataType_S64:
			// WARN: not full supported
			return InputScalarNT<int64_t, ImGuiDataType_S64>(vm);
		case ImGuiDataType_U64:
			// WARN: not full supported
			return InputScalarNT<uint64_t, ImGuiDataType_U64>(vm);
		case ImGuiDataType_Float:
			return InputScalarNT<float, ImGuiDataType_Float>(vm);
		case ImGuiDataType_Double:
			return InputScalarNT<double, ImGuiDataType_Double>(vm);
		case ImGuiDataType_Bool:
		case ImGuiDataType_String:
			return luaL_error(vm, "unsupported data type");
		default:
			return luaL_error(vm, "unknown data type");
		}
	}
}

namespace imgui::binding {
	void registerImGuiWidgetsInputWithKeyboard(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "InputFloat"sv, &InputFloat);
		ctx.set_map_value(m, "InputFloat2"sv, &InputFloat2);
		ctx.set_map_value(m, "InputFloat3"sv, &InputFloat3);
		ctx.set_map_value(m, "InputFloat4"sv, &InputFloat4);
		ctx.set_map_value(m, "InputInt"sv, &InputInt);
		ctx.set_map_value(m, "InputInt2"sv, &InputInt2);
		ctx.set_map_value(m, "InputInt3"sv, &InputInt3);
		ctx.set_map_value(m, "InputInt4"sv, &InputInt4);
		ctx.set_map_value(m, "InputDouble"sv, &InputDouble);
		ctx.set_map_value(m, "InputScalar"sv, &InputScalar);
		ctx.set_map_value(m, "InputScalarN"sv, &InputScalarN);
	}
}
