// ImGui::DragX

static int lib_DragFloat(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float v = (float)luaL_checknumber(L, 2);
    bool ret = false;
    if (argc <= 2)
    {
        ret = ImGui::DragFloat(label, &v);
    }
    else if (argc == 3)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        ret = ImGui::DragFloat(label, &v, v_speed);
    }
    else if (argc == 4)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        ret = ImGui::DragFloat(label, &v, v_speed, v_min);
    }
    else if (argc == 5)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        const float v_max = (float)luaL_checknumber(L, 5);
        ret = ImGui::DragFloat(label, &v, v_speed, v_min, v_max);
    }
    else if (argc == 6)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        const float v_max = (float)luaL_checknumber(L, 5);
        const char* format = luaL_checkstring(L, 6);
        ret = ImGui::DragFloat(label, &v, v_speed, v_min, v_max, format);
    }
    else
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        const float v_max = (float)luaL_checknumber(L, 5);
        const char* format = luaL_checkstring(L, 6);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 7);
        ret = ImGui::DragFloat(label, &v, v_speed, v_min, v_max, format, flags);
    }
    lua_pushboolean(L, ret);
    lua_pushnumber(L, (lua_Number)v);
    return 2;
}
static int lib_DragFloat2(lua_State* L)
{
#define N 2
#define F ImGui::DragFloat2

    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float v[N] = {};
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        v[idx] = (float)luaL_checknumber(L, argc + 1);
        lua_pop(L, 1);
    }

    if (argc <= 2)
    {
        ret = F(label, v);
    }
    else if (argc == 3)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        ret = F(label, v, v_speed);
    }
    else if (argc == 4)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        ret = F(label, v, v_speed, v_min);
    }
    else if (argc == 5)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        const float v_max = (float)luaL_checknumber(L, 5);
        ret = F(label, v, v_speed, v_min, v_max);
    }
    else if (argc == 6)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        const float v_max = (float)luaL_checknumber(L, 5);
        const char* format = luaL_checkstring(L, 6);
        ret = F(label, v, v_speed, v_min, v_max, format);
    }
    else
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        const float v_max = (float)luaL_checknumber(L, 5);
        const char* format = luaL_checkstring(L, 6);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 7);
        ret = F(label, v, v_speed, v_min, v_max, format, flags);
    }

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_pushnumber(L, (lua_Number)v[idx]);
        lua_settable(L, 2);
    }

    lua_pushboolean(L, ret);
    lua_pushvalue(L, 2);
    return 2;

#undef N
#undef F
}
static int lib_DragFloat3(lua_State* L)
{
#define N 3
#define F ImGui::DragFloat3

    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float v[N] = {};
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        v[idx] = (float)luaL_checknumber(L, argc + 1);
        lua_pop(L, 1);
    }

    if (argc <= 2)
    {
        ret = F(label, v);
    }
    else if (argc == 3)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        ret = F(label, v, v_speed);
    }
    else if (argc == 4)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        ret = F(label, v, v_speed, v_min);
    }
    else if (argc == 5)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        const float v_max = (float)luaL_checknumber(L, 5);
        ret = F(label, v, v_speed, v_min, v_max);
    }
    else if (argc == 6)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        const float v_max = (float)luaL_checknumber(L, 5);
        const char* format = luaL_checkstring(L, 6);
        ret = F(label, v, v_speed, v_min, v_max, format);
    }
    else
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        const float v_max = (float)luaL_checknumber(L, 5);
        const char* format = luaL_checkstring(L, 6);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 7);
        ret = F(label, v, v_speed, v_min, v_max, format, flags);
    }

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_pushnumber(L, (lua_Number)v[idx]);
        lua_settable(L, 2);
    }

    lua_pushboolean(L, ret);
    lua_pushvalue(L, 2);
    return 2;

