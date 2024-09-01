// This file was generated through a code generator, DO NOT edit it, please edit the code generator

#include "ColorMember.hpp"

namespace LuaSTG {

ColorMember MapColorMember(char const* const key, size_t const len) noexcept {
    if (key == nullptr || len == 0 || len > 4) {
        return ColorMember::__unknown__;
    }
    int state{ 0 };
    #define make_condition(last_state, character) ((last_state << 8) | static_cast<int>(character))
    for (size_t i = 0; i < len; i += 1) {
        switch (i) {
        case 0:
            switch (make_condition(state, key[i])) {
            case make_condition(0, 'A'): state = 1; continue; // -> AHSV, ARGB
            case make_condition(0, 'a'): state = 8; continue; // -> a, argb
            case make_condition(0, 'b'): state = 12; continue; // -> b
            case make_condition(0, 'g'): state = 13; continue; // -> g
            case make_condition(0, 'h'): state = 14; continue; // -> h
            case make_condition(0, 'r'): state = 15; continue; // -> r
            case make_condition(0, 's'): state = 16; continue; // -> s
            case make_condition(0, 'v'): state = 17; continue; // -> v
            default: return ColorMember::__unknown__;
            }
        case 1:
            switch (make_condition(state, key[i])) {
            case make_condition(1, 'H'): state = 2; continue; // -> AHSV
            case make_condition(1, 'R'): state = 5; continue; // -> ARGB
            case make_condition(8, 'r'): state = 9; continue; // -> argb
            default: return ColorMember::__unknown__;
            }
        case 2:
            switch (make_condition(state, key[i])) {
            case make_condition(2, 'S'): state = 3; continue; // -> AHSV
            case make_condition(5, 'G'): state = 6; continue; // -> ARGB
            case make_condition(9, 'g'): state = 10; continue; // -> argb
            default: return ColorMember::__unknown__;
            }
        case 3:
            switch (make_condition(state, key[i])) {
            case make_condition(3, 'V'): state = 4; continue; // -> AHSV
            case make_condition(6, 'B'): state = 7; continue; // -> ARGB
            case make_condition(10, 'b'): state = 11; continue; // -> argb
            default: return ColorMember::__unknown__;
            }
        default: return ColorMember::__unknown__;
        }
    }
    #undef make_condition
    switch (state) {
    case 4: return ColorMember::f_AHSV;
    case 7: return ColorMember::f_ARGB;
    case 8: return ColorMember::m_a;
    case 11: return ColorMember::m_argb;
    case 12: return ColorMember::m_b;
    case 13: return ColorMember::m_g;
    case 14: return ColorMember::m_h;
    case 15: return ColorMember::m_r;
    case 16: return ColorMember::m_s;
    case 17: return ColorMember::m_v;
    default: return ColorMember::__unknown__;
    }
}

}
