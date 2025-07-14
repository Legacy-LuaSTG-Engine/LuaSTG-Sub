#include "lua_imgui_common.hpp"
#include "lua_imgui_ImGui.hpp"
#include "lua_imgui_type.hpp"
#include "lua_imgui_binding.hpp"
#include <cstdint>
#include <tuple>

// TODO TODO TODO TODO TODO TODO TODO TODO
// - [ ] get/push ImGuiID integer (int32 ?) -> number (double)
// - [ ] multi select feature

template<typename T>
struct type_array
{
    T* data = nullptr;
    T& operator[](size_t idx)
    {
        return data[idx];
    }
    T* operator*()
    {
        return data;
    }
    type_array(size_t n)
    {
        data = new T[n];
        memset(data, 0, n * sizeof(T));
    }
    ~type_array()
    {
        delete[] data;
        data = nullptr;
    }
};
using char_array = type_array<char*>;
using float_array = type_array<float>;
using integer_array = type_array<lua_Integer>;
using number_array = type_array<lua_Number>;

static lua_State* GLUA = NULL;
static const int IMGUI_REGISTRY = 9961;
#define IMGUI_GET_REGISTRY(L) lua_pushlightuserdata(L, (void*)&IMGUI_REGISTRY); lua_gettable(L, LUA_REGISTRYINDEX);
#define LUA_IMGUI_NOT_SUPPORT return luaL_error(L, "not supported");

// XXXX: will not implement (or should not export to lua API)
// !!!!: works in progress

//////// Widgets: Drag Sliders

#include "lua_imgui_ImGui_DragX.inl"

//////// Widgets: Regular Sliders

#include "lua_imgui_ImGui_SliderX.inl"

//////// Widgets: Input with Keyboard

#include "lua_imgui_ImGui_InputX.inl"

//////// Widgets: Color Editor/Picker

#include "lua_imgui_ImGui_ColorX.inl"

//////// Widgets: Trees

static int lib_TreeNode(lua_State* L)
{
    const int argc = lua_gettop(L);
    bool ret = false;
    switch (lua_type(L, 1))
    {
    case LUA_TUSERDATA:
        {
            const void* ptr_id = lua_touserdata(L, 1);
            const char* fmt = luaL_checkstring(L, 2);
            ret = ImGui::TreeNode(ptr_id, fmt);
            break;
        }
    case LUA_TNUMBER:
        {
            const void* ptr_id = (void*)(ptrdiff_t)lua_tointeger(L, 1);
            const char* fmt = luaL_checkstring(L, 2);
            ret = ImGui::TreeNode(ptr_id, fmt);
            break;
        }
    case LUA_TSTRING:
    default:
        {
            const char* v1 = lua_tostring(L, 1);
            if (argc <= 1)
            {
                ret = ImGui::TreeNode(v1);
            }
            else
            {
                const char* fmt = luaL_checkstring(L, 2);
                ret = ImGui::TreeNode(v1, fmt);
            }
            break;
        }
    }
    lua_pushboolean(L, ret);
    return 1;
}
static /* XXXX */ int lib_TreeNodeV(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static int lib_TreeNodeEx(lua_State* L)
{
    const int argc = lua_gettop(L);
    bool ret = false;
    switch (lua_type(L, 1))
    {
    case LUA_TUSERDATA:
        {
            const void* ptr_id = lua_touserdata(L, 1);
            const ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)luaL_checkinteger(L, 2);
            const char* fmt = luaL_checkstring(L, 3);
            ret = ImGui::TreeNodeEx(ptr_id, flags, fmt);
            break;
        }
    case LUA_TNUMBER:
        {
            const void* ptr_id = (void*)(ptrdiff_t)lua_tointeger(L, 1);
            const ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)luaL_checkinteger(L, 2);
            const char* fmt = luaL_checkstring(L, 3);
            ret = ImGui::TreeNodeEx(ptr_id, flags, fmt);
            break;
        }
    case LUA_TSTRING:
        default:
        {
            const char* v1 = lua_tostring(L, 1);
            if (argc <= 1)
            {
                ret = ImGui::TreeNodeEx(v1);
            }
            else if (argc == 2)
            {
                const ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)luaL_checkinteger(L, 2);
                ret = ImGui::TreeNodeEx(v1, flags);
            }
            else
            {
                const ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)luaL_checkinteger(L, 2);
                const char* fmt = luaL_checkstring(L, 3);
                ret = ImGui::TreeNodeEx(v1, flags, fmt);
            }
            break;
        }
    }
    lua_pushboolean(L, ret);
    return 1;
}
static /* XXXX */ int lib_TreeNodeExV(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static int lib_TreePush(lua_State* L)
{
    switch (lua_type(L, 1))
    {
    case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
        {
            const void* ptr_id = lua_touserdata(L, 1);
            ImGui::TreePush(ptr_id);
            break;
        }
    case LUA_TNUMBER:
        {
            const void* ptr_id = (void*)(ptrdiff_t)lua_tointeger(L, 1);
            ImGui::TreePush(ptr_id);
            break;
        }
    case LUA_TSTRING:
    default:
        {
            const char* str_id = lua_tostring(L, 1);
            ImGui::TreePush(str_id);
            break;
        }
    }
    return 0;
}
static int lib_TreePop(lua_State* L)
{
    std::ignore = L;
    ImGui::TreePop();
    return 0;
}
static int lib_GetTreeNodeToLabelSpacing(lua_State* L)
{
    const float ret = ImGui::GetTreeNodeToLabelSpacing();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_CollapsingHeader(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    if (argc <= 1)
    {
        const bool ret = ImGui::CollapsingHeader(label);
        lua_pushboolean(L, ret);
        return 1;
    }
    else if (argc == 2)
    {
        if (lua_type(L, 2) == LUA_TNUMBER)
        {
            const ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)luaL_checkinteger(L, 2);
            const bool ret = ImGui::CollapsingHeader(label, flags);
            lua_pushboolean(L, ret);
            return 1;
        }
        else
        {
            bool open_ = lua_toboolean(L, 2);
            const bool ret = ImGui::CollapsingHeader(label, &open_);
            lua_pushboolean(L, ret);
            lua_pushboolean(L, open_);
            return 2;
        }
    }
    else
    {
        bool open_ = lua_toboolean(L, 2);
        const ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)luaL_checkinteger(L, 3);
        const bool ret = ImGui::CollapsingHeader(label, &open_, flags);
        lua_pushboolean(L, ret);
        lua_pushboolean(L, open_);
        return 2;
    }
}
static int lib_SetNextItemOpen(lua_State* L)
{
    const bool is_open = lua_toboolean(L, 1);
    if (lua_gettop(L) <= 1)
    {
        ImGui::SetNextItemOpen(is_open);
    }
    else
    {
        const ImGuiCond cond = (ImGuiCond)luaL_checkinteger(L, 2);
        ImGui::SetNextItemOpen(is_open, cond);
    }
    return 0;
}
static int lib_SetNextItemStorageID(lua_State* L) {
    auto const imgui_id = static_cast<ImGuiID>(luaL_checknumber(L, 1));
    ImGui::SetNextItemStorageID(imgui_id);
    return 0;
}

