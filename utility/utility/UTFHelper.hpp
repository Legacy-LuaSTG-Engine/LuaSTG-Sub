#pragma once
#include <cstdint>
#include <string>

#ifdef _MSVC_LANG
#if (_MSVC_LANG <= 201703L)
using char8_t = char;
#endif
#else
#if (__cplusplus <= 201703L)
using char8_t = char;
#endif
#endif

namespace utf
{
    constexpr uint8_t utf8bom[3]    = { 0xEF, 0xBB, 0xBF };
    constexpr uint8_t utf16bebom[2] = { 0xFE, 0xFF };
    constexpr uint8_t utf16lebom[2] = { 0xFF, 0xFE };
    constexpr uint8_t utf32bebom[4] = { 0x00, 0x00, 0xFE, 0xFF };
    constexpr uint8_t utf32lebom[4] = { 0xFF, 0xFE, 0x00, 0x00 };
    
    enum class type
    {
        unknown,
        utf8,
        utf16be,
        utf16le,
        utf32be,
        utf32le,
    };
    
    type checktype(uint8_t* buffer, size_t len, size_t* bom);
    
    void char32to8(char32_t code, char8_t* o, size_t* w);
    void char32to16(char32_t code, char16_t* o, size_t* w);
    
    void char8to32(char8_t* i, size_t* r, char32_t* code);
    void char16to32(char16_t* i, size_t* r, char32_t* code);
    
    void char8to16(char8_t* i, size_t* r, char16_t* o, size_t* w);
    void char16to8(char16_t* i, size_t* r, char8_t* o, size_t* w);
    
    size_t char16swap(uint8_t* buffer, size_t len);
    size_t char32swap(uint8_t* buffer, size_t len);
    
    std::string convert(const std::wstring& str);
    std::wstring convert(const std::string& str);
};
