#include "imgui/lua_imgui_common.hpp"
#include "imgui/lua_imgui_ImGui.hpp"
#include "imgui/lua_imgui_type.hpp"
#include "imgui.h"

template<typename T, auto V>
struct type_array
{
    T* data = nullptr;
    type_array(size_t n)
    {
        data = new T[n];
        for (size_t i = 0; i < n; i += 1)
        {
            data[i] = (T)V;
        }
    }
    ~type_array()
    {
        delete[] data;
        data = nullptr;
    }
};
using char_array = type_array<char*, nullptr>;
using float_array = type_array<float, 0>;

//////// Context creation and access

// - CreateContext
// - DestroyContext
// - GetCurrentContext
// - SetCurrentContext

//////// Main

static /* !!!! */ int lib_GetIO(lua_State* L)
{
    return 0;
}
static int lib_GetStyle(lua_State* L)
{
    imgui_binding_lua_ref_ImGuiStyle(L, &ImGui::GetStyle());
    return 1;
}
static int lib_NewFrame(lua_State* L)
{
    ImGui::NewFrame();
    return 0;
}
static int lib_EndFrame(lua_State* L)
{
    ImGui::EndFrame();
    return 0;
}
static int lib_Render(lua_State* L)
{
    ImGui::Render();
    return 0;
}
static /* !!!! */ int lib_GetDrawData(lua_State* L)
{
    return 0;
}

//////// Demo, Debug, Information

static int lib_ShowDemoWindow(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        bool v = lua_toboolean(L, 1);
        ImGui::ShowDemoWindow(&v);
        lua_pushboolean(L, v);
        return 1;
    }
    else
    {
        ImGui::ShowDemoWindow();
        return 0;
    }
}
static int lib_ShowAboutWindow(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        bool v = lua_toboolean(L, 1);
        ImGui::ShowAboutWindow(&v);
        lua_pushboolean(L, v);
        return 1;
    }
    else
    {
        ImGui::ShowAboutWindow();
        return 0;
    }
}
static int lib_ShowMetricsWindow(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        bool v = lua_toboolean(L, 1);
        ImGui::ShowMetricsWindow(&v);
        lua_pushboolean(L, v);
        return 1;
    }
    else
    {
        ImGui::ShowMetricsWindow();
        return 0;
    }
}
static int lib_ShowStyleEditor(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        ImGuiStyle* style = imgui_binding_lua_to_ImGuiStyle(L, 1);
        ImGui::ShowStyleEditor(style);
    }
    else
    {
        ImGui::ShowStyleEditor();
    }
    return 0;
}
static int lib_ShowStyleSelector(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    const bool r = ImGui::ShowStyleSelector(label);
    lua_pushboolean(L, r);
    return 1;
}
static int lib_ShowFontSelector(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    ImGui::ShowFontSelector(label);
    return 0;
}
static int lib_ShowUserGuide(lua_State* L)
{
    ImGui::ShowUserGuide();
    return 0;
}
static int lib_GetVersion(lua_State* L)
{
    lua_pushstring(L, ImGui::GetVersion());
    return 1;
}

//////// Styles

static int lib_StyleColorsDark(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        ImGuiStyle* style = imgui_binding_lua_to_ImGuiStyle(L, 1);
        ImGui::StyleColorsDark(style);
    }
    else
    {
        ImGui::StyleColorsDark();
    }
    return 0;
}
static int lib_StyleColorsClassic(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        ImGuiStyle* style = imgui_binding_lua_to_ImGuiStyle(L, 1);
        ImGui::StyleColorsClassic(style);
    }
    else
    {
        ImGui::StyleColorsClassic();
    }
    return 0;
}
static int lib_StyleColorsLight(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        ImGuiStyle* style = imgui_binding_lua_to_ImGuiStyle(L, 1);
        ImGui::StyleColorsLight(style);
    }
    else
    {
        ImGui::StyleColorsLight();
    }
    return 0;
}

//////// Windows

static int lib_Begin(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    if (lua_gettop(L) <= 1)
    {
        const bool r = ImGui::Begin(name);
        lua_pushboolean(L, r);
        return 1;
    }
    else
    {
        bool p_open = lua_toboolean(L, 2);
        const ImGuiWindowFlags flags = (ImGuiWindowFlags)luaL_optinteger(L, 3, 0);
        const bool r = ImGui::Begin(name, &p_open, flags);
        lua_pushboolean(L, r);
        lua_pushboolean(L, p_open);
        return 2;
    }
}
static int lib_End(lua_State* L)
{
    ImGui::End();
    return 0;
}

//////// Child Windows

static int lib_BeginChild(lua_State* L)
{
    switch(lua_type(L, 1))
    {
    case LUA_TNUMBER:
        if (lua_gettop(L) <= 1)
        {
            const ImGuiID id = (ImGuiID)luaL_checkinteger(L, 1);
            const bool r = ImGui::BeginChild(id);
            lua_pushboolean(L, r);
        }
        else
        {
            const ImGuiID id = (ImGuiID)luaL_checkinteger(L, 1);
            ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
            const bool border = lua_toboolean(L, 3);
            const ImGuiWindowFlags flags = (ImGuiWindowFlags)luaL_optinteger(L, 4, 0);
            const bool r = ImGui::BeginChild(id, *size, border, flags);
            lua_pushboolean(L, r);
        }
        return 1;
    case LUA_TSTRING:
    default:
        if (lua_gettop(L) <= 1)
        {
            const char* str_id = luaL_checkstring(L, 1);
            const bool r = ImGui::BeginChild(str_id);
            lua_pushboolean(L, r);
        }
        else
        {
            const char* str_id = luaL_checkstring(L, 1);
            ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
            const bool border = lua_toboolean(L, 3);
            const ImGuiWindowFlags flags = (ImGuiWindowFlags)luaL_optinteger(L, 4, 0);
            const bool r = ImGui::BeginChild(str_id, *size, border, flags);
            lua_pushboolean(L, r);
        }
        return 1;
    };
}
static int lib_EndChild(lua_State* L)
{
    ImGui::EndChild();
    return 0;
}

//////// Windows Utilities