//////// Widgets: Selectables

static int lib_Selectable(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    if (argc <= 1)
    {
        const bool ret = ImGui::Selectable(label);
        lua_pushboolean(L, ret);
        return 1;
    }
    else if (argc == 2)
    {
        bool selected = lua_toboolean(L, 2);
        const bool ret = ImGui::Selectable(label, &selected);
        lua_pushboolean(L, ret);
        lua_pushboolean(L, selected);
        return 2;
    }
    else if (argc == 3)
    {
        bool selected = lua_toboolean(L, 2);
        const ImGuiSelectableFlags flags = (ImGuiSelectableFlags)luaL_checkinteger(L, 3);
        const bool ret = ImGui::Selectable(label, &selected, flags);
        lua_pushboolean(L, ret);
        lua_pushboolean(L, selected);
        return 2;
    }
    else
    {
        bool selected = lua_toboolean(L, 2);
        const ImGuiSelectableFlags flags = (ImGuiSelectableFlags)luaL_checkinteger(L, 3);
        ImVec2* size = imgui::binding::ImVec2Binding::as(L, 4);
        const bool ret = ImGui::Selectable(label, &selected, flags, *size);
        lua_pushboolean(L, ret);
        lua_pushboolean(L, selected);
        return 2;
    }
}

//////// Widgets: List Boxes

static int lib_BeginListBox(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    if (lua_gettop(L) <= 1)
    {
        const bool ret = ImGui::BeginListBox(label);
        lua_pushboolean(L, ret);
    }
    else
    {
        ImVec2* size = imgui::binding::ImVec2Binding::as(L, 2);
        const bool ret = ImGui::BeginListBox(label, *size);
        lua_pushboolean(L, ret);
    }
    return 1;
}
static int lib_EndListBox(lua_State* L)
{
    std::ignore = L;
    ImGui::EndListBox();
    return 0;
}
static int lib_ListBox(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    int current_item = (int)luaL_checkinteger(L, 2);
    bool ret = false;
    if (lua_type(L, 3) == LUA_TFUNCTION)
    {
        const int items_count = (int)luaL_checkinteger(L, 4);
        struct Wrapper
        {
            static char const* Getter(void* data, int idx)
            {
                lua_State* L = (lua_State*)data;
                lua_pushvalue(L, 3);
                lua_pushinteger(L, idx + 1);
                lua_call(L, 1, 1);
                if (lua_isstring(L, -1))
                {
                    char const *out_text = lua_tostring(L, -1);
                    lua_pop(L, 1);
                    return out_text;
                }
                else
                {
                    lua_pop(L, 1);
                    return "";
                }
            }
        };
        if (argc <= 4)
        {
            ret = ImGui::ListBox(label, &current_item, &Wrapper::Getter, L, items_count);
        }
        else
        {
            const int height_in_items = (int)luaL_checkinteger(L, 5);
            ret = ImGui::ListBox(label, &current_item, &Wrapper::Getter, L, items_count, height_in_items);
        }
    }
    else
    {
        const int items_count = (argc >= 4) ? (int)luaL_checkinteger(L, 4) : _luaL_len(L, 3);
        char_array items(items_count);
        for (int i = 0; i < items_count; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_gettable(L, 3);
            items.data[i] = (char*)luaL_checkstring(L, -1);
            lua_pop(L, 1);
        }
        if (argc <= 4)
        {
            ret = ImGui::ListBox(label, &current_item, items.data, items_count);
        }
        else
        {
            const int height_in_items = (int)luaL_checkinteger(L, 5);
            ret = ImGui::ListBox(label, &current_item, items.data, items_count, height_in_items);
        }
    }
    lua_pushboolean(L, ret);
    lua_pushinteger(L, (lua_Integer)current_item);
    return 2;
}

//////// Widgets: Data Plotting

#include "lua_imgui_ImGui_PlotX.inl"

//////// Widgets: Value() Helpers.

// because lua can't identify integer and float number, I provide 3 extra functions
static int lib_ValueB(lua_State* L)
{
    const char* prefix = luaL_checkstring(L, 1);
    const bool b = lua_toboolean(L, 2);
    ImGui::Value(prefix, b);
    return 0;
}
static int lib_ValueI(lua_State* L)
{
    const char* prefix = luaL_checkstring(L, 1);
    const int v = (int)luaL_checkinteger(L, 2);
    ImGui::Value(prefix, v);
    return 0;
}
static int lib_ValueF(lua_State* L)
{
    const char* prefix = luaL_checkstring(L, 1);
    const float v = (float)luaL_checknumber(L, 2);
    if (lua_gettop(L) <= 2)
    {
        ImGui::Value(prefix, v);
    }
    else
    {
        const char* float_format = luaL_checkstring(L, 3);
        ImGui::Value(prefix, v, float_format);
    }
    return 0;
}
static int lib_Value(lua_State* L)
{
    switch (lua_type(L, 2))
    {
    case LUA_TNUMBER:
        return lib_ValueF(L);
    default:
        return lib_ValueB(L);
    }
}

//////// Widgets: Menus

