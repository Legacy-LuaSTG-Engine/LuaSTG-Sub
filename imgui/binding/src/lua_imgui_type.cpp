#include "lua_imgui_common.hpp"
#include "lua_imgui_type.hpp"
#include "lua_imgui_hash.hpp"

struct __ImVec2
{
    ImVec2* data = nullptr;
    ptrdiff_t reft = false;
};

void imgui_binding_lua_register_ImVec2(lua_State* L)
{
    struct Binding
    {
        static int __index(lua_State* L)
        {
            ImVec2* data = imgui_binding_lua_to_ImVec2(L, 1);
            const char* key = luaL_checkstring(L, 2);
            if (key[1] == 0)
            {
                switch(key[0])
                {
                case 'x':
                    lua_pushnumber(L, (lua_Number)data->x);
                    return 1;
                case 'y':
                    lua_pushnumber(L, (lua_Number)data->y);
                    return 1;
                }
            }
            return 0;
        };
        static int __newindex(lua_State* L)
        {
            ImVec2* data = imgui_binding_lua_to_ImVec2(L, 1);
            const char* key = luaL_checkstring(L, 2);
            if (key[1] == 0)
            {
                switch(key[0])
                {
                case 'x':
                    data->x = (float)luaL_checknumber(L, 3);
                    return 0;
                case 'y':
                    data->y = (float)luaL_checknumber(L, 3);
                    return 0;
                }
            }
            return 0;
        };
        static int __gc(lua_State* L)
        {
            __ImVec2* p = (__ImVec2*)luaL_checkudata(L, 1, lua_class_imgui_ImVec2);
            if (!p->reft)
            {
                delete p->data;
                p->data = nullptr;
            }
            p->reft = false;
            return 0;
        };
        static int __tostring(lua_State* L)
        {
            lua_pushstring(L, lua_class_imgui_ImVec2);
            return 1;
        };
        
        static int create(lua_State* L)
        {
            const int args = lua_gettop(L);
            ImVec2* data = imgui_binding_lua_new_ImVec2(L);
            switch(args)
            {
            case 0:
                data->x = 0.0f;
                data->y = 0.0f;
                break;
            case 2:
                data->x = (float)luaL_checknumber(L, 1);
                data->y = (float)luaL_checknumber(L, 2);
                break;
            }
            return 1;
        };
    };
    
    const luaL_Reg mt_lib[] = {
        {"__index", &Binding::__index},
        {"__newindex", &Binding::__newindex},
        {"__gc", &Binding::__gc},
        {"__tostring", &Binding::__tostring},
        {NULL, NULL},
    };
    
    luaL_newmetatable(L, lua_class_imgui_ImVec2);
    _luaL_setfuncs(L, mt_lib);
    lua_pop(L, 1);
    
    const luaL_Reg cls_lib[] = {
        {"ImVec2", &Binding::create},
        {NULL, NULL},
    };
    
    _luaL_setfuncs(L, cls_lib);
}
ImVec2* imgui_binding_lua_new_ImVec2(lua_State* L)
{
    __ImVec2* p = (__ImVec2*)lua_newuserdata(L, sizeof(__ImVec2));
    p->data = new ImVec2;
    p->reft = false;
    luaL_getmetatable(L, lua_class_imgui_ImVec2);
    lua_setmetatable(L, -2);
    return p->data;
}
ImVec2* imgui_binding_lua_ref_ImVec2(lua_State* L, ImVec2* v)
{
    __ImVec2* p = (__ImVec2*)lua_newuserdata(L, sizeof(__ImVec2));
    p->data = v;
    p->reft = true;
    luaL_getmetatable(L, lua_class_imgui_ImVec2);
    lua_setmetatable(L, -2);
    return p->data;
}
ImVec2* imgui_binding_lua_to_ImVec2(lua_State* L, int idx)
{
    __ImVec2* p = (__ImVec2*)luaL_checkudata(L, idx, lua_class_imgui_ImVec2);
    return p->data;
}

struct __ImVec4
{
    ImVec4* data = nullptr;
    ptrdiff_t reft = false;
};