static int lib_IsWindowAppearing(lua_State* L)
{
    const bool r = ImGui::IsWindowAppearing();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsWindowCollapsed(lua_State* L)
{
    const bool r = ImGui::IsWindowCollapsed();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsWindowFocused(lua_State* L)
{
    const ImGuiFocusedFlags flags = (ImGuiFocusedFlags)luaL_optinteger(L, 1, 0);
    const bool r = ImGui::IsWindowFocused(flags);
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsWindowHovered(lua_State* L)
{
    const ImGuiFocusedFlags flags = (ImGuiFocusedFlags)luaL_optinteger(L, 1, 0);
    const bool r = ImGui::IsWindowHovered(flags);
    lua_pushboolean(L, r);
    return 1;
}
static /* !!!! */ int lib_GetWindowDrawList(lua_State* L)
{
    return 0;
}
static int lib_GetWindowPos(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetWindowPos();
    return 1;
}
static int lib_GetWindowSize(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetWindowSize();
    return 1;
}
static int lib_GetWindowWidth(lua_State* L)
{
    const float f = ImGui::GetWindowWidth();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}
static int lib_GetWindowHeight(lua_State* L)
{
    const float f = ImGui::GetWindowHeight();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}

static int lib_SetNextWindowPos(lua_State* L)
{
    if(lua_gettop(L) >= 3)
    {
        ImVec2* pos = imgui_binding_lua_to_ImVec2(L, 1);
        const int cond = luaL_checkinteger(L, 2);
        ImVec2* pivot = imgui_binding_lua_to_ImVec2(L, 3);
        ImGui::SetNextWindowPos(*pos, cond, *pivot);
    }
    else
    {
        ImVec2* pos = imgui_binding_lua_to_ImVec2(L, 1);
        const int cond = luaL_optinteger(L, 2, 0);
        ImGui::SetNextWindowPos(*pos, cond);
    }
    return 0;
}
static int lib_SetNextWindowSize(lua_State* L)
{
    ImVec2* size = imgui_binding_lua_to_ImVec2(L, 1);
    const int cond = luaL_optinteger(L, 2, 0);
    ImGui::SetNextWindowSize(*size, cond);
    return 0;
}
static /* !!!! */ int lib_SetNextWindowSizeConstraints(lua_State* L)
{
    ImVec2* v1 = imgui_binding_lua_to_ImVec2(L, 1);
    ImVec2* v2 = imgui_binding_lua_to_ImVec2(L, 2);
    ImGui::SetNextWindowSizeConstraints(*v1, *v2);
    return 0;
}
static int lib_SetNextWindowContentSize(lua_State* L)
{
    ImVec2* size = imgui_binding_lua_to_ImVec2(L, 1);
    ImGui::SetNextWindowContentSize(*size);
    return 0;
}
static int lib_SetNextWindowCollapsed(lua_State* L)
{
    const bool coll = lua_toboolean(L, 1);
    const int cond = luaL_optinteger(L, 2, 0);
    ImGui::SetNextWindowCollapsed(coll, cond);
    return 0;
}
static int lib_SetNextWindowFocus(lua_State* L)
{
    ImGui::SetNextWindowFocus();
    return 0;
}
static int lib_SetNextWindowBgAlpha(lua_State* L)
{
    const float f = (float)luaL_checknumber(L, 1);
    ImGui::SetNextWindowBgAlpha(f);
    return 0;
}
static int lib_SetWindowPos(lua_State* L)
{
    switch(lua_type(L, 1))
    {
    default:
        {
            ImVec2* pos = imgui_binding_lua_to_ImVec2(L, 1);
            const int cond = luaL_optinteger(L, 2, 0);
            ImGui::SetWindowPos(*pos, cond);
            return 0;
        }
    case LUA_TSTRING:
        {
            const char* name = luaL_checkstring(L, 1);
            ImVec2* pos = imgui_binding_lua_to_ImVec2(L, 2);
            const int cond = luaL_optinteger(L, 3, 0);
            ImGui::SetWindowPos(name, *pos, cond);
            return 0;
        }
    }
}
static int lib_SetWindowSize(lua_State* L)
{
    switch(lua_type(L, 1))
    {
    default:
        {
            ImVec2* size = imgui_binding_lua_to_ImVec2(L, 1);
            const int cond = luaL_optinteger(L, 2, 0);
            ImGui::SetWindowSize(*size, cond);
            return 0;
        }
    case LUA_TSTRING:
        {
            const char* name = luaL_checkstring(L, 1);
            ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
            const int cond = luaL_optinteger(L, 3, 0);
            ImGui::SetWindowSize(name, *size, cond);
            return 0;
        }
    }
}
static int lib_SetWindowCollapsed(lua_State* L)
{
    switch(lua_type(L, 1))
    {
    default:
        {
            const bool coll = lua_toboolean(L, 1);
            const int cond = luaL_optinteger(L, 2, 0);
            ImGui::SetWindowCollapsed(coll, cond);
            return 0;
        }
    case LUA_TSTRING:
        {
            const char* name = luaL_checkstring(L, 1);
            const bool coll = lua_toboolean(L, 2);
            const int cond = luaL_optinteger(L, 3, 0);
            ImGui::SetWindowCollapsed(name, coll, cond);
            return 0;
        }
    }
}
static int lib_SetWindowFocus(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        const char* name = luaL_checkstring(L, 1);
        ImGui::SetWindowFocus(name);
        return 0;
    }
    else
    {
        ImGui::SetWindowFocus();
        return 0;
    }
}
static int lib_SetWindowFontScale(lua_State* L)
{
    const float f = (float)luaL_checknumber(L, 1);
    ImGui::SetWindowFontScale(f);
    return 0;
}

//////// Content region

static int lib_GetContentRegionMax(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetContentRegionMax();
    return 1;
}
static int lib_GetContentRegionAvail(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetContentRegionAvail();
    return 1;
}
static int lib_GetWindowContentRegionMin(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetWindowContentRegionMin();
    return 1;
}
static int lib_GetWindowContentRegionMax(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetWindowContentRegionMax();
    return 1;
}
static int lib_GetWindowContentRegionWidth(lua_State* L)
{
    const float f = ImGui::GetWindowContentRegionWidth();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}

//////// Windows Scrolling

static int lib_GetScrollX(lua_State* L)
{
    const float f = ImGui::GetScrollX();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}
static int lib_GetScrollY(lua_State* L)
{
    const float f = ImGui::GetScrollY();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}
static int lib_GetScrollMaxX(lua_State* L)
{
    const float f = ImGui::GetScrollMaxX();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}
static int lib_GetScrollMaxY(lua_State* L)
{
    const float f = ImGui::GetScrollMaxY();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}
static int lib_SetScrollX(lua_State* L)
{
    const float v = (float)luaL_checknumber(L, 1);
    ImGui::SetScrollX(v);
    return 0;
}
static int lib_SetScrollY(lua_State* L)
{
    const float v = (float)luaL_checknumber(L, 1);
    ImGui::SetScrollY(v);
    return 0;
}
static int lib_SetScrollHereX(lua_State* L)
{
    const float v = (float)luaL_optnumber(L, 1, (lua_Number)0.5f);
    ImGui::SetScrollHereX(v);
    return 0;
}
static int lib_SetScrollHereY(lua_State* L)
{
    const float v = (float)luaL_optnumber(L, 1, (lua_Number)0.5f);
    ImGui::SetScrollHereY(v);
    return 0;
}
static int lib_SetScrollFromPosX(lua_State* L)
{
    const float v1 = (float)luaL_checknumber(L, 1);
    const float v2 = (float)luaL_optnumber(L, 2, (lua_Number)0.5f);
    ImGui::SetScrollFromPosX(v1, v2);
    return 0;
}
static int lib_SetScrollFromPosY(lua_State* L)
{
    const float v1 = (float)luaL_checknumber(L, 1);
    const float v2 = (float)luaL_optnumber(L, 2, (lua_Number)0.5f);
    ImGui::SetScrollFromPosY(v1, v2);
    return 0;
}

//////// Parameters stacks (shared)

static /* !!!! */ int lib_PushFont(lua_State* L)
{
    return 0;
}
static int lib_PopFont(lua_State* L)
{
    ImGui::PopFont();
    return 0;
}
static int lib_PushStyleColor(lua_State* L)
{
    const int idx = (int)luaL_checkinteger(L, 1);
    switch(lua_type(L, 2))
    {
    case LUA_TUSERDATA:
        {
            ImVec4* vec4 = imgui_binding_lua_to_ImVec4(L, 2);
            ImGui::PushStyleColor(idx, *vec4);
        }
        return 0;
    default:
    case LUA_TNUMBER:
        {
            const ImU32 c = (ImU32)luaL_checkinteger(L, 2);
            ImGui::PushStyleColor(idx, c);
        }
        return 0;
    }
}
static int lib_PopStyleColor(lua_State* L)
{
    const int n = (int)luaL_optinteger(L, 1, 1);
    ImGui::PopStyleColor(n);
    return 0;
}
static int lib_PushStyleVar(lua_State* L)
{
    const int idx = (int)luaL_checkinteger(L, 1);
    switch(lua_type(L, 2))
    {
    case LUA_TUSERDATA:
        {
            ImVec2* vec2 = imgui_binding_lua_to_ImVec2(L, 2);
            ImGui::PushStyleVar(idx, *vec2);
        }
        return 0;
    default:
    case LUA_TNUMBER:
        {
            const float f = (float)luaL_checknumber(L, 2);
            ImGui::PushStyleVar(idx, f);
        }
        return 0;
    }
}
static int lib_PopStyleVar(lua_State* L)
{
    const int n = (int)luaL_optinteger(L, 1, 1);
    ImGui::PopStyleVar(n);
    return 0;
}
static int lib_GetStyleColorVec4(lua_State* L)
{
    const int idx = (int)luaL_checkinteger(L, 1);
    ImVec4* vec4 = imgui_binding_lua_new_ImVec4(L);
    *vec4 = ImGui::GetStyleColorVec4(idx);
    return 1;
}
static /* !!!! */ int lib_GetFont(lua_State* L)
{
    return 0;
}
static int lib_GetFontSize(lua_State* L)
{
    const float f = ImGui::GetFontSize();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}
static int lib_GetFontTexUvWhitePixel(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetFontTexUvWhitePixel();
    return 1;
}
static int lib_GetColorU32(lua_State* L)
{
    switch(lua_type(L, 1))
    {
    case LUA_TUSERDATA:
        {
            ImVec4* c = imgui_binding_lua_to_ImVec4(L, 1);
            const ImU32 r = ImGui::GetColorU32(*c);
            lua_pushinteger(L, (lua_Integer)r);
        }
        return 1;
    default:
    case LUA_TNUMBER:
        {
            const int idx = (int)luaL_checkinteger(L, 1);
            const float mul = (float)luaL_optnumber(L, 2, (lua_Number)1.0f);
            const ImU32 r = ImGui::GetColorU32(idx, mul);
            lua_pushinteger(L, (lua_Integer)r);
        }
        return 1;
    }
}

//////// Parameters stacks (current window)

static int lib_PushItemWidth(lua_State* L)
{
    const float f = (float)luaL_checknumber(L, 1);
    ImGui::PushItemWidth(f);
    return 0;
}
static int lib_PopItemWidth(lua_State* L)
{
    ImGui::PopItemWidth();
    return 0;
}
static int lib_SetNextItemWidth(lua_State* L)
{
    const float f = (float)luaL_checknumber(L, 1);
    ImGui::SetNextItemWidth(f);
    return 0;
}
static int lib_CalcItemWidth(lua_State* L)
{
    const float f = ImGui::CalcItemWidth();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}
static int lib_PushTextWrapPos(lua_State* L)
{
    const float f = (float)luaL_optnumber(L, 1, (lua_Number)0.0f);
    ImGui::PushTextWrapPos(f);
    return 0;
}
static int lib_PopTextWrapPos(lua_State* L)
{
    ImGui::PopTextWrapPos();
    return 0;
}
static int lib_PushAllowKeyboardFocus(lua_State* L)
{
    const bool b = lua_toboolean(L, 1);
    ImGui::PushAllowKeyboardFocus(b);
    return 0;
}
static int lib_PopAllowKeyboardFocus(lua_State* L)
{
    ImGui::PopAllowKeyboardFocus();
    return 0;
}
static int lib_PushButtonRepeat(lua_State* L)
{
    const bool b = lua_toboolean(L, 1);
    ImGui::PushButtonRepeat(b);
    return 0;
}
static int lib_PopButtonRepeat(lua_State* L)
{
    ImGui::PopButtonRepeat();
    return 0;
}

//////// Cursor / Layout

static int lib_Separator(lua_State* L)
{
    ImGui::Separator();
    return 0;
}
static int lib_SameLine(lua_State* L)
{
    const float v1 = (float)luaL_optnumber(L, 1, (lua_Number)0.0f);
    const float v2 = (float)luaL_optnumber(L, 2, (lua_Number)-1.0f);
    ImGui::SameLine(v1, v2);
    return 0;
}
static int lib_NewLine(lua_State* L)
{
    ImGui::NewLine();
    return 0;
}
static int lib_Spacing(lua_State* L)
{
    ImGui::Spacing();
    return 0;
}
static int lib_Dummy(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_to_ImVec2(L, 1);
    ImGui::Dummy(*vec2);
    return 0;
}
static int lib_Indent(lua_State* L)
{
    const float v = (float)luaL_optnumber(L, 1, (lua_Number)0.0f);
    ImGui::Indent(v);
    return 0;
}
static int lib_Unindent(lua_State* L)
{
    const float v = (float)luaL_optnumber(L, 1, (lua_Number)0.0f);
    ImGui::Unindent(v);
    return 0;
}
static int lib_BeginGroup(lua_State* L)
{
    ImGui::BeginGroup();
    return 0;
}
static int lib_EndGroup(lua_State* L)
{
    ImGui::EndGroup();
    return 0;
}
static int lib_GetCursorPos(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetCursorPos();
    return 1;
}
static int lib_GetCursorPosX(lua_State* L)
{
    const float f = ImGui::GetCursorPosX();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}
static int lib_GetCursorPosY(lua_State* L)
{
    const float f = ImGui::GetCursorPosY();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}
static int lib_SetCursorPos(lua_State* L)
{
    ImVec2* pos = imgui_binding_lua_to_ImVec2(L, 1);
    ImGui::SetCursorPos(*pos);
    return 0;
}
static int lib_SetCursorPosX(lua_State* L)
{
    const float v = (float)luaL_checknumber(L, 1);
    ImGui::SetCursorPosX(v);
    return 0;
}
static int lib_SetCursorPosY(lua_State* L)
{
    const float v = (float)luaL_checknumber(L, 1);
    ImGui::SetCursorPosY(v);
    return 0;
}
static int lib_GetCursorStartPos(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetCursorStartPos();
    return 1;
}
static int lib_GetCursorScreenPos(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetCursorScreenPos();
    return 1;
}
static int lib_SetCursorScreenPos(lua_State* L)
{
    ImVec2* pos = imgui_binding_lua_to_ImVec2(L, 1);
    ImGui::SetCursorScreenPos(*pos);
    return 0;
}
static int lib_AlignTextToFramePadding(lua_State* L)
{
    ImGui::AlignTextToFramePadding();
    return 0;
}
static int lib_GetTextLineHeight(lua_State* L)
{
    const float f = ImGui::GetTextLineHeight();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}
static int lib_GetTextLineHeightWithSpacing(lua_State* L)
{
    const float f = ImGui::GetTextLineHeightWithSpacing();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}
static int lib_GetFrameHeight(lua_State* L)
{
    const float f = ImGui::GetFrameHeight();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}
static int lib_GetFrameHeightWithSpacing(lua_State* L)
{
    const float f = ImGui::GetFrameHeightWithSpacing();
    lua_pushnumber(L, (lua_Number)f);
    return 1;
}

//////// ID stack/scopes

static int lib_PushID(lua_State* L)
{
    switch(lua_type(L, 1))
    {
    case LUA_TLIGHTUSERDATA:
    case LUA_TUSERDATA:
        {
            const void* p = lua_touserdata(L, 1);
            ImGui::PushID(p);
        }
        return 0;
    case LUA_TNUMBER:
        {
            const int i = (int)lua_tointeger(L, 1);
            ImGui::PushID(i);
        }
        return 0;
    default:
    case LUA_TSTRING:
        {
            const char* s = lua_tostring(L, 1);
            ImGui::PushID(s);
        }
        return 0;
    }
}
static int lib_PopID(lua_State* L)
{
    ImGui::PopID();
    return 0;
}
static int lib_GetID(lua_State* L)
{
    switch(lua_type(L, 1))
    {
    case LUA_TLIGHTUSERDATA:
    case LUA_TUSERDATA:
        {
            const void* p = lua_touserdata(L, 1);
            const ImGuiID r = ImGui::GetID(p);
            lua_pushinteger(L, (lua_Integer)r);
        }
        return 1;
    case LUA_TNUMBER:
        {
            const int i = (int)lua_tointeger(L, 1);
            lua_pushinteger(L, (lua_Integer)(ImGuiID)i);
        }
        return 1;
    default:
    case LUA_TSTRING:
        {
            const char* s = lua_tostring(L, 1);
            const ImGuiID r = ImGui::GetID(s);
            lua_pushinteger(L, (lua_Integer)r);
        }
        return 1;
    }
}

//////// Widgets: Text

static int lib_TextUnformatted(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    ImGui::TextUnformatted(s);
    return 0;
}
static int lib_Text(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    ImGui::Text(s);
    return 0;
}
// - TextV
static int lib_TextColored(lua_State* L)
{
    ImVec4* vec4 = imgui_binding_lua_to_ImVec4(L, 1);
    const char* s = luaL_checkstring(L, 2);
    ImGui::TextColored(*vec4, s);
    return 0;
}
// - TextColoredV
static int lib_TextDisabled(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    ImGui::TextDisabled(s);
    return 0;
}
// - TextDisabledV
static int lib_TextWrapped(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    ImGui::TextWrapped(s);
    return 0;
}
// - TextWrappedV
static int lib_LabelText(lua_State* L)
{
    const char* s1 = luaL_checkstring(L, 1);
    const char* s2 = luaL_checkstring(L, 2);
    ImGui::LabelText(s1, s2);
    return 0;
}
// - LabelTextV
static int lib_BulletText(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    ImGui::BulletText(s);
    return 0;
}
// - BulletTextV

//////// Widgets: Main

static int lib_Button(lua_State* L)
{
    switch(lua_gettop(L))
    {
    default:
    case 1:
        {
            const char* s = luaL_checkstring(L, 1);
            const bool r = ImGui::Button(s);
            lua_pushboolean(L, r);
            return 1;
        }
    case 2:
        {
            const char* s = luaL_checkstring(L, 1);
            ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
            const bool r = ImGui::Button(s, *size);
            lua_pushboolean(L, r);
            return 1;
        }
    }
}
static int lib_SmallButton(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    const bool r = ImGui::SmallButton(s);
    lua_pushboolean(L, r);
    return 1;
}
static int lib_InvisibleButton(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
    const int flag = (int)luaL_optinteger(L, 3, 0);
    const bool r = ImGui::InvisibleButton(s, *size, flag);
    lua_pushboolean(L, r);
    return 1;
}
static int lib_ArrowButton(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    const int flag = (int)luaL_checkinteger(L, 2);
    const bool r = ImGui::ArrowButton(s, flag);
    lua_pushboolean(L, r);
    return 1;
}
static /* !!!! */ int lib_Image(lua_State* L)
{
    return 0;
}
static /* !!!! */ int lib_ImageButton(lua_State* L)
{
    return 0;
}
static int lib_Checkbox(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    bool b = lua_toboolean(L, 2);
    const bool r = ImGui::Checkbox(s, &b);
    lua_pushboolean(L, r);
    lua_pushboolean(L, b);
    return 2;
}
static int lib_CheckboxFlags(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    unsigned int v2 = (unsigned int)luaL_checkinteger(L, 2);
    const unsigned int v3 = (unsigned int)luaL_checkinteger(L, 3);
    const bool r = ImGui::CheckboxFlags(s, &v2, v3);
    lua_pushboolean(L, r);
    lua_pushinteger(L, (lua_Integer)v2);
    return 2;
}
static int lib_RadioButton(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    switch(lua_gettop(L))
    {
    default:
    case 2:
        {
            const bool b = lua_toboolean(L, 2);
            const bool r = ImGui::RadioButton(s, b);
            lua_pushboolean(L, r);
            return 1;
        }
    case 3:
        {
            int v2 = (int)luaL_checkinteger(L, 2);
            const int v3 = (int)luaL_checkinteger(L, 3);
            const bool r = ImGui::RadioButton(s, &v2, v3);
            lua_pushboolean(L, r);
            lua_pushinteger(L, (lua_Integer)v2);
            return 2;
        }
    }
}
static int lib_ProgressBar(lua_State* L)
{
    const float f = (float)luaL_checknumber(L, 1);
    switch(lua_gettop(L))
    {
    default:
    case 1:
        {
            ImGui::ProgressBar(f);
            return 0;
        }
    case 2:
        {
            ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
            ImGui::ProgressBar(f, *size);
            return 0;
        }
    case 3:
        {
            ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
            const char* s = luaL_checkstring(L, 3);
            ImGui::ProgressBar(f, *size, s);
            return 0;
        }
    }
}
static int lib_Bullet(lua_State* L)
{
    ImGui::Bullet();
    return 0;
}

//////// Widgets: Combo Box

static int lib_BeginCombo(lua_State* L)
{
    const char* s1 = luaL_checkstring(L, 1);
    const char* s2 = luaL_checkstring(L, 2);
    const int flag = (int)luaL_optinteger(L, 3, 0);
    const bool r = ImGui::BeginCombo(s1, s2, flag);
    lua_pushboolean(L, r);
    return 1;
}
static int lib_EndCombo(lua_State* L)
{
    ImGui::EndCombo();
    return 0;
}
static int lib_Combo(lua_State* L)
{
    const char* v1 = luaL_checkstring(L, 1);
    int v2 = (int)luaL_checkinteger(L, 2) - 1;
    switch(lua_type(L, 3))
    {
    default:
    case LUA_TSTRING:
        {
            const char* v3 = luaL_checkstring(L, 3);
            const int v4 = (int)luaL_optinteger(L, 4, -1);
            const bool r = ImGui::Combo(v1, &v2, v3, v4);
            lua_pushboolean(L, r);
        }
        break;
    case LUA_TTABLE:
        {
            // read all item
            const int n = _imgui_binding_lua_len(L, 3);
            char_array v3(n);
            for (int i = 1; i <= n; i += 1)
            {
                lua_pushinteger(L, i);
                lua_gettable(L, 3);
                v3.data[i - 1] = (char*)luaL_checkstring(L, -1);
                lua_pop(L, 1);
            }
            // Combo
            const int v4 = (int)luaL_optinteger(L, 4, -1);
            const bool r = ImGui::Combo(v1, &v2, v3.data, n, v4);
            lua_pushboolean(L, r);
        }
        break;
    case LUA_TFUNCTION:
        {
            // get item count
            const int v4 = (int)luaL_checkinteger(L, 4);
            // read all item from callback
            char_array v3(v4);
            for (int i = 1; i <= v4; i += 1)
            {
                lua_pushvalue(L, 3);
                lua_pushinteger(L, i);
                lua_call(L, 1, 1);
                v3.data[i - 1] = (char*)luaL_checkstring(L, -1);
                lua_pop(L, 1);
            }
            // Combo
            const int v5 = (int)luaL_optinteger(L, 5, -1);
            const bool r = ImGui::Combo(v1, &v2, v3.data, v4, v5);
            lua_pushboolean(L, r);
        }
        break;
    }
    lua_pushinteger(L, (lua_Integer)(v2 + 1));
    return 2;
}

//////// Widgets: Drag Sliders

template<const int N, auto F>
int _lib_DragFloatX(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    // read data
    float v2[N] = { 0 };
    for (int i = 0; i < N; i = 1)
    {
        lua_pushinteger(L, (lua_Integer)(i + 1));
        lua_gettable(L, 2);
        v2[i] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    // DragFloatX
    if (lua_gettop(L) <= 2)
    {
        const bool r = F(s, v2, 1.0f, 0.0f, 0.0f, "%.3f", 0);
        lua_pushboolean(L, r);
    }
    else
    {
        const float v3 = (float)luaL_optnumber(L, 3, (lua_Number)1.0f);
        const float v4 = (float)luaL_optnumber(L, 4, (lua_Number)0.0f);
        const float v5 = (float)luaL_optnumber(L, 5, (lua_Number)0.0f);
        const char* v6 = luaL_optstring(L, 6, "%.3f");
        const int v7 = (int)luaL_optinteger(L,  7, 0);
        const bool r = F(s, v2, v3, v4, v5, v6, v7);
        lua_pushboolean(L, r);
    }
    // write data
    for (int i = 0; i < N; i = 1)
    {
        lua_pushinteger(L, (lua_Integer)(i + 1));
        lua_pushnumber(L, (lua_Number)v2[i]);
        lua_settable(L, 2);
    }
    lua_pushvalue(L, 2);
    return 2;
}
template<const int N, auto F>
int _lib_DragIntX(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    // read data
    int v2[N] = { 0 };
    for (int i = 0; i < N; i = 1)
    {
        lua_pushinteger(L, (lua_Integer)(i + 1));
        lua_gettable(L, 2);
        v2[i] = (int)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    // DragFloatX
    if (lua_gettop(L) <= 2)
    {
        const bool r = F(s, v2, 1, 0, 0, "%d", 0);
        lua_pushboolean(L, r);
    }
    else
    {
        const int v3 = (int)luaL_optinteger(L, 3, (lua_Integer)1);
        const int v4 = (int)luaL_optinteger(L, 4, (lua_Integer)0);
        const int v5 = (int)luaL_optinteger(L, 5, (lua_Integer)0);
        const char* v6 = luaL_optstring(L, 6, "%d");
        const int v7 = (int)luaL_optinteger(L,  7, (lua_Integer)0);
        const bool r = F(s, v2, v3, v4, v5, v6, v7);
        lua_pushboolean(L, r);
    }
    // write data
    for (int i = 0; i < N; i = 1)
    {
        lua_pushinteger(L, (lua_Integer)(i + 1));
        lua_pushinteger(L, (lua_Integer)v2[i]);
        lua_settable(L, 2);
    }
    lua_pushvalue(L, 2);
    return 2;
}

static int lib_DragFloat(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    float v2 = (float)luaL_checknumber(L, 2);
    if (lua_gettop(L) <= 2)
    {
        const bool r = ImGui::DragFloat(s, &v2);
        lua_pushboolean(L, r);
    }
    else
    {
        const float v3 = (float)luaL_optnumber(L, 3, (lua_Number)1.0f);
        const float v4 = (float)luaL_optnumber(L, 4, (lua_Number)0.0f);
        const float v5 = (float)luaL_optnumber(L, 5, (lua_Number)0.0f);
        const char* v6 = luaL_optstring(L, 6, "%.3f");
        const int v7 = (int)luaL_optinteger(L,  7, 0);
        const bool r = ImGui::DragFloat(s, &v2, v3, v4, v5, v6, v7);
        lua_pushboolean(L, r);
    }
    lua_pushnumber(L, (lua_Number)v2);
    return 2;
}
static int lib_DragFloat2(lua_State* L)
{
    return _lib_DragFloatX<2, &ImGui::DragFloat2>(L);
}
static int lib_DragFloat3(lua_State* L)
{
    return _lib_DragFloatX<3, &ImGui::DragFloat3>(L);
}
static int lib_DragFloat4(lua_State* L)
{
    return _lib_DragFloatX<4, &ImGui::DragFloat4>(L);
}
static int lib_DragFloatRange2(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    float v2 = (float)luaL_checknumber(L, 2);
    float v3 = (float)luaL_checknumber(L, 3);
    if (lua_gettop(L) <= 3)
    {
        const bool r = ImGui::DragFloatRange2(s, &v2, &v3);
        lua_pushboolean(L, r);
    }
    else
    {
        const float v4 = (float)luaL_optnumber(L, 4, (lua_Number)1.0f);
        const float v5 = (float)luaL_optnumber(L, 5, (lua_Number)0.0f);
        const float v6 = (float)luaL_optnumber(L, 6, (lua_Number)0.0f);
        const char* v7 = luaL_optstring(L, 7, "%.3f");
        const char* v8 = luaL_optstring(L, 8, nullptr);
        const int v9 = (int)luaL_optinteger(L, 9, 0);
        const bool r = ImGui::DragFloatRange2(s, &v2, &v3, v4, v5, v6, v7, v8, v9);
        lua_pushboolean(L, r);
    }
    lua_pushnumber(L, (lua_Number)v2);
    lua_pushnumber(L, (lua_Number)v3);
    return 3;
}
static int lib_DragInt(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    int v2 = (int)luaL_checkinteger(L, 2);
    if (lua_gettop(L) <= 2)
    {
        const bool r = ImGui::DragInt(s, &v2);
        lua_pushboolean(L, r);
    }
    else
    {
        const int v3 = (int)luaL_optinteger(L, 3, (lua_Integer)1);
        const int v4 = (int)luaL_optinteger(L, 4, (lua_Integer)0);
        const int v5 = (int)luaL_optinteger(L, 5, (lua_Integer)0);
        const char* v6 = luaL_optstring(L, 6, "%d");
        const int v7 = (int)luaL_optinteger(L,  7, 0);
        const bool r = ImGui::DragInt(s, &v2, v3, v4, v5, v6, v7);
        lua_pushboolean(L, r);
    }
    lua_pushinteger(L, (lua_Integer)v2);
    return 2;
}
static int lib_DragInt2(lua_State* L)
{
    return _lib_DragIntX<2, &ImGui::DragInt2>(L);
}
static int lib_DragInt3(lua_State* L)
{
    return _lib_DragIntX<3, &ImGui::DragInt3>(L);
}
static int lib_DragInt4(lua_State* L)
{
    return _lib_DragIntX<4, &ImGui::DragInt4>(L);
}
static int lib_DragIntRange2(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    int v2 = (int)luaL_checkinteger(L, 2);
    int v3 = (int)luaL_checkinteger(L, 3);
    if (lua_gettop(L) <= 3)
    {
        const bool r = ImGui::DragIntRange2(s, &v2, &v3);
        lua_pushboolean(L, r);
    }
    else
    {
        const int v4 = (int)luaL_optinteger(L, 4, (lua_Integer)1);
        const int v5 = (int)luaL_optinteger(L, 5, (lua_Integer)0);
        const int v6 = (int)luaL_optinteger(L, 6, (lua_Integer)0);
        const char* v7 = luaL_optstring(L, 7, "%.3f");
        const char* v8 = luaL_optstring(L, 8, nullptr);
        const int v9 = (int)luaL_optinteger(L, 9, 0);
        const bool r = ImGui::DragIntRange2(s, &v2, &v3, v4, v5, v6, v7, v8, v9);
        lua_pushboolean(L, r);
    }
    lua_pushinteger(L, (lua_Integer)v2);
    lua_pushinteger(L, (lua_Integer)v3);
    return 3;
}
static /* !!!! */ int lib_DragScalar(lua_State* L)
{
    return 0;
}
static /* !!!! */ int lib_DragScalarN(lua_State* L)
{
    return 0;
}

//////// Widgets: Regular Sliders

template<const int N, auto F>
int _lib_SliderFloatX(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    float v[N];
    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        v[idx] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    const float v_min = (float)luaL_checknumber(L, 3);
    const float v_max = (float)luaL_checknumber(L, 4);
    const char* format = luaL_optstring(L, 5, "%.3f");
    const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_optinteger(L, 6, 0);
    
    const bool r = F(label, v, v_min, v_max, format, flags);
    
    lua_pushboolean(L, r);
    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_pushnumber(L, (lua_Number)v[idx]);
        lua_settable(L, 2);
    }
    lua_pushvalue(L, 2);
    return 2;
}
template<const int N, auto F>
int _lib_SliderIntX(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    int v[N];
    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        v[idx] = (int)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    const int v_min = (int)luaL_checkinteger(L, 3);
    const int v_max = (int)luaL_checkinteger(L, 4);
    const char* format = luaL_optstring(L, 5, "%d");
    const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_optinteger(L, 6, 0);
    
    const bool r = F(label, v, v_min, v_max, format, flags);
    
    lua_pushboolean(L, r);
    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_pushinteger(L, (lua_Integer)v[idx]);
        lua_settable(L, 2);
    }
    lua_pushvalue(L, 2);
    return 2;
}

static int lib_SliderFloat(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    float v = (float)luaL_checknumber(L, 2);
    const float v_min = (float)luaL_checknumber(L, 3);
    const float v_max = (float)luaL_checknumber(L, 4);
    const char* format = luaL_optstring(L, 5, "%.3f");
    const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_optinteger(L, 6, 0);
    const bool r = ImGui::SliderFloat(label, &v, v_min, v_max, format, flags);
    lua_pushboolean(L, r);
    lua_pushnumber(L, (lua_Number)v);
    return 2;
}
static int lib_SliderFloat2(lua_State* L)
{
    return _lib_SliderFloatX<2, &ImGui::SliderFloat2>(L);
}
static int lib_SliderFloat3(lua_State* L)
{
    return _lib_SliderFloatX<3, &ImGui::SliderFloat3>(L);
}
static int lib_SliderFloat4(lua_State* L)
{
    return _lib_SliderFloatX<4, &ImGui::SliderFloat4>(L);
}
static int lib_SliderAngle(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    float v_rad = (float)luaL_checknumber(L, 2);
    if (lua_gettop(L) <= 2)
    {
        const bool r = ImGui::SliderAngle(label, &v_rad);
        lua_pushboolean(L, r);
    }
    else
    {
        const float v_degrees_min = (float)luaL_optnumber(L, 3, -360.0f);
        const float v_degrees_max = (float)luaL_optnumber(L, 4, 360.0f);
        const char* format = luaL_optstring(L, 5, "%.0f deg");
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_optinteger(L, 6, 0);
        const bool r = ImGui::SliderAngle(
            label, &v_rad, v_degrees_min, v_degrees_max, format, flags);
        lua_pushboolean(L, r);
    }
    lua_pushnumber(L, (lua_Number)v_rad);
    return 2;
}
static int lib_SliderInt(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    int v = (int)luaL_checkinteger(L, 2);
    const int v_min = (float)luaL_checkinteger(L, 3);
    const int v_max = (float)luaL_checkinteger(L, 4);
    const char* format = luaL_optstring(L, 5, "%d");
    const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_optinteger(L, 6, 0);
    const bool r = ImGui::SliderInt(label, &v, v_min, v_max, format, flags);
    lua_pushboolean(L, r);
    lua_pushinteger(L, (lua_Integer)v);
    return 2;
}
static int lib_SliderInt2(lua_State* L)
{
    return _lib_SliderIntX<2, &ImGui::SliderInt2>(L);
}
static int lib_SliderInt3(lua_State* L)
{
    return _lib_SliderIntX<3, &ImGui::SliderInt3>(L);
}
static int lib_SliderInt4(lua_State* L)
{
    return _lib_SliderIntX<4, &ImGui::SliderInt4>(L);
}
static /* !!!! */ int lib_SliderScalar(lua_State* L)
{
    return 0;
}
static /* !!!! */ int lib_SliderScalarN(lua_State* L)
{
    return 0;
}
static int lib_VSliderFloat(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
    float v = (float)luaL_checknumber(L, 3);
    const float v_min = (float)luaL_checknumber(L, 4);
    const float v_max = (float)luaL_checknumber(L, 5);
    const char* format = luaL_optstring(L, 6, "%.3f");
    const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_optinteger(L, 7, 0);
    const bool r = ImGui::VSliderFloat(label, *size, &v, v_min, v_max, format, flags);
    lua_pushboolean(L, r);
    lua_pushnumber(L, (lua_Number)v);
    return 2;
}
static int lib_VSliderInt(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
    int v = (int)luaL_checkinteger(L, 3);
    const int v_min = (float)luaL_checkinteger(L, 4);
    const int v_max = (float)luaL_checkinteger(L, 5);
    const char* format = luaL_optstring(L, 6, "%d");
    const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_optinteger(L, 7, 0);
    const bool r = ImGui::VSliderInt(label, *size, &v, v_min, v_max, format, flags);
    lua_pushboolean(L, r);
    lua_pushinteger(L, (lua_Integer)v);
    return 2;
}
static /* !!!! */ int lib_VSliderScalar(lua_State* L)
{
    return 0;
}

//////// Widgets: Input with Keyboard

template<const int N, auto F>
int _lib_InputFloatX(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    float v[N];
    for (int i = 0; i < N; i += 1)
    {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, 2);
        v[i] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    const char* format = luaL_optstring(L, 3, "%.3f");
    const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_optinteger(L, 4, 0);
    
    const bool r = F(label, v, format, flags);
    
    lua_pushboolean(L, r);
    for (int i = 0; i < N; i += 1)
    {
        lua_pushinteger(L, i + 1);
        lua_pushnumber(L, (lua_Number)v[i]);
        lua_settable(L, 2);
    }
    lua_pushvalue(L, 2);
    return 2;
}
template<const int N, auto F>
int _lib_InputIntX(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    int v[N];
    for (int i = 0; i < N; i += 1)
    {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, 2);
        v[i] = (int)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_optinteger(L, 3, 0);
    
    const bool r = F(label, v, flags);
    
    lua_pushboolean(L, r);
    for (int i = 0; i < N; i += 1)
    {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, (lua_Integer)v[i]);
        lua_settable(L, 2);
    }
    lua_pushvalue(L, 2);
    return 2;
}

static int lib_InputText(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    ImGuiTextBuffer* buf = imgui_binding_lua_to_ImGuiTextBuffer(L, 2);
    const size_t buf_size = (size_t)luaL_checkinteger(L, 3);
    const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_optinteger(L, 4, 0);
    const bool r = ImGui::InputText(label, buf->Buf.Data, buf_size, flags);
    lua_pushboolean(L, r);
    return 1;
}
static int lib_InputTextMultiline(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    ImGuiTextBuffer* buf = imgui_binding_lua_to_ImGuiTextBuffer(L, 2);
    const size_t buf_size = (size_t)luaL_checkinteger(L, 3);
    if (lua_gettop(L) <= 3)
    {
        const bool r = ImGui::InputTextMultiline(label, buf->Buf.Data, buf_size);
        lua_pushboolean(L, r);
    }
    else
    {
        ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
        const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_optinteger(L, 5, 0);
        const bool r = ImGui::InputTextMultiline(label, buf->Buf.Data, buf_size, *size, flags);
        lua_pushboolean(L, r);
    }
    return 1;
}
static int lib_InputTextWithHint(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    const char* hint = luaL_checkstring(L, 2);
    ImGuiTextBuffer* buf = imgui_binding_lua_to_ImGuiTextBuffer(L, 3);
    const size_t buf_size = (size_t)luaL_checkinteger(L, 4);
    const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_optinteger(L, 5, 0);
    const bool r = ImGui::InputTextWithHint(label, hint, buf->Buf.Data, buf_size, flags);
    lua_pushboolean(L, r);
    return 1;
}
static int lib_InputFloat(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    float v = (float)luaL_checknumber(L, 2);
    const float step = (float)luaL_optnumber(L, 3, (lua_Number)0.0f);
    const float step_fast = (float)luaL_optnumber(L, 4, (lua_Number)0.0f);
    const char* format = luaL_optstring(L, 5, "%.3f");
    const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_optinteger(L, 6, 0);
    const bool r = ImGui::InputFloat(label, &v, step, step_fast, format, flags);
    lua_pushboolean(L, r);
    lua_pushnumber(L, (lua_Number)v);
    return 2;
}
static int lib_InputFloat2(lua_State* L)
{
    return _lib_InputFloatX<2, &ImGui::InputFloat2>(L);
}
static int lib_InputFloat3(lua_State* L)
{
    return _lib_InputFloatX<3, &ImGui::InputFloat3>(L);
}
static int lib_InputFloat4(lua_State* L)
{
    return _lib_InputFloatX<4, &ImGui::InputFloat4>(L);
}
static int lib_InputInt(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    int v = (int)luaL_checkinteger(L, 2);
    const int step = (int)luaL_optinteger(L, 3, 1);
    const int step_fast = (int)luaL_optinteger(L, 4, 100);
    const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_optinteger(L, 5, 0);
    const bool r = ImGui::InputInt(label, &v, step, step_fast, flags);
    lua_pushboolean(L, r);
    lua_pushinteger(L, (lua_Integer)v);
    return 2;
}
static int lib_InputInt2(lua_State* L)
{
    return _lib_InputIntX<2, &ImGui::InputInt2>(L);
}
static int lib_InputInt3(lua_State* L)
{
    return _lib_InputIntX<3, &ImGui::InputInt3>(L);
}
static int lib_InputInt4(lua_State* L)
{
    return _lib_InputIntX<4, &ImGui::InputInt4>(L);
}
static int lib_InputDouble(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    double v = (double)luaL_checknumber(L, 2);
    const double step = (double)luaL_optnumber(L, 3, (lua_Number)0.0);
    const double step_fast = (double)luaL_optnumber(L, 4, (lua_Number)0.0);
    const char* format = luaL_optstring(L, 5, "%.6f");
    const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_optinteger(L, 6, 0);
    const bool r = ImGui::InputDouble(label, &v, step, step_fast, format, flags);
    lua_pushboolean(L, r);
    lua_pushnumber(L, (lua_Number)v);
    return 2;
}
static /* !!!! */ int lib_InputScalar(lua_State* L)
{
    return 0;
}
static /* !!!! */ int lib_InputScalarN(lua_State* L)
{
    return 0;
}

//////// Widgets: Color Editor/Picker

template<const int N, auto F>
int _lib_ColorXXXXxXXXX(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    float col[N];
    for (int i = 0; i < N; i += 1)
    {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, 2);
        col[i] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    const ImGuiColorEditFlags flags = (ImGuiColorEditFlags)luaL_optinteger(L, 3, 0);
    
    const bool r = F(label, col, flags);
    
    lua_pushboolean(L, r);
    for (int i = 0; i < N; i += 1)
    {
        lua_pushinteger(L, i + 1);
        lua_pushnumber(L, (lua_Number)col[i]);
        lua_settable(L, 2);
    }
    lua_pushvalue(L, 2);
    return 2;
}

static int lib_ColorEdit3(lua_State* L)
{
    return _lib_ColorXXXXxXXXX<3, &ImGui::ColorEdit3>(L);
}
static int lib_ColorEdit4(lua_State* L)
{
    return _lib_ColorXXXXxXXXX<4, &ImGui::ColorEdit4>(L);
}
static int lib_ColorPicker3(lua_State* L)
{
    return _lib_ColorXXXXxXXXX<3, &ImGui::ColorPicker3>(L);
}
static int lib_ColorPicker4(lua_State* L)
{
    const int N = 4;
    const char* label = luaL_checkstring(L, 1);
    float col[N];
    for (int i = 0; i < N; i += 1)
    {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, 2);
        col[i] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    const ImGuiColorEditFlags flags = (ImGuiColorEditFlags)luaL_optinteger(L, 3, 0);
    float ref_col[4];
    const bool _ref_col = (lua_gettop(L) >= 4);
    if (_ref_col)
    {
        for (int i = 0; i < N; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_gettable(L, 4);
            ref_col[i] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
    }
    
    const bool r = ImGui::ColorPicker4(label, col, flags, _ref_col ? ref_col : NULL);
    
    lua_pushboolean(L, r);
    for (int i = 0; i < N; i += 1)
    {
        lua_pushinteger(L, i + 1);
        lua_pushnumber(L, (lua_Number)col[i]);
        lua_settable(L, 2);
    }
    lua_pushvalue(L, 2);
    return 2;
}
static int lib_ColorButton(lua_State* L)
{
    const char* desc_id = luaL_checkstring(L, 1);
    ImVec4* col = imgui_binding_lua_to_ImVec4(L, 2);
    const ImGuiColorEditFlags flags = (ImGuiColorEditFlags)luaL_optinteger(L, 3, 0);
    if (lua_gettop(L) <= 3)
    {
        const bool r = ImGui::ColorButton(desc_id, *col, flags);
        lua_pushboolean(L, r);
    }
    else
    {
        ImVec2* size = imgui_binding_lua_to_ImVec2(L, 4);
        const bool r = ImGui::ColorButton(desc_id, *col, flags, *size);
        lua_pushboolean(L, r);
    }
    return 1;
}
static int lib_SetColorEditOptions(lua_State* L)
{
    const ImGuiColorEditFlags flags = (ImGuiColorEditFlags)luaL_checkinteger(L, 1);
    ImGui::SetColorEditOptions(flags);
    return 0;
}

//////// Widgets: Trees

static int lib_TreeNode(lua_State* L)
{
    bool r = false;
    if (lua_gettop(L) <= 1)
    {
        const char* label = luaL_checkstring(L, 1);
        r = ImGui::TreeNode(label);
    }
    else
    {
        const char* fmt = luaL_checkstring(L, 2);
        switch (lua_type(L, 2))
        {
        case LUA_TUSERDATA:
            {
                const void* ptr_id = lua_touserdata(L, 1);
                r = ImGui::TreeNode(ptr_id, fmt);
            }
        case LUA_TSTRING:
        default:
            {
                const char* str_id = lua_tostring(L, 1);
                r = ImGui::TreeNode(str_id, fmt);
            }
        }
    }
    lua_pushboolean(L, r);
    return 1;
}
// - TreeNodeV
static int lib_TreeNodeEx(lua_State* L)
{
    bool r = false;
    if (lua_gettop(L) <= 2)
    {
        const char* label = luaL_checkstring(L, 1);
        const ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)luaL_optinteger(L, 2, 0);
        r = ImGui::TreeNodeEx(label, flags);
    }
    else
    {
        const ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)luaL_checkinteger(L, 2);
        const char* fmt = luaL_checkstring(L, 3);
        switch (lua_type(L, 2))
        {
        case LUA_TUSERDATA:
            {
                const void* ptr_id = lua_touserdata(L, 1);
                r = ImGui::TreeNodeEx(ptr_id, flags, fmt);
            }
        case LUA_TSTRING:
        default:
            {
                const char* str_id = lua_tostring(L, 1);
                r = ImGui::TreeNodeEx(str_id, flags, fmt);
            }
        }
    }
    lua_pushboolean(L, r);
    return 1;
}
// - TreeNodeExV
static int lib_TreePush(lua_State* L)
{
    switch (lua_type(L, 1))
    {
    case LUA_TNONE:
    case LUA_TNIL:
        ImGui::TreePush();
        break;
    case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
        {
            const void* ptr_id = lua_touserdata(L, 1);
            ImGui::TreePush(ptr_id);
        }
        break;
    case LUA_TSTRING:
    default:
        {
            const char* str_id = lua_tostring(L, 1);
            ImGui::TreePush(str_id);
        }
        break;
    }
    return 0;
}
static int lib_TreePop(lua_State* L)
{
    ImGui::TreePop();
    return 0;
}
static int lib_GetTreeNodeToLabelSpacing(lua_State* L)
{
    const float r = ImGui::GetTreeNodeToLabelSpacing();
    lua_pushnumber(L, (lua_Number)r);
    return 1;
}
static int lib_CollapsingHeader(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    if (lua_type(L, 2) == LUA_TBOOLEAN)
    {
        bool p_open = lua_toboolean(L, 2);
        const ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)luaL_optinteger(L, 3, 0);
        const bool r = ImGui::CollapsingHeader(label, &p_open, flags);
        lua_pushboolean(L, r);
        lua_pushboolean(L, p_open);
        return 2;
    }
    else
    {
        const ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)luaL_optinteger(L, 2, 0);
        const bool r = ImGui::CollapsingHeader(label, flags);
        lua_pushboolean(L, r);
        return 1;
    }
    
}
static int lib_SetNextItemOpen(lua_State* L)
{
    const bool is_open = lua_toboolean(L, 1);
    const ImGuiCond cond = (ImGuiCond)luaL_optinteger(L, 2, 0);
    ImGui::SetNextItemOpen(is_open, cond);
    return 0;
}

