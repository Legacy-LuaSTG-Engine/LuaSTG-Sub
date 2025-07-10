// ImGui::ColorX

static int lib_ColorEdit3(lua_State* L)
{
#define N 3
#define F ImGui::ColorEdit3

    const char* label = luaL_checkstring(L, 1);
    float col[N] = {};
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        col[idx] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }

    if (lua_gettop(L) <= 2)
    {
        ret = F(label, col);
    }
    else
    {
        const ImGuiColorEditFlags flags = (ImGuiColorEditFlags)luaL_checkinteger(L, 3);
        ret = F(label, col, flags);
    }
    
    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_pushnumber(L, (lua_Number)col[idx]);
        lua_settable(L, 2);
    }

    lua_pushboolean(L, ret);
    lua_pushvalue(L, 2);
    return 2;

#undef N
#undef F
}
static int lib_ColorEdit4(lua_State* L)
{
#define N 4
#define F ImGui::ColorEdit4

    const char* label = luaL_checkstring(L, 1);
    float col[N] = {};
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        col[idx] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }

    if (lua_gettop(L) <= 2)
    {
        ret = F(label, col);
    }
    else
    {
        const ImGuiColorEditFlags flags = (ImGuiColorEditFlags)luaL_checkinteger(L, 3);
        ret = F(label, col, flags);
    }
    
    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_pushnumber(L, (lua_Number)col[idx]);
        lua_settable(L, 2);
    }

    lua_pushboolean(L, ret);
    lua_pushvalue(L, 2);
    return 2;

#undef N
#undef F
}
static int lib_ColorPicker3(lua_State* L)
{
#define N 3
#define F ImGui::ColorPicker3

    const char* label = luaL_checkstring(L, 1);
    float col[N] = {};
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        col[idx] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }

    if (lua_gettop(L) <= 2)
    {
        ret = F(label, col);
    }
    else
    {
        const ImGuiColorEditFlags flags = (ImGuiColorEditFlags)luaL_checkinteger(L, 3);
        ret = F(label, col, flags);
    }
    
    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_pushnumber(L, (lua_Number)col[idx]);
        lua_settable(L, 2);
    }

    lua_pushboolean(L, ret);
    lua_pushvalue(L, 2);
    return 2;

#undef N
#undef F
}
static int lib_ColorPicker4(lua_State* L)
{
#define N 4
#define F ImGui::ColorPicker4

    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float col[N] = {};
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        col[idx] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }

    if (argc <= 2)
    {
        ret = F(label, col);
    }
    else if (argc == 3)
    {
        const ImGuiColorEditFlags flags = (ImGuiColorEditFlags)luaL_checkinteger(L, 3);
        ret = F(label, col, flags);
    }
    else
    {
        float ref_col[N];

        for (int idx = 0; idx < N; idx += 1)
        {
            lua_pushinteger(L, idx + 1);
            lua_gettable(L, 4);
            ref_col[idx] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }

        const ImGuiColorEditFlags flags = (ImGuiColorEditFlags)luaL_checkinteger(L, 3);
        ret = F(label, col, flags, ref_col);
    }
    
    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_pushnumber(L, (lua_Number)col[idx]);
        lua_settable(L, 2);
    }

    lua_pushboolean(L, ret);
    lua_pushvalue(L, 2);
    return 2;

#undef N
#undef F
}

static int lib_ColorButton(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* desc_id = luaL_checkstring(L, 1);
    ImVec4* col = imgui::binding::ImVec4Binding::as(L, 2);
    bool ret = false;
    if (argc <= 2)
    {
        ret = ImGui::ColorButton(desc_id, *col);
    }
    else if (argc == 3)
    {
        const ImGuiColorEditFlags flags = (ImGuiColorEditFlags)luaL_checkinteger(L, 3);
        ret = ImGui::ColorButton(desc_id, *col, flags);
    }
    else
    {
        const ImGuiColorEditFlags flags = (ImGuiColorEditFlags)luaL_checkinteger(L, 3);
        ImVec2* size = imgui::binding::ImVec2Binding::as(L, 4);
        ret = ImGui::ColorButton(desc_id, *col, flags, *size);
    }
    lua_pushboolean(L, ret);
    return 1;
}

static int lib_SetColorEditOptions(lua_State* L)
{
    const ImGuiColorEditFlags flags = (ImGuiColorEditFlags)luaL_checkinteger(L, 1);
    ImGui::SetColorEditOptions(flags);
    return 0;
}
