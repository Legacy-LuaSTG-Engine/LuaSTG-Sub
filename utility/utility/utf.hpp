#pragma once

namespace utf
{
    constexpr unsigned char utf8bom[3]    = { 0xEF, 0xBB, 0xBF };
    constexpr unsigned char utf16lebom[2] = { 0xFF, 0xFE };
    constexpr unsigned char utf16bebom[2] = { 0xFE, 0xFF };
    constexpr unsigned char utf32lebom[4] = { 0xFF, 0xFE, 0x00, 0x00 };
    constexpr unsigned char utf32bebom[4] = { 0x00, 0x00, 0xFE, 0xFF };
    
    enum class type
    {
        unknown = 0,
        utf8    = 1,
        utf16be = 2,
        utf16le = 3,
        utf32be = 4,
        utf32le = 5,
    };
    
    inline type testtype(const void* buffer, size_t size, size_t& bomsize)
    {
        char unsigned const * v = static_cast<char unsigned const *>(buffer);
        if (size >= 4)
        {
            if (v[0] == utf32lebom[0] && v[1] == utf32lebom[1] && v[2] == utf32lebom[2] && v[3] == utf32lebom[3])
            {
                bomsize = 4;
                return type::utf32le;
            }
            if (v[0] == utf32bebom[0] && v[1] == utf32bebom[1] && v[2] == utf32bebom[2] && v[3] == utf32bebom[3])
            {
                bomsize = 4;
                return type::utf32be;
            }
        }
        if (size >= 3)
        {
            if (v[0] == utf8bom[0] && v[1] == utf8bom[1] && v[2] == utf8bom[2])
            {
                bomsize = 3;
                return type::utf8;
            }
        }
        if (size >= 2)
        {
            if (v[0] == utf16lebom[0] && v[1] == utf16lebom[1])
            {
                bomsize = 2;
                return type::utf16le;
            }
            if (v[0] == utf16bebom[0] && v[1] == utf16bebom[1])
            {
                bomsize = 2;
                return type::utf16be;
            }
        }
        bomsize = 0;
        return type::utf8;
    }
    inline type testtype(const void* buffer, size_t size)
    {
        size_t _ = 0;
        return testtype(buffer, size, _);
    }
    inline type testtype(const void* buffer)
    {
        // warning: unsafe!!!
        size_t _ = 0;
        return testtype(buffer, 4, _);
    }
    
    inline size_t testbom(const void* buffer, size_t size)
    {
        size_t bomsize = 0;
        testtype(buffer, size, bomsize);
        return bomsize;
    }
    inline size_t testbom(const void* buffer)
    {
        // warning: unsafe!!!
        size_t bomsize = 0;
        testtype(buffer, 4, bomsize);
        return bomsize;
    }
    
    class utf8reader
    {
    private:
        char const * _str = nullptr;
        size_t       _len = 0;
    public:
        bool operator()(char32_t& code)
        {
            code = 0;
            if (_str == nullptr || _len == 0)
            {
                return false;
            }
            // ascii
            if ((_str[0] & 0x80) == 0x00)
            {
                code = _str[0] & 0xFF;
                _str += 1;
                _len -= 1;
                return true;
            }
            // 2 bytes utf8
            if (_len >= 2 && (_str[0] & 0xE0) == 0xC0 && (_str[1] & 0xC0) == 0x80)
            {
                code = ((_str[0] & 0x1F) << 6) | ((_str[1] & 0x3F) << 0);
                _str += 2;
                _len -= 2;
                return true;
            }
            // 3 bytes utf8
            if (_len >= 3 && (_str[0] & 0xF0) == 0xE0 && (_str[1] & 0xC0) == 0x80 && (_str[2] & 0xC0) == 0x80)
            {
                code = ((_str[0] & 0x0F) << 12) | ((_str[1] & 0x3F) << 6) | ((_str[2] & 0x3F) << 0);
                _str += 3;
                _len -= 3;
                return true;
            }
            // 4 byte utf8
            if (_len >= 4 && (_str[0] & 0xF8) == 0xF0 && (_str[1] & 0xC0) == 0x80 && (_str[2] & 0xC0) == 0x80 && (_str[3] & 0xC0) == 0x80)
            {
                code = ((_str[0] & 0x07) << 18) | ((_str[1] & 0x3F) << 12) | ((_str[2] & 0x3F) << 6) | ((_str[3] & 0x3F) << 0);
                _str += 4;
                _len -= 4;
                return true;
            }
            // unknown
            code = 0x3F; // ?
            _str += 1;
            _len -= 1;
            return true;
        }
        char32_t operator()()
        {
            char32_t code = 0;
            operator()(code);
            return code;
        }
    public:
        utf8reader(char const * str, size_t len) : _str(str), _len(len) {}
        utf8reader(char const * str) : _str(str), _len(0)
        {
            // warning: unsafe!!!
            while (_str[_len] != 0)
            {
                _len += 1;
            }
        }
    };
    
    class utf16reader
    {
    private:
        char16_t const * _str = nullptr;
        size_t           _len = 0;
    public:
        bool operator()(char32_t& code)
        {
            code = 0;
            if (_str == nullptr || _len == 0)
            {
                return false;
            }
            // 4 byte utf16
            if (_len >= 2 && (_str[0] & 0xFC00) == 0xD800 && (_str[1] & 0xFC00) == 0xDC00)
            {
                code = ((_str[0] & 0x3FF) << 10) | ((_str[1] & 0x3FF) << 0);
                _str += 2;
                _len -= 2;
                return true;
            }
            // 2 byte utf16
            if ((_str[0] & 0xFC00) != 0xDC00) // not equal to!
            {
                code = _str[0] & 0xFFFF;
                _str += 1;
                _len -= 1;
                return true;
            }
            // unknown
            code = 0x3F; // ?
            _str += 1;
            _len -= 1;
            return true;
        }
        char32_t operator()()
        {
            char32_t code = 0;
            operator()(code);
            return code;
        }
    public:
        utf16reader(char16_t const * str, size_t len) : _str(str), _len(len) {}
        utf16reader(char16_t const * str) : _str(str), _len(0)
        {
            // warning: unsafe!!!
            while (_str[_len] != 0)
            {
                _len += 1;
            }
        }
    };
};
