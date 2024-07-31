#pragma once
#include "imgui.h"
#include "lua.hpp"

constexpr const char lua_module_imgui[] = "imgui";

constexpr const char lua_class_imgui_ImGuiStyle[] = "imgui.ImGuiStyle";

void imgui_binding_lua_register_ImGuiStyle(lua_State* L);
ImGuiStyle* imgui_binding_lua_new_ImGuiStyle(lua_State* L);
ImGuiStyle* imgui_binding_lua_ref_ImGuiStyle(lua_State* L, ImGuiStyle* v);
ImGuiStyle* imgui_binding_lua_to_ImGuiStyle(lua_State* L, int idx);

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

namespace lua {
    template<typename T>
    void register_type(lua_State* L);

    template<typename T>
    T* create_type_instance(lua_State* L);

    template<typename T>
    T* create_type_instance(lua_State* L, const T& init);

    template<typename T>
    T* as_type_instance(lua_State* L, int index);

    template<typename T>
    bool is_type_instance(lua_State* L, int index);

    // ImVec2

    template<>
    void register_type<ImVec2>(lua_State* L);

    template<>
    ImVec2* create_type_instance<ImVec2>(lua_State* L);

    template<>
    ImVec2* create_type_instance<ImVec2>(lua_State* L, const ImVec2& init);

    template<>
    ImVec2* as_type_instance<ImVec2>(lua_State* L, int index);

    template<>
    bool is_type_instance<ImVec2>(lua_State* L, int index);

    // ImVec4

    template<>
    void register_type<ImVec4>(lua_State* L);

    template<>
    ImVec4* create_type_instance<ImVec4>(lua_State* L);

    template<>
    ImVec4* create_type_instance<ImVec4>(lua_State* L, const ImVec4& init);

    template<>
    ImVec4* as_type_instance<ImVec4>(lua_State* L, int index);

    template<>
    bool is_type_instance<ImVec4>(lua_State* L, int index);
}