#undef N
#undef F
}
static int lib_DragFloat4(lua_State* L)
{
#define N 4
#define F ImGui::DragFloat4

    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float v[N] = {};
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        v[idx] = (float)luaL_checknumber(L, argc + 1);
        lua_pop(L, 1);
    }

    if (argc <= 2)
    {
        ret = F(label, v);
    }
    else if (argc == 3)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        ret = F(label, v, v_speed);
    }
    else if (argc == 4)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        ret = F(label, v, v_speed, v_min);
    }
    else if (argc == 5)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        const float v_max = (float)luaL_checknumber(L, 5);
        ret = F(label, v, v_speed, v_min, v_max);
    }
    else if (argc == 6)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        const float v_max = (float)luaL_checknumber(L, 5);
        const char* format = luaL_checkstring(L, 6);
        ret = F(label, v, v_speed, v_min, v_max, format);
    }
    else
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const float v_min = (float)luaL_checknumber(L, 4);
        const float v_max = (float)luaL_checknumber(L, 5);
        const char* format = luaL_checkstring(L, 6);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 7);
        ret = F(label, v, v_speed, v_min, v_max, format, flags);
    }

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_pushnumber(L, (lua_Number)v[idx]);
        lua_settable(L, 2);
    }

    lua_pushboolean(L, ret);
    lua_pushvalue(L, 2);
    return 2;

#undef N
#undef F
}
static int lib_DragFloatRange2(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float v_current_min = (float)luaL_checknumber(L, 2);
    float v_current_max = (float)luaL_checknumber(L, 3);
    bool ret = false;
    if (argc <= 3)
    {
        ret = ImGui::DragFloatRange2(label, &v_current_min, &v_current_max);
    }
    else if (argc == 4)
    {
        const float v_speed = (float)luaL_checknumber(L, 4);
        ret = ImGui::DragFloatRange2(label, &v_current_min, &v_current_max, v_speed);
    }
    else if (argc == 5)
    {
        const float v_speed = (float)luaL_checknumber(L, 4);
        const float v_min = (float)luaL_checknumber(L, 5);
        ret = ImGui::DragFloatRange2(label, &v_current_min, &v_current_max, v_speed, v_min);
    }
    else if (argc == 6)
    {
        const float v_speed = (float)luaL_checknumber(L, 4);
        const float v_min = (float)luaL_checknumber(L, 5);
        const float v_max = (float)luaL_checknumber(L, 6);
        ret = ImGui::DragFloatRange2(label, &v_current_min, &v_current_max,
            v_speed, v_min, v_max);
    }
    else if (argc == 7)
    {
        const float v_speed = (float)luaL_checknumber(L, 4);
        const float v_min = (float)luaL_checknumber(L, 5);
        const float v_max = (float)luaL_checknumber(L, 6);
        const char* format = luaL_checkstring(L, 7);
        ret = ImGui::DragFloatRange2(label, &v_current_min, &v_current_max,
            v_speed, v_min, v_max, format);
    }
    else if (argc == 8)
    {
        const float v_speed = (float)luaL_checknumber(L, 4);
        const float v_min = (float)luaL_checknumber(L, 5);
        const float v_max = (float)luaL_checknumber(L, 6);
        const char* format = luaL_checkstring(L, 7);
        const char* format_max = luaL_checkstring(L, 8);
        ret = ImGui::DragFloatRange2(label, &v_current_min, &v_current_max,
            v_speed, v_min, v_max, format, format_max);
    }
    else
    {
        const float v_speed = (float)luaL_checknumber(L, 4);
        const float v_min = (float)luaL_checknumber(L, 5);
        const float v_max = (float)luaL_checknumber(L, 6);
        const char* format = luaL_checkstring(L, 7);
        const char* format_max = luaL_checkstring(L, 8);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 9);
        ret = ImGui::DragFloatRange2(label, &v_current_min, &v_current_max,
            v_speed, v_min, v_max, format, format_max, flags);
    }
    lua_pushboolean(L, ret);
    lua_pushnumber(L, (lua_Number)v_current_min);
    lua_pushnumber(L, (lua_Number)v_current_max);
    return 3;
}
static int lib_DragInt(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    int v = (int)luaL_checkinteger(L, 2);
    bool ret = false;
    if (argc <= 2)
    {
        ret = ImGui::DragInt(label, &v);
    }
    else if (argc == 3)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        ret = ImGui::DragInt(label, &v, v_speed);
    }
    else if (argc == 4)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        ret = ImGui::DragInt(label, &v, v_speed, v_min);
    }
    else if (argc == 5)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        const int v_max = (int)luaL_checkinteger(L, 5);
        ret = ImGui::DragInt(label, &v, v_speed, v_min, v_max);
    }
    else if (argc == 6)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        const int v_max = (int)luaL_checkinteger(L, 5);
        const char* format = luaL_checkstring(L, 6);
        ret = ImGui::DragInt(label, &v, v_speed, v_min, v_max, format);
    }
    else
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        const int v_max = (int)luaL_checkinteger(L, 5);
        const char* format = luaL_checkstring(L, 6);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 7);
        ret = ImGui::DragInt(label, &v, v_speed, v_min, v_max, format, flags);
    }
    lua_pushboolean(L, ret);
    lua_pushinteger(L, (lua_Integer)v);
    return 2;
}
static int lib_DragInt2(lua_State* L)
{
#define N 2
#define F ImGui::DragInt2

    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    int v[N] = {};
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        v[idx] = (int)luaL_checkinteger(L, argc + 1);
        lua_pop(L, 1);
    }

    if (argc <= 2)
    {
        ret = F(label, v);
    }
    else if (argc == 3)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        ret = F(label, v, v_speed);
    }
    else if (argc == 4)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        ret = F(label, v, v_speed, v_min);
    }
    else if (argc == 5)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        const int v_max = (int)luaL_checkinteger(L, 5);
        ret = F(label, v, v_speed, v_min, v_max);
    }
    else if (argc == 6)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        const int v_max = (int)luaL_checkinteger(L, 5);
        const char* format = luaL_checkstring(L, 6);
        ret = F(label, v, v_speed, v_min, v_max, format);
    }
    else
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        const int v_max = (int)luaL_checkinteger(L, 5);
        const char* format = luaL_checkstring(L, 6);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 7);
        ret = F(label, v, v_speed, v_min, v_max, format, flags);
    }

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_pushinteger(L, (lua_Integer)v[idx]);
        lua_settable(L, 2);
    }

    lua_pushboolean(L, ret);
    lua_pushvalue(L, 2);
    return 2;

