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
        ImVec2* size = imgui::binding::ImVec2Binding::as(L, 2);
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
