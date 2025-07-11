#pragma once
#include "imgui.h"
#include "lua.hpp"

constexpr const char lua_module_imgui[] = "imgui";

constexpr const char lua_class_imgui_string[] = "imgui.string";
constexpr const char lua_class_imgui_ImGuiTextBuffer[] = "imgui.ImGuiTextBuffer";

void imgui_binding_lua_register_ImGuiTextBuffer(lua_State* L);
ImGuiTextBuffer* imgui_binding_lua_new_ImGuiTextBuffer(lua_State* L);
ImGuiTextBuffer* imgui_binding_lua_ref_ImGuiTextBuffer(lua_State* L, ImGuiTextBuffer* v);
ImGuiTextBuffer* imgui_binding_lua_to_ImGuiTextBuffer(lua_State* L, int idx);

constexpr ImGuiDataType ImGuiDataType_Integer =
(sizeof(lua_Integer) >= 8
    ? (ImGuiDataType)ImGuiDataType_S64
    : (ImGuiDataType)ImGuiDataType_S32
);
constexpr ImGuiDataType ImGuiDataType_Number =
(sizeof(lua_Number) >= 8
    ? (ImGuiDataType)ImGuiDataType_Double
    : (ImGuiDataType)ImGuiDataType_Float
);

constexpr char const* ImGuiDataTypeName[] = {
    "S8",
    "U8",
    "S16",
    "U16",
    "S32",
    "U32",
    "S64",
    "U64",
    "Float",
    "Double",
};
