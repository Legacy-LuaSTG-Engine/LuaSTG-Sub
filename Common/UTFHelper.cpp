#include "Common/UTFHelper.hpp"
#include <cassert>

static_assert(sizeof(char8_t) == sizeof(char), "invalid type size");

namespace utf
{
    type checktype(uint8_t* buffer, size_t len, size_t* bom)
    {
        if (len > 0)
        {
            switch (buffer[0])
            {
            case 0x00:
                if (len >= 4)
                {
                    if (buffer[1] == 0x00 && buffer[2] == 0xFE && buffer[3] == 0xFF)
                    {
                        *bom = 4;
                        return type::utf32be;
                    }
                }
            case 0xEF:
                if (len >= 3)
                {
                    if (buffer[1] == 0xBB && buffer[2] == 0xBF)
                    {
                        *bom = 3;
                        return type::utf8;
                    }
                }
            case 0xFE:
                if (len >= 2)
                {
                    if (buffer[1] == 0xFF)
                    {
                        *bom = 2;
                        return type::utf16be;
                    }
                }
            case 0xFF:
                if (len >= 4)
                {
                    if (buffer[1] == 0xFE && buffer[2] == 0x00 && buffer[3] == 0x00)
                    {
                        *bom = 4;
                        return type::utf32le;
                    }
                    else if (buffer[1] == 0xFF)
                    {
                        *bom = 2;
                        return type::utf16le;
                    }
                }
                else if (len >= 2)
                {
                    if (buffer[1] == 0xFF)
                    {
                        *bom = 2;
                        return type::utf16le;
                    }
                }
            }
        }
        // default
        *bom = 0;
        return type::utf8;
    }
    
    void char32to8(char32_t code, char8_t* o, size_t* w)
    {
        assert((o != nullptr) && (w != nullptr));
        // ASCII, single byte
        if (code >= 0x0 && code <= 0x7F)
        {
            o[0] = (char8_t)code;
            *w = 1;
        }
        // 2 bytes utf8
        else if (code >= 0x80 && code <= 0x7FF)
        {
            o[0] = 0xC0 | ((code & 0x7C0) >> 6);
            o[1] = 0x80 | (code & 0x3F);
            *w = 2;
        }
        // 3 bytes utf8
        else if (code >= 0x800 && code <= 0xFFFF)
        {
            o[0] = 0xE0 | ((code & 0xF000) >> 12);
            o[1] = 0x80 | ((code & 0xFC0) >> 6);
            o[2] = 0x80 | (code & 0x3F);
            *w = 3;
        }
        // 4 bytes utf8
        else if (code >= 0x10000 && code <= 0x10FFFF)
        {
            o[0] = 0xF0 | ((code & 0x1C0000) >> 18);
            o[1] = 0x80 | ((code & 0x3F000) >> 12);
            o[2] = 0x80 | ((code & 0xFC0) >> 6);
            o[3] = 0x80 | (code & 0x3F);
            *w = 4;
        }
        // error
        else
        {
            o[0] = '?';
            *w = 1;
        }
    }
    void char32to16(char32_t code, char16_t* o, size_t* w)
    {
        assert((o != nullptr) && (w != nullptr));
        // single word utf16
        if (code >= 0x0 && code <= 0xFFFF)
        {
            o[0] = (char16_t)code;
            *w = 1;
        }
        // double word utf16
        else if (code >= 0x10000 && code <= 0x10FFFF)
        {
            code -= 0x10000;
            o[0] = 0xD800 | ((code & 0xFFC00) >> 10);
            o[1] = 0xDC00 | (code & 0x3FF);
            *w = 2;
        }
        // error
        else
        {
            o[0] = L'?';
            *w = 1;
        }
    }
    
