#pragma once
#include "imgui.h"
#include "lua.hpp"

constexpr auto imgui_binding_lua_class_ImVec2 = "imgui.ImVec2";

void imgui_binding_lua_register_ImVec2(lua_State* L);
ImVec2* imgui_binding_lua_new_ImVec2(lua_State* L);
ImVec2* imgui_binding_lua_ref_ImVec2(lua_State* L, ImVec2* v);
ImVec2* imgui_binding_lua_to_ImVec2(lua_State* L, int idx);

constexpr auto imgui_binding_lua_class_ImVec4 = "imgui.ImVec4";

void imgui_binding_lua_register_ImVec4(lua_State* L);
ImVec4* imgui_binding_lua_new_ImVec4(lua_State* L);
ImVec4* imgui_binding_lua_ref_ImVec4(lua_State* L, ImVec4* v);
ImVec4* imgui_binding_lua_to_ImVec4(lua_State* L, int idx);

constexpr auto imgui_binding_lua_class_ImGuiStyle = "imgui.ImGuiStyle";

void imgui_binding_lua_register_ImGuiStyle(lua_State* L);
ImGuiStyle* imgui_binding_lua_new_ImGuiStyle(lua_State* L);
ImGuiStyle* imgui_binding_lua_ref_ImGuiStyle(lua_State* L, ImGuiStyle* v);
ImGuiStyle* imgui_binding_lua_to_ImGuiStyle(lua_State* L, int idx);

constexpr auto imgui_binding_lua_class_string = "imgui.string";
constexpr auto imgui_binding_lua_class_ImGuiTextBuffer = "imgui.ImGuiTextBuffer";

void imgui_binding_lua_register_ImGuiTextBuffer(lua_State* L);
ImGuiTextBuffer* imgui_binding_lua_new_ImGuiTextBuffer(lua_State* L);
ImGuiTextBuffer* imgui_binding_lua_ref_ImGuiTextBuffer(lua_State* L, ImGuiTextBuffer* v);
ImGuiTextBuffer* imgui_binding_lua_to_ImGuiTextBuffer(lua_State* L, int idx);
