// This file was generated through a code generator, DO NOT edit it, please edit the code generator

#ifndef _LuaSTG_BlendModeX_h_
#define _LuaSTG_BlendModeX_h_

namespace LuaSTG {

enum class BlendModeX : int {
    __unknown__ = 0,
    MulAlpha = 1,
    MulAdd = 2,
    MulRev = 3,
    MulSub = 4,
    AddAlpha = 5,
    AddAdd = 6,
    AddRev = 7,
    AddSub = 8,
    AlphaBal = 9,
    MulMin = 10,
    MulMax = 11,
    MulMutiply = 12,
    MulScreen = 13,
    AddMin = 14,
    AddMax = 15,
    AddMutiply = 16,
    AddScreen = 17,
    One = 18,
};

BlendModeX MapBlendModeX(char const* const key, size_t const len) noexcept;

}

#endif // _LuaSTG_BlendModeX_h_
