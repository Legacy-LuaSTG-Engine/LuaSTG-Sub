// This file was generated through a code generator, DO NOT edit it, please edit the code generator

#include "ImGuiTextBufferMember.hpp"

namespace imgui_binding_lua {

ImGuiTextBufferMember mapImGuiTextBufferMember(char const* const key, size_t const len) noexcept {
    if (key == nullptr || len == 0 || len > 7) {
        return ImGuiTextBufferMember::__unknown__;
    }
    int state{ 0 };
    #define make_condition(last_state, character) ((last_state << 8) | static_cast<int>(character))
    for (size_t i = 0; i < len; i += 1) {
        switch (i) {
        case 0:
            switch (make_condition(state, key[i])) {
            case make_condition(0, 'a'): state = 1; continue; // -> append
            case make_condition(0, 'c'): state = 7; continue; // -> c_str, clear
            case make_condition(0, 'e'): state = 16; continue; // -> empty
            case make_condition(0, 'r'): state = 21; continue; // -> reserve
            case make_condition(0, 's'): state = 28; continue; // -> size
            default: return ImGuiTextBufferMember::__unknown__;
            }
        case 1:
            switch (make_condition(state, key[i])) {
            case make_condition(1, 'p'): state = 2; continue; // -> append
            case make_condition(7, '_'): state = 8; continue; // -> c_str
            case make_condition(7, 'l'): state = 12; continue; // -> clear
            case make_condition(16, 'm'): state = 17; continue; // -> empty
            case make_condition(21, 'e'): state = 22; continue; // -> reserve
            case make_condition(28, 'i'): state = 29; continue; // -> size
            default: return ImGuiTextBufferMember::__unknown__;
            }
        case 2:
            switch (make_condition(state, key[i])) {
            case make_condition(2, 'p'): state = 3; continue; // -> append
            case make_condition(8, 's'): state = 9; continue; // -> c_str
            case make_condition(12, 'e'): state = 13; continue; // -> clear
            case make_condition(17, 'p'): state = 18; continue; // -> empty
            case make_condition(22, 's'): state = 23; continue; // -> reserve
            case make_condition(29, 'z'): state = 30; continue; // -> size
            default: return ImGuiTextBufferMember::__unknown__;
            }
        case 3:
            switch (make_condition(state, key[i])) {
            case make_condition(3, 'e'): state = 4; continue; // -> append
            case make_condition(9, 't'): state = 10; continue; // -> c_str
            case make_condition(13, 'a'): state = 14; continue; // -> clear
            case make_condition(18, 't'): state = 19; continue; // -> empty
            case make_condition(23, 'e'): state = 24; continue; // -> reserve
            case make_condition(30, 'e'): state = 31; continue; // -> size
            default: return ImGuiTextBufferMember::__unknown__;
            }
        case 4:
            switch (make_condition(state, key[i])) {
            case make_condition(4, 'n'): state = 5; continue; // -> append
            case make_condition(10, 'r'): state = 11; continue; // -> c_str
            case make_condition(14, 'r'): state = 15; continue; // -> clear
            case make_condition(19, 'y'): state = 20; continue; // -> empty
            case make_condition(24, 'r'): state = 25; continue; // -> reserve
            default: return ImGuiTextBufferMember::__unknown__;
            }
        case 5:
            switch (make_condition(state, key[i])) {
            case make_condition(5, 'd'): state = 6; continue; // -> append
            case make_condition(25, 'v'): state = 26; continue; // -> reserve
            default: return ImGuiTextBufferMember::__unknown__;
            }
        case 6:
            switch (make_condition(state, key[i])) {
            case make_condition(26, 'e'): state = 27; continue; // -> reserve
            default: return ImGuiTextBufferMember::__unknown__;
            }
        default: return ImGuiTextBufferMember::__unknown__;
        }
    }
    #undef make_condition
    switch (state) {
    case 6: return ImGuiTextBufferMember::append;
    case 11: return ImGuiTextBufferMember::c_str;
    case 15: return ImGuiTextBufferMember::clear;
    case 20: return ImGuiTextBufferMember::empty;
    case 27: return ImGuiTextBufferMember::reserve;
    case 31: return ImGuiTextBufferMember::size;
    default: return ImGuiTextBufferMember::__unknown__;
    }
}

}