//////// Widgets: Selectables

static int lib_Selectable(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    const int top = lua_gettop(L);
    if (top <= 1)
    {
        const bool r = ImGui::Selectable(label);
        lua_pushboolean(L, r);
        return 1;
    }
    else if (top <= 3)
    {
        bool p_selected = lua_toboolean(L, 2);
        const ImGuiSelectableFlags flags = (ImGuiSelectableFlags)luaL_optinteger(L, 3, 0);
        const bool r = ImGui::Selectable(label, &p_selected, flags);
        lua_pushboolean(L, r);
        lua_pushboolean(L, p_selected);
        return 2;
    }
    else
    {
        bool p_selected = lua_toboolean(L, 2);
        const ImGuiSelectableFlags flags = (ImGuiSelectableFlags)luaL_checkinteger(L, 3);
        ImVec2* size = imgui_binding_lua_to_ImVec2(L, 4);
        const bool r = ImGui::Selectable(label, &p_selected, flags, *size);
        lua_pushboolean(L, r);
        lua_pushboolean(L, p_selected);
        return 2;
    }
}

//////// Widgets: List Boxes

static /* !!!! */ int lib_ListBox(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    int current_item = (int)luaL_checkinteger(L, 2);
    const int items_count = (int)luaL_checkinteger(L, 4);
    char_array items(items_count);
    for (int i = 0; i < items_count; i += 1)
    {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, 3);
        items.data[i] = (char*)luaL_checkstring(L, -1);
        lua_pop(L, 1);
    }
    const int height_in_items = (int)luaL_optinteger(L, 5, -1);
    const bool r = ImGui::ListBox(label, &current_item, items.data, items_count, height_in_items);
    lua_pushboolean(L, r);
    lua_pushinteger(L, (lua_Integer)current_item);
    return 2;
}
static int lib_ListBoxHeader(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    if (lua_gettop(L) <= 1)
    {
        const bool r = ImGui::ListBoxHeader(label);
        lua_pushboolean(L, r);
        return 1;
    }
    else
    {
        if (lua_type(L, 2) == LUA_TNUMBER)
        {
            const int items_count = (int)luaL_checkinteger(L, 2);
            const int height_in_items = (int)luaL_optinteger(L, 3, -1);
            const bool r = ImGui::ListBoxHeader(label, items_count, height_in_items);
            lua_pushboolean(L, r);
            return 1;
        }
        else
        {
            ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
            const bool r = ImGui::ListBoxHeader(label, *size);
            lua_pushboolean(L, r);
            return 1;
        }
    }
    
}
static int lib_ListBoxFooter(lua_State* L)
{
    ImGui::ListBoxFooter();
    return 0;
}