#undef N
#undef F
}
static int lib_DragInt3(lua_State* L)
{
#define N 3
#define F ImGui::DragInt3

    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    int v[N] = {};
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        v[idx] = (int)luaL_checkinteger(L, argc + 1);
        lua_pop(L, 1);
    }

    if (argc <= 2)
    {
        ret = F(label, v);
    }
    else if (argc == 3)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        ret = F(label, v, v_speed);
    }
    else if (argc == 4)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        ret = F(label, v, v_speed, v_min);
    }
    else if (argc == 5)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        const int v_max = (int)luaL_checkinteger(L, 5);
        ret = F(label, v, v_speed, v_min, v_max);
    }
    else if (argc == 6)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        const int v_max = (int)luaL_checkinteger(L, 5);
        const char* format = luaL_checkstring(L, 6);
        ret = F(label, v, v_speed, v_min, v_max, format);
    }
    else
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        const int v_max = (int)luaL_checkinteger(L, 5);
        const char* format = luaL_checkstring(L, 6);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 7);
        ret = F(label, v, v_speed, v_min, v_max, format, flags);
    }

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_pushinteger(L, (lua_Integer)v[idx]);
        lua_settable(L, 2);
    }

    lua_pushboolean(L, ret);
    lua_pushvalue(L, 2);
    return 2;

#undef N
#undef F
}
static int lib_DragInt4(lua_State* L)
{
#define N 4
#define F ImGui::DragInt4

    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    int v[N] = {};
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        v[idx] = (int)luaL_checkinteger(L, argc + 1);
        lua_pop(L, 1);
    }

    if (argc <= 2)
    {
        ret = F(label, v);
    }
    else if (argc == 3)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        ret = F(label, v, v_speed);
    }
    else if (argc == 4)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        ret = F(label, v, v_speed, v_min);
    }
    else if (argc == 5)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        const int v_max = (int)luaL_checkinteger(L, 5);
        ret = F(label, v, v_speed, v_min, v_max);
    }
    else if (argc == 6)
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        const int v_max = (int)luaL_checkinteger(L, 5);
        const char* format = luaL_checkstring(L, 6);
        ret = F(label, v, v_speed, v_min, v_max, format);
    }
    else
    {
        const float v_speed = (float)luaL_checknumber(L, 3);
        const int v_min = (int)luaL_checkinteger(L, 4);
        const int v_max = (int)luaL_checkinteger(L, 5);
        const char* format = luaL_checkstring(L, 6);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 7);
        ret = F(label, v, v_speed, v_min, v_max, format, flags);
    }

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_pushinteger(L, (lua_Integer)v[idx]);
        lua_settable(L, 2);
    }

    lua_pushboolean(L, ret);
    lua_pushvalue(L, 2);
    return 2;