static int lib_BeginMenuBar(lua_State* L)
{
    const bool ret = ImGui::BeginMenuBar();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_EndMenuBar(lua_State* L)
{
    std::ignore = L;
    ImGui::EndMenuBar();
    return 0;
}
static int lib_BeginMainMenuBar(lua_State* L)
{
    const bool ret = ImGui::BeginMainMenuBar();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_EndMainMenuBar(lua_State* L)
{
    std::ignore = L;
    ImGui::EndMainMenuBar();
    return 0;
}
static int lib_BeginMenu(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    if(lua_gettop(L) <= 1)
    {
        const bool ret = ImGui::BeginMenu(label);
        lua_pushboolean(L, ret);
    }
    else
    {
        const bool enable = lua_toboolean(L, 2);
        const bool ret = ImGui::BeginMenu(label, enable);
        lua_pushboolean(L, ret);
    }
    return 1;
}
static int lib_EndMenu(lua_State* L)
{
    std::ignore = L;
    ImGui::EndMenu();
    return 0;
}
static int lib_MenuItem(lua_State* L)
{
    // only provide the bool* p_selected one binding
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    if (argc <= 1)
    {
        const bool ret = ImGui::MenuItem(label);
        lua_pushboolean(L, ret);
        return 1;
    }
    else
    {
        const char* shortcut = lua_isnoneornil(L, 2) ? NULL : luaL_checkstring(L, 2);
        if (argc == 2)
        {
            const bool ret = ImGui::MenuItem(label, shortcut);
            lua_pushboolean(L, ret);
            return 1;
        }
        else if (argc == 3)
        {
            bool p_selected = lua_toboolean(L, 3);
            const bool ret = ImGui::MenuItem(label, shortcut, &p_selected);
            lua_pushboolean(L, ret);
            lua_pushboolean(L, p_selected);
            return 2;
        }
        else
        {
            bool p_selected = lua_toboolean(L, 3);
            const bool enabled = lua_toboolean(L, 4);
            const bool ret = ImGui::MenuItem(label, shortcut, &p_selected, enabled);
            lua_pushboolean(L, ret);
            lua_pushboolean(L, p_selected);
            return 2;
        }
    }
}

//////// Tooltips

static int lib_BeginTooltip(lua_State* L)
{
    std::ignore = L;
    bool const result = ImGui::BeginTooltip();
    lua_pushboolean(L, result);
    return 1;
}
static int lib_EndTooltip(lua_State* L)
{
    std::ignore = L;
    ImGui::EndTooltip();
    return 0;
}
static int lib_SetTooltip(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    ImGui::SetTooltip(text);
    return 0;
}
static /* XXXX */ int lib_SetTooltipV(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}

static int lib_BeginItemTooltip(lua_State* L)
{
    std::ignore = L;
    bool const result = ImGui::BeginItemTooltip();
    lua_pushboolean(L, result);
    return 1;
}
static int lib_SetItemTooltip(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    ImGui::SetItemTooltip(text);
    return 0;
}
static /* XXXX */ int lib_SetItemTooltipV(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}

//////// Popups, Modals

static int lib_BeginPopup(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 1);
    if (lua_gettop(L) <= 1)
    {
        const bool ret = ImGui::BeginPopup(str_id);
        lua_pushboolean(L, ret);
    }
    else
    {
        const ImGuiWindowFlags flags = (ImGuiWindowFlags)luaL_checkinteger(L, 2);
        const bool ret = ImGui::BeginPopup(str_id, flags);
        lua_pushboolean(L, ret);
    }
    return 1;
}
static int lib_BeginPopupModal(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* name = luaL_checkstring(L, 1);
    if (argc <= 1)
    {
        const bool ret = ImGui::BeginPopupModal(name);
        lua_pushboolean(L, ret);
        return 1;
    }
    else if (argc == 2)
    {
        bool p_open = lua_toboolean(L, 2);
        const bool ret = ImGui::BeginPopupModal(name, &p_open);
        lua_pushboolean(L, ret);
        lua_pushboolean(L, p_open);
        return 2;
    }
    else
    {
        bool p_open = lua_toboolean(L, 2);
        const ImGuiWindowFlags flags = (ImGuiWindowFlags)luaL_checkinteger(L, 3);
        const bool ret = ImGui::BeginPopupModal(name, &p_open, flags);
        lua_pushboolean(L, ret);
        lua_pushboolean(L, p_open);
        return 2;
    }
}
static int lib_EndPopup(lua_State* L)
{
    std::ignore = L;
    ImGui::EndPopup();
    return 0;
}
//////// Popups: open/close functions
static int lib_OpenPopup(lua_State* L)
{
    switch(lua_type(L, 1))
    {
    case LUA_TNUMBER:
        {
            const ImGuiID id = (ImGuiID)luaL_checkinteger(L, 1);
            if (lua_gettop(L) <= 1)
            {
                ImGui::OpenPopup(id);
            }
            else
            {
                const ImGuiPopupFlags flags = (ImGuiPopupFlags)luaL_checkinteger(L, 2);
                ImGui::OpenPopup(id, flags);
            }
            return 0;
        }
    case LUA_TSTRING:
    default:
        {
            const char* str_id = luaL_checkstring(L, 1);
            if (lua_gettop(L) <= 1)
            {
                ImGui::OpenPopup(str_id);
            }
            else
            {
                const ImGuiPopupFlags flags = (ImGuiPopupFlags)luaL_checkinteger(L, 2);
                ImGui::OpenPopup(str_id, flags);
            }
            return 0;
        }
    }
}
static int lib_OpenPopupOnItemClick(lua_State* L)
{
    const int argc = lua_gettop(L);
    if (argc <= 0)
    {
        ImGui::OpenPopupOnItemClick();
    }
    else if (argc == 1)
    {
        const char* str_id = luaL_checkstring(L, 1);
        ImGui::OpenPopupOnItemClick(str_id);
    }
    else
    {
        const char* str_id = luaL_checkstring(L, 1);
        const ImGuiPopupFlags popup_flags = (ImGuiPopupFlags)luaL_checkinteger(L, 2);
        ImGui::OpenPopupOnItemClick(str_id, popup_flags);
    }
    return 0;
}
static int lib_CloseCurrentPopup(lua_State* L)
{
    std::ignore = L;
    ImGui::CloseCurrentPopup();
    return 0;
}
//////// Popups: open+begin combined functions helpers
static int lib_BeginPopupContextItem(lua_State* L)
{
    const int argc = lua_gettop(L);
    if (argc <= 0)
    {
        const bool ret = ImGui::BeginPopupContextItem();
        lua_pushboolean(L, ret);
    }
    else if (argc == 1)
    {
        const char* str_id = luaL_checkstring(L, 1);
        const bool ret = ImGui::BeginPopupContextItem(str_id);
        lua_pushboolean(L, ret);
    }
    else
    {
        const char* str_id = luaL_checkstring(L, 1);
        const ImGuiPopupFlags popup_flags = (ImGuiPopupFlags)luaL_checkinteger(L, 2);
        const bool ret = ImGui::BeginPopupContextItem(str_id, popup_flags);
        lua_pushboolean(L, ret);
    }
    return 1;
}
static int lib_BeginPopupContextWindow(lua_State* L)
{
    const int argc = lua_gettop(L);
    if (argc <= 0)
    {
        const bool ret = ImGui::BeginPopupContextWindow();
        lua_pushboolean(L, ret);
    }
    else if (argc == 1)
    {
        const char* str_id = luaL_checkstring(L, 1);
        const bool ret = ImGui::BeginPopupContextWindow(str_id);
        lua_pushboolean(L, ret);
    }
    else
    {
        const char* str_id = luaL_checkstring(L, 1);
        const ImGuiPopupFlags popup_flags = (ImGuiPopupFlags)luaL_checkinteger(L, 2);
        const bool ret = ImGui::BeginPopupContextWindow(str_id, popup_flags);
        lua_pushboolean(L, ret);
    }
    return 1;
}
static int lib_BeginPopupContextVoid(lua_State* L)
{
    const int argc = lua_gettop(L);
    if (argc <= 0)
    {
        const bool ret = ImGui::BeginPopupContextVoid();
        lua_pushboolean(L, ret);
    }
    else if (argc == 1)
    {
        const char* str_id = luaL_checkstring(L, 1);
        const bool ret = ImGui::BeginPopupContextVoid(str_id);
        lua_pushboolean(L, ret);
    }
    else
    {
        const char* str_id = luaL_checkstring(L, 1);
        const ImGuiPopupFlags popup_flags = (ImGuiPopupFlags)luaL_checkinteger(L, 2);
        const bool ret = ImGui::BeginPopupContextVoid(str_id, popup_flags);
        lua_pushboolean(L, ret);
    }
    return 1;
}
//////// Popups: test function
static int lib_IsPopupOpen(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 1);
    if (lua_gettop(L) <= 1)
    {
        const bool ret = ImGui::IsPopupOpen(str_id);
        lua_pushboolean(L, ret);
    }
    else
    {
        const ImGuiPopupFlags flags = (ImGuiPopupFlags)luaL_checkinteger(L, 2);
        const bool ret = ImGui::IsPopupOpen(str_id, flags);
        lua_pushboolean(L, ret);
    }
    return 1;
}