void imgui_binding_lua_register_ImVec4(lua_State* L)
{
    struct Binding
    {
        static int __index(lua_State* L)
        {
            ImVec4* data = imgui_binding_lua_to_ImVec4(L, 1);
            const char* key = luaL_checkstring(L, 2);
            if (key[1] == 0)
            {
                switch(key[0])
                {
                case 'x':
                    lua_pushnumber(L, (lua_Number)data->x);
                    return 1;
                case 'y':
                    lua_pushnumber(L, (lua_Number)data->y);
                    return 1;
                case 'z':
                    lua_pushnumber(L, (lua_Number)data->z);
                    return 1;
                case 'w':
                    lua_pushnumber(L, (lua_Number)data->w);
                    return 1;
                }
            }
            return 0;
        };
        static int __newindex(lua_State* L)
        {
            ImVec4* data = imgui_binding_lua_to_ImVec4(L, 1);
            const char* key = luaL_checkstring(L, 2);
            if (key[1] == 0)
            {
                switch(key[0])
                {
                case 'x':
                    data->x = (float)luaL_checknumber(L, 3);
                    return 0;
                case 'y':
                    data->y = (float)luaL_checknumber(L, 3);
                    return 0;
                case 'z':
                    data->z = (float)luaL_checknumber(L, 3);
                    return 0;
                case 'w':
                    data->w = (float)luaL_checknumber(L, 3);
                    return 0;
                }
            }
            return 0;
        };
        static int __gc(lua_State* L)
        {
            __ImVec4* p = (__ImVec4*)luaL_checkudata(L, 1, lua_class_imgui_ImVec4);
            if (!p->reft)
            {
                delete p->data;
                p->data = nullptr;
            }
            p->reft = false;
            return 0;
        };
        static int __tostring(lua_State* L)
        {
            lua_pushstring(L, lua_class_imgui_ImVec4);
            return 1;
        };
        
        static int create(lua_State* L)
        {
            const int args = lua_gettop(L);
            ImVec4* data = imgui_binding_lua_new_ImVec4(L);
            switch(args)
            {
            case 0:
                data->x = 0.0f;
                data->y = 0.0f;
                data->z = 0.0f;
                data->w = 0.0f;
                break;
            case 4:
                data->x = (float)luaL_checknumber(L, 1);
                data->y = (float)luaL_checknumber(L, 2);
                data->z = (float)luaL_checknumber(L, 3);
                data->w = (float)luaL_checknumber(L, 4);
                break;
            }
            return 1;
        };
    };
    
    const luaL_Reg mt_lib[] = {
        {"__index", &Binding::__index},
        {"__newindex", &Binding::__newindex},
        {"__gc", &Binding::__gc},
        {"__tostring", &Binding::__tostring},
        {NULL, NULL},
    };
    
    luaL_newmetatable(L, lua_class_imgui_ImVec4);
    _luaL_setfuncs(L, mt_lib);
    lua_pop(L, 1);
    
    const luaL_Reg cls_lib[] = {
        {"ImVec4", &Binding::create},
        {NULL, NULL},
    };
    
    _luaL_setfuncs(L, cls_lib);
}
ImVec4* imgui_binding_lua_new_ImVec4(lua_State* L)
{
    __ImVec4* p = (__ImVec4*)lua_newuserdata(L, sizeof(__ImVec4));
    p->data = new ImVec4;
    p->reft = false;
    luaL_getmetatable(L, lua_class_imgui_ImVec4);
    lua_setmetatable(L, -2);
    return p->data;
}
ImVec4* imgui_binding_lua_ref_ImVec4(lua_State* L, ImVec4* v)
{
    __ImVec4* p = (__ImVec4*)lua_newuserdata(L, sizeof(__ImVec4));
    p->data = v;
    p->reft = true;
    luaL_getmetatable(L, lua_class_imgui_ImVec4);
    lua_setmetatable(L, -2);
    return p->data;
}
ImVec4* imgui_binding_lua_to_ImVec4(lua_State* L, int idx)
{
    __ImVec4* p = (__ImVec4*)luaL_checkudata(L, idx, lua_class_imgui_ImVec4);
    return p->data;
}

constexpr auto imgui_binding_lua_class_array_ImVec4 = "imgui.ImVec4[]";
struct __array_ImVec4
{
    ImVec4* data = nullptr;
    size_t size = 0;
    ptrdiff_t reft = false;
};

