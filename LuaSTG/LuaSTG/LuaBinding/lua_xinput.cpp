#include "lua_xinput.hpp"
#include "Platform/XInput.hpp"
#include <array>
#include <string_view>

namespace xinput = Platform::XInput;

static int xinput_isConnected(lua_State* L)
{
    const int idx = luaL_checkinteger(L, 1);
    const bool ret = xinput::isConnected(idx - 1);
    lua_pushboolean(L, ret);
    return 1;
}
static int xinput_refresh(lua_State* L)
{
    const int ret = xinput::refresh();
    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}
static int xinput_update(lua_State*)
{
    xinput::update();
    return 0;
}
static int xinput_getKeyState(lua_State* L)
{
    if (lua_gettop(L) >= 2)
    {
        lua_pushboolean(L, xinput::getKeyState((int)luaL_checkinteger(L, 1) - 1, (int)luaL_checkinteger(L, 2)));
    }
    else
    {
        lua_pushboolean(L, xinput::getKeyState((int)luaL_checkinteger(L, 1)));
    }
    return 1;
}

#define CommonGetFloat(_F) \
    static int xinput_##_F (lua_State* L)\
    {\
        if (lua_gettop(L) >= 1)\
        {\
            lua_pushnumber(L, (lua_Number)xinput::_F((int)luaL_checkinteger(L, 1) - 1));\
        }\
        else\
        {\
            lua_pushnumber(L, (lua_Number)xinput::_F());\
        }\
        return 1;\
    }

CommonGetFloat(getLeftTrigger);
CommonGetFloat(getRightTrigger);
CommonGetFloat(getLeftThumbX);
CommonGetFloat(getLeftThumbY);
CommonGetFloat(getRightThumbX);
CommonGetFloat(getRightThumbY);

int lua_xinput_open(lua_State* L)
{
    #define MakePair(_Name) {#_Name, &xinput_##_Name}
    const luaL_Reg lib[] = {
        MakePair(isConnected),
        MakePair(refresh),
        MakePair(update),
        MakePair(getKeyState),
        MakePair(getLeftTrigger),
        MakePair(getRightTrigger),
        MakePair(getLeftThumbX),
        MakePair(getLeftThumbY),
        MakePair(getRightThumbX),
        MakePair(getRightThumbY),
        {NULL, NULL},
    };
    luaL_register(L, "xinput", lib);
    #define MakeValue(_Name) std::make_pair<std::string_view, lua_Integer>(#_Name, (lua_Integer)xinput::_Name)
    const std::array<std::pair<std::string_view, lua_Integer>, 15> libval = {
        MakeValue(Null         ),
        MakeValue(Up           ),
        MakeValue(Down         ),
        MakeValue(Left         ),
        MakeValue(Right        ),
        MakeValue(Start        ),
        MakeValue(Back         ),
        MakeValue(LeftThumb    ),
        MakeValue(RightThumb   ),
        MakeValue(LeftShoulder ),
        MakeValue(RightShoulder),
        MakeValue(A            ),
        MakeValue(B            ),
        MakeValue(X            ),
        MakeValue(Y            ),
    };
    for (auto& v : libval)
    {
        lua_pushinteger(L, v.second);
        lua_setfield(L, -2, v.first.data());
    }
    return 1;
}
