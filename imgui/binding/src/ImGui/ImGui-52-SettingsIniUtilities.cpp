#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int LoadIniSettingsFromDisk(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const ini_filename = ctx.get_value<std::string_view>(1);
		ImGui::LoadIniSettingsFromDisk(ini_filename.data());
		return 0;
	}
	int LoadIniSettingsFromMemory(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const ini_data = ctx.get_value<std::string_view>(1);
		ImGui::LoadIniSettingsFromMemory(ini_data.data(), ini_data.size());
		return 0;
	}
	int SaveIniSettingsToDisk(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const ini_filename = ctx.get_value<std::string_view>(1);
		ImGui::SaveIniSettingsToDisk(ini_filename.data());
		return 0;
	}
	int SaveIniSettingsToMemory(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		size_t out_ini_size{};
		auto const result = ImGui::SaveIniSettingsToMemory(&out_ini_size);
		ctx.push_value(std::string_view(result, out_ini_size));
		return 1;
	}
}

namespace imgui::binding {
	void registerImGuiSettingsIniUtilities(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "LoadIniSettingsFromDisk"sv, &LoadIniSettingsFromDisk);
		ctx.set_map_value(m, "LoadIniSettingsFromMemory"sv, &LoadIniSettingsFromMemory);
		ctx.set_map_value(m, "SaveIniSettingsToDisk"sv, &SaveIniSettingsToDisk);
		ctx.set_map_value(m, "SaveIniSettingsToMemory"sv, &SaveIniSettingsToMemory);
	}
}