void imgui_binding_lua_register_array_ImVec4(lua_State* L)
{
    struct Binding
    {
        static int __index(lua_State* L)
        {
            __array_ImVec4* p = (__array_ImVec4*)luaL_checkudata(L, 1, imgui_binding_lua_class_array_ImVec4);
            const auto idx = luaL_checkinteger(L, 2);
            if(idx >= 0 && idx < p->size)
            {
                imgui_binding_lua_ref_ImVec4(L, &p->data[idx]);
                return 1;
            }
            else
            {
                return luaL_error(L, "attempt to index a nil value [%d]", idx);
            }
        };
        static int __newindex(lua_State* L)
        {
            __array_ImVec4* p = (__array_ImVec4*)luaL_checkudata(L, 1, imgui_binding_lua_class_array_ImVec4);
            const auto idx = luaL_checkinteger(L, 2);
            if(idx >= 0 && idx < p->size)
            {
                p->data[idx] = *imgui_binding_lua_to_ImVec4(L, 3);
                return 0;
            }
            else
            {
                return luaL_error(L, "attempt to index a nil value [%d]", idx);
            }
        };
        static int __gc(lua_State* L)
        {
            __array_ImVec4* p = (__array_ImVec4*)luaL_checkudata(L, 1, imgui_binding_lua_class_array_ImVec4);
            if (!p->reft)
            {
                delete[] p->data;
                p->data = nullptr;
            }
            p->size = 0;
            p->reft = false;
            return 0;
        };
        static int __tostring(lua_State* L)
        {
            lua_pushstring(L, imgui_binding_lua_class_array_ImVec4);
            return 1;
        };
        
        static int create(lua_State* L)
        {
            return 0;
        };
    };
    
    const luaL_Reg mt_lib[] = {
        {"__index", &Binding::__index},
        {"__newindex", &Binding::__newindex},
        {"__gc", &Binding::__gc},
        {"__tostring", &Binding::__tostring},
        {NULL, NULL},
    };
    
    luaL_newmetatable(L, imgui_binding_lua_class_array_ImVec4);
    _luaL_setfuncs(L, mt_lib);
    lua_pop(L, 1);
    
    const luaL_Reg cls_lib[] = {
        {"ImVec4[]", &Binding::create},
        {NULL, NULL},
    };
    
    // _luaL_setfuncs(L, cls_lib);
}
ImVec4* imgui_binding_lua_new_array_ImVec4(lua_State* L, size_t n)
{
    __array_ImVec4* p = (__array_ImVec4*)lua_newuserdata(L, sizeof(__array_ImVec4));
    p->data = new ImVec4[n];
    p->size = n;
    p->reft = false;
    luaL_getmetatable(L, imgui_binding_lua_class_array_ImVec4);
    lua_setmetatable(L, -2);
    return p->data;
}
ImVec4* imgui_binding_lua_ref_array_ImVec4(lua_State* L, ImVec4* v, size_t n)
{
    __array_ImVec4* p = (__array_ImVec4*)lua_newuserdata(L, sizeof(__array_ImVec4));
    p->data = v;
    p->size = n;
    p->reft = true;
    luaL_getmetatable(L, imgui_binding_lua_class_array_ImVec4);
    lua_setmetatable(L, -2);
    return p->data;
}
ImVec4* imgui_binding_lua_to_array_ImVec4(lua_State* L, int idx)
{
    __array_ImVec4* p = (__array_ImVec4*)luaL_checkudata(L, idx, imgui_binding_lua_class_array_ImVec4);
    return p->data;
}

struct __ImGuiStyle
{
    ImGuiStyle* data = nullptr;
    ptrdiff_t reft = false;
};

