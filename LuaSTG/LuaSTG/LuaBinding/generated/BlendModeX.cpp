// This file was generated through a code generator, DO NOT edit it, please edit the code generator

#include "BlendModeX.hpp"

namespace LuaSTG {

BlendModeX MapBlendModeX(char const* const key, size_t const len) noexcept {
    if (key == nullptr || len == 0 || len > 10) {
        return BlendModeX::__unknown__;
    }
    int state{ 0 };
    #define make_condition(last_state, character) ((last_state << 8) | static_cast<int>(character))
    for (size_t i = 0; i < len; i += 1) {
        switch (i) {
        case 0:
            switch (make_condition(state, key[i])) {
            case make_condition(0, 'a'): state = 1; continue; // -> add+add, add+alpha, add+max, add+min, add+mul, add+rev, add+screen, add+sub, alpha+bal
            case make_condition(0, 'm'): state = 38; continue; // -> mul+add, mul+alpha, mul+max, mul+min, mul+mul, mul+rev, mul+screen, mul+sub
            case make_condition(0, 'o'): state = 67; continue; // -> one
            default: return BlendModeX::__unknown__;
            }
        case 1:
            switch (make_condition(state, key[i])) {
            case make_condition(1, 'd'): state = 2; continue; // -> add+add, add+alpha, add+max, add+min, add+mul, add+rev, add+screen, add+sub
            case make_condition(1, 'l'): state = 30; continue; // -> alpha+bal
            case make_condition(38, 'u'): state = 39; continue; // -> mul+add, mul+alpha, mul+max, mul+min, mul+mul, mul+rev, mul+screen, mul+sub
            case make_condition(67, 'n'): state = 68; continue; // -> one
            default: return BlendModeX::__unknown__;
            }
        case 2:
            switch (make_condition(state, key[i])) {
            case make_condition(2, 'd'): state = 3; continue; // -> add+add, add+alpha, add+max, add+min, add+mul, add+rev, add+screen, add+sub
            case make_condition(30, 'p'): state = 31; continue; // -> alpha+bal
            case make_condition(39, 'l'): state = 40; continue; // -> mul+add, mul+alpha, mul+max, mul+min, mul+mul, mul+rev, mul+screen, mul+sub
            case make_condition(68, 'e'): state = 69; continue; // -> one
            default: return BlendModeX::__unknown__;
            }
        case 3:
            switch (make_condition(state, key[i])) {
            case make_condition(3, '+'): state = 4; continue; // -> add+add, add+alpha, add+max, add+min, add+mul, add+rev, add+screen, add+sub
            case make_condition(31, 'h'): state = 32; continue; // -> alpha+bal
            case make_condition(40, '+'): state = 41; continue; // -> mul+add, mul+alpha, mul+max, mul+min, mul+mul, mul+rev, mul+screen, mul+sub
            default: return BlendModeX::__unknown__;
            }
        case 4:
            switch (make_condition(state, key[i])) {
            case make_condition(4, 'a'): state = 5; continue; // -> add+add, add+alpha
            case make_condition(4, 'm'): state = 12; continue; // -> add+max, add+min, add+mul
            case make_condition(4, 'r'): state = 19; continue; // -> add+rev
            case make_condition(4, 's'): state = 22; continue; // -> add+screen, add+sub
            case make_condition(32, 'a'): state = 33; continue; // -> alpha+bal
            case make_condition(41, 'a'): state = 42; continue; // -> mul+add, mul+alpha
            case make_condition(41, 'm'): state = 49; continue; // -> mul+max, mul+min, mul+mul
            case make_condition(41, 'r'): state = 56; continue; // -> mul+rev
            case make_condition(41, 's'): state = 59; continue; // -> mul+screen, mul+sub
            default: return BlendModeX::__unknown__;
            }
        case 5:
            switch (make_condition(state, key[i])) {
            case make_condition(5, 'd'): state = 6; continue; // -> add+add
            case make_condition(5, 'l'): state = 8; continue; // -> add+alpha
            case make_condition(12, 'a'): state = 13; continue; // -> add+max
            case make_condition(12, 'i'): state = 15; continue; // -> add+min
            case make_condition(12, 'u'): state = 17; continue; // -> add+mul
            case make_condition(19, 'e'): state = 20; continue; // -> add+rev
            case make_condition(22, 'c'): state = 23; continue; // -> add+screen
            case make_condition(22, 'u'): state = 28; continue; // -> add+sub
            case make_condition(33, '+'): state = 34; continue; // -> alpha+bal
            case make_condition(42, 'd'): state = 43; continue; // -> mul+add
            case make_condition(42, 'l'): state = 45; continue; // -> mul+alpha
            case make_condition(49, 'a'): state = 50; continue; // -> mul+max
            case make_condition(49, 'i'): state = 52; continue; // -> mul+min
            case make_condition(49, 'u'): state = 54; continue; // -> mul+mul
            case make_condition(56, 'e'): state = 57; continue; // -> mul+rev
            case make_condition(59, 'c'): state = 60; continue; // -> mul+screen
            case make_condition(59, 'u'): state = 65; continue; // -> mul+sub
            default: return BlendModeX::__unknown__;
            }
        case 6:
            switch (make_condition(state, key[i])) {
            case make_condition(6, 'd'): state = 7; continue; // -> add+add
            case make_condition(8, 'p'): state = 9; continue; // -> add+alpha
            case make_condition(13, 'x'): state = 14; continue; // -> add+max
            case make_condition(15, 'n'): state = 16; continue; // -> add+min
            case make_condition(17, 'l'): state = 18; continue; // -> add+mul
            case make_condition(20, 'v'): state = 21; continue; // -> add+rev
            case make_condition(23, 'r'): state = 24; continue; // -> add+screen
            case make_condition(28, 'b'): state = 29; continue; // -> add+sub
            case make_condition(34, 'b'): state = 35; continue; // -> alpha+bal
            case make_condition(43, 'd'): state = 44; continue; // -> mul+add
            case make_condition(45, 'p'): state = 46; continue; // -> mul+alpha
            case make_condition(50, 'x'): state = 51; continue; // -> mul+max
            case make_condition(52, 'n'): state = 53; continue; // -> mul+min
            case make_condition(54, 'l'): state = 55; continue; // -> mul+mul
            case make_condition(57, 'v'): state = 58; continue; // -> mul+rev
            case make_condition(60, 'r'): state = 61; continue; // -> mul+screen
            case make_condition(65, 'b'): state = 66; continue; // -> mul+sub
            default: return BlendModeX::__unknown__;
            }
        case 7:
            switch (make_condition(state, key[i])) {
            case make_condition(9, 'h'): state = 10; continue; // -> add+alpha
            case make_condition(24, 'e'): state = 25; continue; // -> add+screen
            case make_condition(35, 'a'): state = 36; continue; // -> alpha+bal
            case make_condition(46, 'h'): state = 47; continue; // -> mul+alpha
            case make_condition(61, 'e'): state = 62; continue; // -> mul+screen
            default: return BlendModeX::__unknown__;
            }
        case 8:
            switch (make_condition(state, key[i])) {
            case make_condition(10, 'a'): state = 11; continue; // -> add+alpha
            case make_condition(25, 'e'): state = 26; continue; // -> add+screen
            case make_condition(36, 'l'): state = 37; continue; // -> alpha+bal
            case make_condition(47, 'a'): state = 48; continue; // -> mul+alpha
            case make_condition(62, 'e'): state = 63; continue; // -> mul+screen
            default: return BlendModeX::__unknown__;
            }
        case 9:
            switch (make_condition(state, key[i])) {
            case make_condition(26, 'n'): state = 27; continue; // -> add+screen
            case make_condition(63, 'n'): state = 64; continue; // -> mul+screen
            default: return BlendModeX::__unknown__;
            }
        default: return BlendModeX::__unknown__;
        }
    }
    #undef make_condition
    switch (state) {
    case 7: return BlendModeX::AddAdd;
    case 11: return BlendModeX::AddAlpha;
    case 14: return BlendModeX::AddMax;
    case 16: return BlendModeX::AddMin;
    case 18: return BlendModeX::AddMutiply;
    case 21: return BlendModeX::AddRev;
    case 27: return BlendModeX::AddScreen;
    case 29: return BlendModeX::AddSub;
    case 37: return BlendModeX::AlphaBal;
    case 44: return BlendModeX::MulAdd;
    case 48: return BlendModeX::MulAlpha;
    case 51: return BlendModeX::MulMax;
    case 53: return BlendModeX::MulMin;
    case 55: return BlendModeX::MulMutiply;
    case 58: return BlendModeX::MulRev;
    case 64: return BlendModeX::MulScreen;
    case 66: return BlendModeX::MulSub;
    case 69: return BlendModeX::One;
    default: return BlendModeX::__unknown__;
    }
}

}