    void char8to32(char8_t* i, size_t* r, char32_t* code)
    {
        assert((i != nullptr) && (r != nullptr) && (code != nullptr));
        *code = 0;
        // byte[0]
        char8_t a = *i;
        // len 1
        if ((a & 0x80) == 0x00)
        {
            *code = (char32_t)a;
            *r = 1;
            return;
        }
        // len 2
        else if ((a & 0xE0) == 0xC0)
        {
            *code |= ((a & 0x1F) << 6);
            // byte[1]
            a = *(i + 1);
            if ((a & 0xC0) == 0x80)
            {
                *code |= (a & 0x3F);
                *r = 2;
                return;
            }
        }
        // len 3
        else if ((a & 0xF0) == 0xE0)
        {
            *code |= ((a & 0x0F) << 12);
            // byte[1]
            a = *(i + 1);
            if ((a & 0xC0) == 0x80)
            {
                *code |= ((a & 0x3F) << 6);
                // byte[2]
                a = *(i + 2);
                if ((a & 0xC0) == 0x80)
                {
                    *code |= (a & 0x3F);
                    *r = 3;
                    return;
                }
            }
        }
        // len 4
        else if ((a & 0xF8) == 0xF0)
        {
            *code |= ((a & 0x07) << 18);
            // byte[1]
            a = *(i + 1);
            if ((a & 0xC0) == 0x80)
            {
                *code |= ((a & 0x3F) << 12);
                // byte[2]
                a = *(i + 2);
                if ((a & 0xC0) == 0x80)
                {
                    *code |= ((a & 0x3F) << 6);
                    // byte[3]
                    a = *(i + 3);
                    if ((a & 0xC0) == 0x80)
                    {
                        *code |= (a & 0x3F);
                        *r = 4;
                        return;
                    }
                }
            }
        }
        // default
        *code = U'?';
        *r = 1;
    }
    void char16to32(char16_t* i, size_t* r, char32_t* code)
    {
        assert((i != nullptr) && (r != nullptr) && (code != nullptr));
        *code = 0;
        // word[0]
        char16_t a = *i;
        if ((a & 0xFC00) == 0xD800)
        {
            *code |= ((a & 0x3FF) << 10);
            // word[1]
            a = *(i + 1);
            if ((a & 0xFC00) == 0xDC00)
            {
                *code |= (a & 0x3FF);
                *code += 0x10000;
                *r = 2;
            }
            else
            {
                *code = U'?';
                *r = 1;
            }
        }
        else if ((a & 0xFC00) == 0xDC00)
        {
            *code = U'?';
            *r = 1;
        }
        else
        {
            *code = (char32_t)a;
            *r = 1;
        }
    }
    
    void char8to16(char8_t* i, size_t* r, char16_t* o, size_t* w)
    {
        char32_t code = 0;
        char8to32(i, r, &code);
        char32to16(code, o, w);
    }
    void char16to8(char16_t* i, size_t* r, char8_t* o, size_t* w)
    {
        char32_t code = 0;
        char16to32(i, r, &code);
        char32to8(code, o, w);
    }
    
    size_t char16swap(uint8_t* buffer, size_t len)
    {
        assert((buffer != nullptr) || (len == 0));
        len /= 2;
        uint8_t tmp = 0;
        for (size_t i = 0; i < len; i += 1)
        {
            tmp = buffer[0];
            buffer[0] = buffer[1];
            buffer[1] = tmp;
            buffer += 2;
        }
        return len *= 2;
    }
    size_t char32swap(uint8_t* buffer, size_t len)
    {
        assert((buffer != nullptr) || (len == 0));
        len /= 4;
        uint8_t tmp = 0;
        for (size_t i = 0; i < len; i += 1)
        {
            tmp = buffer[0];
            buffer[0] = buffer[3];
            buffer[3] = tmp;
            tmp = buffer[1];
            buffer[1] = buffer[2];
            buffer[2] = tmp;
            buffer += 4;
        }
        return len *= 4;
    }
    
    std::string convert(const std::wstring& str)
    {
        char16_t* cstart = (char16_t*)str.c_str();
        char16_t* cend   = cstart + str.length();
        if (cstart >= cend)
        {
            return "";
        }
        
        std::string buffer;
        buffer.reserve(str.length());
        
        size_t read = 0, write = 0;
        char8_t utf8[4] = { 0 };
        while (true)
        {
            if (cstart < cend)
            {
                char16to8(cstart, &read, utf8, &write);
                cstart += read;
                buffer.append((char*)utf8, write);
            }
            else
            {
                break;
            }
        }
        return std::move(buffer);
    }
    std::wstring convert(const std::string& str)
    {
        char8_t* cstart = (char8_t*)str.c_str();
        char8_t* cend   = cstart + str.length();
        if (cstart >= cend)
        {
            return L"";
        }
        
        std::wstring buffer;
        buffer.reserve(str.length());
        
        size_t read = 0, write = 0;
        char16_t utf16[2] = { 0 };
        while (true)
        {
            if (cstart < cend)
            {
                char8to16(cstart, &read, utf16, &write);
                cstart += read;
                buffer.append((wchar_t*)utf16, write);
            }
            else
            {
                break;
            }
        }
        return std::move(buffer);
    }
};