//////// Widgets: Data Plotting

static /* !!!! */ int lib_PlotLines(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    const int values_count = (int)luaL_checkinteger(L, 3);
    float_array values(values_count);
    for (int i = 0; i < values_count; i += 1)
    {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, 2);
        values.data[i] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    const int values_offset = (int)luaL_optinteger(L, 4, 0);
    const int top = lua_gettop(L);
    if (top <= 4)
    {
        ImGui::PlotLines(label, values.data, values_count, values_offset);
    }
    else if (top <= 7)
    {
        const char* overlay_text = luaL_checkstring(L, 5);
        const float scale_min = (float)luaL_optnumber(L, 6, (lua_Number)FLT_MAX);
        const float scale_max = (float)luaL_optnumber(L, 7, (lua_Number)FLT_MAX);
        ImGui::PlotLines(
            label, values.data, values_count, values_offset,
            overlay_text, scale_min, scale_max);
    }
    else
    {
        const char* overlay_text = luaL_checkstring(L, 5);
        const float scale_min = (float)luaL_checknumber(L, 6);
        const float scale_max = (float)luaL_checknumber(L, 7);
        ImVec2* graph_size = imgui_binding_lua_to_ImVec2(L, 8);
        const int stride = (int)luaL_optinteger(L, 9, sizeof(float));
        ImGui::PlotLines(
            label, values.data, values_count, values_offset,
            overlay_text, scale_min, scale_max, *graph_size, stride);
    }
    return 0;
}
static /* !!!! */ int lib_PlotHistogram(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    const int values_count = (int)luaL_checkinteger(L, 3);
    float_array values(values_count);
    for (int i = 0; i < values_count; i += 1)
    {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, 2);
        values.data[i] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    const int values_offset = (int)luaL_optinteger(L, 4, 0);
    const int top = lua_gettop(L);
    if (top <= 4)
    {
        ImGui::PlotHistogram(label, values.data, values_count, values_offset);
    }
    else if (top <= 7)
    {
        const char* overlay_text = luaL_checkstring(L, 5);
        const float scale_min = (float)luaL_optnumber(L, 6, (lua_Number)FLT_MAX);
        const float scale_max = (float)luaL_optnumber(L, 7, (lua_Number)FLT_MAX);
        ImGui::PlotHistogram(
            label, values.data, values_count, values_offset,
            overlay_text, scale_min, scale_max);
    }
    else
    {
        const char* overlay_text = luaL_checkstring(L, 5);
        const float scale_min = (float)luaL_checknumber(L, 6);
        const float scale_max = (float)luaL_checknumber(L, 7);
        ImVec2* graph_size = imgui_binding_lua_to_ImVec2(L, 8);
        const int stride = (int)luaL_optinteger(L, 9, sizeof(float));
        ImGui::PlotHistogram(
            label, values.data, values_count, values_offset,
            overlay_text, scale_min, scale_max, *graph_size, stride);
    }
    return 0;
}

