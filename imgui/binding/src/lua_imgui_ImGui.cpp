#include "lua_imgui_common.hpp"
#include "lua_imgui_ImGui.hpp"
#include "lua_imgui_type.hpp"
#include <cstdint>
#include <tuple>
#include "imgui.h"

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

//////// Context creation and access

static /* XXXX */ int lib_CreateContext(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* XXXX */ int lib_DestroyContext(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* XXXX */ int lib_GetCurrentContext(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* XXXX */ int lib_SetCurrentContext(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}

//////// Main

static /* !!!! */ int lib_GetIO(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* !!!! */ int lib_GetPlatformIO(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static int lib_GetStyle(lua_State* L)
{
    imgui_binding_lua_ref_ImGuiStyle(L, &ImGui::GetStyle());
    return 1;
}
static int lib_NewFrame(lua_State* L)
{
    std::ignore = L;
    ImGui::NewFrame();
    return 0;
}
static int lib_EndFrame(lua_State* L)
{
    std::ignore = L;
    ImGui::EndFrame();
    return 0;
}
static int lib_Render(lua_State* L)
{
    std::ignore = L;
    ImGui::Render();
    return 0;
}
static /* !!!! */ int lib_GetDrawData(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}

//////// Demo, Debug, Information

static int lib_ShowDemoWindow(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        bool p_open = lua_toboolean(L, 1);
        ImGui::ShowDemoWindow(&p_open);
        lua_pushboolean(L, p_open);
        return 1;
    }
    else
    {
        ImGui::ShowDemoWindow();
        return 0;
    }
}
static int lib_ShowMetricsWindow(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        bool p_open = lua_toboolean(L, 1);
        ImGui::ShowMetricsWindow(&p_open);
        lua_pushboolean(L, p_open);
        return 1;
    }
    else
    {
        ImGui::ShowMetricsWindow();
        return 0;
    }
}
static int lib_ShowDebugLogWindow(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        bool p_open = lua_toboolean(L, 1);
        ImGui::ShowDebugLogWindow(&p_open);
        lua_pushboolean(L, p_open);
        return 1;
    }
    else
    {
        ImGui::ShowDebugLogWindow();
        return 0;
    }
}
static int lib_ShowIDStackToolWindow(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        bool p_open = lua_toboolean(L, 1);
        ImGui::ShowIDStackToolWindow(&p_open);
        lua_pushboolean(L, p_open);
        return 1;
    }
    else
    {
        ImGui::ShowIDStackToolWindow();
        return 0;
    }
}
static int lib_ShowAboutWindow(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        bool p_open = lua_toboolean(L, 1);
        ImGui::ShowAboutWindow(&p_open);
        lua_pushboolean(L, p_open);
        return 1;
    }
    else
    {
        ImGui::ShowAboutWindow();
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
    const bool ret = ImGui::ShowStyleSelector(label);
    lua_pushboolean(L, ret);
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
    std::ignore = L;
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
    const int argc = lua_gettop(L);
    const char* name = luaL_checkstring(L, 1);
    if (argc <= 1)
    {
        const bool ret = ImGui::Begin(name);
        lua_pushboolean(L, ret);
        return 1;
    }
    else if (argc == 2)
    {
        bool p_open = lua_toboolean(L, 2);
        const bool ret = ImGui::Begin(name, &p_open);
        lua_pushboolean(L, ret);
        lua_pushboolean(L, p_open);
        return 2;
    }
    else
    {
        bool p_open = lua_toboolean(L, 2);
        const ImGuiWindowFlags flags = (ImGuiWindowFlags)luaL_checkinteger(L, 3);
        const bool ret = ImGui::Begin(name, &p_open, flags);
        lua_pushboolean(L, ret);
        lua_pushboolean(L, p_open);
        return 2;
    }
}
static int lib_End(lua_State* L)
{
    std::ignore = L;
    ImGui::End();
    return 0;
}

//////// Child Windows

static int lib_BeginChild(lua_State* L)
{
    const int argc = lua_gettop(L);
    switch(lua_type(L, 1))
    {
    case LUA_TNUMBER:
        {
            const ImGuiID id = (ImGuiID)luaL_checkinteger(L, 1);
            bool ret = false;
            if (argc <= 1)
            {
                ret = ImGui::BeginChild(id);
            }
            else if (argc == 2)
            {
                ImVec2* size = lua::as_type_instance<ImVec2>(L, 2);
                ret = ImGui::BeginChild(id, *size);
            }
            else if (argc == 3)
            {
                ImVec2* size = lua::as_type_instance<ImVec2>(L, 2);
                const auto flags = (ImGuiChildFlags)luaL_checkinteger(L, 3);
                ret = ImGui::BeginChild(id, *size, flags);
            }
            else
            {
                ImVec2* size = lua::as_type_instance<ImVec2>(L, 2);
                const auto flags = (ImGuiChildFlags)luaL_checkinteger(L, 3);
                const auto window_flags = (ImGuiWindowFlags)luaL_checkinteger(L, 4);
                ret = ImGui::BeginChild(id, *size, flags, window_flags);
            }
            lua_pushboolean(L, ret);
            return 1;
        }
    case LUA_TSTRING:
    default:
        {
            const char* str_id = luaL_checkstring(L, 1);
            bool ret = false;
            if (argc <= 1)
            {
                ret = ImGui::BeginChild(str_id);
            }
            else if (argc == 2)
            {
                ImVec2* size = lua::as_type_instance<ImVec2>(L, 2);
                ret = ImGui::BeginChild(str_id, *size);
            }
            else if (argc == 3)
            {
                ImVec2* size = lua::as_type_instance<ImVec2>(L, 2);
                const auto flags = (ImGuiChildFlags)luaL_checkinteger(L, 3);
                ret = ImGui::BeginChild(str_id, *size, flags);
            }
            else
            {
                ImVec2* size = lua::as_type_instance<ImVec2>(L, 2);
                const auto flags = (ImGuiChildFlags)luaL_checkinteger(L, 3);
                const auto window_flags = (ImGuiWindowFlags)luaL_checkinteger(L, 4);
                ret = ImGui::BeginChild(str_id, *size, flags, window_flags);
            }
            lua_pushboolean(L, ret);
            return 1;
        }
    };
}
static int lib_EndChild(lua_State* L)
{
    std::ignore = L;
    ImGui::EndChild();
    return 0;
}

//////// Windows Utilities

static int lib_IsWindowAppearing(lua_State* L)
{
    const bool ret = ImGui::IsWindowAppearing();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsWindowCollapsed(lua_State* L)
{
    const bool ret = ImGui::IsWindowCollapsed();
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsWindowFocused(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        const ImGuiFocusedFlags flags = (ImGuiFocusedFlags)luaL_checkinteger(L, 1);
        const bool ret = ImGui::IsWindowFocused(flags);
        lua_pushboolean(L, ret);
    }
    else
    {
        const bool ret = ImGui::IsWindowFocused();
        lua_pushboolean(L, ret);
    }
    return 1;
}
static int lib_IsWindowHovered(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        const ImGuiFocusedFlags flags = (ImGuiFocusedFlags)luaL_checkinteger(L, 1);
        const bool ret = ImGui::IsWindowHovered(flags);
        lua_pushboolean(L, ret);
    }
    else
    {
        const bool ret = ImGui::IsWindowHovered();
        lua_pushboolean(L, ret);
    }
    return 1;
}
static /* !!!! */ int lib_GetWindowDrawList(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static int lib_GetWindowPos(lua_State* L)
{
    ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
    *vec2 = ImGui::GetWindowPos();
    return 1;
}
static int lib_GetWindowSize(lua_State* L)
{
    ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
    *vec2 = ImGui::GetWindowSize();
    return 1;
}
static int lib_GetWindowWidth(lua_State* L)
{
    const float ret = ImGui::GetWindowWidth();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_GetWindowHeight(lua_State* L)
{
    const float ret = ImGui::GetWindowHeight();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int lib_SetNextWindowPos(lua_State* L)
{
    const int argc = lua_gettop(L);
    ImVec2* pos = lua::as_type_instance<ImVec2>(L, 1);
    if (argc <= 1)
    {
        ImGui::SetNextWindowPos(*pos);
    }
    else if (argc == 2)
    {
        const ImGuiCond cond = (ImGuiCond)luaL_checkinteger(L, 2);
        ImGui::SetNextWindowPos(*pos, cond);
    }
    else
    {
        const ImGuiCond cond = (ImGuiCond)luaL_checkinteger(L, 2);
        ImVec2* pivot = lua::as_type_instance<ImVec2>(L, 3);
        ImGui::SetNextWindowPos(*pos, cond, *pivot);
    }
    return 0;
}
static int lib_SetNextWindowSize(lua_State* L)
{
    ImVec2* size = lua::as_type_instance<ImVec2>(L, 1);
    if (lua_gettop(L) <= 1)
    {
        ImGui::SetNextWindowSize(*size);
    }
    else
    {
        const ImGuiCond cond = (ImGuiCond)luaL_checkinteger(L, 2);
        ImGui::SetNextWindowSize(*size, cond);
    }
    return 0;
}
static /* !!!! */ int lib_SetNextWindowSizeConstraints(lua_State* L)
{
    ImVec2* v1 = lua::as_type_instance<ImVec2>(L, 1);
    ImVec2* v2 = lua::as_type_instance<ImVec2>(L, 2);
    if (lua_gettop(L) <= 2)
    {
        ImGui::SetNextWindowSizeConstraints(*v1, *v2);
    }
    else
    {
        static lua_Integer uuid = 0;
        static lua_Integer hashtk = 0;
        struct Wrapper
        {
            static void Callback(ImGuiSizeCallbackData *data)
            {
                if (!GLUA) return;
                //                                                              // ???
                IMGUI_GET_REGISTRY(GLUA);                                       // ??? reg
                lua_pushlightuserdata(GLUA, &uuid);                             // ??? reg k
                lua_gettable(GLUA, -2);                                         // ??? reg funt
                lua_rawgeti(GLUA, -1, (lua_Integer)(ptrdiff_t)data->UserData);  // ??? reg funt fun
                lua_pushnil(GLUA); // TODO: push &ImGuiSizeCallbackData         // ??? reg funt fun data
                int callr = lua_pcall(GLUA, 1, 0, 0);                           // ??? reg funt
                if (callr != 0) //                                              // ??? reg funt emsg
                {
                    const char* errmsg = lua_tostring(GLUA, -1);
                    printf("%s\n", errmsg);
                    lua_pop(GLUA, 1);                                           // ??? reg funt
                }
                lua_pop(GLUA, 2);                                               // ???
            };
        };
        //                                              // vec2 vec2 fun ...
        IMGUI_GET_REGISTRY(L);                          // vec2 vec2 fun ... reg
        // find callback function
        lua_pushlightuserdata(L, &hashtk);              // vec2 vec2 fun ... reg k
        lua_gettable(L, -2);                            // vec2 vec2 fun ... reg fmap
        lua_pushvalue(L, 3);                            // vec2 vec2 fun ... reg fmap fun
        lua_gettable(L, -2);                            // vec2 vec2 fun ... reg fmap bool
        if (!lua_toboolean(L, -1))
        {
            lua_pop(L, 1);                              // vec2 vec2 fun ... reg fmap
            // mark callback function
            lua_pushvalue(L, 3);                        // vec2 vec2 fun ... reg fmap fun
            lua_pushboolean(L, true);                   // vec2 vec2 fun ... reg fmap fun true
            lua_settable(L, -3);                        // vec2 vec2 fun ... reg fmap
            lua_pop(L, 1);                              // vec2 vec2 fun ... reg
            // storage callback function
            lua_pushlightuserdata(L, &uuid);            // vec2 vec2 fun ... reg k
            lua_gettable(L, -2);                        // vec2 vec2 fun ... reg funt
            lua_pushvalue(L, 3);                        // vec2 vec2 fun ... reg funt fun
            lua_rawseti(L, -2, uuid);                   // vec2 vec2 fun ... reg funt
            lua_pop(L, 1);                              // vec2 vec2 fun ... reg
        }
        lua_pop(L, 1);                                  // vec2 vec2 fun ...
        ImGui::SetNextWindowSizeConstraints(*v1, *v2, &Wrapper::Callback, (void*)(ptrdiff_t)uuid);
        uuid = (uuid + 1) % 0x40000000;
    }
    return 0;
}
static int lib_SetNextWindowContentSize(lua_State* L)
{
    ImVec2* size = lua::as_type_instance<ImVec2>(L, 1);
    ImGui::SetNextWindowContentSize(*size);
    return 0;
}
static int lib_SetNextWindowCollapsed(lua_State* L)
{
    const bool collapsed = lua_toboolean(L, 1);
    if (lua_gettop(L) <= 1)
    {
        ImGui::SetNextWindowCollapsed(collapsed);
    }
    else
    {
        const int cond = luaL_checkinteger(L, 2);
        ImGui::SetNextWindowCollapsed(collapsed, cond);
    }
    return 0;
}
static int lib_SetNextWindowFocus(lua_State* L)
{
    std::ignore = L;
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
    const int argc = lua_gettop(L);
    switch(lua_type(L, 1))
    {
    default:
        {
            ImVec2* pos = lua::as_type_instance<ImVec2>(L, 1);
            if (argc <= 1)
            {
                ImGui::SetWindowPos(*pos);
            }
            else
            {
                const int cond = luaL_checkinteger(L, 2);
                ImGui::SetWindowPos(*pos, cond);
            }
            return 0;
        }
    case LUA_TSTRING:
        {
            const char* name = luaL_checkstring(L, 1);
            ImVec2* pos = lua::as_type_instance<ImVec2>(L, 2);
            if (argc <= 2)
            {
                ImGui::SetWindowPos(name, *pos);
            }
            else
            {
                const int cond = luaL_checkinteger(L, 2);
                ImGui::SetWindowPos(name, *pos, cond);
            }
            return 0;
        }
    }
}
static int lib_SetWindowSize(lua_State* L)
{
    const int argc = lua_gettop(L);
    switch (lua_type(L, 1))
    {
    default:
        {
            ImVec2* size = lua::as_type_instance<ImVec2>(L, 1);
            if (argc <= 1)
            {
                ImGui::SetWindowSize(*size);
            }
            else
            {
                const int cond = luaL_checkinteger(L, 2);
                ImGui::SetWindowSize(*size, cond);
            }
            return 0;
        }
    case LUA_TSTRING:
        {
            const char* name = luaL_checkstring(L, 1);
            ImVec2* size = lua::as_type_instance<ImVec2>(L, 2);
            if (argc <= 2)
            {
                ImGui::SetWindowSize(name, *size);
            }
            else
            {
                const int cond = luaL_checkinteger(L, 2);
                ImGui::SetWindowSize(name, *size, cond);
            }
            return 0;
        }
    }
}
static int lib_SetWindowCollapsed(lua_State* L)
{
    const int argc = lua_gettop(L);
    switch (lua_type(L, 1))
    {
    default:
        {
            const bool collapsed = lua_toboolean(L, 1);
            if (argc <= 1)
            {
                ImGui::SetWindowCollapsed(collapsed);
            }
            else
            {
                const int cond = luaL_checkinteger(L, 2);
                ImGui::SetWindowCollapsed(collapsed, cond);
            }
            return 0;
        }
    case LUA_TSTRING:
        {
            const char* name = luaL_checkstring(L, 1);
            const bool collapsed = lua_toboolean(L, 2);
            if (argc <= 2)
            {
                ImGui::SetWindowCollapsed(name, collapsed);
            }
            else
            {
                const int cond = luaL_checkinteger(L, 2);
                ImGui::SetWindowCollapsed(name, collapsed, cond);
            }
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
    const float scale = (float)luaL_checknumber(L, 1);
    ImGui::SetWindowFontScale(scale);
    return 0;
}

//////// Windows Scrolling

static int lib_GetScrollX(lua_State* L)
{
    const float ret = ImGui::GetScrollX();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_GetScrollY(lua_State* L)
{
    const float ret = ImGui::GetScrollY();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_SetScrollX(lua_State* L)
{
    const float scroll_x = (float)luaL_checknumber(L, 1);
    ImGui::SetScrollX(scroll_x);
    return 0;
}
static int lib_SetScrollY(lua_State* L)
{
    const float scroll_y = (float)luaL_checknumber(L, 1);
    ImGui::SetScrollY(scroll_y);
    return 0;
}
static int lib_GetScrollMaxX(lua_State* L)
{
    const float ret = ImGui::GetScrollMaxX();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_GetScrollMaxY(lua_State* L)
{
    const float ret = ImGui::GetScrollMaxY();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_SetScrollHereX(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        const float center_x_ratio = (float)luaL_checknumber(L, 1);
        ImGui::SetScrollHereX(center_x_ratio);
    }
    else
    {
        ImGui::SetScrollHereX();
    }
    return 0;
}
static int lib_SetScrollHereY(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        const float center_y_ratio = (float)luaL_checknumber(L, 1);
        ImGui::SetScrollHereY(center_y_ratio);
    }
    else
    {
        ImGui::SetScrollHereY();
    }
    return 0;
}
static int lib_SetScrollFromPosX(lua_State* L)
{
    const float local_x = (float)luaL_checknumber(L, 1);
    if (lua_gettop(L) <= 1)
    {
        ImGui::SetScrollFromPosX(local_x);
    }
    else
    {
        const float center_x_ratio = (float)luaL_checknumber(L, 2);
        ImGui::SetScrollFromPosX(local_x, center_x_ratio);
    }
    return 0;
}
static int lib_SetScrollFromPosY(lua_State* L)
{
    const float local_y = (float)luaL_checknumber(L, 1);
    if (lua_gettop(L) <= 1)
    {
        ImGui::SetScrollFromPosY(local_y);
    }
    else
    {
        const float center_y_ratio = (float)luaL_checknumber(L, 2);
        ImGui::SetScrollFromPosY(local_y, center_y_ratio);
    }
    return 0;
}

//////// Parameters stacks (shared)

static /* !!!! */ int lib_PushFont(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static int lib_PopFont(lua_State* L)
{
    std::ignore = L;
    ImGui::PopFont();
    return 0;
}
static int lib_PushStyleColor(lua_State* L)
{
    const ImGuiCol idx = (ImGuiCol)luaL_checkinteger(L, 1);
    switch(lua_type(L, 2))
    {
    case LUA_TUSERDATA:
        {
            ImVec4* col = lua::as_type_instance<ImVec4>(L, 2);
            ImGui::PushStyleColor(idx, *col);
        }
        return 0;
    default:
    case LUA_TNUMBER:
        {
            const ImU32 col = (ImU32)luaL_checknumber(L, 2);
            ImGui::PushStyleColor(idx, col);
        }
        return 0;
    }
}
static int lib_PopStyleColor(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        const int count = (int)luaL_checkinteger(L, 1);
        ImGui::PopStyleColor(count);
    }
    else
    {
        ImGui::PopStyleColor();
    }
    return 0;
}
static int lib_PushStyleVar(lua_State* L)
{
    const ImGuiStyleVar idx = (ImGuiStyleVar)luaL_checkinteger(L, 1);
    switch(lua_type(L, 2))
    {
    case LUA_TUSERDATA:
        {
            ImVec2* val = lua::as_type_instance<ImVec2>(L, 2);
            ImGui::PushStyleVar(idx, *val);
        }
        return 0;
    default:
    case LUA_TNUMBER:
        {
            const float val = (float)luaL_checknumber(L, 2);
            ImGui::PushStyleVar(idx, val);
        }
        return 0;
    }
}
static int lib_PopStyleVar(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        const int count = (int)luaL_checkinteger(L, 1);
        ImGui::PopStyleVar(count);
    }
    else
    {
        ImGui::PopStyleVar();
    }
    return 0;
}
static int lib_PushStyleVarX(lua_State* L) {
    auto const idx = static_cast<ImGuiStyleVar>(luaL_checkinteger(L, 1));
    auto const val_x = static_cast<float>(luaL_checknumber(L, 2));
    ImGui::PushStyleVarX(idx, val_x);
    return 0;
}
static int lib_PushStyleVarY(lua_State* L) {
    auto const idx = static_cast<ImGuiStyleVar>(luaL_checkinteger(L, 1));
    auto const val_y = static_cast<float>(luaL_checknumber(L, 2));
    ImGui::PushStyleVarY(idx, val_y);
    return 0;
}
static int lib_PushItemFlag(lua_State* L)
{
    auto const flags = (ImGuiItemFlags)luaL_checkinteger(L, 1);
    auto const enable = lua_toboolean(L, 2);
    ImGui::PushItemFlag(flags, enable);
    return 0;
}
static int lib_PopItemFlag(lua_State*)
{
    ImGui::PopItemFlag();
    return 0;
}

//////// Parameters stacks (current window)

static int lib_PushItemWidth(lua_State* L)
{
    const float item_width = (float)luaL_checknumber(L, 1);
    ImGui::PushItemWidth(item_width);
    return 0;
}
static int lib_PopItemWidth(lua_State* L)
{
    std::ignore = L;
    ImGui::PopItemWidth();
    return 0;
}
static int lib_SetNextItemWidth(lua_State* L)
{
    const float item_width = (float)luaL_checknumber(L, 1);
    ImGui::SetNextItemWidth(item_width);
    return 0;
}
static int lib_CalcItemWidth(lua_State* L)
{
    const float ret = ImGui::CalcItemWidth();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_PushTextWrapPos(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        const float wrap_local_pos_x = (float)luaL_checknumber(L, 1);
        ImGui::PushTextWrapPos(wrap_local_pos_x);
    }
    else
    {
        ImGui::PushTextWrapPos();
    }
    return 0;
}
static int lib_PopTextWrapPos(lua_State* L)
{
    std::ignore = L;
    ImGui::PopTextWrapPos();
    return 0;
}

//////// Style read access

static /* !!!! */ int lib_GetFont(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static int lib_GetFontSize(lua_State* L)
{
    const float ret = ImGui::GetFontSize();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_GetFontTexUvWhitePixel(lua_State* L)
{
    ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
    *vec2 = ImGui::GetFontTexUvWhitePixel();
    return 1;
}
static /* !!!! */ int lib_GetColorU32(lua_State* L)
{
    switch (lua_type(L, 1))
    {
    case LUA_TUSERDATA:
        {
            ImVec4* col = lua::as_type_instance<ImVec4>(L, 1);
            const ImU32 ret = ImGui::GetColorU32(*col);
            lua_pushnumber(L, (lua_Number)ret);
            return 1;
        }
    default:
    case LUA_TNUMBER:
        {
            // WARNING:
            // - [x] ImU32 GetColorU32(ImGuiCol idx, float alpha_mul = 1.0f)
            // - [x] ImU32 GetColorU32(const ImVec4& col)
            // - [ ] ImU32 GetColorU32(ImU32 col, float alpha_mul = 1.0f) <<<<<<<<<<<< stupid!
            const ImGuiCol idx = (ImGuiCol)luaL_checkinteger(L, 1);
            if (lua_gettop(L) <= 1)
            {
                const ImU32 ret = ImGui::GetColorU32(idx);
                lua_pushnumber(L, (lua_Number)ret);
            }
            else
            {
                const float alpha_mul = (float)luaL_checknumber(L, 2);
                const ImU32 ret = ImGui::GetColorU32(idx, alpha_mul);
                lua_pushnumber(L, (lua_Number)ret);
            }
            return 1;
        }
    }
}
static int lib_GetStyleColorVec4(lua_State* L)
{
    const ImGuiCol idx = (ImGuiCol)luaL_checkinteger(L, 1);
    ImVec4* vec4 = lua::create_type_instance<ImVec4>(L);
    *vec4 = ImGui::GetStyleColorVec4(idx);
    return 1;
}

//////// Layout cursor positioning

static int lib_GetCursorScreenPos(lua_State* L)
{
    ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
    *vec2 = ImGui::GetCursorScreenPos();
    return 1;
}
static int lib_SetCursorScreenPos(lua_State* L)
{
    ImVec2* pos = lua::as_type_instance<ImVec2>(L, 1);
    ImGui::SetCursorScreenPos(*pos);
    return 0;
}
static int lib_GetContentRegionAvail(lua_State* L)
{
    ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
    *vec2 = ImGui::GetContentRegionAvail();
    return 1;
}
static int lib_GetCursorPos(lua_State* L)
{
    ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
    *vec2 = ImGui::GetCursorPos();
    return 1;
}
static int lib_GetCursorPosX(lua_State* L)
{
    const float ret = ImGui::GetCursorPosX();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_GetCursorPosY(lua_State* L)
{
    const float ret = ImGui::GetCursorPosY();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_SetCursorPos(lua_State* L)
{
    ImVec2* local_pos = lua::as_type_instance<ImVec2>(L, 1);
    ImGui::SetCursorPos(*local_pos);
    return 0;
}
static int lib_SetCursorPosX(lua_State* L)
{
    const float local_x = (float)luaL_checknumber(L, 1);
    ImGui::SetCursorPosX(local_x);
    return 0;
}
static int lib_SetCursorPosY(lua_State* L)
{
    const float local_y = (float)luaL_checknumber(L, 1);
    ImGui::SetCursorPosY(local_y);
    return 0;
}
static int lib_GetCursorStartPos(lua_State* L)
{
    ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
    *vec2 = ImGui::GetCursorStartPos();
    return 1;
}

//////// Other layout functions

static int lib_Separator(lua_State* L)
{
    std::ignore = L;
    ImGui::Separator();
    return 0;
}
static int lib_SameLine(lua_State* L)
{
    const int argc = lua_gettop(L);
    if (argc < 1)
    {
        ImGui::SameLine();
    }
    else if (argc == 1)
    {
        const float offset_from_start_x = (float)luaL_checknumber(L, 1);
        ImGui::SameLine(offset_from_start_x);
    }
    else
    {
        const float offset_from_start_x = (float)luaL_checknumber(L, 1);
        const float spacing = (float)luaL_checknumber(L, 2);
        ImGui::SameLine(offset_from_start_x, spacing);
    }
    return 0;
}
static int lib_NewLine(lua_State* L)
{
    std::ignore = L;
    ImGui::NewLine();
    return 0;
}
static int lib_Spacing(lua_State* L)
{
    std::ignore = L;
    ImGui::Spacing();
    return 0;
}
static int lib_Dummy(lua_State* L)
{
    ImVec2* size = lua::as_type_instance<ImVec2>(L, 1);
    ImGui::Dummy(*size);
    return 0;
}
static int lib_Indent(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        const float indent_w = (float)luaL_checknumber(L, 1);
        ImGui::Indent(indent_w);
    }
    else
    {
        ImGui::Indent();
    }
    return 0;
}
static int lib_Unindent(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        const float indent_w = (float)luaL_checknumber(L, 1);
        ImGui::Unindent(indent_w);
    }
    else
    {
        ImGui::Unindent();
    }
    return 0;
}
static int lib_BeginGroup(lua_State* L)
{
    std::ignore = L;
    ImGui::BeginGroup();
    return 0;
}
static int lib_EndGroup(lua_State* L)
{
    std::ignore = L;
    ImGui::EndGroup();
    return 0;
}
static int lib_AlignTextToFramePadding(lua_State* L)
{
    std::ignore = L;
    ImGui::AlignTextToFramePadding();
    return 0;
}
static int lib_GetTextLineHeight(lua_State* L)
{
    const float ret = ImGui::GetTextLineHeight();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_GetTextLineHeightWithSpacing(lua_State* L)
{
    const float ret = ImGui::GetTextLineHeightWithSpacing();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_GetFrameHeight(lua_State* L)
{
    const float ret = ImGui::GetFrameHeight();
    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}
static int lib_GetFrameHeightWithSpacing(lua_State* L)
{
    const float ret = ImGui::GetFrameHeightWithSpacing();
    lua_pushnumber(L, (lua_Number)ret);
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
            void const* ptr_id = lua_touserdata(L, 1);
            ImGui::PushID(ptr_id);
            return 0;
        }
    case LUA_TNUMBER:
        {
            auto const int_id = luaL_checkinteger(L, 1);
            ImGui::PushID(static_cast<int>(int_id));
            return 0;
        }
    default:
    case LUA_TSTRING:
        {
            size_t len{};
            char const* str_id = luaL_checklstring(L, 1, &len);
            ImGui::PushID(str_id, str_id + len);
            return 0;
        }
    }
}
static int lib_PopID(lua_State* L)
{
    std::ignore = L;
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
            void const* ptr_id = lua_touserdata(L, 1);
            auto const imgui_id = ImGui::GetID(ptr_id);
            lua_pushnumber(L, static_cast<lua_Number>(imgui_id));
            return 1;
        }
    case LUA_TNUMBER:
        {
            auto const int_id = luaL_checkinteger(L, 1);
            auto const imgui_id = ImGui::GetID(static_cast<int>(int_id));
            lua_pushnumber(L, static_cast<lua_Number>(imgui_id));
            return 1;
        }
    default:
    case LUA_TSTRING:
        {
            size_t len{};
            char const* str_id = luaL_checklstring(L, 1, &len);
            auto const imgui_id = ImGui::GetID(str_id, str_id + len);
            lua_pushnumber(L, static_cast<lua_Number>(imgui_id));
            return 1;
        }
    }
}

//////// Widgets: Text

static int lib_TextUnformatted(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    // no text_end
    ImGui::TextUnformatted(text);
    return 0;
}
static int lib_Text(lua_State* L)
{
    const char* fmt = luaL_checkstring(L, 1);
    ImGui::Text(fmt);
    return 0;
}
static /* XXXX */ int lib_TextV(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static int lib_TextColored(lua_State* L)
{
    ImVec4* col = lua::as_type_instance<ImVec4>(L, 1);
    const char* fmt = luaL_checkstring(L, 2);
    ImGui::TextColored(*col, fmt);
    return 0;
}
static /* XXXX */ int lib_TextColoredV(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static int lib_TextDisabled(lua_State* L)
{
    const char* fmt = luaL_checkstring(L, 1);
    ImGui::TextDisabled(fmt);
    return 0;
}
static /* XXXX */ int lib_TextDisabledV(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static int lib_TextWrapped(lua_State* L)
{
    const char* fmt = luaL_checkstring(L, 1);
    ImGui::TextWrapped(fmt);
    return 0;
}
static /* XXXX */ int lib_TextWrappedV(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static int lib_LabelText(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    const char* fmt = luaL_checkstring(L, 2);
    ImGui::LabelText(label, fmt);
    return 0;
}
static /* XXXX */ int lib_LabelTextV(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static int lib_BulletText(lua_State* L)
{
    const char* fmt = luaL_checkstring(L, 1);
    ImGui::BulletText(fmt);
    return 0;
}
static /* XXXX */ int lib_BulletTextV(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static int lib_SeparatorText(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    ImGui::SeparatorText(label);
    return 0;
}

//////// Widgets: Main

static int lib_Button(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    if (lua_gettop(L) <= 1)
    {
        const bool ret = ImGui::Button(label);
        lua_pushboolean(L, ret);
    }
    else
    {
        ImVec2* size = lua::as_type_instance<ImVec2>(L, 2);
        const bool ret = ImGui::Button(label, *size);
        lua_pushboolean(L, ret);
    }
    return 1;
}
static int lib_SmallButton(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    const bool ret = ImGui::SmallButton(label);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_InvisibleButton(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 1);
    ImVec2* size = lua::as_type_instance<ImVec2>(L, 2);
    if (lua_gettop(L) <= 2)
    {
        const bool ret = ImGui::InvisibleButton(str_id, *size);
        lua_pushboolean(L, ret);
    }
    else
    {
        const ImGuiButtonFlags flags = (ImGuiButtonFlags)luaL_checkinteger(L, 3);
        const bool ret = ImGui::InvisibleButton(str_id, *size, flags);
        lua_pushboolean(L, ret);
    }
    return 1;
}
static int lib_ArrowButton(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 1);
    const ImGuiDir dir = (ImGuiDir)luaL_checkinteger(L, 2);
    const bool ret = ImGui::ArrowButton(str_id, dir);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_Checkbox(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    bool v = lua_toboolean(L, 2);
    const bool ret = ImGui::Checkbox(label, &v);
    lua_pushboolean(L, ret);
    lua_pushboolean(L, v);
    return 2;
}
static int lib_CheckboxFlags(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    unsigned int flags = (unsigned int)luaL_checkinteger(L, 2);
    const unsigned int flags_value = (unsigned int)luaL_checkinteger(L, 3);
    const bool ret = ImGui::CheckboxFlags(label, &flags, flags_value);
    lua_pushboolean(L, ret);
    lua_pushinteger(L, (lua_Integer)flags);
    return 2;
}
static int lib_RadioButton(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    if (lua_gettop(L) <= 2)
    {
        const bool active = lua_toboolean(L, 2);
        const bool ret = ImGui::RadioButton(label, active);
        lua_pushboolean(L, ret);
        return 1;
    }
    else
    {
        int v = (int)luaL_checkinteger(L, 2);
        const int v_button = (int)luaL_checkinteger(L, 3);
        const bool ret = ImGui::RadioButton(label, &v, v_button);
        lua_pushboolean(L, ret);
        lua_pushinteger(L, (lua_Integer)v);
        return 2;
    }
}
static int lib_ProgressBar(lua_State* L)
{
    int argc = lua_gettop(L);
    const float fraction = (float)luaL_checknumber(L, 1);
    if (argc <= 1)
    {
        ImGui::ProgressBar(fraction);
        return 0;
    }
    else if (argc == 2)
    {
        ImVec2* size_arg = lua::as_type_instance<ImVec2>(L, 2);
        ImGui::ProgressBar(fraction, *size_arg);
        
    }
    else
    {
        ImVec2* size_arg = lua::as_type_instance<ImVec2>(L, 2);
        const char* overlay = luaL_checkstring(L, 3);
        ImGui::ProgressBar(fraction, *size_arg, overlay);
    }
    return 0;
}
static int lib_Bullet(lua_State* L)
{
    std::ignore = L;
    ImGui::Bullet();
    return 0;
}
static int lib_TextLink(lua_State* L)
{
    char const* label = luaL_checkstring(L, 1);
    auto const r = ImGui::TextLink(label);
    lua_pushboolean(L, r);
    return 1;
}
static int lib_TextLinkOpenURL(lua_State* L)
{
    char const* label = luaL_checkstring(L, 1);
    char const* url = luaL_optstring(L, 2, nullptr);
    ImGui::TextLinkOpenURL(label, url);
    return 0;
}

//////// Widgets: Images

static /* !!!! */ int lib_Image(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}
static /* !!!! */ int lib_ImageButton(lua_State* L)
{
    LUA_IMGUI_NOT_SUPPORT;
}

//////// Widgets: Combo Box (Dropdown)

static int lib_BeginCombo(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    const char* preview_value = luaL_checkstring(L, 2);
    if (lua_gettop(L) <= 2)
    {
        const bool ret = ImGui::BeginCombo(label, preview_value);
        lua_pushboolean(L, ret);
    }
    else
    {
        const ImGuiComboFlags flags = (ImGuiComboFlags)luaL_checkinteger(L, 3);
        const bool ret = ImGui::BeginCombo(label, preview_value, flags);
        lua_pushboolean(L, ret);
    }
    return 1;
}
static int lib_EndCombo(lua_State* L)
{
    std::ignore = L;
    ImGui::EndCombo();
    return 0;
}
static int lib_Combo(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    bool ret = false;
    int current_item = (int)luaL_checkinteger(L, 2) - 1;
    switch(lua_type(L, 3))
    {
    case LUA_TTABLE:
        {
            if (argc <= 4)
            {
                const int items_count = (argc < 4) ? _luaL_len(L, 3) : (int)luaL_checkinteger(L, 4);
                char_array items(items_count);
                for (int i = 1; i <= items_count; i += 1)
                {
                    lua_pushinteger(L, i);
                    lua_gettable(L, 3);
                    items.data[i - 1] = (char*)luaL_checkstring(L, -1);
                    lua_pop(L, 1);
                }
                ret = ImGui::Combo(label, &current_item, items.data, items_count);
            }
            else
            {
                const int items_count = (int)luaL_checkinteger(L, 4);
                char_array items(items_count);
                for (int i = 1; i <= items_count; i += 1)
                {
                    lua_pushinteger(L, i);
                    lua_gettable(L, 3);
                    items.data[i - 1] = (char*)luaL_checkstring(L, -1);
                    lua_pop(L, 1);
                }
                const int popup_max_height_in_items = (int)luaL_checkinteger(L, 5);
                ret = ImGui::Combo(label, &current_item, items.data, items_count, popup_max_height_in_items);
            }
            break;
        }
    case LUA_TFUNCTION:
        {
            // currently, ImGui::Combo will call getter callback function immediately
            // so it's easy to handle lua getter callback function (same lua_State)
            struct Wrapper
            {
                static char const* Getter(void* data, int idx)
                {
                    lua_State* L = (lua_State*)data;
                    lua_pushvalue(L, 3);
                    lua_pushinteger(L, idx + 1); // lua style index
                    // ignore void* data param, out_text -> return strintg
                    lua_call(L, 1, 1);
                    if (lua_type(L, -1) == LUA_TSTRING)
                    {
                        char const *out_text = luaL_checkstring(L, -1);
                        lua_pop(L, 1);
                        return out_text;
                    }
                    else
                    {
                        lua_pop(L, 1);
                        return "";
                    }
                };
            };
            // ignore void* data param
            const int items_count = (int)luaL_checkinteger(L, 4);
            if (argc <= 4)
            {
                ret = ImGui::Combo(label, &current_item, &Wrapper::Getter, L, items_count);
            }
            else
            {
                const int popup_max_height_in_items = (int)luaL_optinteger(L, 5, -1);
                ret = ImGui::Combo(label, &current_item, &Wrapper::Getter, L, items_count, popup_max_height_in_items);
            }
            break;
        }
    default:
    case LUA_TSTRING:
        {
            const char* items_separated_by_zeros = luaL_checkstring(L, 3);
            if (argc <= 3)
            {
                ret = ImGui::Combo(label, &current_item, items_separated_by_zeros);
            }
            else
            {
                const int popup_max_height_in_items = (int)luaL_checkinteger(L, 4);
                ret = ImGui::Combo(label, &current_item, items_separated_by_zeros, popup_max_height_in_items);
            }
            break;
        }
    }
    lua_pushboolean(L, ret);
    lua_pushinteger(L, (lua_Integer)(current_item + 1));
    return 2;
}

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
        ImVec2* size = lua::as_type_instance<ImVec2>(L, 4);
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
        ImVec2* size = lua::as_type_instance<ImVec2>(L, 2);
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
        ImVec2* outer_size = lua::as_type_instance<ImVec2>(L, 4);
        const bool ret = ImGui::BeginTable(str_id, column, row_flags, *outer_size);
        lua_pushboolean(L, ret);
    }
    else
    {
        const ImGuiTableFlags row_flags = (ImGuiTableFlags)luaL_checkinteger(L, 3);
        ImVec2* outer_size = lua::as_type_instance<ImVec2>(L, 4);
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
    ImVec2* clip_rect_min = lua::as_type_instance<ImVec2>(L, 1);
    ImVec2* clip_rect_max = lua::as_type_instance<ImVec2>(L, 2);
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
    ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
    *vec2 = ImGui::GetItemRectMin();
    return 1;
}
static int lib_GetItemRectMax(lua_State* L)
{
    ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
    *vec2 = ImGui::GetItemRectMax();
    return 1;
}
static int lib_GetItemRectSize(lua_State* L)
{
    ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
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
    ImVec2* v1 = lua::as_type_instance<ImVec2>(L, 1);
    if (lua_gettop(L) >= 2)
    {
        ImVec2* v2 = lua::as_type_instance<ImVec2>(L, 2);
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
        ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
        *vec2 = ImGui::CalcTextSize(text, text_end, hide_text_after_double_hash, wrap_width);
    }
    else
    {
        size_t text_length = 0;
        const char* text = luaL_checklstring(L, 1, &text_length);
        const char* text_end = text + text_length;
        const bool hide_text_after_double_hash = (argc >= 2) ? lua_toboolean(L, 2) : false;
        const float wrap_width = (argc >= 3) ? (float)luaL_checknumber(L, 3) : -1.0f;
        ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
        *vec2 = ImGui::CalcTextSize(text, text_end, hide_text_after_double_hash, wrap_width);
    }
    return 1;
}

//////// Color Utilities

static int lib_ColorConvertU32ToFloat4(lua_State* L)
{
    const ImU32 in_ = (ImU32)luaL_checkinteger(L, 1);
    ImVec4* ret = lua::create_type_instance<ImVec4>(L);
    *ret = ImGui::ColorConvertU32ToFloat4(in_);
    return 1;
}
static int lib_ColorConvertFloat4ToU32(lua_State* L)
{
    ImVec4* in_ = lua::as_type_instance<ImVec4>(L, 1);
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
    const ImGuiMouseButton delay = (float)luaL_checknumber(L, 2);
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
    ImVec2* r_min = lua::as_type_instance<ImVec2>(L, 1);
    ImVec2* r_max = lua::as_type_instance<ImVec2>(L, 2);
    const bool clip = (argc >= 3) ? lua_toboolean(L, 3) : true;
    const bool ret = ImGui::IsMouseHoveringRect(*r_min, *r_max, clip);
    lua_pushboolean(L, ret);
    return 1;
}
static int lib_IsMousePosValid(lua_State* L)
{
    const int argc = lua_gettop(L);
    ImVec2* mouse_pos = (argc >= 1) ? lua::as_type_instance<ImVec2>(L, 1) : NULL;
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
    ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
    *vec2 = ImGui::GetMousePos();
    return 1;
}
static int lib_GetMousePosOnOpeningCurrentPopup(lua_State* L)
{
    ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
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
    ImVec2* vec2 = lua::create_type_instance<ImVec2>(L);
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
    lua_createtable(L, 0, lib_func);    // ? M k t
    _luaL_setfuncs(L, lib_fun);         // ? M k t
    lua_settable(L, -3);                // ? M
}
