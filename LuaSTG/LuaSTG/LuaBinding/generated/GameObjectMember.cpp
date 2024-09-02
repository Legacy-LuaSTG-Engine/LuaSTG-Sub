// This file was generated through a code generator, DO NOT edit it, please edit the code generator

#include "GameObjectMember.hpp"

namespace LuaSTG {

GameObjectMember MapGameObjectMember(char const* const key, size_t const len) noexcept {
    if (key == nullptr || len == 0 || len > 8) {
        return GameObjectMember::__unknown__;
    }
    int state{ 0 };
    #define make_condition(last_state, character) ((last_state << 8) | static_cast<int>(character))
    for (size_t i = 0; i < len; i += 1) {
        switch (i) {
        case 0:
            switch (make_condition(state, key[i])) {
            case make_condition(0, '_'): state = 1; continue; // -> _a, _angle, _b, _blend, _color, _g, _r, _speed
            case make_condition(0, 'a'): state = 24; continue; // -> a, ag, ani, ax, ay
            case make_condition(0, 'b'): state = 30; continue; // -> b, bound
            case make_condition(0, 'c'): state = 35; continue; // -> class, colli, collider
            case make_condition(0, 'd'): state = 47; continue; // -> dx, dy
            case make_condition(0, 'g'): state = 50; continue; // -> group
            case make_condition(0, 'h'): state = 55; continue; // -> hide, hscale
            case make_condition(0, 'i'): state = 64; continue; // -> img
            case make_condition(0, 'l'): state = 67; continue; // -> layer
            case make_condition(0, 'm'): state = 72; continue; // -> maxv, maxvx, maxvy
            case make_condition(0, 'n'): state = 78; continue; // -> navi, nopause
            case make_condition(0, 'o'): state = 88; continue; // -> omega, omiga
            case make_condition(0, 'p'): state = 96; continue; // -> pause
            case make_condition(0, 'r'): state = 101; continue; // -> rc, rect, rmove, rot
            case make_condition(0, 's'): state = 112; continue; // -> status
            case make_condition(0, 't'): state = 118; continue; // -> timer
            case make_condition(0, 'v'): state = 123; continue; // -> vscale, vx, vy
            case make_condition(0, 'w'): state = 131; continue; // -> world
            case make_condition(0, 'x'): state = 136; continue; // -> x
            case make_condition(0, 'y'): state = 137; continue; // -> y
            default: return GameObjectMember::__unknown__;
            }
        case 1:
            switch (make_condition(state, key[i])) {
            case make_condition(1, 'a'): state = 2; continue; // -> _a, _angle
            case make_condition(1, 'b'): state = 7; continue; // -> _b, _blend
            case make_condition(1, 'c'): state = 12; continue; // -> _color
            case make_condition(1, 'g'): state = 17; continue; // -> _g
            case make_condition(1, 'r'): state = 18; continue; // -> _r
            case make_condition(1, 's'): state = 19; continue; // -> _speed
            case make_condition(24, 'g'): state = 25; continue; // -> ag
            case make_condition(24, 'n'): state = 26; continue; // -> ani
            case make_condition(24, 'x'): state = 28; continue; // -> ax
            case make_condition(24, 'y'): state = 29; continue; // -> ay
            case make_condition(30, 'o'): state = 31; continue; // -> bound
            case make_condition(35, 'l'): state = 36; continue; // -> class
            case make_condition(35, 'o'): state = 40; continue; // -> colli, collider
            case make_condition(47, 'x'): state = 48; continue; // -> dx
            case make_condition(47, 'y'): state = 49; continue; // -> dy
            case make_condition(50, 'r'): state = 51; continue; // -> group
            case make_condition(55, 'i'): state = 56; continue; // -> hide
            case make_condition(55, 's'): state = 59; continue; // -> hscale
            case make_condition(64, 'm'): state = 65; continue; // -> img
            case make_condition(67, 'a'): state = 68; continue; // -> layer
            case make_condition(72, 'a'): state = 73; continue; // -> maxv, maxvx, maxvy
            case make_condition(78, 'a'): state = 79; continue; // -> navi
            case make_condition(78, 'o'): state = 82; continue; // -> nopause
            case make_condition(88, 'm'): state = 89; continue; // -> omega, omiga
            case make_condition(96, 'a'): state = 97; continue; // -> pause
            case make_condition(101, 'c'): state = 102; continue; // -> rc
            case make_condition(101, 'e'): state = 103; continue; // -> rect
            case make_condition(101, 'm'): state = 106; continue; // -> rmove
            case make_condition(101, 'o'): state = 110; continue; // -> rot
            case make_condition(112, 't'): state = 113; continue; // -> status
            case make_condition(118, 'i'): state = 119; continue; // -> timer
            case make_condition(123, 's'): state = 124; continue; // -> vscale
            case make_condition(123, 'x'): state = 129; continue; // -> vx
            case make_condition(123, 'y'): state = 130; continue; // -> vy
            case make_condition(131, 'o'): state = 132; continue; // -> world
            default: return GameObjectMember::__unknown__;
            }
        case 2:
            switch (make_condition(state, key[i])) {
            case make_condition(2, 'n'): state = 3; continue; // -> _angle
            case make_condition(7, 'l'): state = 8; continue; // -> _blend
            case make_condition(12, 'o'): state = 13; continue; // -> _color
            case make_condition(19, 'p'): state = 20; continue; // -> _speed
            case make_condition(26, 'i'): state = 27; continue; // -> ani
            case make_condition(31, 'u'): state = 32; continue; // -> bound
            case make_condition(36, 'a'): state = 37; continue; // -> class
            case make_condition(40, 'l'): state = 41; continue; // -> colli, collider
            case make_condition(51, 'o'): state = 52; continue; // -> group
            case make_condition(56, 'd'): state = 57; continue; // -> hide
            case make_condition(59, 'c'): state = 60; continue; // -> hscale
            case make_condition(65, 'g'): state = 66; continue; // -> img
            case make_condition(68, 'y'): state = 69; continue; // -> layer
            case make_condition(73, 'x'): state = 74; continue; // -> maxv, maxvx, maxvy
            case make_condition(79, 'v'): state = 80; continue; // -> navi
            case make_condition(82, 'p'): state = 83; continue; // -> nopause
            case make_condition(89, 'e'): state = 90; continue; // -> omega
            case make_condition(89, 'i'): state = 93; continue; // -> omiga
            case make_condition(97, 'u'): state = 98; continue; // -> pause
            case make_condition(103, 'c'): state = 104; continue; // -> rect
            case make_condition(106, 'o'): state = 107; continue; // -> rmove
            case make_condition(110, 't'): state = 111; continue; // -> rot
            case make_condition(113, 'a'): state = 114; continue; // -> status
            case make_condition(119, 'm'): state = 120; continue; // -> timer
            case make_condition(124, 'c'): state = 125; continue; // -> vscale
            case make_condition(132, 'r'): state = 133; continue; // -> world
            default: return GameObjectMember::__unknown__;
            }
        case 3:
            switch (make_condition(state, key[i])) {
            case make_condition(3, 'g'): state = 4; continue; // -> _angle
            case make_condition(8, 'e'): state = 9; continue; // -> _blend
            case make_condition(13, 'l'): state = 14; continue; // -> _color
            case make_condition(20, 'e'): state = 21; continue; // -> _speed
            case make_condition(32, 'n'): state = 33; continue; // -> bound
            case make_condition(37, 's'): state = 38; continue; // -> class
            case make_condition(41, 'l'): state = 42; continue; // -> colli, collider
            case make_condition(52, 'u'): state = 53; continue; // -> group
            case make_condition(57, 'e'): state = 58; continue; // -> hide
            case make_condition(60, 'a'): state = 61; continue; // -> hscale
            case make_condition(69, 'e'): state = 70; continue; // -> layer
            case make_condition(74, 'v'): state = 75; continue; // -> maxv, maxvx, maxvy
            case make_condition(80, 'i'): state = 81; continue; // -> navi
            case make_condition(83, 'a'): state = 84; continue; // -> nopause
            case make_condition(90, 'g'): state = 91; continue; // -> omega
            case make_condition(93, 'g'): state = 94; continue; // -> omiga
            case make_condition(98, 's'): state = 99; continue; // -> pause
            case make_condition(104, 't'): state = 105; continue; // -> rect
            case make_condition(107, 'v'): state = 108; continue; // -> rmove
            case make_condition(114, 't'): state = 115; continue; // -> status
            case make_condition(120, 'e'): state = 121; continue; // -> timer
            case make_condition(125, 'a'): state = 126; continue; // -> vscale
            case make_condition(133, 'l'): state = 134; continue; // -> world
            default: return GameObjectMember::__unknown__;
            }
        case 4:
            switch (make_condition(state, key[i])) {
            case make_condition(4, 'l'): state = 5; continue; // -> _angle
            case make_condition(9, 'n'): state = 10; continue; // -> _blend
            case make_condition(14, 'o'): state = 15; continue; // -> _color
            case make_condition(21, 'e'): state = 22; continue; // -> _speed
            case make_condition(33, 'd'): state = 34; continue; // -> bound
            case make_condition(38, 's'): state = 39; continue; // -> class
            case make_condition(42, 'i'): state = 43; continue; // -> colli, collider
            case make_condition(53, 'p'): state = 54; continue; // -> group
            case make_condition(61, 'l'): state = 62; continue; // -> hscale
            case make_condition(70, 'r'): state = 71; continue; // -> layer
            case make_condition(75, 'x'): state = 76; continue; // -> maxvx
            case make_condition(75, 'y'): state = 77; continue; // -> maxvy
            case make_condition(84, 'u'): state = 85; continue; // -> nopause
            case make_condition(91, 'a'): state = 92; continue; // -> omega
            case make_condition(94, 'a'): state = 95; continue; // -> omiga
            case make_condition(99, 'e'): state = 100; continue; // -> pause
            case make_condition(108, 'e'): state = 109; continue; // -> rmove
            case make_condition(115, 'u'): state = 116; continue; // -> status
            case make_condition(121, 'r'): state = 122; continue; // -> timer
            case make_condition(126, 'l'): state = 127; continue; // -> vscale
            case make_condition(134, 'd'): state = 135; continue; // -> world
            default: return GameObjectMember::__unknown__;
            }
        case 5:
            switch (make_condition(state, key[i])) {
            case make_condition(5, 'e'): state = 6; continue; // -> _angle
            case make_condition(10, 'd'): state = 11; continue; // -> _blend
            case make_condition(15, 'r'): state = 16; continue; // -> _color
            case make_condition(22, 'd'): state = 23; continue; // -> _speed
            case make_condition(43, 'd'): state = 44; continue; // -> collider
            case make_condition(62, 'e'): state = 63; continue; // -> hscale
            case make_condition(85, 's'): state = 86; continue; // -> nopause
            case make_condition(116, 's'): state = 117; continue; // -> status
            case make_condition(127, 'e'): state = 128; continue; // -> vscale
            default: return GameObjectMember::__unknown__;
            }
        case 6:
            switch (make_condition(state, key[i])) {
            case make_condition(44, 'e'): state = 45; continue; // -> collider
            case make_condition(86, 'e'): state = 87; continue; // -> nopause
            default: return GameObjectMember::__unknown__;
            }
        case 7:
            switch (make_condition(state, key[i])) {
            case make_condition(45, 'r'): state = 46; continue; // -> collider
            default: return GameObjectMember::__unknown__;
            }
        default: return GameObjectMember::__unknown__;
        }
    }
    #undef make_condition
    switch (state) {
    case 2: return GameObjectMember::_A;
    case 6: return GameObjectMember::VANGLE;
    case 7: return GameObjectMember::_B;
    case 11: return GameObjectMember::_BLEND;
    case 16: return GameObjectMember::_COLOR;
    case 17: return GameObjectMember::_G;
    case 18: return GameObjectMember::_R;
    case 23: return GameObjectMember::VSPEED;
    case 24: return GameObjectMember::A;
    case 25: return GameObjectMember::AG;
    case 27: return GameObjectMember::ANI;
    case 28: return GameObjectMember::AX;
    case 29: return GameObjectMember::AY;
    case 30: return GameObjectMember::B;
    case 34: return GameObjectMember::BOUND;
    case 39: return GameObjectMember::CLASS;
    case 43: return GameObjectMember::COLLI;
    case 46: return GameObjectMember::COLLIDER;
    case 48: return GameObjectMember::DX;
    case 49: return GameObjectMember::DY;
    case 54: return GameObjectMember::GROUP;
    case 58: return GameObjectMember::HIDE;
    case 63: return GameObjectMember::HSCALE;
    case 66: return GameObjectMember::IMG;
    case 71: return GameObjectMember::LAYER;
    case 75: return GameObjectMember::MAXV;
    case 76: return GameObjectMember::MAXVX;
    case 77: return GameObjectMember::MAXVY;
    case 81: return GameObjectMember::NAVI;
    case 87: return GameObjectMember::IGNORESUPERPAUSE;
    case 92: return GameObjectMember::OMEGA;
    case 95: return GameObjectMember::OMIGA;
    case 100: return GameObjectMember::PAUSE;
    case 102: return GameObjectMember::RES_RC;
    case 105: return GameObjectMember::RECT;
    case 109: return GameObjectMember::RESOLVEMOVE;
    case 111: return GameObjectMember::ROT;
    case 117: return GameObjectMember::STATUS;
    case 122: return GameObjectMember::TIMER;
    case 128: return GameObjectMember::VSCALE;
    case 129: return GameObjectMember::VX;
    case 130: return GameObjectMember::VY;
    case 135: return GameObjectMember::WORLD;
    case 136: return GameObjectMember::X;
    case 137: return GameObjectMember::Y;
    default: return GameObjectMember::__unknown__;
    }
}

}