//////// Widgets: Value() Helpers.

// - Value

//////// Widgets: Menus

static int lib_BeginMenuBar(lua_State* L)
{
    const bool r = ImGui::BeginMenuBar();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_EndMenuBar(lua_State* L)
{
    ImGui::EndMenuBar();
    return 0;
}
static int lib_BeginMainMenuBar(lua_State* L)
{
    const bool r = ImGui::BeginMainMenuBar();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_EndMainMenuBar(lua_State* L)
{
    ImGui::EndMainMenuBar();
    return 0;
}
static int lib_BeginMenu(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    bool enable = true;
    if(lua_gettop(L) >= 2)
    {
        enable = lua_toboolean(L, 2);
    }
    const bool r = ImGui::BeginMenu(text, enable);
    lua_pushboolean(L, r);
    return 1;
}
static int lib_EndMenu(lua_State* L)
{
    ImGui::EndMenu();
    return 0;
}
static int lib_MenuItem(lua_State* L)
{
    const char* s1 = luaL_checkstring(L, 1);
    const char* s2 = luaL_optstring(L, 2, NULL);
    switch (lua_gettop(L))
    {
    default:
    case 1:
    case 2:
        {
            const bool r = ImGui::MenuItem(s1, s2);
            lua_pushboolean(L, r);
            return 1;
        }
    case 3:
        {
            bool v3 = lua_toboolean(L, 3);
            const bool r = ImGui::MenuItem(s1, s2, &v3);
            lua_pushboolean(L, r);
            lua_pushboolean(L, v3);
            return 2;
        }
    case 4:
        {
            bool v3 = lua_toboolean(L, 3);
            const bool v4 = lua_toboolean(L, 4);
            const bool r = ImGui::MenuItem(s1, s2, &v3, v4);
            lua_pushboolean(L, r);
            lua_pushboolean(L, v3);
            return 2;
        }
    }
}

//////// Tooltips

static int lib_BeginTooltip(lua_State* L)
{
    ImGui::BeginTooltip();
    return 0;
}
static int lib_EndTooltip(lua_State* L)
{
    ImGui::EndTooltip();
    return 0;
}
static int lib_SetTooltip(lua_State* L)
{
    const char* text = lua_tostring(L, 1);
    ImGui::SetTooltip(text);
    return 0;
}
// - SetTooltipV

//////// Popups, Modals

static int lib_BeginPopup(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    const int flag = (int)luaL_optinteger(L, 2, 0);
    const bool r = ImGui::BeginPopup(text, flag);
    lua_pushboolean(L, r);
    return 1;
}
static int lib_BeginPopupModal(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    if(lua_gettop(L) >= 2)
    {
        bool open = lua_toboolean(L, 2);
        const int flag = (int)luaL_optinteger(L, 3, 0);
        const bool r = ImGui::BeginPopupModal(text, &open, flag);
        lua_pushboolean(L, r);
        lua_pushboolean(L, open);
        return 2;
    }
    else
    {
        const bool r = ImGui::BeginPopupModal(text);
        lua_pushboolean(L, r);
        return 1;
    }
}
static int lib_EndPopup(lua_State* L)
{
    ImGui::EndPopup();
    return 0;
}
static int lib_OpenPopup(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    const int flag = (int)luaL_optinteger(L, 2, 0);
    ImGui::OpenPopup(text, flag);
    return 0;
}
static int lib_OpenPopupOnItemClick(lua_State* L)
{
    
    if(lua_gettop(L) >= 1)
    {
        const char* text = luaL_checkstring(L, 1);
        const int flag = (int)luaL_optinteger(L, 2, 1);
        ImGui::OpenPopupOnItemClick(text, flag);
    }
    else
    {
        ImGui::OpenPopupOnItemClick();
    }
    return 0;
}
static int lib_CloseCurrentPopup(lua_State* L)
{
    ImGui::CloseCurrentPopup();
    return 0;
}
static int lib_BeginPopupContextItem(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        const char* text = luaL_checkstring(L, 1);
        const int flag = (int)luaL_optinteger(L, 2, 1);
        const bool r = ImGui::BeginPopupContextItem(text, flag);
        lua_pushboolean(L, r);
    }
    else
    {
        const bool r = ImGui::BeginPopupContextItem();
        lua_pushboolean(L, r);
    }
    return 1;
}
static int lib_BeginPopupContextWindow(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        const char* text = luaL_checkstring(L, 1);
        const int flag = (int)luaL_optinteger(L, 2, 1);
        const bool r = ImGui::BeginPopupContextWindow(text, flag);
        lua_pushboolean(L, r);
    }
    else
    {
        const bool r = ImGui::BeginPopupContextWindow();
        lua_pushboolean(L, r);
    }
    return 1;
}
static int lib_BeginPopupContextVoid(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        const char* text = luaL_checkstring(L, 1);
        const int flag = (int)luaL_optinteger(L, 2, 1);
        const bool r = ImGui::BeginPopupContextVoid(text, flag);
        lua_pushboolean(L, r);
    }
    else
    {
        const bool r = ImGui::BeginPopupContextVoid();
        lua_pushboolean(L, r);
    }
    return 1;
}
static int lib_IsPopupOpen(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    const int flag = (int)luaL_optinteger(L, 2, 0);
    const bool r = ImGui::IsPopupOpen(text, flag);
    lua_pushboolean(L, r);
    return 1;
}