//////// Tables

static int lib_BeginTable(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* str_id = luaL_checkstring(L, 1);
    const int column = (int)luaL_checkinteger(L, 2);
    if (argc <= 2)
    {
        const bool ret = ImGui::BeginTable(str_id, column);
        lua_pushboolean(L, ret);
    }
    else if (argc == 3)
    {
        const ImGuiTableFlags row_flags = (ImGuiTableFlags)luaL_checkinteger(L, 3);
        const bool ret = ImGui::BeginTable(str_id, column, row_flags);
        lua_pushboolean(L, ret);
    }
    else if (argc == 4)
    {
        const ImGuiTableFlags row_flags = (ImGuiTableFlags)luaL_checkinteger(L, 3);
        ImVec2* outer_size = imgui::binding::ImVec2Binding::as(L, 4);
        const bool ret = ImGui::BeginTable(str_id, column, row_flags, *outer_size);
        lua_pushboolean(L, ret);
    }
    else
    {
        const ImGuiTableFlags row_flags = (ImGuiTableFlags)luaL_checkinteger(L, 3);
        ImVec2* outer_size = imgui::binding::ImVec2Binding::as(L, 4);
        const float inner_width = (float)luaL_checknumber(L, 5);
        const bool ret = ImGui::BeginTable(str_id, column, row_flags, *outer_size, inner_width);
        lua_pushboolean(L, ret);
    }
    return 1;
}
static int lib_EndTable(lua_State* L)
{
    std::ignore = L;
    ImGui::EndTable();
    return 0;
}
static int lib_TableNextRow(lua_State* L)
{
    const int argc = lua_gettop(L);
    if (argc <= 0)
    {
        ImGui::TableNextRow();
    }
    else if (argc == 1)
    {
        const ImGuiTableRowFlags row_flags = (ImGuiTableRowFlags)luaL_checkinteger(L, 1);
        ImGui::TableNextRow(row_flags);
    }
    else
    {
        const ImGuiTableRowFlags row_flags = (ImGuiTableRowFlags)luaL_checkinteger(L, 1);
        const float min_row_height = (float)luaL_checknumber(L, 2);
        ImGui::TableNextRow(row_flags, min_row_height);
    }
    return 0;
}
static int lib_TableNextColumn(lua_State* L)
{
    const bool ret = ImGui::TableNextColumn();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_TableSetColumnIndex(lua_State* L)
{
    const int column_n = (int)luaL_checkinteger(L, 1) - 1;
    const bool ret = ImGui::TableSetColumnIndex(column_n);
    lua_pushboolean(L, ret);
    return 1;
}
//////// Tables: Headers & Columns declaration
static int lib_TableSetupColumn(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    if (argc <= 1)
    {
        ImGui::TableSetupColumn(label);
    }
    else if (argc == 2)
    {
        const ImGuiTableColumnFlags flags = (ImGuiTableColumnFlags)luaL_checkinteger(L, 2);
        ImGui::TableSetupColumn(label, flags);
    }
    else if (argc == 3)
    {
        const ImGuiTableColumnFlags flags = (ImGuiTableColumnFlags)luaL_checkinteger(L, 2);
        const float init_width_or_weight = (float)luaL_checknumber(L, 3);
        ImGui::TableSetupColumn(label, flags, init_width_or_weight);
    }
    else
    {
        const ImGuiTableColumnFlags flags = (ImGuiTableColumnFlags)luaL_checkinteger(L, 2);
        const float init_width_or_weight = (float)luaL_checknumber(L, 3);
        const ImGuiID user_id = (ImGuiID)luaL_checkinteger(L, 4);
        ImGui::TableSetupColumn(label, flags, init_width_or_weight, user_id);
    }
    return 0;
}
static int lib_TableSetupScrollFreeze(lua_State* L)
{
    const int cols = (int)luaL_checkinteger(L, 1);
    const int rows = (int)luaL_checkinteger(L, 2);
    ImGui::TableSetupScrollFreeze(cols, rows);
    return 0;
}
static int lib_TableHeader(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    ImGui::TableHeader(label);
    return 0;
}
static int lib_TableHeadersRow(lua_State* L)
{
    std::ignore = L;
    ImGui::TableHeadersRow();
    return 0;
}
static int lib_TableAngledHeadersRow(lua_State* L)
{
    std::ignore = L;
    ImGui::TableAngledHeadersRow();
    return 0;
}
//////// Tables: Sorting & Miscellaneous functions
static /* !!!! */ int lib_TableGetSortSpecs(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static int lib_TableGetColumnCount(lua_State* L)
{
    const int ret = ImGui::TableGetColumnCount();
    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}
static int lib_TableGetColumnIndex(lua_State* L)
{
    const int ret = ImGui::TableGetColumnIndex() + 1;
    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}
static int lib_TableGetRowIndex(lua_State* L)
{
    const int ret = ImGui::TableGetRowIndex() + 1;
    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}
static int lib_TableGetColumnName(lua_State* L)
{
    if (lua_gettop(L) <= 0)
    {
        const char* ret = ImGui::TableGetColumnName();
        lua_pushstring(L, ret);
    }
    else
    {
        const int column_n = (int)luaL_checkinteger(L, 1);
        const char* ret = ImGui::TableGetColumnName(column_n);
        lua_pushstring(L, ret);
    }
    return 1;
}
static int lib_TableGetColumnFlags(lua_State* L)
{
    if (lua_gettop(L) <= 0)
    {
        const ImGuiTableColumnFlags ret = ImGui::TableGetColumnFlags();
        lua_pushinteger(L, (lua_Integer)ret);
    }
    else
    {
        const int column_n = (int)luaL_checkinteger(L, 1);
        const ImGuiTableColumnFlags ret = ImGui::TableGetColumnFlags(column_n);
        lua_pushinteger(L, (lua_Integer)ret);
    }
    return 1;
}
static int lib_TableSetColumnEnabled(lua_State* L)
{
    const int column_n = (int)luaL_checkinteger(L, 1);
    const bool v = lua_toboolean(L, 2);
    ImGui::TableSetColumnEnabled(column_n, v);
    return 0;
}
static int lib_TableGetHoveredColumn(lua_State* L)
{
    const auto column = ImGui::TableGetHoveredColumn();
    lua_pushinteger(L, column);
    return 1;
}
static int lib_TableSetBgColor(lua_State* L)
{
    if (lua_gettop(L) <= 2)
    {
        const ImGuiTableBgTarget target = (ImGuiTableBgTarget)luaL_checkinteger(L, 1);
        const ImU32 color = (ImU32)luaL_checkinteger(L, 2);
        ImGui::TableSetBgColor(target, color);
    }
    else
    {
        const ImGuiTableBgTarget target = (ImGuiTableBgTarget)luaL_checkinteger(L, 1);
        const ImU32 color = (ImU32)luaL_checkinteger(L, 2);
        const int column_n = (int)luaL_checkinteger(L, 3);
        ImGui::TableSetBgColor(target, color, column_n);
    }
    return 0;
}

//////// Legacy Columns API

static int lib_Columns(lua_State* L)
{
    const int argc = lua_gettop(L);
    if (argc <= 0)
    {
        ImGui::Columns();
    }
    else if (argc == 1)
    {
        const int count = luaL_checkinteger(L, 1);
        ImGui::Columns(count);
    }
    else if (argc == 2)
    {
        const int count = luaL_checkinteger(L, 1);
        const char* id = luaL_checkstring(L, 2);
        ImGui::Columns(count, id);
    }
    else
    {
        const int count = luaL_checkinteger(L, 1);
        const char* id = luaL_checkstring(L, 2);
        const bool border = lua_toboolean(L, 3);
        ImGui::Columns(count, id, border);
    }
    return 0;
}
static int lib_NextColumn(lua_State* L)
{
    std::ignore = L;
    ImGui::NextColumn();
    return 0;
}
static int lib_GetColumnIndex(lua_State* L)
{
    const int ret = ImGui::GetColumnIndex();
    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}
static int lib_GetColumnWidth(lua_State* L)
{
    const int column_index = luaL_optinteger(L, 1, -1);
    const float ret = ImGui::GetColumnWidth(column_index);
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_SetColumnWidth(lua_State* L)
{
    const int column_index = luaL_checkinteger(L, 1);
    const float width = (float)luaL_checknumber(L, 2);
    ImGui::SetColumnWidth(column_index, width);
    return 0;
}
static int lib_GetColumnOffset(lua_State* L)
{
    const int column_index = luaL_optinteger(L, 1, -1);
    const float ret = ImGui::GetColumnOffset(column_index);
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_SetColumnOffset(lua_State* L)
{
    const int column_index = luaL_checkinteger(L, 1);
    const float offset_x = (float)luaL_checknumber(L, 2);
    ImGui::SetColumnOffset(column_index, offset_x);
    return 0;
}
static int lib_GetColumnsCount(lua_State* L)
{
    const int ret = ImGui::GetColumnsCount();
    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

//////// Tab Bars, Tabs

static int lib_BeginTabBar(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 1);
    const ImGuiTabBarFlags flags = (ImGuiTabBarFlags)luaL_optinteger(L, 2, 0);
    const bool ret = ImGui::BeginTabBar(str_id, flags);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_EndTabBar(lua_State* L)
{
    std::ignore = L;
    ImGui::EndTabBar();
    return 0;
}
static int lib_BeginTabItem(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    if (argc <= 1)
    {
        const bool ret = ImGui::BeginTabItem(label);
        lua_pushboolean(L, ret);
        return 1;
    }
    else if (argc == 2)
    {
        bool p_open = lua_toboolean(L, 2);
        const bool ret = ImGui::BeginTabItem(label, &p_open);
        lua_pushboolean(L, ret);
        lua_pushboolean(L, p_open);
        return 2;
    }
    else
    {
        bool p_open = lua_toboolean(L, 2);
        const ImGuiTabItemFlags flags = (ImGuiTabItemFlags)luaL_checkinteger(L, 3);
        const bool ret = ImGui::BeginTabItem(label, &p_open, flags);
        lua_pushboolean(L, ret);
        lua_pushboolean(L, p_open);
        return 2;
    }
}
static int lib_EndTabItem(lua_State* L)
{
    std::ignore = L;
    ImGui::EndTabItem();
    return 0;
}
static int lib_TabItemButton(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    const ImGuiTabItemFlags flags = (ImGuiTabItemFlags)luaL_optinteger(L, 2, 0);
    const bool ret = ImGui::TabItemButton(label, flags);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_SetTabItemClosed(lua_State* L)
{
    const char* tab_or_docked_window_label = luaL_checkstring(L, 1);
    ImGui::SetTabItemClosed(tab_or_docked_window_label);
    return 0;
}

//////// Logging/Capture

static int lib_LogToTTY(lua_State* L)
{
    const int auto_open_depth = (int)luaL_optinteger(L, 1, -1);
    ImGui::LogToTTY(auto_open_depth);
    return 0;
}
static int lib_LogToFile(lua_State* L)
{
    const int argc = lua_gettop(L);
    if (argc <= 0)
    {
        ImGui::LogToFile();
    }
    else if (argc == 1)
    {
        const int auto_open_depth = (int)luaL_checkinteger(L, 1);
        ImGui::LogToFile(auto_open_depth);
    }
    else
    {
        const int auto_open_depth = (int)luaL_checkinteger(L, 1);
        const char* filename = luaL_checkstring(L, 2);
        ImGui::LogToFile(auto_open_depth, filename);
    }
    return 0;
}
static int lib_LogToClipboard(lua_State* L)
{
    const int auto_open_depth = (int)luaL_optinteger(L, 1, -1);
    ImGui::LogToClipboard(auto_open_depth);
    return 0;
}
static int lib_LogFinish(lua_State* L)
{
    std::ignore = L;
    ImGui::LogFinish();
    return 0;
}
static int lib_LogButtons(lua_State* L)
{
    std::ignore = L;
    ImGui::LogButtons();
    return 0;
}
static int lib_LogText(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    ImGui::LogText(text);
    return 0;
}

//////// Drag and Drop

static /* !!!! */ int lib_BeginDragDropSource(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* !!!! */ int lib_SetDragDropPayload(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* !!!! */ int lib_EndDragDropSource(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* !!!! */ int lib_BeginDragDropTarget(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* !!!! */ int lib_AcceptDragDropPayload(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* !!!! */ int lib_EndDragDropTarget(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* !!!! */ int lib_GetDragDropPayload(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}

//////// Disabling

static int lib_BeginDisabled(lua_State* L)
{
    if (lua_gettop(L) < 1)
    {
        ImGui::BeginDisabled();
    }
    else
    {
        const bool disabled = lua_toboolean(L, 1);
        ImGui::BeginDisabled(disabled);
    }
    return 0;
}
static int lib_EndDisabled(lua_State* L)
{
    std::ignore = L;
    ImGui::EndDisabled();
    return 0;
}

//////// Clipping

static int lib_PushClipRect(lua_State* L)
{
    ImVec2* clip_rect_min = imgui::binding::ImVec2Binding::as(L, 1);
    ImVec2* clip_rect_max = imgui::binding::ImVec2Binding::as(L, 2);
    const bool intersect_with_current_clip_rect = lua_toboolean(L, 3);
    ImGui::PushClipRect(*clip_rect_min, *clip_rect_max, intersect_with_current_clip_rect);
    return 0;
}
static int lib_PopClipRect(lua_State* L)
{
    std::ignore = L;
    ImGui::PopClipRect();
    return 0;
}

//////// Focus, Activation

static int lib_SetItemDefaultFocus(lua_State* L)
{
    std::ignore = L;
    ImGui::SetItemDefaultFocus();
    return 0;
}
static int lib_SetKeyboardFocusHere(lua_State* L)
{
    const int offset = (int)luaL_optinteger(L, 1, 0);
    ImGui::SetKeyboardFocusHere(offset);
    return 0;
}

//////// Keyboard/Gamepad Navigation

static int lib_SetNavCursorVisible(lua_State* L) {
    auto const visible = lua_toboolean(L, 1);
    ImGui::SetNavCursorVisible(visible);
    return 0;
}

//////// Overlapping mode

static int lib_SetNextItemAllowOverlap(lua_State* L)
{
    std::ignore = L;
    ImGui::SetNextItemAllowOverlap();
    return 0;
}

//////// Item/Widgets Utilities

static int lib_IsItemHovered(lua_State* L)
{
    const ImGuiHoveredFlags flags = (ImGuiHoveredFlags)luaL_optinteger(L, 1, 0);
    const bool ret = ImGui::IsItemHovered(flags);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsItemActive(lua_State* L)
{
    const bool ret = ImGui::IsItemActive();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsItemFocused(lua_State* L)
{
    const bool ret = ImGui::IsItemFocused();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsItemClicked(lua_State* L)
{
    const ImGuiMouseButton mouse_button = (ImGuiMouseButton)luaL_optinteger(L, 1, 0);
    const bool ret = ImGui::IsItemClicked(mouse_button);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsItemVisible(lua_State* L)
{
    const bool ret = ImGui::IsItemVisible();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsItemEdited(lua_State* L)
{
    const bool ret = ImGui::IsItemEdited();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsItemActivated(lua_State* L)
{
    const bool ret = ImGui::IsItemActivated();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsItemDeactivated(lua_State* L)
{
    const bool ret = ImGui::IsItemDeactivated();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsItemDeactivatedAfterEdit(lua_State* L)
{
    const bool ret = ImGui::IsItemDeactivatedAfterEdit();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsItemToggledOpen(lua_State* L)
{
    const bool ret = ImGui::IsItemToggledOpen();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsAnyItemHovered(lua_State* L)
{
    const bool ret = ImGui::IsAnyItemHovered();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsAnyItemActive(lua_State* L)
{
    const bool ret = ImGui::IsAnyItemActive();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsAnyItemFocused(lua_State* L)
{
    const bool ret = ImGui::IsAnyItemFocused();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_GetItemID(lua_State* L)
{
    const ImGuiID ret = ImGui::GetItemID();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_GetItemRectMin(lua_State* L)
{
    ImVec2* vec2 = imgui::binding::ImVec2Binding::create(L);
    *vec2 = ImGui::GetItemRectMin();
    return 1;
}
static int lib_GetItemRectMax(lua_State* L)
{
    ImVec2* vec2 = imgui::binding::ImVec2Binding::create(L);
    *vec2 = ImGui::GetItemRectMax();
    return 1;
}
static int lib_GetItemRectSize(lua_State* L)
{
    ImVec2* vec2 = imgui::binding::ImVec2Binding::create(L);
    *vec2 = ImGui::GetItemRectSize();
    return 1;
}

//////// Background/Foreground Draw Lists

static /* !!!! */ int lib_GetBackgroundDrawList(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* !!!! */ int lib_GetForegroundDrawList(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}

//////// Miscellaneous Utilities

static int lib_IsRectVisible(lua_State* L)
{
    ImVec2* v1 = imgui::binding::ImVec2Binding::as(L, 1);
    if (lua_gettop(L) >= 2)
    {
        ImVec2* v2 = imgui::binding::ImVec2Binding::as(L, 2);
        const bool ret = ImGui::IsRectVisible(*v1, *v2);
        lua_pushboolean(L, ret);
    }
    else
    {
        const bool ret = ImGui::IsRectVisible(*v1);
        lua_pushboolean(L, ret);
    }
    return 1;
}
static int lib_GetTime(lua_State* L)
{
    const double ret = ImGui::GetTime();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_GetFrameCount(lua_State* L)
{
    const int ret = ImGui::GetFrameCount();
    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}
static /* !!!! */ int lib_GetDrawListSharedData(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static int lib_GetStyleColorName(lua_State* L)
{
    const ImGuiCol idx = (ImGuiCol)luaL_checkinteger(L, 1);
    const char* ret = ImGui::GetStyleColorName(idx);
    lua_pushstring(L, ret);
    return 1;
}
static /* !!!! */ int lib_SetStateStorage(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* !!!! */ int lib_GetStateStorage(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}

//////// Text Utilities

static int lib_CalcTextSize(lua_State* L)
{
    const int argc = lua_gettop(L);
    if (lua_type(L, 2) == LUA_TNUMBER)
    {
        const auto text_length = lua_tointeger(L, 2);
        const char* text = luaL_checkstring(L, 1);
        const char* text_end = text + text_length;
        const bool hide_text_after_double_hash = (argc >= 3) ? lua_toboolean(L, 3) : false;
        const float wrap_width = (argc >= 4) ? (float)luaL_checknumber(L, 4) : -1.0f;
        ImVec2* vec2 = imgui::binding::ImVec2Binding::create(L);
        *vec2 = ImGui::CalcTextSize(text, text_end, hide_text_after_double_hash, wrap_width);
    }
    else
    {
        size_t text_length = 0;
        const char* text = luaL_checklstring(L, 1, &text_length);
        const char* text_end = text + text_length;
        const bool hide_text_after_double_hash = (argc >= 2) ? lua_toboolean(L, 2) : false;
        const float wrap_width = (argc >= 3) ? (float)luaL_checknumber(L, 3) : -1.0f;
        ImVec2* vec2 = imgui::binding::ImVec2Binding::create(L);
        *vec2 = ImGui::CalcTextSize(text, text_end, hide_text_after_double_hash, wrap_width);
    }
    return 1;
}

//////// Color Utilities

static int lib_ColorConvertU32ToFloat4(lua_State* L)
{
    const ImU32 in_ = (ImU32)luaL_checkinteger(L, 1);
    ImVec4* ret = imgui::binding::ImVec4Binding::create(L);
    *ret = ImGui::ColorConvertU32ToFloat4(in_);
    return 1;
}
static int lib_ColorConvertFloat4ToU32(lua_State* L)
{
    ImVec4* in_ = imgui::binding::ImVec4Binding::as(L, 1);
    const ImU32 ret = ImGui::ColorConvertFloat4ToU32(*in_);
    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}
static int lib_ColorConvertRGBtoHSV(lua_State* L)
{
    const float r = (float)luaL_checknumber(L, 1);
    const float g = (float)luaL_checknumber(L, 2);
    const float b = (float)luaL_checknumber(L, 3);
    float out_h = 0.0f, out_s = 0.0f, out_v = 0.0f;
    ImGui::ColorConvertRGBtoHSV(r, g, b, out_h, out_s, out_v);
    lua_pushnumber(L, (lua_Number)out_h);
    lua_pushnumber(L, (lua_Number)out_s);
    lua_pushnumber(L, (lua_Number)out_v);
    return 3;
}
static int lib_ColorConvertHSVtoRGB(lua_State* L)
{
    const float h = (float)luaL_checknumber(L, 1);
    const float s = (float)luaL_checknumber(L, 2);
    const float v = (float)luaL_checknumber(L, 3);
    float out_r = 0.0f, out_g = 0.0f, out_b = 0.0f;
    ImGui::ColorConvertHSVtoRGB(h, s, v, out_r, out_g, out_b);
    lua_pushnumber(L, (lua_Number)out_r);
    lua_pushnumber(L, (lua_Number)out_g);
    lua_pushnumber(L, (lua_Number)out_b);
    return 3;
}

//////// Inputs Utilities: Keyboard/Mouse/Gamepad

static int lib_IsKeyDown(lua_State* L)
{
    const ImGuiKey key = (ImGuiKey)luaL_checkinteger(L, 1);
    const bool ret = ImGui::IsKeyDown(key);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsKeyPressed(lua_State* L)
{
    const int argc = lua_gettop(L);
    const ImGuiKey key = (ImGuiKey)luaL_checkinteger(L, 1);
    const bool repeat = (argc >= 2) ? lua_toboolean(L, 2) : true;
    const bool ret = ImGui::IsKeyPressed(key, repeat);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsKeyReleased(lua_State* L)
{
    const ImGuiKey key = (ImGuiKey)luaL_checkinteger(L, 1);
    const bool ret = ImGui::IsKeyReleased(key);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsKeyChordPressed(lua_State* L)
{
    const auto key = (ImGuiKeyChord)luaL_checkinteger(L, 1);
    const bool ret = ImGui::IsKeyChordPressed(key);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_GetKeyPressedAmount(lua_State* L)
{
    const ImGuiKey key = (ImGuiKey)luaL_checkinteger(L, 1);
    const float repeat_delay = (float)luaL_checknumber(L, 2);
    const float rate = (float)luaL_checknumber(L, 3);
    const int ret = ImGui::GetKeyPressedAmount(key, repeat_delay, rate);
    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}
static int lib_GetKeyName(lua_State* L)
{
    const ImGuiKey key = (ImGuiKey)luaL_checkinteger(L, 1);
    const char* name = ImGui::GetKeyName(key);
    lua_pushstring(L, name);
    return 1;
}
static int lib_SetNextFrameWantCaptureKeyboard(lua_State* L)
{
    const bool want_capture_keyboard = lua_toboolean(L, 1);
    ImGui::SetNextFrameWantCaptureKeyboard(want_capture_keyboard);
    return 0;
}

//////// Inputs Utilities: Shortcut Testing & Routing [BETA]

static int lib_Shortcut(lua_State* L)
{
    const auto key_chord = (ImGuiKeyChord)luaL_checkinteger(L, 1);
    const auto flags = (ImGuiInputFlags)luaL_optinteger(L, 2, 0);
    const bool ret = ImGui::Shortcut(key_chord, flags);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_SetNextItemShortcut(lua_State* L)
{
    const auto key_chord = (ImGuiKeyChord)luaL_checkinteger(L, 1);
    const auto flags = (ImGuiInputFlags)luaL_optinteger(L, 2, 0);
    ImGui::SetNextItemShortcut(key_chord, flags);
    return 0;
}

//////// Inputs Utilities: Key/Input Ownership [BETA]

static int lib_SetItemKeyOwner(lua_State* L)
{
    auto const key = (ImGuiKey)luaL_checkinteger(L, 1);
    ImGui::SetItemKeyOwner(key);
    return 0;
}

//////// Inputs Utilities: Mouse specific

static int lib_IsMouseDown(lua_State* L)
{
    const ImGuiMouseButton button = (ImGuiMouseButton)luaL_checkinteger(L, 1);
    const bool ret = ImGui::IsMouseDown(button);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsMouseClicked(lua_State* L)
{
    const int argc = lua_gettop(L);
    const ImGuiMouseButton button = (ImGuiMouseButton)luaL_checkinteger(L, 1);
    const bool repeat = (argc >= 2) ? lua_toboolean(L, 2) : false;
    const bool ret = ImGui::IsMouseClicked(button, repeat);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsMouseReleased(lua_State* L)
{
    const ImGuiMouseButton button = (ImGuiMouseButton)luaL_checkinteger(L, 1);
    const bool ret = ImGui::IsMouseReleased(button);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsMouseDoubleClicked(lua_State* L)
{
    const ImGuiMouseButton button = (ImGuiMouseButton)luaL_checkinteger(L, 1);
    const bool ret = ImGui::IsMouseDoubleClicked(button);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsMouseReleasedWithDelay(lua_State* L)
{
    const ImGuiMouseButton button = (ImGuiMouseButton)luaL_checkinteger(L, 1);
    const float delay = (float)luaL_checknumber(L, 2);
    const bool ret = ImGui::IsMouseReleasedWithDelay(button, delay);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_GetMouseClickedCount(lua_State* L)
{
    const ImGuiMouseButton button = (ImGuiMouseButton)luaL_checkinteger(L, 1);
    const bool ret = ImGui::GetMouseClickedCount(button);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsMouseHoveringRect(lua_State* L)
{
    const int argc = lua_gettop(L);
    ImVec2* r_min = imgui::binding::ImVec2Binding::as(L, 1);
    ImVec2* r_max = imgui::binding::ImVec2Binding::as(L, 2);
    const bool clip = (argc >= 3) ? lua_toboolean(L, 3) : true;
    const bool ret = ImGui::IsMouseHoveringRect(*r_min, *r_max, clip);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsMousePosValid(lua_State* L)
{
    const int argc = lua_gettop(L);
    ImVec2* mouse_pos = (argc >= 1) ? imgui::binding::ImVec2Binding::as(L, 1) : NULL;
    const bool ret = ImGui::IsMousePosValid(mouse_pos);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsAnyMouseDown(lua_State* L)
{
    const bool ret = ImGui::IsAnyMouseDown();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_GetMousePos(lua_State* L)
{
    ImVec2* vec2 = imgui::binding::ImVec2Binding::create(L);
    *vec2 = ImGui::GetMousePos();
    return 1;
}
static int lib_GetMousePosOnOpeningCurrentPopup(lua_State* L)
{
    ImVec2* vec2 = imgui::binding::ImVec2Binding::create(L);
    *vec2 = ImGui::GetMousePosOnOpeningCurrentPopup();
    return 1;
}
static int lib_IsMouseDragging(lua_State* L)
{
    const ImGuiMouseButton button = (ImGuiMouseButton)luaL_optinteger(L, 1, 0);
    const float lock_threshold = (float)luaL_optnumber(L, 2, -1.0f);
    const bool ret = ImGui::IsMouseDragging(button, lock_threshold);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_GetMouseDragDelta(lua_State* L)
{
    const ImGuiMouseButton button = (ImGuiMouseButton)luaL_optinteger(L, 1, 0);
    const float lock_threshold = (float)luaL_optnumber(L, 2, -1.0f);
    ImVec2* vec2 = imgui::binding::ImVec2Binding::create(L);
    *vec2 = ImGui::GetMouseDragDelta(button, lock_threshold);
    return 1;
}
static int lib_ResetMouseDragDelta(lua_State* L)
{
    const ImGuiMouseButton button = (ImGuiMouseButton)luaL_optinteger(L, 1, 0);
    ImGui::ResetMouseDragDelta(button);
    return 0;
}
static int lib_GetMouseCursor(lua_State* L)
{
    const ImGuiMouseCursor ret = ImGui::GetMouseCursor();
    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}
static int lib_SetMouseCursor(lua_State* L)
{
    const ImGuiMouseCursor cursor_type = (ImGuiMouseCursor)luaL_checkinteger(L, 1);
    ImGui::SetMouseCursor(cursor_type);
    return 0;
}
static int lib_SetNextFrameWantCaptureMouse(lua_State* L)
{
    const bool want_capture_mouse = lua_toboolean(L, 1);
    ImGui::SetNextFrameWantCaptureMouse(want_capture_mouse);
    return 0;
}

//////// Clipboard Utilities

static int lib_GetClipboardText(lua_State* L)
{
    const char* text = ImGui::GetClipboardText();
    lua_pushstring(L, text);
    return 1;
}
static int lib_SetClipboardText(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    ImGui::SetClipboardText(text);
    return 0;
}

//////// Settings/.Ini Utilities

static int lib_LoadIniSettingsFromDisk(lua_State* L)
{
    const char* file = luaL_checkstring(L, 1);
    ImGui::LoadIniSettingsFromDisk(file);
    return 0;
}
static int lib_LoadIniSettingsFromMemory(lua_State* L)
{
    const char* data = luaL_checkstring(L, 1);
    if (lua_gettop(L) >= 2)
    {
        const size_t size = (size_t)luaL_checkinteger(L, 2);
        ImGui::LoadIniSettingsFromMemory(data, size);
    }
    else
    {
        const size_t size = _luaL_szlen(L, 1);
        ImGui::LoadIniSettingsFromMemory(data, size);
    }
    return 0;
}
static int lib_SaveIniSettingsToDisk(lua_State* L)
{
    const char* file = luaL_checkstring(L, 1);
    ImGui::SaveIniSettingsToDisk(file);
    return 0;
}
static int lib_SaveIniSettingsToMemory(lua_State* L)
{
    size_t size = 0;
    const char* data = ImGui::SaveIniSettingsToMemory(&size);
    lua_pushlstring(L, data, size);
    lua_pushinteger(L, (lua_Integer)size);
    return 2;
}

//////// Debug Utilities

static int lib_DebugTextEncoding(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    ImGui::DebugTextEncoding(text);
    return 0;
}
static int lib_DebugFlashStyleColor(lua_State* L)
{
    const auto idx = (ImGuiCol)luaL_checkinteger(L, 1);
    ImGui::DebugFlashStyleColor(idx);
    return 0;
}
static int lib_DebugStartItemPicker(lua_State*)
{
    ImGui::DebugStartItemPicker();
    return 0;
}
static int lib_DebugCheckVersionAndDataLayout(lua_State* L)
{
    const bool ret = IMGUI_CHECKVERSION();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_DebugLog(lua_State* L)
{
    size_t len{};
    char const* str = luaL_checklstring(L, 1, &len);
    ImGui::DebugLog("%.*s", static_cast<int>(len), str);
    return 0;
}
static /* XXXX */ int lib_DebugLogV(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}

//////// Memory Allocators

static /* XXXX */ int lib_SetAllocatorFunctions(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* XXXX */ int lib_GetAllocatorFunctions(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* XXXX */ int lib_MemAlloc(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* XXXX */ int lib_MemFree(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}

void imgui_binding_lua_register_ImGui(lua_State* L)
{
#include "lua_imgui_ImGui_register.inl"
    //                                  // ? M
    lua_pushstring(L, "ImGui");         // ? M k
    lua_gettable(L, -2);                // ? M t
    _luaL_setfuncs(L, lib_fun);         // ? M t
    lua_pop(L, 1);                      // ? M
}
