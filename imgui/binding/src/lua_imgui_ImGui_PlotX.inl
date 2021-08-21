// ImGui::PlotX

static int lib_PlotLines(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    if (lua_isfunction(L, 2))
    {
        const int values_count = (int)luaL_checkinteger(L, 3);
        struct Wrapper
        {
            static float Getter(void* data, int idx)
            {
                lua_State* L = (lua_State*)data;
                lua_pushvalue(L, 2);
                lua_pushinteger(L, idx + 1);
                lua_call(L, 1, 1);
                if (lua_isnumber(L, -1))
                {
                    const float ret = (float)lua_tonumber(L, -1);
                    lua_pop(L, 1);
                    return ret;
                }
                else
                {
                    lua_pop(L, 1);
                    return 0.0f;
                }
            }
        };
        if (argc <= 3)
        {
            ImGui::PlotLines(label, &Wrapper::Getter, L, values_count);
        }
        else if (argc == 4)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            ImGui::PlotLines(label, &Wrapper::Getter, L, values_count, values_offset);
        }
        else if (argc == 5)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            ImGui::PlotLines(label, &Wrapper::Getter, L, values_count, values_offset, overlay_text);
        }
        else if (argc == 6)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            const float scale_min = (float)luaL_checknumber(L, 6);
            ImGui::PlotLines(label, &Wrapper::Getter, L, values_count, values_offset, overlay_text, scale_min);
        }
        else if (argc == 7)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            const float scale_min = (float)luaL_checknumber(L, 6);
            const float scale_max = (float)luaL_checknumber(L, 7);
            ImGui::PlotLines(label, &Wrapper::Getter, L, values_count, values_offset,
                overlay_text, scale_min, scale_max);
        }
        else
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            const float scale_min = (float)luaL_checknumber(L, 6);
            const float scale_max = (float)luaL_checknumber(L, 7);
            ImVec2* graph_size = imgui_binding_lua_to_ImVec2(L, 8);
            ImGui::PlotLines(label, &Wrapper::Getter, L, values_count, values_offset,
                overlay_text, scale_min, scale_max, *graph_size);
        }
    }
    else
    {
        const int values_count = (argc >= 3) ? (int)luaL_checkinteger(L, 3) : _luaL_len(L, 2);
        float_array values(values_count);
        for (int i = 0; i < values_count; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_gettable(L, 2);
            values.data[i] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        if (argc <= 3)
        {
            ImGui::PlotLines(label, values.data, values_count);
        }
        else if (argc == 4)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            ImGui::PlotLines(label, values.data, values_count, values_offset);
        }
        else if (argc == 5)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            ImGui::PlotLines(label, values.data, values_count, values_offset, overlay_text);
        }
        else if (argc == 6)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            const float scale_min = (float)luaL_checknumber(L, 6);
            ImGui::PlotLines(label, values.data, values_count, values_offset, overlay_text, scale_min);
        }
        else if (argc == 7)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            const float scale_min = (float)luaL_checknumber(L, 6);
            const float scale_max = (float)luaL_checknumber(L, 7);
            ImGui::PlotLines(label, values.data, values_count, values_offset,
                overlay_text, scale_min, scale_max);
        }
        else if (argc == 8)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            const float scale_min = (float)luaL_checknumber(L, 6);
            const float scale_max = (float)luaL_checknumber(L, 7);
            ImVec2* graph_size = imgui_binding_lua_to_ImVec2(L, 8);
            ImGui::PlotLines(label, values.data, values_count, values_offset,
                overlay_text, scale_min, scale_max, *graph_size);
        }
        else
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            const float scale_min = (float)luaL_checknumber(L, 6);
            const float scale_max = (float)luaL_checknumber(L, 7);
            ImVec2* graph_size = imgui_binding_lua_to_ImVec2(L, 8);
            const int stride = (int)luaL_checkinteger(L, 9);
            ImGui::PlotLines(label, values.data, values_count, values_offset,
                overlay_text, scale_min, scale_max, *graph_size, stride * sizeof(float));
        }
    }
    return 0;
}
static int lib_PlotHistogram(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    if (lua_isfunction(L, 2))
    {
        const int values_count = (int)luaL_checkinteger(L, 3);
        struct Wrapper
        {
            static float Getter(void* data, int idx)
            {
                lua_State* L = (lua_State*)data;
                lua_pushvalue(L, 2);
                lua_pushinteger(L, idx + 1);
                lua_call(L, 1, 1);
                if (lua_isnumber(L, -1))
                {
                    const float ret = (float)lua_tonumber(L, -1);
                    lua_pop(L, 1);
                    return ret;
                }
                else
                {
                    lua_pop(L, 1);
                    return 0.0f;
                }
            }
        };
        if (argc <= 3)
        {
            ImGui::PlotHistogram(label, &Wrapper::Getter, L, values_count);
        }
        else if (argc == 4)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            ImGui::PlotHistogram(label, &Wrapper::Getter, L, values_count, values_offset);
        }
        else if (argc == 5)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            ImGui::PlotHistogram(label, &Wrapper::Getter, L, values_count, values_offset, overlay_text);
        }
        else if (argc == 6)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            const float scale_min = (float)luaL_checknumber(L, 6);
            ImGui::PlotHistogram(label, &Wrapper::Getter, L, values_count, values_offset, overlay_text, scale_min);
        }
        else if (argc == 7)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            const float scale_min = (float)luaL_checknumber(L, 6);
            const float scale_max = (float)luaL_checknumber(L, 7);
            ImGui::PlotHistogram(label, &Wrapper::Getter, L, values_count, values_offset,
                overlay_text, scale_min, scale_max);
        }
        else
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            const float scale_min = (float)luaL_checknumber(L, 6);
            const float scale_max = (float)luaL_checknumber(L, 7);
            ImVec2* graph_size = imgui_binding_lua_to_ImVec2(L, 8);
            ImGui::PlotHistogram(label, &Wrapper::Getter, L, values_count, values_offset,
                overlay_text, scale_min, scale_max, *graph_size);
        }
    }
    else
    {
        const int values_count = (argc >= 3) ? (int)luaL_checkinteger(L, 3) : _luaL_len(L, 2);
        float_array values(values_count);
        for (int i = 0; i < values_count; i += 1)
        {
            lua_pushinteger(L, i + 1);
            lua_gettable(L, 2);
            values.data[i] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        if (argc <= 3)
        {
            ImGui::PlotHistogram(label, values.data, values_count);
        }
        else if (argc == 4)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            ImGui::PlotHistogram(label, values.data, values_count, values_offset);
        }
        else if (argc == 5)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            ImGui::PlotHistogram(label, values.data, values_count, values_offset, overlay_text);
        }
        else if (argc == 6)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            const float scale_min = (float)luaL_checknumber(L, 6);
            ImGui::PlotHistogram(label, values.data, values_count, values_offset, overlay_text, scale_min);
        }
        else if (argc == 7)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            const float scale_min = (float)luaL_checknumber(L, 6);
            const float scale_max = (float)luaL_checknumber(L, 7);
            ImGui::PlotHistogram(label, values.data, values_count, values_offset,
                overlay_text, scale_min, scale_max);
        }
        else if (argc == 8)
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            const float scale_min = (float)luaL_checknumber(L, 6);
            const float scale_max = (float)luaL_checknumber(L, 7);
            ImVec2* graph_size = imgui_binding_lua_to_ImVec2(L, 8);
            ImGui::PlotHistogram(label, values.data, values_count, values_offset,
                overlay_text, scale_min, scale_max, *graph_size);
        }
        else
        {
            const int values_offset = (int)luaL_checkinteger(L, 4);
            const char* overlay_text = luaL_checkstring(L, 5);
            const float scale_min = (float)luaL_checknumber(L, 6);
            const float scale_max = (float)luaL_checknumber(L, 7);
            ImVec2* graph_size = imgui_binding_lua_to_ImVec2(L, 8);
            const int stride = (int)luaL_checkinteger(L, 9);
            ImGui::PlotHistogram(label, values.data, values_count, values_offset,
                overlay_text, scale_min, scale_max, *graph_size, stride * sizeof(float));
        }
    }
    return 0;
}
