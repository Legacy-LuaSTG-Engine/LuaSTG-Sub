// This file was generated through a code generator, DO NOT edit it, please edit the code generator

#ifndef _LuaSTG_GameObjectMember_h_
#define _LuaSTG_GameObjectMember_h_

namespace LuaSTG {

enum class GameObjectMember : int {
    __unknown__ = -1,
    STATUS = 36,
    CLASS = 15,
    TIMER = 37,
    X = 42,
    Y = 43,
    ROT = 35,
    HSCALE = 22,
    VSCALE = 38,
    DX = 18,
    DY = 19,
    OMEGA = 30,
    AX = 11,
    AY = 12,
    AG = 9,
    VX = 39,
    VY = 40,
    MAXV = 25,
    MAXVX = 26,
    MAXVY = 27,
    NAVI = 28,
    LAYER = 24,
    HIDE = 21,
    IMG = 23,
    RES_RC = 32,
    ANI = 10,
    _BLEND = 3,
    _COLOR = 4,
    _A = 0,
    _R = 6,
    _G = 5,
    _B = 2,
    BOUND = 14,
    GROUP = 20,
    COLLI = 16,
    A = 8,
    B = 13,
    RECT = 33,
    COLLIDER = 17,
    VANGLE = 1,
    VSPEED = 7,
    PAUSE = 31,
    IGNORESUPERPAUSE = 29,
    RESOLVEMOVE = 34,
    WORLD = 41,
};

GameObjectMember MapGameObjectMember(char const* const key, size_t const len) noexcept;

}

#endif // _LuaSTG_GameObjectMember_h_