//////// Columns

static int lib_Columns(lua_State* L)
{
    const int v1 = luaL_optinteger(L, 1, 1);
    switch(lua_gettop(L))
    {
    case 1:
        ImGui::Columns(v1);
        break;
    case 2:
        {
            const char* v2 = luaL_checkstring(L, 2);
            ImGui::Columns(v1, v2);
        }
        break;
    case 3:
        {
            const char* v2 = luaL_checkstring(L, 2);
            const bool v3 = lua_toboolean(L, 3);
            ImGui::Columns(v1, v2, v3);
        }
        break;
    }
    return 0;
}
static int lib_NextColumn(lua_State* L)
{
    ImGui::NextColumn();
    return 0;
}
static int lib_GetColumnIndex(lua_State* L)
{
    const int v = ImGui::GetColumnIndex();
    lua_pushinteger(L, (lua_Integer)v);
    return 1;
}
static int lib_GetColumnWidth(lua_State* L)
{
    const int v = luaL_optinteger(L, 1, -1);
    const float r = ImGui::GetColumnWidth(v);
    lua_pushnumber(L, (lua_Number)r);
    return 1;
}
static int lib_SetColumnWidth(lua_State* L)
{
    const int v1 = luaL_checkinteger(L, 1);
    const float v2 = (float)luaL_checknumber(L, 2);
    ImGui::SetColumnWidth(v1, v2);
    return 0;
}
static int lib_GetColumnOffset(lua_State* L)
{
    const int v = luaL_optinteger(L, 1, -1);
    const float r = ImGui::GetColumnOffset(v);
    lua_pushnumber(L, (lua_Number)r);
    return 1;
}
static int lib_SetColumnOffset(lua_State* L)
{
    const int v1 = luaL_checkinteger(L, 1);
    const float v2 = (float)luaL_checknumber(L, 2);
    ImGui::SetColumnOffset(v1, v2);
    return 0;
}
static int lib_GetColumnsCount(lua_State* L)
{
    const int v = ImGui::GetColumnsCount();
    lua_pushinteger(L, (lua_Integer)v);
    return 1;
}

//////// Tab Bars, Tabs

static int lib_BeginTabBar(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    const int flag = (int)luaL_checkinteger(L, 2);
    const bool r = ImGui::BeginTabBar(text, flag);
    lua_pushboolean(L, r);
    return 1;
}
static int lib_EndTabBar(lua_State* L)
{
    ImGui::EndTabBar();
    return 0;
}
static int lib_BeginTabItem(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    if(lua_gettop(L) >= 2)
    {
        bool open = lua_toboolean(L, 2);
        const int flag = (int)luaL_checkinteger(L, 3);
        const bool r = ImGui::BeginTabItem(text, &open, flag);
        lua_pushboolean(L, r);
        lua_pushboolean(L, open);
        return 2;
    }
    else
    {
        const bool r = ImGui::BeginTabItem(text);
        lua_pushboolean(L, r);
        return 1;
    }
}
static int lib_EndTabItem(lua_State* L)
{
    ImGui::EndTabItem();
    return 0;
}
static int lib_TabItemButton(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    const int flag = (int)luaL_checkinteger(L, 2);
    const bool r = ImGui::TabItemButton(text, flag);
    lua_pushboolean(L, r);
    return 1;
}
static int lib_SetTabItemClosed(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    ImGui::SetTabItemClosed(text);
    return 0;
}

//////// Logging/Capture

static int lib_LogToTTY(lua_State* L)
{
    const int v = (int)luaL_optinteger(L, 1, -1);
    ImGui::LogToTTY(v);
    return 0;
}
static int lib_LogToFile(lua_State* L)
{
    
    const int v = (int)luaL_optinteger(L, 1, -1);
    if(lua_gettop(L) == 2)
    {
        const char* t = lua_tostring(L, 2);
        ImGui::LogToFile(v, t);
    }
    else
    {
        ImGui::LogToFile(v);
    }
    return 0;
}
static int lib_LogToClipboard(lua_State* L)
{
    const int v = (int)luaL_optinteger(L, 1, -1);
    ImGui::LogToClipboard(v);
    return 0;
}
static int lib_LogFinish(lua_State* L)
{
    ImGui::LogFinish();
    return 0;
}
static int lib_LogButtons(lua_State* L)
{
    ImGui::LogButtons();
    return 0;
}
static int lib_LogText(lua_State* L)
{
    const char* text = lua_tostring(L, 1);
    ImGui::LogText(text);
    return 0;
}

//////// Drag and Drop

// - BeginDragDropSource
// - SetDragDropPayload
// - EndDragDropSource
// - BeginDragDropTarget
// - AcceptDragDropPayload
// - EndDragDropTarget
// - GetDragDropPayload

//////// Clipping

static int lib_PushClipRect(lua_State* L)
{
    ImVec2* v1 = imgui_binding_lua_to_ImVec2(L, 1);
    ImVec2* v2 = imgui_binding_lua_to_ImVec2(L, 2);
    const bool v3 = lua_toboolean(L, 3);
    ImGui::PushClipRect(*v1, *v2, v3);
    return 0;
}
static int lib_PopClipRect(lua_State* L)
{
    ImGui::PopClipRect();
    return 0;
}

//////// Focus, Activation

static int lib_SetItemDefaultFocus(lua_State* L)
{
    ImGui::SetItemDefaultFocus();
    return 0;
}
static int lib_SetKeyboardFocusHere(lua_State* L)
{
    const int offset = (int)luaL_optinteger(L, 1, 0);
    ImGui::SetKeyboardFocusHere(offset);
    return 0;
}

//////// Item/Widgets Utilities

