// ImGui::InputX

static /* !!!! */ int lib_InputText(lua_State* L)
{
    const char* label = luaL_checkstring(L, 1);
    ImGuiTextBuffer* buf = imgui_binding_lua_to_ImGuiTextBuffer(L, 2);
    const size_t buf_size = (size_t)luaL_checkinteger(L, 3);
    const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_optinteger(L, 4, 0);
    const bool r = ImGui::InputText(label, buf->Buf.Data, buf_size, flags);
    lua_pushboolean(L, r);
    return 1;
}
static /* !!!! */ int lib_InputTextMultiline(lua_State* L)
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
static /* !!!! */ int lib_InputTextWithHint(lua_State* L)
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
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float v = (float)luaL_checknumber(L, 2);
    bool ret = false;
    if (argc <= 2)
    {
        ret = ImGui::InputFloat(label, &v);
    }
    else if (argc == 3)
    {
        const float step = (float)luaL_checknumber(L, 3);
        ret = ImGui::InputFloat(label, &v, step);
    }
    else if (argc == 4)
    {
        const float step = (float)luaL_checknumber(L, 3);
        const float step_fast = (float)luaL_checknumber(L, 4);
        ret = ImGui::InputFloat(label, &v, step, step_fast);
    }
    else if (argc == 5)
    {
        const float step = (float)luaL_checknumber(L, 3);
        const float step_fast = (float)luaL_checknumber(L, 4);
        const char* format = luaL_checkstring(L, 5);
        ret = ImGui::InputFloat(label, &v, step, step_fast, format);
    }
    else
    {
        const float step = (float)luaL_checknumber(L, 3);
        const float step_fast = (float)luaL_checknumber(L, 4);
        const char* format = luaL_checkstring(L, 5);
        const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 6);
        ret = ImGui::InputFloat(label, &v, step, step_fast, format, flags);
    }
    lua_pushboolean(L, ret);
    lua_pushnumber(L, (lua_Number)v);
    return 2;
}
static int lib_InputFloat2(lua_State* L)
{
#define N 2
#define F ImGui::InputFloat2
    
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
        const char* format = luaL_checkstring(L, 3);
        ret = F(label, v, format);
    }
    else
    {
        const char* format = luaL_checkstring(L, 3);
        const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 4);
        ret = F(label, v, format, flags);
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
static int lib_InputFloat3(lua_State* L)
{
#define N 3
#define F ImGui::InputFloat3
    
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
        const char* format = luaL_checkstring(L, 3);
        ret = F(label, v, format);
    }
    else
    {
        const char* format = luaL_checkstring(L, 3);
        const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 4);
        ret = F(label, v, format, flags);
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
static int lib_InputFloat4(lua_State* L)
{
#define N 4
#define F ImGui::InputFloat4
    
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
        const char* format = luaL_checkstring(L, 3);
        ret = F(label, v, format);
    }
    else
    {
        const char* format = luaL_checkstring(L, 3);
        const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 4);
        ret = F(label, v, format, flags);
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
static int lib_InputInt(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    int v = (int)luaL_checkinteger(L, 2);
    bool ret = false;
    if (argc <= 2)
    {
        ret = ImGui::InputInt(label, &v);
    }
    else if (argc == 3)
    {
        const int step = (int)luaL_checkinteger(L, 3);
        ret = ImGui::InputInt(label, &v, step);
    }
    else if (argc == 4)
    {
        const int step = (int)luaL_checkinteger(L, 3);
        const int step_fast = (int)luaL_checkinteger(L, 4);
        ret = ImGui::InputInt(label, &v, step, step_fast);
    }
    else
    {
        const int step = (int)luaL_checkinteger(L, 3);
        const int step_fast = (int)luaL_checkinteger(L, 4);
        const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 5);
        ret = ImGui::InputInt(label, &v, step, step_fast, flags);
    }
    lua_pushboolean(L, ret);
    lua_pushinteger(L, (lua_Integer)v);
    return 2;
}
static int lib_InputInt2(lua_State* L)
{
#define N 2
#define F ImGui::InputInt2

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
    else
    {
        const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 3);
        ret = F(label, v, flags);
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
static int lib_InputInt3(lua_State* L)
{
#define N 3
#define F ImGui::InputInt3

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
    else
    {
        const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 3);
        ret = F(label, v, flags);
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
static int lib_InputInt4(lua_State* L)
{
#define N 4
#define F ImGui::InputInt4

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
    else
    {
        const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 3);
        ret = F(label, v, flags);
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
static int lib_InputDouble(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    double v = (double)luaL_checknumber(L, 2);
    bool ret = false;
    if (argc <= 2)
    {
        ret = ImGui::InputDouble(label, &v);
    }
    else if (argc == 3)
    {
        const double step = (double)luaL_checknumber(L, 3);
        ret = ImGui::InputDouble(label, &v, step);
    }
    else if (argc == 4)
    {
        const double step = (double)luaL_checknumber(L, 3);
        const double step_fast = (double)luaL_checknumber(L, 4);
        ret = ImGui::InputDouble(label, &v, step, step_fast);
    }
    else if (argc == 5)
    {
        const double step = (double)luaL_checknumber(L, 3);
        const double step_fast = (double)luaL_checknumber(L, 4);
        const char* format = luaL_checkstring(L, 5);
        ret = ImGui::InputDouble(label, &v, step, step_fast, format);
    }
    else
    {
        const double step = (double)luaL_checknumber(L, 3);
        const double step_fast = (double)luaL_checknumber(L, 4);
        const char* format = luaL_checkstring(L, 5);
        const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 6);
        ret = ImGui::InputDouble(label, &v, step, step_fast, format, flags);
    }
    lua_pushboolean(L, ret);
    lua_pushnumber(L, (lua_Number)v);
    return 2;
}
static int lib_InputScalar(lua_State* L)
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
            ret = ImGui::InputScalar(label, data_type, &data);
        }
        else if (argc == 4)
        {
            const lua_Integer step = luaL_checkinteger(L, 4);
            ret = ImGui::InputScalar(label, data_type, &data, &step);
        }
        else if (argc == 5)
        {
            const lua_Integer step = luaL_checkinteger(L, 4);
            const lua_Integer step_fast = luaL_checkinteger(L, 5);
            ret = ImGui::InputScalar(label, data_type, &data,
                &step, &step_fast);
        }
        else if (argc == 6)
        {
            const lua_Integer step = luaL_checkinteger(L, 4);
            const lua_Integer step_fast = luaL_checkinteger(L, 5);
            const char* format = luaL_checkstring(L, 6);
            ret = ImGui::InputScalar(label, data_type, &data,
                &step, &step_fast, format);
        }
        else
        {
            const lua_Integer step = luaL_checkinteger(L, 4);
            const lua_Integer step_fast = luaL_checkinteger(L, 5);
            const char* format = luaL_checkstring(L, 6);
            const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 7);
            ret = ImGui::InputScalar(label, data_type, &data,
                &step, &step_fast, format, flags);
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
            ret = ImGui::InputScalar(label, data_type, &data);
        }
        else if (argc == 4)
        {
            const lua_Number step = luaL_checknumber(L, 4);
            ret = ImGui::InputScalar(label, data_type, &data, &step);
        }
        else if (argc == 5)
        {
            const lua_Number step = luaL_checknumber(L, 4);
            const lua_Number step_fast = luaL_checknumber(L, 5);
            ret = ImGui::InputScalar(label, data_type, &data,
                &step, &step_fast);
        }
        else if (argc == 6)
        {
            const lua_Number step = luaL_checknumber(L, 4);
            const lua_Number step_fast = luaL_checknumber(L, 5);
            const char* format = luaL_checkstring(L, 6);
            ret = ImGui::InputScalar(label, data_type, &data,
                &step, &step_fast, format);
        }
        else
        {
            const lua_Number step = luaL_checknumber(L, 4);
            const lua_Number step_fast = luaL_checknumber(L, 5);
            const char* format = luaL_checkstring(L, 6);
            const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 7);
            ret = ImGui::InputScalar(label, data_type, &data,
                &step, &step_fast, format, flags);
        }
        lua_pushboolean(L, ret);
        lua_pushnumber(L, data);
        return 2;
    }
    else
    {
        if (data_type >= 0 && data_type < (ImGuiDataType)ImGuiDataType_COUNT)
        {
            return luaL_error(L, R"(unsupported data type '%s')", ImGuiDataTypeName[data_type]);
        }
        return luaL_error(L, R"(unsupported data type '?')");
    }
}
static int lib_InputScalarN(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    const ImGuiDataType data_type = (ImGuiDataType)luaL_checkinteger(L, 2);
    bool ret = false;
    if (data_type == ImGuiDataType_Integer)
    {
        const int components = (int)luaL_checkinteger(L, 4);
        integer_array data(components);

        for (int i = 0; i < components; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_gettable(L, 3);
            data[i] = luaL_checkinteger(L, -1);
            lua_pop(L, 1);
        }

        if (argc <= 4)
        {
            ret = ImGui::InputScalarN(label, data_type, *data, components);
        }
        else if (argc == 5)
        {
            const lua_Integer step = luaL_checkinteger(L, 5);
            ret = ImGui::InputScalarN(label, data_type, *data, components, &step);
        }
        else if (argc == 6)
        {
            const lua_Integer step = luaL_checkinteger(L, 5);
            const lua_Integer step_fast = luaL_checkinteger(L, 6);
            ret = ImGui::InputScalarN(label, data_type, *data, components,
                &step, &step_fast);
        }
        else if (argc == 7)
        {
            const lua_Integer step = luaL_checkinteger(L, 5);
            const lua_Integer step_fast = luaL_checkinteger(L, 6);
            const char* format = luaL_checkstring(L, 7);
            ret = ImGui::InputScalarN(label, data_type, *data, components,
                &step, &step_fast, format);
        }
        else
        {
            const lua_Integer step = luaL_checkinteger(L, 5);
            const lua_Integer step_fast = luaL_checkinteger(L, 6);
            const char* format = luaL_checkstring(L, 7);
            const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 8);
            ret = ImGui::InputScalarN(label, data_type, *data, components,
                &step, &step_fast, format, flags);
        }

        for (int i = 0; i < components; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_pushinteger(L, data[i]);
            lua_settable(L, 3);
        }

        lua_pushboolean(L, ret);
        lua_pushvalue(L, 3);
        return 2;
    }
    else if (data_type == ImGuiDataType_Number)
    {
        const int components = (int)luaL_checkinteger(L, 4);
        number_array data(components);

        for (int i = 0; i < components; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_gettable(L, 3);
            data[i] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }

        if (argc <= 4)
        {
            ret = ImGui::InputScalarN(label, data_type, &data, components);
        }
        else if (argc == 5)
        {
            const lua_Number step = luaL_checknumber(L, 5);
            ret = ImGui::InputScalarN(label, data_type, &data, components, &step);
        }
        else if (argc == 6)
        {
            const lua_Number step = luaL_checknumber(L, 5);
            const lua_Number step_fast = luaL_checknumber(L, 6);
            ret = ImGui::InputScalarN(label, data_type, &data, components,
                &step, &step_fast);
        }
        else if (argc == 7)
        {
            const lua_Number step = luaL_checknumber(L, 5);
            const lua_Number step_fast = luaL_checknumber(L, 6);
            const char* format = luaL_checkstring(L, 7);
            ret = ImGui::InputScalarN(label, data_type, &data, components,
                &step, &step_fast, format);
        }
        else
        {
            const lua_Number step = luaL_checknumber(L, 5);
            const lua_Number step_fast = luaL_checknumber(L, 6);
            const char* format = luaL_checkstring(L, 7);
            const ImGuiInputTextFlags flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 8);
            ret = ImGui::InputScalarN(label, data_type, &data, components,
                &step, &step_fast, format, flags);
        }

        for (int i = 0; i < components; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_pushnumber(L, data[i]);
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
            return luaL_error(L, R"(unsupported data type '%s')", ImGuiDataTypeName[data_type]);
        }
        return luaL_error(L, R"(unsupported data type '?')");
    }
}