#undef N
#undef F
}
static int lib_DragIntRange2(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    int v_current_min = (int)luaL_checkinteger(L, 2);
    int v_current_max = (int)luaL_checkinteger(L, 3);
    bool ret = false;
    if (argc <= 3)
    {
        ret = ImGui::DragIntRange2(label, &v_current_min, &v_current_max);
    }
    else if (argc == 4)
    {
        const float v_speed = (float)luaL_checknumber(L, 4);
        ret = ImGui::DragIntRange2(label, &v_current_min, &v_current_max, v_speed);
    }
    else if (argc == 5)
    {
        const float v_speed = (float)luaL_checknumber(L, 4);
        const int v_min = (int)luaL_checkinteger(L, 5);
        ret = ImGui::DragIntRange2(label, &v_current_min, &v_current_max, v_speed, v_min);
    }
    else if (argc == 6)
    {
        const float v_speed = (float)luaL_checknumber(L, 4);
        const int v_min = (int)luaL_checkinteger(L, 5);
        const int v_max = (int)luaL_checkinteger(L, 6);
        ret = ImGui::DragIntRange2(label, &v_current_min, &v_current_max,
            v_speed, v_min, v_max);
    }
    else if (argc == 7)
    {
        const float v_speed = (float)luaL_checknumber(L, 4);
        const int v_min = (int)luaL_checkinteger(L, 5);
        const int v_max = (int)luaL_checkinteger(L, 6);
        const char* format = luaL_checkstring(L, 7);
        ret = ImGui::DragIntRange2(label, &v_current_min, &v_current_max,
            v_speed, v_min, v_max, format);
    }
    else if (argc == 8)
    {
        const float v_speed = (float)luaL_checknumber(L, 4);
        const int v_min = (int)luaL_checkinteger(L, 5);
        const int v_max = (int)luaL_checkinteger(L, 6);
        const char* format = luaL_checkstring(L, 7);
        const char* format_max = luaL_checkstring(L, 8);
        ret = ImGui::DragIntRange2(label, &v_current_min, &v_current_max,
            v_speed, v_min, v_max, format, format_max);
    }
    else
    {
        const float v_speed = (float)luaL_checknumber(L, 4);
        const int v_min = (int)luaL_checkinteger(L, 5);
        const int v_max = (int)luaL_checkinteger(L, 6);
        const char* format = luaL_checkstring(L, 7);
        const char* format_max = luaL_checkstring(L, 8);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 9);
        ret = ImGui::DragIntRange2(label, &v_current_min, &v_current_max,
            v_speed, v_min, v_max, format, format_max, flags);
    }
    lua_pushboolean(L, ret);
    lua_pushinteger(L, (lua_Integer)v_current_min);
    lua_pushinteger(L, (lua_Integer)v_current_max);
    return 3;
}
static int lib_DragScalar(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    const ImGuiDataType data_type = (ImGuiDataType)luaL_checkinteger(L, 2);
    bool ret = false;
    if (data_type == ImGuiDataType_Integer)
    {
        lua_Integer data = luaL_checkinteger(L, 3);
        if (argc <= 3)
        {
            ret = ImGui::DragScalar(label, data_type, &data);
        }
        else if (argc == 4)
        {
            const float v_speed = (float)luaL_checknumber(L, 4);
            ret = ImGui::DragScalar(label, data_type, &data, v_speed);
        }
        else if (argc == 5)
        {
            const float v_speed = (float)luaL_checknumber(L, 4);
            const lua_Integer min_ = luaL_checkinteger(L, 5);
            ret = ImGui::DragScalar(label, data_type, &data, v_speed, &min_);
        }
        else if (argc == 6)
        {
            const float v_speed = (float)luaL_checknumber(L, 4);
            const lua_Integer min_ = luaL_checkinteger(L, 5);
            const lua_Integer max_ = luaL_checkinteger(L, 6);
            ret = ImGui::DragScalar(label, data_type, &data, v_speed,
                &min_, &max_);
        }
        else if (argc == 7)
        {
            const float v_speed = (float)luaL_checknumber(L, 4);
            const lua_Integer min_ = luaL_checkinteger(L, 5);
            const lua_Integer max_ = luaL_checkinteger(L, 6);
            const char* format = luaL_checkstring(L, 7);
            ret = ImGui::DragScalar(label, data_type, &data, v_speed,
                &min_, &max_, format);
        }
        else
        {
            const float v_speed = (float)luaL_checknumber(L, 4);
            const lua_Integer min_ = luaL_checkinteger(L, 5);
            const lua_Integer max_ = luaL_checkinteger(L, 6);
            const char* format = luaL_checkstring(L, 7);
            const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 8);
            ret = ImGui::DragScalar(label, data_type, &data, v_speed,
                &min_, &max_, format, flags);
        }
        lua_pushboolean(L, ret);
        lua_pushinteger(L, data);
        return 2;
    }
    else if (data_type == ImGuiDataType_Number)
    {
        lua_Number data = luaL_checknumber(L, 3);
        if (argc <= 3)
        {
            ret = ImGui::DragScalar(label, data_type, &data, 1.0f);
        }
        else if (argc == 4)
        {
            const float v_speed = (float)luaL_checknumber(L, 4);
            ret = ImGui::DragScalar(label, data_type, &data, v_speed);
        }
        else if (argc == 5)
        {
            const float v_speed = (float)luaL_checknumber(L, 4);
            const lua_Number min_ = luaL_checknumber(L, 5);
            ret = ImGui::DragScalar(label, data_type, &data, v_speed, &min_);
        }
        else if (argc == 6)
        {
            const float v_speed = (float)luaL_checknumber(L, 4);
            const lua_Number min_ = luaL_checknumber(L, 5);
            const lua_Number max_ = luaL_checknumber(L, 6);
            ret = ImGui::DragScalar(label, data_type, &data, v_speed,
                &min_, &max_);
        }
        else if (argc == 7)
        {
            const float v_speed = (float)luaL_checknumber(L, 4);
            const lua_Number min_ = luaL_checknumber(L, 5);
            const lua_Number max_ = luaL_checknumber(L, 6);
            const char* format = luaL_checkstring(L, 7);
            ret = ImGui::DragScalar(label, data_type, &data, v_speed,
                &min_, &max_, format);
        }
        else
        {
            const float v_speed = (float)luaL_checknumber(L, 4);
            const lua_Number min_ = luaL_checknumber(L, 5);
            const lua_Number max_ = luaL_checknumber(L, 6);
            const char* format = luaL_checkstring(L, 7);
            const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 8);
            ret = ImGui::DragScalar(label, data_type, &data, v_speed,
                &min_, &max_, format, flags);
        }
        lua_pushboolean(L, ret);
        lua_pushnumber(L, data);
        return 2;
    }
    else
    {
        if (data_type >= 0 && data_type < (ImGuiDataType)ImGuiDataType_COUNT)
        {
            return luaL_error(L, R"(unsupported data type '%s')", imgui_binding_lua_ImGuiDataType_name[data_type]);
        }
        return luaL_error(L, R"(unsupported data type '?')");
    }
}
static int lib_DragScalarN(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    const ImGuiDataType data_type = (ImGuiDataType)luaL_checkinteger(L, 2);
    bool ret = false;
    if (data_type == ImGuiDataType_Integer)
    {
        const int components = (argc >= 4) ? (int)luaL_checkinteger(L, 4) : _imgui_binding_lua_len(L, 3);
        integer_array data(components);
        for (int i = 0; i < components; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_gettable(L, 3);
            data.data[i] = luaL_checkinteger(L, -1);
            lua_pop(L, 1);
        }
        if (argc <= 4)
        {
            ret = ImGui::DragScalarN(label, data_type, data.data, components);
        }
        else if (argc == 5)
        {
            const float v_speed = (float)luaL_checknumber(L, 5);
            ret = ImGui::DragScalarN(label, data_type, data.data, components, v_speed);
        }
        else if (argc == 6)
        {
            const float v_speed = (float)luaL_checknumber(L, 5);
            const lua_Integer min_ = luaL_checkinteger(L, 6);
            ret = ImGui::DragScalarN(label, data_type, data.data, components, v_speed, &min_);
        }
        else if (argc == 7)
        {
            const float v_speed = (float)luaL_checknumber(L, 5);
            const lua_Integer min_ = luaL_checkinteger(L, 6);
            const lua_Integer max_ = luaL_checkinteger(L, 7);
            ret = ImGui::DragScalarN(label, data_type, data.data, components,
                v_speed, &min_, &max_);
        }
        else if (argc == 8)
        {
            const float v_speed = (float)luaL_checknumber(L, 5);
            const lua_Integer min_ = luaL_checkinteger(L, 6);
            const lua_Integer max_ = luaL_checkinteger(L, 7);
            const char* format = luaL_checkstring(L, 8);
            ret = ImGui::DragScalarN(label, data_type, data.data, components,
                v_speed, &min_, &max_, format);
        }
        else
        {
            const float v_speed = (float)luaL_checknumber(L, 5);
            const lua_Integer min_ = luaL_checkinteger(L, 6);
            const lua_Integer max_ = luaL_checkinteger(L, 7);
            const char* format = luaL_checkstring(L, 8);
            const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 9);
            ret = ImGui::DragScalarN(label, data_type, data.data, components,
                v_speed, &min_, &max_, format, flags);
        }
        for (int i = 0; i < components; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_pushinteger(L, data.data[i]);
            lua_settable(L, 3);
        }
        lua_pushboolean(L, ret);
        lua_pushvalue(L, 3);
        return 2;
    }
    else if (data_type == ImGuiDataType_Number)
    {
        const int components = (argc >= 4) ? (int)luaL_checkinteger(L, 4) : _imgui_binding_lua_len(L, 3);
        number_array data(components);
        for (int i = 0; i < components; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_gettable(L, 3);
            data.data[i] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        if (argc <= 3)
        {
            ret = ImGui::DragScalarN(label, data_type, data.data, components, 1.0f); // helper
        }
        else if (argc == 4)
        {
            ret = ImGui::DragScalarN(label, data_type, data.data, components, 1.0f); // helper
        }
        else if (argc == 5)
        {
            const float v_speed = (float)luaL_checknumber(L, 5);
            ret = ImGui::DragScalarN(label, data_type, data.data, components, v_speed);
        }
        else if (argc == 6)
        {
            const float v_speed = (float)luaL_checknumber(L, 5);
            const lua_Number min_ = luaL_checknumber(L, 6);
            ret = ImGui::DragScalarN(label, data_type, data.data, components, v_speed, &min_);
        }
        else if (argc == 7)
        {
            const float v_speed = (float)luaL_checknumber(L, 5);
            const lua_Number min_ = luaL_checknumber(L, 6);
            const lua_Number max_ = luaL_checknumber(L, 7);
            ret = ImGui::DragScalarN(label, data_type, data.data, components,
                v_speed, &min_, &max_);
        }
        else if (argc == 8)
        {
            const float v_speed = (float)luaL_checknumber(L, 5);
            const lua_Number min_ = luaL_checknumber(L, 6);
            const lua_Number max_ = luaL_checknumber(L, 7);
            const char* format = luaL_checkstring(L, 8);
            ret = ImGui::DragScalarN(label, data_type, data.data, components,
                v_speed, &min_, &max_, format);
        }
        else
        {
            const float v_speed = (float)luaL_checknumber(L, 5);
            const lua_Number min_ = luaL_checknumber(L, 6);
            const lua_Number max_ = luaL_checknumber(L, 7);
            const char* format = luaL_checkstring(L, 8);
            const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 9);
            ret = ImGui::DragScalarN(label, data_type, data.data, components,
                v_speed, &min_, &max_, format, flags);
        }
        for (int i = 0; i < components; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_pushnumber(L, data.data[i]);
            lua_settable(L, 3);
        }
        lua_pushboolean(L, ret);
        lua_pushvalue(L, 3);
        return 2;
    }
    else
    {
        if (data_type >= 0 && data_type < (ImGuiDataType)ImGuiDataType_COUNT)
        {
            return luaL_error(L, R"(unsupported data type '%s')", imgui_binding_lua_ImGuiDataType_name[data_type]);
        }
        return luaL_error(L, R"(unsupported data type '?')");
    }
}
