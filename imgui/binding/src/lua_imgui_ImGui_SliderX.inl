// ImGui::SliderX

static int lib_SliderFloat(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float v = (float)luaL_checknumber(L, 2);
    const float v_min = (float)luaL_checknumber(L, 3);
    const float v_max = (float)luaL_checknumber(L, 4);
    bool ret = false;
    if (argc <= 4)
    {
        ret = ImGui::SliderFloat(label, &v, v_min, v_max);
    }
    else if (argc == 5)
    {
        const char* format = luaL_checkstring(L, 5);
        ret = ImGui::SliderFloat(label, &v, v_min, v_max, format);
    }
    else
    {
        const char* format = luaL_checkstring(L, 5);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 6);
        ret = ImGui::SliderFloat(label, &v, v_min, v_max, format, flags);
    }
    lua_pushboolean(L, ret);
    lua_pushnumber(L, (lua_Number)v);
    return 2;
}
static int lib_SliderFloat2(lua_State* L)
{
#define N 2
#define F ImGui::SliderFloat2
    
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float v[N] = {};
    const float v_min = (float)luaL_checknumber(L, 3);
    const float v_max = (float)luaL_checknumber(L, 4);
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        v[idx] = (float)luaL_checknumber(L, argc + 1);
        lua_pop(L, 1);
    }

    if (argc <= 4)
    {
        ret = F(label, v, v_min, v_max);
    }
    else if (argc == 5)
    {
        const char* format = luaL_checkstring(L, 5);
        ret = F(label, v, v_min, v_max, format);
    }
    else
    {
        const char* format = luaL_checkstring(L, 5);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 6);
        ret = F(label, v, v_min, v_max, format, flags);
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
static int lib_SliderFloat3(lua_State* L)
{
#define N 3
#define F ImGui::SliderFloat3
    
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float v[N] = {};
    const float v_min = (float)luaL_checknumber(L, 3);
    const float v_max = (float)luaL_checknumber(L, 4);
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        v[idx] = (float)luaL_checknumber(L, argc + 1);
        lua_pop(L, 1);
    }

    if (argc <= 4)
    {
        ret = F(label, v, v_min, v_max);
    }
    else if (argc == 5)
    {
        const char* format = luaL_checkstring(L, 5);
        ret = F(label, v, v_min, v_max, format);
    }
    else
    {
        const char* format = luaL_checkstring(L, 5);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 6);
        ret = F(label, v, v_min, v_max, format, flags);
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
static int lib_SliderFloat4(lua_State* L)
{
#define N 4
#define F ImGui::SliderFloat4
    
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float v[N] = {};
    const float v_min = (float)luaL_checknumber(L, 3);
    const float v_max = (float)luaL_checknumber(L, 4);
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        v[idx] = (float)luaL_checknumber(L, argc + 1);
        lua_pop(L, 1);
    }

    if (argc <= 4)
    {
        ret = F(label, v, v_min, v_max);
    }
    else if (argc == 5)
    {
        const char* format = luaL_checkstring(L, 5);
        ret = F(label, v, v_min, v_max, format);
    }
    else
    {
        const char* format = luaL_checkstring(L, 5);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 6);
        ret = F(label, v, v_min, v_max, format, flags);
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
static int lib_SliderAngle(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float v_rad = (float)luaL_checknumber(L, 2);
    bool ret = false;
    if (argc <= 2)
    {
        ret = ImGui::SliderAngle(label, &v_rad);
    }
    else if (argc == 3)
    {
        const float v_degrees_min = (float)luaL_checknumber(L, 3);
        ret = ImGui::SliderAngle(label, &v_rad, v_degrees_min);
    }
    else if (argc == 4)
    {
        const float v_degrees_min = (float)luaL_checknumber(L, 3);
        const float v_degrees_max = (float)luaL_checknumber(L, 4);
        ret = ImGui::SliderAngle(label, &v_rad,
            v_degrees_min, v_degrees_max);
    }
    else if (argc == 5)
    {
        const float v_degrees_min = (float)luaL_checknumber(L, 3);
        const float v_degrees_max = (float)luaL_checknumber(L, 4);
        const char* format = luaL_checkstring(L, 5);
        ret = ImGui::SliderAngle(label, &v_rad,
            v_degrees_min, v_degrees_max, format);
    }
    else
    {
        const float v_degrees_min = (float)luaL_checknumber(L, 3);
        const float v_degrees_max = (float)luaL_checknumber(L, 4);
        const char* format = luaL_checkstring(L, 5);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 6);
        ret = ImGui::SliderAngle(label, &v_rad,
            v_degrees_min, v_degrees_max, format, flags);
    }
    lua_pushboolean(L, ret);
    lua_pushnumber(L, (lua_Number)v_rad);
    return 2;
}
static int lib_SliderInt(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    int v = (int)luaL_checkinteger(L, 2);
    const int v_min = (float)luaL_checkinteger(L, 3);
    const int v_max = (float)luaL_checkinteger(L, 4);
    bool ret = false;
    if (argc <= 4)
    {
        ret = ImGui::SliderInt(label, &v, v_min, v_max);
    }
    else if (argc == 5)
    {
        const char* format = luaL_checkstring(L, 5);
        ret = ImGui::SliderInt(label, &v, v_min, v_max, format);
    }
    else
    {
        const char* format = luaL_checkstring(L, 5);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 6);
        ret = ImGui::SliderInt(label, &v, v_min, v_max, format, flags);
    }
    lua_pushboolean(L, ret);
    lua_pushinteger(L, (lua_Integer)v);
    return 2;
}
static int lib_SliderInt2(lua_State* L)
{
#define N 2
#define F ImGui::SliderInt2
    
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    int v[N] = {};
    const int v_min = (float)luaL_checkinteger(L, 3);
    const int v_max = (float)luaL_checkinteger(L, 4);
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        v[idx] = (int)luaL_checkinteger(L, argc + 1);
        lua_pop(L, 1);
    }

    if (argc <= 4)
    {
        ret = F(label, v, v_min, v_max);
    }
    else if (argc == 5)
    {
        const char* format = luaL_checkstring(L, 5);
        ret = F(label, v, v_min, v_max, format);
    }
    else
    {
        const char* format = luaL_checkstring(L, 5);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 6);
        ret = F(label, v, v_min, v_max, format, flags);
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
static int lib_SliderInt3(lua_State* L)
{
#define N 3
#define F ImGui::SliderInt3
    
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    int v[N] = {};
    const int v_min = (float)luaL_checkinteger(L, 3);
    const int v_max = (float)luaL_checkinteger(L, 4);
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        v[idx] = (int)luaL_checkinteger(L, argc + 1);
        lua_pop(L, 1);
    }

    if (argc <= 4)
    {
        ret = F(label, v, v_min, v_max);
    }
    else if (argc == 5)
    {
        const char* format = luaL_checkstring(L, 5);
        ret = F(label, v, v_min, v_max, format);
    }
    else
    {
        const char* format = luaL_checkstring(L, 5);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 6);
        ret = F(label, v, v_min, v_max, format, flags);
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
static int lib_SliderInt4(lua_State* L)
{
#define N 4
#define F ImGui::SliderInt4
    
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    int v[N] = {};
    const int v_min = (float)luaL_checkinteger(L, 3);
    const int v_max = (float)luaL_checkinteger(L, 4);
    bool ret = false;

    for (int idx = 0; idx < N; idx += 1)
    {
        lua_pushinteger(L, idx + 1);
        lua_gettable(L, 2);
        v[idx] = (int)luaL_checkinteger(L, argc + 1);
        lua_pop(L, 1);
    }

    if (argc <= 4)
    {
        ret = F(label, v, v_min, v_max);
    }
    else if (argc == 5)
    {
        const char* format = luaL_checkstring(L, 5);
        ret = F(label, v, v_min, v_max, format);
    }
    else
    {
        const char* format = luaL_checkstring(L, 5);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 6);
        ret = F(label, v, v_min, v_max, format, flags);
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
static int lib_SliderScalar(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    const ImGuiDataType data_type = (ImGuiDataType)luaL_checkinteger(L, 2);
    bool ret = false;
    if (data_type == ImGuiDataType_Integer)
    {
        lua_Integer data = luaL_checkinteger(L, 3);
        const lua_Integer min_ = luaL_checkinteger(L, 4);
        const lua_Integer max_ = luaL_checkinteger(L, 5);
        if (argc <= 5)
        {
            ret = ImGui::SliderScalar(label, data_type, &data, &min_, &max_);
        }
        else if (argc == 6)
        {
            const char* format = luaL_checkstring(L, 6);
            ret = ImGui::SliderScalar(label, data_type, &data,
                &min_, &max_, format);
        }
        else
        {
            const char* format = luaL_checkstring(L, 6);
            const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 7);
            ret = ImGui::SliderScalar(label, data_type, &data,
                &min_, &max_, format, flags);
        }
        lua_pushboolean(L, ret);
        lua_pushinteger(L, data);
        return 2;
    }
    else if (data_type == ImGuiDataType_Number)
    {
        lua_Number data = luaL_checknumber(L, 3);
        const lua_Number min_ = luaL_checknumber(L, 4);
        const lua_Number max_ = luaL_checknumber(L, 5);
        if (argc <= 5)
        {
            ret = ImGui::SliderScalar(label, data_type, &data, &min_, &max_);
        }
        else if (argc == 6)
        {
            const char* format = luaL_checkstring(L, 6);
            ret = ImGui::SliderScalar(label, data_type, &data,
                &min_, &max_, format);
        }
        else
        {
            const char* format = luaL_checkstring(L, 6);
            const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 7);
            ret = ImGui::SliderScalar(label, data_type, &data,
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
            return luaL_error(L, R"(unsupported data type '%s')", ImGuiDataTypeName[data_type]);
        }
        return luaL_error(L, R"(unsupported data type '?')");
    }
}
static int lib_SliderScalarN(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    const ImGuiDataType data_type = (ImGuiDataType)luaL_checkinteger(L, 2);
    bool ret = false;
    if (data_type == ImGuiDataType_Integer)
    {
        const int components = (int)luaL_checkinteger(L, 4);
        integer_array v(components);
        const lua_Integer min_ = luaL_checkinteger(L, 5);
        const lua_Integer max_ = luaL_checkinteger(L, 6);
        
        for (int i = 0; i < components; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_gettable(L, 3);
            v[i] = luaL_checkinteger(L, -1);
            lua_pop(L, 1);
        }

        if (argc <= 6)
        {
            ret = ImGui::SliderScalarN(label, data_type, *v, components, &min_, &max_);
        }
        else if (argc == 7)
        {
            const char* format = luaL_checkstring(L, 7);
            ret = ImGui::SliderScalarN(label, data_type, *v, components,
                &min_, &max_, format);
        }
        else
        {
            const char* format = luaL_checkstring(L, 7);
            const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 8);
            ret = ImGui::SliderScalarN(label, data_type, *v, components,
                &min_, &max_, format, flags);
        }

        for (int i = 0; i < components; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_pushinteger(L, v[i]);
            lua_settable(L, 3);
        }

        lua_pushboolean(L, ret);
        lua_pushvalue(L, 3);
        return 2;
    }
    else if (data_type == ImGuiDataType_Number)
    {
        const int components = (int)luaL_checkinteger(L, 4);
        number_array v(components);
        const lua_Number min_ = luaL_checknumber(L, 5);
        const lua_Number max_ = luaL_checknumber(L, 6);

        for (int i = 0; i < components; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_gettable(L, 3);
            v[i] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }

        if (argc <= 6)
        {
            ret = ImGui::SliderScalarN(label, data_type, *v, components, &min_, &max_);
        }
        else if (argc == 7)
        {
            const char* format = luaL_checkstring(L, 7);
            ret = ImGui::SliderScalarN(label, data_type, *v, components,
                &min_, &max_, format);
        }
        else
        {
            const char* format = luaL_checkstring(L, 7);
            const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 8);
            ret = ImGui::SliderScalarN(label, data_type, *v, components,
                &min_, &max_, format, flags);
        }

        for (int i = 0; i < components; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_pushnumber(L, v[i]);
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
static int lib_VSliderFloat(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
    float v = (float)luaL_checknumber(L, 3);
    const float v_min = (float)luaL_checknumber(L, 4);
    const float v_max = (float)luaL_checknumber(L, 5);
    bool ret = false;
    if (argc <= 5)
    {
        ret = ImGui::VSliderFloat(label, *size, &v, v_min, v_max);
    }
    else if (argc == 6)
    {
        const char* format = luaL_checkstring(L, 6);
        ret = ImGui::VSliderFloat(label, *size, &v, v_min, v_max, format);
    }
    else
    {
        const char* format = luaL_checkstring(L, 6);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 7);
        ret = ImGui::VSliderFloat(label, *size, &v, v_min, v_max, format, flags);
    }
    lua_pushboolean(L, ret);
    lua_pushnumber(L, (lua_Number)v);
    return 2;
}
static int lib_VSliderInt(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
    int v = (int)luaL_checkinteger(L, 3);
    const int v_min = (float)luaL_checkinteger(L, 4);
    const int v_max = (float)luaL_checkinteger(L, 5);
    bool ret = false;
    if (argc <= 5)
    {
        ret = ImGui::VSliderInt(label, *size, &v, v_min, v_max);
    }
    else if (argc == 6)
    {
        const char* format = luaL_checkstring(L, 6);
        ret = ImGui::VSliderInt(label, *size, &v, v_min, v_max, format);
    }
    else
    {
        const char* format = luaL_checkstring(L, 6);
        const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 7);
        ret = ImGui::VSliderInt(label, *size, &v, v_min, v_max, format, flags);
    }
    lua_pushboolean(L, ret);
    lua_pushinteger(L, (lua_Integer)v);
    return 2;
}
static int lib_VSliderScalar(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    ImVec2* size = imgui_binding_lua_to_ImVec2(L, 2);
    const ImGuiDataType data_type = (ImGuiDataType)luaL_checkinteger(L, 3);
    bool ret = false;
    if (data_type == ImGuiDataType_Integer)
    {
        lua_Integer data = luaL_checkinteger(L, 4);
        const lua_Integer min_ = luaL_checkinteger(L, 5);
        const lua_Integer max_ = luaL_checkinteger(L, 6);
        if (argc <= 6)
        {
            ret = ImGui::SliderScalar(label, data_type, &data, &min_, &max_);
        }
        else if (argc == 6)
        {
            const char* format = luaL_checkstring(L, 7);
            ret = ImGui::SliderScalar(label, data_type, &data,
                &min_, &max_, format);
        }
        else
        {
            const char* format = luaL_checkstring(L, 7);
            const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 8);
            ret = ImGui::SliderScalar(label, data_type, &data,
                &min_, &max_, format, flags);
        }
        lua_pushboolean(L, ret);
        lua_pushinteger(L, data);
        return 2;
    }
    else if (data_type == ImGuiDataType_Number)
    {
        lua_Number data = luaL_checknumber(L, 4);
        const lua_Number min_ = luaL_checknumber(L, 5);
        const lua_Number max_ = luaL_checknumber(L, 6);
        if (argc <= 6)
        {
            ret = ImGui::SliderScalar(label, data_type, &data, &min_, &max_);
        }
        else if (argc == 7)
        {
            const char* format = luaL_checkstring(L, 7);
            ret = ImGui::SliderScalar(label, data_type, &data,
                &min_, &max_, format);
        }
        else
        {
            const char* format = luaL_checkstring(L, 7);
            const ImGuiSliderFlags flags = (ImGuiSliderFlags)luaL_checkinteger(L, 8);
            ret = ImGui::SliderScalar(label, data_type, &data,
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
            return luaL_error(L, R"(unsupported data type '%s')", ImGuiDataTypeName[data_type]);
        }
        return luaL_error(L, R"(unsupported data type '?')");
    }
}
