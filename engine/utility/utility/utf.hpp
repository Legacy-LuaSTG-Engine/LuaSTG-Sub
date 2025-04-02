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
    
    namespace detail
    {
        inline bool is_utf8_l1(char const* str)
        {
            return (str[0] & 0x80) == 0x00;
        }
        inline bool is_utf8_l2(char const* str)
        {
            return (str[0] & 0xE0) == 0xC0
                && (str[1] & 0xC0) == 0x80;
        }
        inline bool is_utf8_l3(char const* str)
        {
            return (str[0] & 0xF0) == 0xE0
                && (str[1] & 0xC0) == 0x80
                && (str[2] & 0xC0) == 0x80;
        }
        inline bool is_utf8_l4(char const* str)
        {
            return (str[0] & 0xF8) == 0xF0
                && (str[1] & 0xC0) == 0x80
                && (str[2] & 0xC0) == 0x80
                && (str[3] & 0xC0) == 0x80;
        }
        inline char32_t utf8_to_utf32_l1(char const* str)
        {
            return char32_t(str[0]);
        }
        inline char32_t utf8_to_utf32_l2(char const* str)
        {
            return char32_t(
                ((char32_t(str[0]) & 0x1F) << 6)
                | (char32_t(str[1]) & 0x3F));
        }
        inline char32_t utf8_to_utf32_l3(char const* str)
        {
            return char32_t(
                ((char32_t(str[0]) & 0x0F) << 12)
                | ((char32_t(str[1]) & 0x3F) << 6)
                | (char32_t(str[2]) & 0x3F));
        }
        inline char32_t utf8_to_utf32_l4(char const* str)
        {
            return char32_t(
                ((char32_t(str[0]) & 0x07) << 18)
                | ((char32_t(str[1]) & 0x3F) << 12)
                | ((char32_t(str[2]) & 0x3F) << 6)
                | (char32_t(str[3]) & 0x3F));
        }
    }
    
    class utf8reader
    {
    private:
        char const*  _raw_str = nullptr;
        size_t       _raw_len = 0;
        char const * _str = nullptr;
        size_t       _len = 0;
    public:
        bool back(char32_t& code)
        {
            auto const adv = _str - _raw_str;
            // no space
            if (_str == nullptr || adv < 1)
            {
                code = 0;
                return false;
            }
            // ascii
            if (adv >= 1 && detail::is_utf8_l1(_str - 1))
            {
                _str -= 1;
                _len += 1;
                code = detail::utf8_to_utf32_l1(_str);
                return true;
            }
            // 2 bytes utf8
            if (adv >= 2 && detail::is_utf8_l1(_str - 2))
            {
                _str -= 2;
                _len += 2;
                code = detail::utf8_to_utf32_l2(_str);
                return true;
            }
            // 3 bytes utf8
            if (adv >= 3 && detail::is_utf8_l1(_str - 3))
            {
                _str -= 3;
                _len += 3;
                code = detail::utf8_to_utf32_l3(_str);
                return true;
            }
            // 4 byte utf8
            if (adv >= 4 && detail::is_utf8_l1(_str - 4))
            {
                _str -= 4;
                _len += 4;
                code = detail::utf8_to_utf32_l4(_str);
                return true;
            }
            // unknown
            _str -= 1;
            _len += 1;
            code = 0x3F; // ?
            return true;
        }
        bool step(char32_t& code)
        {
            // no space
            if (_str == nullptr || _len == 0)
            {
                code = 0;
                return false;
            }
            // ascii
            if (detail::is_utf8_l1(_str))
            {
                code = detail::utf8_to_utf32_l1(_str);
                _str += 1;
                _len -= 1;
                return true;
            }
            // 2 bytes utf8
            if (_len >= 2 && detail::is_utf8_l2(_str))
            {
                code = detail::utf8_to_utf32_l2(_str);
                _str += 2;
                _len -= 2;
                return true;
            }
            // 3 bytes utf8
            if (_len >= 3 && detail::is_utf8_l3(_str))
            {
                code = detail::utf8_to_utf32_l3(_str);
                _str += 3;
                _len -= 3;
                return true;
            }
            // 4 byte utf8
            if (_len >= 4 && detail::is_utf8_l4(_str))
            {
                code = detail::utf8_to_utf32_l4(_str);
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
        char32_t last()
        {
            auto const adv = _str - _raw_str;
            // no space
            if (_str == nullptr || adv < 1)
            {
                return 0;
            }
            // ascii
            if (adv >= 1 && detail::is_utf8_l1(_str - 1))
            {
                return detail::utf8_to_utf32_l1(_str - 1);
            }
            // 2 bytes utf8
            if (adv >= 2 && detail::is_utf8_l1(_str - 2))
            {
                return detail::utf8_to_utf32_l2(_str - 2);
            }
            // 3 bytes utf8
            if (adv >= 3 && detail::is_utf8_l1(_str - 3))
            {
                return detail::utf8_to_utf32_l3(_str - 3);
            }
            // 4 byte utf8
            if (adv >= 4 && detail::is_utf8_l1(_str - 4))
            {
                return detail::utf8_to_utf32_l4(_str - 4);
            }
            // unknown
            return 0x3F; // ?
        }
        char32_t next()
        {
            // no space
            if (_str == nullptr || _len == 0)
            {
                return 0;
            }
            // ascii
            if (detail::is_utf8_l1(_str))
            {
                return detail::utf8_to_utf32_l1(_str);
            }
            // 2 bytes utf8
            if (_len >= 2 && detail::is_utf8_l2(_str))
            {
                return detail::utf8_to_utf32_l2(_str);
            }
            // 3 bytes utf8
            if (_len >= 3 && detail::is_utf8_l3(_str))
            {
                return detail::utf8_to_utf32_l3(_str);
            }
            // 4 byte utf8
            if (_len >= 4 && detail::is_utf8_l4(_str))
            {
                return detail::utf8_to_utf32_l4(_str);
            }
            // unknown
            return 0x3F; // ?
        }
        size_t tell() { return _len - _raw_len; }
    public:
        inline bool operator()(char32_t& code) { return step(code); }
        inline char32_t operator()() { char32_t code = 0; step(code); return code; }
    public:
        utf8reader(char const * str, size_t len) : _raw_str(str), _raw_len(len), _str(str), _len(len)
        {
        }
        utf8reader(char const * str) : _raw_str(str), _raw_len(0), _str(str), _len(0)
        {
            // warning: unsafe!!!
            while (_str[_raw_len] != 0)
            {
                _raw_len += 1;
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