void imgui_binding_lua_register_ImGuiStyle(lua_State* L)
{
    using E = imgui_binding_lua_PropertiesHash;
    struct Binding
    {
        static int ScaleAllSizes(lua_State* L)
        {
            ImGuiStyle* data = imgui_binding_lua_to_ImGuiStyle(L, 1);
            const float factor = (float)luaL_checknumber(L, 2);
            data->ScaleAllSizes(factor);
            return 0;
        };
        
        static int __index(lua_State* L)
        {
            ImGuiStyle* data = imgui_binding_lua_to_ImGuiStyle(L, 1);
            const char* key = luaL_checkstring(L, 2);
            switch(imgui_binding_lua_ComputePropertiesHash(key))
            {
            case E::Alpha:
                lua_pushnumber(L, (lua_Number)data->Alpha);
                return 1;
            case E::DisabledAlpha:
                lua_pushnumber(L, (lua_Number)data->DisabledAlpha);
                return 1;
            case E::WindowPadding:
                imgui_binding_lua_ref_ImVec2(L, &data->WindowPadding);
                return 1;
            case E::WindowRounding:
                lua_pushnumber(L, (lua_Number)data->WindowRounding);
                return 1;
            case E::WindowBorderSize:
                lua_pushnumber(L, (lua_Number)data->WindowBorderSize);
                return 1;
            case E::WindowMinSize:
                imgui_binding_lua_ref_ImVec2(L, &data->WindowMinSize);
                return 1;
            case E::WindowTitleAlign:
                imgui_binding_lua_ref_ImVec2(L, &data->WindowTitleAlign);
                return 1;
            case E::WindowMenuButtonPosition:
                lua_pushinteger(L, (lua_Integer)data->WindowMenuButtonPosition);
                return 1;
            case E::ChildRounding:
                lua_pushnumber(L, (lua_Number)data->ChildRounding);
                return 1;
            case E::ChildBorderSize:
                lua_pushnumber(L, (lua_Number)data->ChildBorderSize);
                return 1;
            case E::PopupRounding:
                lua_pushnumber(L, (lua_Number)data->PopupRounding);
                return 1;
            case E::PopupBorderSize:
                lua_pushnumber(L, (lua_Number)data->PopupBorderSize);
                return 1;
            case E::FramePadding:
                imgui_binding_lua_ref_ImVec2(L, &data->FramePadding);
                return 1;
            case E::FrameRounding:
                lua_pushnumber(L, (lua_Number)data->FrameRounding);
                return 1;
            case E::FrameBorderSize:
                lua_pushnumber(L, (lua_Number)data->FrameBorderSize);
                return 1;
            case E::ItemSpacing:
                imgui_binding_lua_ref_ImVec2(L, &data->ItemSpacing);
                return 1;
            case E::ItemInnerSpacing:
                imgui_binding_lua_ref_ImVec2(L, &data->ItemInnerSpacing);
                return 1;
            case E::TouchExtraPadding:
                imgui_binding_lua_ref_ImVec2(L, &data->TouchExtraPadding);
                return 1;
            case E::IndentSpacing:
                lua_pushnumber(L, (lua_Number)data->IndentSpacing);
                return 1;
            case E::ColumnsMinSpacing:
                lua_pushnumber(L, (lua_Number)data->ColumnsMinSpacing);
                return 1;
            case E::ScrollbarSize:
                lua_pushnumber(L, (lua_Number)data->ScrollbarSize);
                return 1;
            case E::ScrollbarRounding:
                lua_pushnumber(L, (lua_Number)data->ScrollbarRounding);
                return 1;
            case E::GrabMinSize:
                lua_pushnumber(L, (lua_Number)data->GrabMinSize);
                return 1;
            case E::GrabRounding:
                lua_pushnumber(L, (lua_Number)data->GrabRounding);
                return 1;
            case E::LogSliderDeadzone:
                lua_pushnumber(L, (lua_Number)data->LogSliderDeadzone);
                return 1;
            case E::TabRounding:
                lua_pushnumber(L, (lua_Number)data->TabRounding);
                return 1;
            case E::TabBorderSize:
                lua_pushnumber(L, (lua_Number)data->TabBorderSize);
                return 1;
            case E::TabMinWidthForCloseButton:
                lua_pushnumber(L, (lua_Number)data->TabMinWidthForCloseButton);
                return 1;
            case E::ColorButtonPosition:
                lua_pushinteger(L, (lua_Integer)data->ColorButtonPosition);
                return 1;
            case E::ButtonTextAlign:
                imgui_binding_lua_ref_ImVec2(L, &data->ButtonTextAlign);
                return 1;
            case E::SelectableTextAlign:
                imgui_binding_lua_ref_ImVec2(L, &data->SelectableTextAlign);
                return 1;
            case E::DisplayWindowPadding:
                imgui_binding_lua_ref_ImVec2(L, &data->DisplayWindowPadding);
                return 1;
            case E::DisplaySafeAreaPadding:
                imgui_binding_lua_ref_ImVec2(L, &data->DisplaySafeAreaPadding);
                return 1;
            case E::MouseCursorScale:
                lua_pushnumber(L, (lua_Number)data->MouseCursorScale);
                return 1;
            case E::AntiAliasedLines:
                lua_pushboolean(L, data->AntiAliasedLines);
                return 1;
            case E::AntiAliasedLinesUseTex:
                lua_pushboolean(L, data->AntiAliasedLinesUseTex);
                return 1;
            case E::AntiAliasedFill:
                lua_pushboolean(L, data->AntiAliasedFill);
                return 1;
            case E::CurveTessellationTol:
                lua_pushnumber(L, (lua_Number)data->CurveTessellationTol);
                return 1;
            case E::CircleTessellationMaxError:
                lua_pushnumber(L, (lua_Number)data->CircleTessellationMaxError);
                return 1;
            case E::Colors:
                imgui_binding_lua_ref_array_ImVec4(L, data->Colors, ImGuiCol_COUNT);
                return 1;
            case E::ScaleAllSizes:
                lua_pushcfunction(L, &ScaleAllSizes);
                return 1;
            case E::__NOT_FOUND:
                return luaL_error(L, "attempt to index a nil value '%s'", key);
            }
            return 0;
        };
        static int __newindex(lua_State* L)
        {
            ImGuiStyle* data = imgui_binding_lua_to_ImGuiStyle(L, 1);
            const char* key = luaL_checkstring(L, 2);
            switch(imgui_binding_lua_ComputePropertiesHash(key))
            {
            case E::Alpha:
                data->Alpha = (float)luaL_checknumber(L, 3);
                break;
            case E::DisabledAlpha:
                data->DisabledAlpha = (float)luaL_checknumber(L, 3);
                break;
            case E::WindowPadding:
                data->WindowPadding = *imgui_binding_lua_to_ImVec2(L, 3);
                break;
            case E::WindowRounding:
                data->WindowRounding = (float)luaL_checknumber(L, 3);
                break;
            case E::WindowBorderSize:
                data->WindowBorderSize = (float)luaL_checknumber(L, 3);
                break;
            case E::WindowMinSize:
                data->WindowMinSize = *imgui_binding_lua_to_ImVec2(L, 3);
                break;
            case E::WindowTitleAlign:
                data->WindowTitleAlign = *imgui_binding_lua_to_ImVec2(L, 3);
                break;
            case E::WindowMenuButtonPosition:
                data->WindowMenuButtonPosition = (int)luaL_checkinteger(L, 3);
                break;
            case E::ChildRounding:
                data->ChildRounding = (float)luaL_checknumber(L, 3);
                break;
            case E::ChildBorderSize:
                data->ChildBorderSize = (float)luaL_checknumber(L, 3);
                break;
            case E::PopupRounding:
                data->PopupRounding = (float)luaL_checknumber(L, 3);
                break;
            case E::PopupBorderSize:
                data->PopupBorderSize = (float)luaL_checknumber(L, 3);
                break;
            case E::FramePadding:
                data->FramePadding = *imgui_binding_lua_to_ImVec2(L, 3);
                break;
            case E::FrameRounding:
                data->FrameRounding = (float)luaL_checknumber(L, 3);
                break;
            case E::FrameBorderSize:
                data->FrameBorderSize = (float)luaL_checknumber(L, 3);
                break;
            case E::ItemSpacing:
                data->WindowMinSize = *imgui_binding_lua_to_ImVec2(L, 3);
                break;
            case E::ItemInnerSpacing:
                data->ItemInnerSpacing = *imgui_binding_lua_to_ImVec2(L, 3);
                break;
            case E::TouchExtraPadding:
                data->TouchExtraPadding = *imgui_binding_lua_to_ImVec2(L, 3);
                break;
            case E::IndentSpacing:
                data->IndentSpacing = (float)luaL_checknumber(L, 3);
                break;
            case E::ColumnsMinSpacing:
                data->ColumnsMinSpacing = (float)luaL_checknumber(L, 3);
                break;
            case E::ScrollbarSize:
                data->ScrollbarSize = (float)luaL_checknumber(L, 3);
                break;
            case E::ScrollbarRounding:
                data->ScrollbarRounding = (float)luaL_checknumber(L, 3);
                break;
            case E::GrabMinSize:
                data->GrabMinSize = (float)luaL_checknumber(L, 3);
                break;
            case E::GrabRounding:
                data->GrabRounding = (float)luaL_checknumber(L, 3);
                break;
            case E::LogSliderDeadzone:
                data->LogSliderDeadzone = (float)luaL_checknumber(L, 3);
                break;
            case E::TabRounding:
                data->TabRounding = (float)luaL_checknumber(L, 3);
                break;
            case E::TabBorderSize:
                data->TabBorderSize = (float)luaL_checknumber(L, 3);
                break;
            case E::TabMinWidthForCloseButton:
                data->TabMinWidthForCloseButton = (float)luaL_checknumber(L, 3);
                break;
            case E::ColorButtonPosition:
                data->ColorButtonPosition = (int)luaL_checkinteger(L, 3);
                break;
            case E::ButtonTextAlign:
                data->ButtonTextAlign = *imgui_binding_lua_to_ImVec2(L, 3);
                break;
            case E::SelectableTextAlign:
                data->SelectableTextAlign = *imgui_binding_lua_to_ImVec2(L, 3);
                break;
            case E::DisplayWindowPadding:
                data->DisplayWindowPadding = *imgui_binding_lua_to_ImVec2(L, 3);
                break;
            case E::DisplaySafeAreaPadding:
                data->DisplaySafeAreaPadding = *imgui_binding_lua_to_ImVec2(L, 3);
                break;
            case E::MouseCursorScale:
                data->MouseCursorScale = (float)luaL_checknumber(L, 3);
                break;
            case E::AntiAliasedLines:
                data->AntiAliasedLines = (bool)lua_toboolean(L, 3);
                break;
            case E::AntiAliasedLinesUseTex:
                data->AntiAliasedLinesUseTex = (bool)lua_toboolean(L, 3);
                break;
            case E::AntiAliasedFill:
                data->AntiAliasedFill = (bool)lua_toboolean(L, 3);
                break;
            case E::CurveTessellationTol:
                data->CurveTessellationTol = (float)luaL_checknumber(L, 3);
                break;
            case E::CircleTessellationMaxError:
                data->CircleTessellationMaxError = (float)luaL_checknumber(L, 3);
                break;
            case E::Colors:
                imgui_binding_lua_ref_array_ImVec4(L, data->Colors, ImGuiCol_COUNT);
                return 1;
            case E::ScaleAllSizes:
            case E::__NOT_FOUND:
                return luaL_error(L, "attempt to index a nil value '%s'", key);
            }
            return 0;
        };
        static int __gc(lua_State* L)
        {
            __ImVec4* p = (__ImVec4*)luaL_checkudata(L, 1, lua_class_imgui_ImGuiStyle);
            if (!p->reft)
            {
                delete p->data;
                p->data = nullptr;
            }
            p->reft = false;
            return 0;
        };
        static int __tostring(lua_State* L)
        {
            lua_pushstring(L, lua_class_imgui_ImGuiStyle);
            return 1;
        };
        
        static int create(lua_State* L)
        {
            imgui_binding_lua_new_ImGuiStyle(L);
            return 1;
        };
    };
    
    const luaL_Reg mt_lib[] = {
        {"__index", &Binding::__index},
        {"__newindex", &Binding::__newindex},
        {"__gc", &Binding::__gc},
        {"__tostring", &Binding::__tostring},
        {NULL, NULL},
    };
    
    luaL_newmetatable(L, lua_class_imgui_ImGuiStyle);
    _luaL_setfuncs(L, mt_lib);
    lua_pop(L, 1);
    
    const luaL_Reg cls_lib[] = {
        {"ImGuiStyle", &Binding::create},
        {NULL, NULL},
    };
    
    _luaL_setfuncs(L, cls_lib);
    
    imgui_binding_lua_register_array_ImVec4(L); // TODO: fuck it
}
ImGuiStyle* imgui_binding_lua_new_ImGuiStyle(lua_State* L)
{
    __ImGuiStyle* p = (__ImGuiStyle*)lua_newuserdata(L, sizeof(__ImGuiStyle));
    p->data = new ImGuiStyle;
    p->reft = false;
    luaL_getmetatable(L, lua_class_imgui_ImGuiStyle);
    lua_setmetatable(L, -2);
    return p->data;
}
ImGuiStyle* imgui_binding_lua_ref_ImGuiStyle(lua_State* L, ImGuiStyle* v)
{
    __ImGuiStyle* p = (__ImGuiStyle*)lua_newuserdata(L, sizeof(__ImGuiStyle));
    p->data = v;
    p->reft = true;
    luaL_getmetatable(L, lua_class_imgui_ImGuiStyle);
    lua_setmetatable(L, -2);
    return p->data;
}
ImGuiStyle* imgui_binding_lua_to_ImGuiStyle(lua_State* L, int idx)
{
    __ImGuiStyle* p = (__ImGuiStyle*)luaL_checkudata(L, idx, lua_class_imgui_ImGuiStyle);
    return p->data;
}