static int lib_IsItemHovered(lua_State* L)
{
    const int flag = (int)luaL_optinteger(L, 1, 0);
    const bool r = ImGui::IsItemHovered(flag);
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsItemActive(lua_State* L)
{
    const bool r = ImGui::IsItemActive();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsItemFocused(lua_State* L)
{
    const bool r = ImGui::IsItemFocused();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsItemClicked(lua_State* L)
{
    const int flag = (int)luaL_optinteger(L, 1, 0);
    const bool r = ImGui::IsItemClicked(flag);
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsItemVisible(lua_State* L)
{
    const bool r = ImGui::IsItemVisible();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsItemEdited(lua_State* L)
{
    const bool r = ImGui::IsItemEdited();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsItemActivated(lua_State* L)
{
    const bool r = ImGui::IsItemActivated();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsItemDeactivated(lua_State* L)
{
    const bool r = ImGui::IsItemDeactivated();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsItemDeactivatedAfterEdit(lua_State* L)
{
    const bool r = ImGui::IsItemDeactivatedAfterEdit();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsItemToggledOpen(lua_State* L)
{
    const bool r = ImGui::IsItemToggledOpen();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsAnyItemHovered(lua_State* L)
{
    const bool r = ImGui::IsAnyItemHovered();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsAnyItemActive(lua_State* L)
{
    const bool r = ImGui::IsAnyItemActive();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_IsAnyItemFocused(lua_State* L)
{
    const bool r = ImGui::IsAnyItemFocused();
    lua_pushboolean(L, r);
    return 1;
}
static int lib_GetItemRectMin(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetItemRectMin();
    return 1;
}
static int lib_GetItemRectMax(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetItemRectMax();
    return 1;
}
static int lib_GetItemRectSize(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetItemRectSize();
    return 1;
}
static int lib_SetItemAllowOverlap(lua_State* L)
{
    ImGui::SetItemAllowOverlap();
    return 0;
}

//////// Miscellaneous Utilities

static int lib_IsRectVisible(lua_State* L)
{
    ImVec2* v1 = imgui_binding_lua_to_ImVec2(L, 1);
    if (lua_gettop(L) == 2)
    {
        ImVec2* v2 = imgui_binding_lua_to_ImVec2(L, 2);
        const bool b = ImGui::IsRectVisible(*v1, *v2);
        lua_pushboolean(L, b);
        return 1;
    }
    else
    {
        const bool b = ImGui::IsRectVisible(*v1);
        lua_pushboolean(L, b);
        return 1;
    }
}
static int lib_GetTime(lua_State* L)
{
    const double t = ImGui::GetTime();
    lua_pushnumber(L, (lua_Number)t);
    return 1;
}
static int lib_GetFrameCount(lua_State* L)
{
    const int v = ImGui::GetFrameCount();
    lua_pushinteger(L, (lua_Integer)v);
    return 1;
}
static /* !!!! */ int lib_GetBackgroundDrawList(lua_State* L)
{
    return 0;
}
static /* !!!! */ int lib_GetForegroundDrawList(lua_State* L)
{
    return 0;
}
static /* !!!! */ int lib_GetDrawListSharedData(lua_State* L)
{
    return 0;
}
static int lib_GetStyleColorName(lua_State* L)
{
    const int value = luaL_checkinteger(L, 1);
    const char* name = ImGui::GetStyleColorName(value);
    lua_pushstring(L, name);
    return 1;
}
static /* !!!! */ int lib_SetStateStorage(lua_State* L)
{
    return 0;
}
static /* !!!! */ int lib_GetStateStorage(lua_State* L)
{
    return 0;
}
static int lib_CalcListClipping(lua_State* L)
{
    const int v1 = luaL_checkinteger(L, 1);
    const float v2 = (float)luaL_checknumber(L, 2);
    int o1 = 0;
    int o2 = 0;
    ImGui::CalcListClipping(v1, v2, &o1, &o2);
    lua_pushinteger(L, (lua_Integer)o1);
    lua_pushinteger(L, (lua_Integer)o2);
    return 2;
}
static int lib_BeginChildFrame(lua_State* L)
{
    const ImGuiID id = (ImGuiID)luaL_checkinteger(L, 1);
    ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
    const int flag = luaL_checkinteger(L, 3);
    const bool b = ImGui::BeginChildFrame(id, *size, flag);
    lua_pushboolean(L, b);
    return 1;
}
static int lib_EndChildFrame(lua_State* L)
{
    ImGui::EndChildFrame();
    return 0;
}

//////// Text Utilities

static int lib_CalcTextSize(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    const bool v3 = lua_toboolean(L, 2);
    const float v4 = (float)luaL_checknumber(L, 3);
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::CalcTextSize(text, NULL, v3, v4);
    return 1;
}

//////// Inputs Utilities: Keyboard

static int lib_GetKeyIndex(lua_State* L)
{
    const int value = luaL_checkinteger(L, 1);
    const int index = ImGui::GetKeyIndex(value);
    lua_pushinteger(L, (lua_Integer)index);
    return 1;
}
static int lib_IsKeyDown(lua_State* L)
{
    const int value = luaL_checkinteger(L, 1);
    const bool b = ImGui::IsKeyDown(value);
    lua_pushboolean(L, b);
    return 1;
}
static int lib_IsKeyPressed(lua_State* L)
{
    const int v1 = luaL_checkinteger(L, 1);
    const bool v2 = lua_toboolean(L, 2);
    const bool b = ImGui::IsKeyPressed(v1, v2);
    lua_pushboolean(L, b);
    return 1;
}
static int lib_IsKeyReleased(lua_State* L)
{
    const int value = luaL_checkinteger(L, 1);
    const bool b = ImGui::IsKeyReleased(value);
    lua_pushboolean(L, b);
    return 1;
}
static int lib_GetKeyPressedAmount(lua_State* L)
{
    const int v1 = luaL_checkinteger(L, 1);
    const float v2 = (float)luaL_checknumber(L, 2);
    const float v3 = (float)luaL_checknumber(L, 3);
    const int r = ImGui::GetKeyPressedAmount(v1, v2, v3);
    lua_pushinteger(L, r);
    return 1;
}
static int lib_CaptureKeyboardFromApp(lua_State* L)
{
    const bool value = lua_toboolean(L, 1);
    ImGui::CaptureKeyboardFromApp(value);
    return 0;
}

//////// Inputs Utilities: Mouse

static int lib_IsMouseDown(lua_State* L)
{
    const int value = luaL_checkinteger(L, 1);
    const bool b = ImGui::IsMouseDown(value);
    lua_pushboolean(L, b);
    return 1;
}
static int lib_IsMouseClicked(lua_State* L)
{
    const int v1 = luaL_checkinteger(L, 1);
    const bool v2 = lua_toboolean(L, 2);
    const bool b = ImGui::IsMouseClicked(v1, v2);
    lua_pushboolean(L, b);
    return 1;
}
static int lib_IsMouseReleased(lua_State* L)
{
    const int value = luaL_checkinteger(L, 1);
    const bool b = ImGui::IsMouseReleased(value);
    lua_pushboolean(L, b);
    return 1;
}
static int lib_IsMouseDoubleClicked(lua_State* L)
{
    const int value = luaL_checkinteger(L, 1);
    const bool b = ImGui::IsMouseDoubleClicked(value);
    lua_pushboolean(L, b);
    return 1;
}
static int lib_IsMouseHoveringRect(lua_State* L)
{
    ImVec2* v1 = imgui_binding_lua_to_ImVec2(L, 1);
    ImVec2* v2 = imgui_binding_lua_to_ImVec2(L, 2);
    const bool v3 = lua_toboolean(L, 3);
    const bool b = ImGui::IsMouseHoveringRect(*v1, *v2, v3);
    lua_pushboolean(L, b);
    return 1;
}
static int lib_IsMousePosValid(lua_State* L)
{
    ImVec2* vec2 = nullptr;
    if(lua_gettop(L) == 1)
    {
        vec2 = imgui_binding_lua_to_ImVec2(L, 1);
    }
    const bool b = ImGui::IsMousePosValid(vec2);
    lua_pushboolean(L, b);
    return 1;
}
static int lib_IsAnyMouseDown(lua_State* L)
{
    const bool b = ImGui::IsAnyMouseDown();
    lua_pushboolean(L, b);
    return 1;
}
static int lib_GetMousePos(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetMousePos();
    return 1;
}
static int lib_GetMousePosOnOpeningCurrentPopup(lua_State* L)
{
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetMousePosOnOpeningCurrentPopup();
    return 1;
}
static int lib_IsMouseDragging(lua_State* L)
{
    const int v1 = luaL_checkinteger(L, 1);
    const float v2 = (float)luaL_checknumber(L, 2);
    const bool b = ImGui::IsMouseDragging(v1, v2);
    lua_pushboolean(L, b);
    return 1;
}
static int lib_GetMouseDragDelta(lua_State* L)
{
    const int v1 = luaL_checkinteger(L, 1);
    const float v2 = (float)luaL_checknumber(L, 2);
    ImVec2* vec2 = imgui_binding_lua_new_ImVec2(L);
    *vec2 = ImGui::GetMouseDragDelta(v1, v2);
    return 1;
}
static int lib_ResetMouseDragDelta(lua_State* L)
{
    const int value = luaL_checkinteger(L, 1);
    ImGui::ResetMouseDragDelta(value);
    return 0;
}
static int lib_GetMouseCursor(lua_State* L)
{
    const int value = ImGui::GetMouseCursor();
    lua_pushinteger(L, (lua_Integer)value);
    return 1;
}
static int lib_SetMouseCursor(lua_State* L)
{
    const int value = luaL_checkinteger(L, 1);
    ImGui::SetMouseCursor(value);
    return 0;
}
static int lib_CaptureMouseFromApp(lua_State* L)
{
    const bool value = lua_toboolean(L, 1);
    ImGui::CaptureMouseFromApp(value);
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
    if (lua_gettop(L) == 2)
    {
        const size_t size = (size_t)luaL_checkinteger(L, 2);
        ImGui::LoadIniSettingsFromMemory(data, size);
    }
    else
    {
        const size_t size = _imgui_binding_lua_len_size_t(L, 1);
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

// - DebugCheckVersionAndDataLayout

//////// Memory Allocators

// - SetAllocatorFunctions
// - MemAlloc
// - MemFree

void imgui_binding_lua_register_ImGui(lua_State* L)
{
    const luaL_Reg lib_fun[] = {
        // Context creation and access
        
        // Main
        {"GetIO", &lib_GetIO},
        {"GetStyle", &lib_GetStyle},
        {"NewFrame", &lib_NewFrame},
        {"EndFrame", &lib_EndFrame},
        {"Render", &lib_Render},
        {"GetDrawData", &lib_GetDrawData},
        
        // Demo, Debug, Information
        {"ShowDemoWindow", &lib_ShowDemoWindow},
        {"ShowAboutWindow", &lib_ShowAboutWindow},
        {"ShowMetricsWindow", &lib_ShowMetricsWindow},
        {"ShowStyleEditor", &lib_ShowStyleEditor},
        {"ShowStyleSelector", &lib_ShowStyleSelector},
        {"ShowFontSelector", &lib_ShowFontSelector},
        {"ShowUserGuide", &lib_ShowUserGuide},
        {"GetVersion", &lib_GetVersion},
        
        // Styles
        {"StyleColorsDark", &lib_StyleColorsDark},
        {"StyleColorsClassic", &lib_StyleColorsClassic},
        {"StyleColorsLight", &lib_StyleColorsLight},
        
        // Windows
        {"Begin", &lib_Begin},
        {"End", &lib_End},
        
        // Child Windows
        {"BeginChild", &lib_BeginChild},
        {"EndChild", &lib_EndChild},
        
        // Windows Utilities
        {"IsWindowAppearing", &lib_IsWindowAppearing},
        {"IsWindowCollapsed", &lib_IsWindowCollapsed},
        {"IsWindowFocused", &lib_IsWindowFocused},
        {"IsWindowHovered", &lib_IsWindowHovered},
        {"GetWindowDrawList", &lib_GetWindowDrawList},
        {"GetWindowPos", &lib_GetWindowPos},
        {"GetWindowSize", &lib_GetWindowSize},
        {"GetWindowWidth", &lib_GetWindowWidth},
        {"GetWindowHeight", &lib_GetWindowHeight},
        
        {"SetNextWindowPos", &lib_SetNextWindowPos},
        {"SetNextWindowSize", &lib_SetNextWindowSize},
        {"SetNextWindowSizeConstraints", &lib_SetNextWindowSizeConstraints},
        {"SetNextWindowContentSize", &lib_SetNextWindowContentSize},
        {"SetNextWindowCollapsed", &lib_SetNextWindowCollapsed},
        {"SetNextWindowFocus", &lib_SetNextWindowFocus},
        {"SetNextWindowBgAlpha", &lib_SetNextWindowBgAlpha},
        {"SetWindowPos", &lib_SetWindowPos},
        {"SetWindowSize", &lib_SetWindowSize},
        {"SetWindowCollapsed", &lib_SetWindowCollapsed},
        {"SetWindowFocus", &lib_SetWindowFocus},
        {"SetWindowFontScale", &lib_SetWindowFontScale},
        
        // Content region
        {"GetContentRegionMax", &lib_GetContentRegionMax},
        {"GetContentRegionAvail", &lib_GetContentRegionAvail},
        {"GetWindowContentRegionMin", &lib_GetWindowContentRegionMin},
        {"GetWindowContentRegionMax", &lib_GetWindowContentRegionMax},
        {"GetWindowContentRegionWidth", &lib_GetWindowContentRegionWidth},
        
        // Windows Scrolling
        {"GetScrollX", &lib_GetScrollX},
        {"GetScrollY", &lib_GetScrollY},
        {"GetScrollMaxX", &lib_GetScrollMaxX},
        {"GetScrollMaxY", &lib_GetScrollMaxY},
        {"SetScrollX", &lib_SetScrollX},
        {"SetScrollY", &lib_SetScrollY},
        {"SetScrollHereX", &lib_SetScrollHereX},
        {"SetScrollHereY", &lib_SetScrollHereY},
        {"SetScrollFromPosX", &lib_SetScrollFromPosX},
        {"SetScrollFromPosY", &lib_SetScrollFromPosY},
        
        // Parameters stacks (shared)
        {"PushFont", &lib_PushFont},
        {"PopFont", &lib_PopFont},
        {"PushStyleColor", &lib_PushStyleColor},
        {"PopStyleColor", &lib_PopStyleColor},
        {"PushStyleVar", &lib_PushStyleVar},
        {"PopStyleVar", &lib_PopStyleVar},
        {"GetStyleColorVec4", &lib_GetStyleColorVec4},
        {"GetFont", &lib_GetFont},
        {"GetFontSize", &lib_GetFontSize},
        {"GetFontTexUvWhitePixel", &lib_GetFontTexUvWhitePixel},
        {"GetColorU32", &lib_GetColorU32},
        
        // Parameters stacks (current window)
        {"PushItemWidth", &lib_PushItemWidth},
        {"PopItemWidth", &lib_PopItemWidth},
        {"SetNextItemWidth", &lib_SetNextItemWidth},
        {"CalcItemWidth", &lib_CalcItemWidth},
        {"PushTextWrapPos", &lib_PushTextWrapPos},
        {"PopTextWrapPos", &lib_PopTextWrapPos},
        {"PushAllowKeyboardFocus", &lib_PushAllowKeyboardFocus},
        {"PopAllowKeyboardFocus", &lib_PopAllowKeyboardFocus},
        {"PushButtonRepeat", &lib_PushButtonRepeat},
        {"PopButtonRepeat", &lib_PopButtonRepeat},
        
        // Cursor / Layout
        {"Separator", &lib_Separator},
        {"SameLine", &lib_SameLine},
        {"NewLine", &lib_NewLine},
        {"Spacing", &lib_Spacing},
        {"Dummy", &lib_Dummy},
        {"Indent", &lib_Indent},
        {"Unindent", &lib_Unindent},
        {"BeginGroup", &lib_BeginGroup},
        {"EndGroup", &lib_EndGroup},
        {"GetCursorPos", &lib_GetCursorPos},
        {"GetCursorPosX", &lib_GetCursorPosX},
        {"GetCursorPosY", &lib_GetCursorPosY},
        {"SetCursorPos", &lib_SetCursorPos},
        {"SetCursorPosX", &lib_SetCursorPosX},
        {"SetCursorPosY", &lib_SetCursorPosY},
        {"GetCursorStartPos", &lib_GetCursorStartPos},
        {"GetCursorScreenPos", &lib_GetCursorScreenPos},
        {"SetCursorScreenPos", &lib_SetCursorScreenPos},
        {"AlignTextToFramePadding", &lib_AlignTextToFramePadding},
        {"GetTextLineHeight", &lib_GetTextLineHeight},
        {"GetTextLineHeightWithSpacing", &lib_GetTextLineHeightWithSpacing},
        {"GetFrameHeight", &lib_GetFrameHeight},
        {"GetFrameHeightWithSpacing", &lib_GetFrameHeightWithSpacing},
        
        // ID stack/scopes
        {"PushID", &lib_PushID},
        {"PopID", &lib_PopID},
        {"GetID", &lib_GetID},
        
        // Widgets: Text
        {"TextUnformatted", &lib_TextUnformatted},
        {"Text", &lib_Text},
        {"TextColored", &lib_TextColored},
        {"TextDisabled", &lib_TextDisabled},
        {"TextWrapped", &lib_TextWrapped},
        {"LabelText", &lib_LabelText},
        {"BulletText", &lib_BulletText},
        
        // Widgets: Main
        {"Button", &lib_Button},
        {"SmallButton", &lib_SmallButton},
        {"InvisibleButton", &lib_InvisibleButton},
        {"ArrowButton", &lib_ArrowButton},
        {"Image", &lib_Image},
        {"ImageButton", &lib_ImageButton},
        {"Checkbox", &lib_Checkbox},
        {"CheckboxFlags", &lib_CheckboxFlags},
        {"RadioButton", &lib_RadioButton},
        {"ProgressBar", &lib_ProgressBar},
        {"Bullet", &lib_Bullet},
        
        // Widgets: Combo Box
        {"BeginCombo", &lib_BeginCombo},
        {"EndCombo", &lib_EndCombo},
        {"Combo", &lib_Combo},
        
        // Widgets: Drag Sliders
        {"DragFloat", &lib_DragFloat},
        {"DragFloat2", &lib_DragFloat2},
        {"DragFloat3", &lib_DragFloat3},
        {"DragFloat4", &lib_DragFloat4},
        {"DragFloatRange2", &lib_DragFloatRange2},
        {"DragInt", &lib_DragInt},
        {"DragInt2", &lib_DragInt2},
        {"DragInt3", &lib_DragInt3},
        {"DragInt4", &lib_DragInt4},
        {"DragIntRange2", &lib_DragIntRange2},
        {"DragScalar", &lib_DragScalar},
        {"DragScalarN", &lib_DragScalarN},
        
        // Widgets: Regular Sliders
        {"SliderFloat", &lib_SliderFloat},
        {"SliderFloat2", &lib_SliderFloat2},
        {"SliderFloat3", &lib_SliderFloat3},
        {"SliderFloat4", &lib_SliderFloat4},
        {"SliderAngle", &lib_SliderAngle},
        {"SliderInt", &lib_SliderInt},
        {"SliderInt2", &lib_SliderInt2},
        {"SliderInt3", &lib_SliderInt3},
        {"SliderInt4", &lib_SliderInt4},
        {"SliderScalar", &lib_SliderScalar},
        {"SliderScalarN", &lib_SliderScalarN},
        {"VSliderFloat", &lib_VSliderFloat},
        {"VSliderInt", &lib_VSliderInt},
        {"VSliderScalar", &lib_VSliderScalar},
        
        // Widgets: Input with Keyboard
        {"InputText", &lib_InputText},
        {"InputTextMultiline", &lib_InputTextMultiline},
        {"InputTextWithHint", &lib_InputTextWithHint},
        {"InputFloat", &lib_InputFloat},
        {"InputFloat2", &lib_InputFloat2},
        {"InputFloat3", &lib_InputFloat3},
        {"InputFloat4", &lib_InputFloat4},
        {"InputInt", &lib_InputInt},
        {"InputInt2", &lib_InputInt2},
        {"InputInt3", &lib_InputInt3},
        {"InputInt4", &lib_InputInt4},
        {"InputDouble", &lib_InputDouble},
        {"InputScalar", &lib_InputScalar},
        {"InputScalarN", &lib_InputScalarN},
        
        // Widgets: Color Editor/Picker
        {"ColorEdit3", &lib_ColorEdit3},
        {"ColorEdit4", &lib_ColorEdit4},
        {"ColorPicker3", &lib_ColorPicker3},
        {"ColorPicker4", &lib_ColorPicker4},
        {"ColorButton", &lib_ColorButton},
        {"SetColorEditOptions", &lib_SetColorEditOptions},
        
        // Widgets: Trees
        {"TreeNode", &lib_TreeNode},
        {"TreeNodeEx", &lib_TreeNodeEx},
        {"TreePush", &lib_TreePush},
        {"TreePop", &lib_TreePop},
        {"GetTreeNodeToLabelSpacing", &lib_GetTreeNodeToLabelSpacing},
        {"CollapsingHeader", &lib_CollapsingHeader},
        {"SetNextItemOpen", &lib_SetNextItemOpen},
        
        // Widgets: Selectables
        {"Selectable", &lib_Selectable},
        
        // Widgets: List Boxes
        {"ListBox", &lib_ListBox},
        {"ListBoxHeader", &lib_ListBoxHeader},
        {"ListBoxFooter", &lib_ListBoxFooter},
        
        // Widgets: Data Plotting
        {"PlotLines", &lib_PlotLines},
        {"PlotHistogram", &lib_PlotHistogram},
        
        // Widgets: Value() Helpers.
        
        // Widgets: Menus
        {"BeginMenuBar", &lib_BeginMenuBar},
        {"EndMenuBar", &lib_EndMenuBar},
        {"BeginMainMenuBar", &lib_BeginMainMenuBar},
        {"EndMainMenuBar", &lib_EndMainMenuBar},
        {"BeginMenu", &lib_BeginMenu},
        {"EndMenu", &lib_EndMenu},
        {"MenuItem", &lib_MenuItem},
        
        // Tooltips
        {"BeginTooltip", &lib_BeginTooltip},
        {"EndTooltip", &lib_EndTooltip},
        {"SetTooltip", &lib_SetTooltip},
        
        // Popups, Modals
        {"BeginPopup", &lib_BeginPopup},
        {"BeginPopupModal", &lib_BeginPopupModal},
        {"EndPopup", &lib_EndPopup},
        {"OpenPopup", &lib_OpenPopup},
        {"OpenPopupOnItemClick", &lib_OpenPopupOnItemClick},
        {"CloseCurrentPopup", &lib_CloseCurrentPopup},
        {"BeginPopupContextItem", &lib_BeginPopupContextItem},
        {"BeginPopupContextWindow", &lib_BeginPopupContextWindow},
        {"BeginPopupContextVoid", &lib_BeginPopupContextVoid},
        {"IsPopupOpen", &lib_IsPopupOpen},
        
        // Columns
        {"Columns", &lib_Columns},
        {"NextColumn", &lib_NextColumn},
        {"GetColumnIndex", &lib_GetColumnIndex},
        {"GetColumnWidth", &lib_GetColumnWidth},
        {"SetColumnWidth", &lib_SetColumnWidth},
        {"GetColumnOffset", &lib_GetColumnOffset},
        {"SetColumnOffset", &lib_SetColumnOffset},
        {"GetColumnsCount", &lib_GetColumnsCount},
        
        // Tab Bars, Tabs
        {"BeginTabBar", &lib_BeginTabBar},
        {"EndTabBar", &lib_EndTabBar},
        {"BeginTabItem", &lib_BeginTabItem},
        {"EndTabItem", &lib_EndTabItem},
        {"TabItemButton", &lib_TabItemButton},
        {"SetTabItemClosed", &lib_SetTabItemClosed},
        
        // Logging/Capture
        {"LogToTTY", &lib_LogToTTY},
        {"LogToFile", &lib_LogToFile},
        {"LogToClipboard", &lib_LogToClipboard},
        {"LogFinish", &lib_LogFinish},
        {"LogButtons", &lib_LogButtons},
        {"LogText", &lib_LogText},
        
        // Drag and Drop
        
        // Clipping
        {"PushClipRect", &lib_PushClipRect},
        {"PopClipRect", &lib_PopClipRect},
        
        // Focus, Activation
        {"SetItemDefaultFocus", &lib_SetItemDefaultFocus},
        {"SetKeyboardFocusHere", &lib_SetKeyboardFocusHere},
        
        // Item/Widgets Utilities
        {"IsItemHovered", &lib_IsItemHovered},
        {"IsItemActive", &lib_IsItemActive},
        {"IsItemFocused", &lib_IsItemFocused},
        {"IsItemClicked", &lib_IsItemClicked},
        {"IsItemVisible", &lib_IsItemVisible},
        {"IsItemEdited", &lib_IsItemEdited},
        {"IsItemActivated", &lib_IsItemActivated},
        {"IsItemDeactivated", &lib_IsItemDeactivated},
        {"IsItemDeactivatedAfterEdit", &lib_IsItemDeactivatedAfterEdit},
        {"IsItemToggledOpen", &lib_IsItemToggledOpen},
        {"IsAnyItemHovered", &lib_IsAnyItemHovered},
        {"IsAnyItemActive", &lib_IsAnyItemActive},
        {"IsAnyItemFocused", &lib_IsAnyItemFocused},
        {"GetItemRectMin", &lib_GetItemRectMin},
        {"GetItemRectMax", &lib_GetItemRectMax},
        {"GetItemRectSize", &lib_GetItemRectSize},
        {"SetItemAllowOverlap", &lib_SetItemAllowOverlap},
        
        // Miscellaneous Utilities
        {"IsRectVisible", &lib_IsRectVisible},
        {"GetTime", &lib_GetTime},
        {"GetFrameCount", &lib_GetFrameCount},
        {"GetBackgroundDrawList", &lib_GetBackgroundDrawList},
        {"GetForegroundDrawList", &lib_GetForegroundDrawList},
        {"GetDrawListSharedData", &lib_GetDrawListSharedData},
        {"GetStyleColorName", &lib_GetStyleColorName},
        {"SetStateStorage", &lib_SetStateStorage},
        {"GetStateStorage", &lib_GetStateStorage},
        {"CalcListClipping", &lib_CalcListClipping},
        {"BeginChildFrame", &lib_BeginChildFrame},
        {"EndChildFrame", &lib_EndChildFrame},
        
        // Text Utilities
        {"CalcTextSize", &lib_CalcTextSize},
        
        // Inputs Utilities: Keyboard
        {"GetKeyIndex", &lib_GetKeyIndex},
        {"IsKeyDown", &lib_IsKeyDown},
        {"IsKeyPressed", &lib_IsKeyPressed},
        {"IsKeyReleased", &lib_IsKeyReleased},
        {"GetKeyPressedAmount", &lib_GetKeyPressedAmount},
        {"CaptureKeyboardFromApp", &lib_CaptureKeyboardFromApp},
        
        // Inputs Utilities: Mouse
        {"IsMouseDown", &lib_IsMouseDown},
        {"IsMouseClicked", &lib_IsMouseClicked},
        {"IsMouseReleased", &lib_IsMouseReleased},
        {"IsMouseDoubleClicked", &lib_IsMouseDoubleClicked},
        {"IsMouseHoveringRect", &lib_IsMouseHoveringRect},
        {"IsMousePosValid", &lib_IsMousePosValid},
        {"IsAnyMouseDown", &lib_IsAnyMouseDown},
        {"GetMousePos", &lib_GetMousePos},
        {"GetMousePosOnOpeningCurrentPopup", &lib_GetMousePosOnOpeningCurrentPopup},
        {"IsMouseDragging", &lib_IsMouseDragging},
        {"GetMouseDragDelta", &lib_GetMouseDragDelta},
        {"ResetMouseDragDelta", &lib_ResetMouseDragDelta},
        {"GetMouseCursor", &lib_GetMouseCursor},
        {"SetMouseCursor", &lib_SetMouseCursor},
        {"CaptureMouseFromApp", &lib_CaptureMouseFromApp},
        
        // Clipboard Utilities
        {"GetClipboardText", &lib_GetClipboardText},
        {"SetClipboardText", &lib_SetClipboardText},
        
        // Settings/.Ini Utilities
        {"LoadIniSettingsFromDisk", &lib_LoadIniSettingsFromDisk},
        {"LoadIniSettingsFromMemory", &lib_LoadIniSettingsFromMemory},
        {"SaveIniSettingsToDisk", &lib_SaveIniSettingsToDisk},
        {"SaveIniSettingsToMemory", &lib_SaveIniSettingsToMemory},
        
        // Debug Utilities
        
        // Memory Allocators
        
        // NULL END
        {NULL, NULL},
    };
    const auto lib_func = (sizeof(lib_fun) / sizeof(luaL_Reg)) - 1;
    
    //                                      // ? m
    lua_pushstring(L, "ImGui");             // ? m k
    lua_createtable(L, 0, lib_func);  // ? m k t
    luaL_setfuncs(L, lib_fun, 0);    // ? m k t
    lua_settable(L, -3);                    // ? m
}
