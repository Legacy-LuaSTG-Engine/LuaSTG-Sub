// This file was generated through a code generator, DO NOT edit it, please edit the code generator

#ifndef _LuaSTG_ColorMember_h_
#define _LuaSTG_ColorMember_h_

namespace LuaSTG {

enum class ColorMember : int {
    __unknown__ = -1,
    m_a = 2,
    m_r = 7,
    m_g = 5,
    m_b = 4,
    m_argb = 3,
    f_ARGB = 1,
    m_h = 6,
    m_s = 8,
    m_v = 9,
    f_AHSV = 0,
};

ColorMember MapColorMember(char const* const key, size_t const len) noexcept;

}

#endif // _LuaSTG_ColorMember_h_
