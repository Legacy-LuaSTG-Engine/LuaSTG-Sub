#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

const char* match_integer_size(size_t size)
{
    if (sizeof(char) == size) return "char";
    if (sizeof(short int) == size) return "short int";
    if (sizeof(int) == size) return "int";
    if (sizeof(long int) == size) return "long int";
    if (sizeof(long long int) == size) return "long long int";
    throw std::runtime_error("unknown type");
}

const char* match_float_size(size_t size)
{
    if (sizeof(float) == size) return "float";
    if (sizeof(double) == size) return "double";
    if (sizeof(long double) == size) return "long double";
    throw std::runtime_error("unknown type");
}

int main(int argc, char** argv)
{
    std::string outpath = "";
    for (int c = 0; c < argc; c += 1)
    {
        if (std::string(argv[c]) == "-o")
        {
            if ((c + 1) < argc)
            {
                outpath = argv[c + 1];
                c += 1;
            }
            else
            {
                throw std::runtime_error("invalid parameter");
            }
        }
    }
    
    std::stringstream header;
    header << "#pragma once" << std::endl;
    header << std::endl;
    header << "namespace slow" << std::endl;
    header << "{" << std::endl;
    
    header << "    using u8 = unsigned " << match_integer_size(1) << ";" << std::endl;
    header << "    using u16 = unsigned " << match_integer_size(2) << ";" << std::endl;
    header << "    using u32 = unsigned " << match_integer_size(4) << ";" << std::endl;
    header << "    using u64 = unsigned " << match_integer_size(8) << ";" << std::endl;
    
    header << "    using i8 = signed " << match_integer_size(1) << ";" << std::endl;
    header << "    using i16 = signed " << match_integer_size(2) << ";" << std::endl;
    header << "    using i32 = signed " << match_integer_size(4) << ";" << std::endl;
    header << "    using i64 = signed " << match_integer_size(8) << ";" << std::endl;
    
    header << "    using f32 = " << match_float_size(4) << ";" << std::endl;
    header << "    using f64 = " << match_float_size(8) << ";" << std::endl;
    
    #if (_MSVC_LANG >= 202002L)
    static_assert(sizeof(char8_t) == 1, "unknown type");
    constexpr char8_t _test_char8_t = 0;
    header << "    using c8 = char8_t;" << std::endl;
    #else
    static_assert(sizeof(char) == 1, "unknown type");
    header << "    using c8 = char;" << std::endl;
    #endif
    #if (_MSVC_LANG >= 201103L) // MSVC always support c++14 and this required c++11
    static_assert(sizeof(char16_t) == 2, "unknown type");
    static_assert(sizeof(char32_t) == 4, "unknown type");
    header << "    using c16 = char16_t;" << std::endl;
    header << "    using c32 = char32_t;" << std::endl;
    #else
    header << "    using c16 = unsigned " << match_integer_size(2) << ";" << std::endl;
    header << "    using c32 = unsigned " << match_integer_size(4) << ";" << std::endl;
    #endif
    
    header << "    using usize = unsigned " << match_integer_size(sizeof(void*)) << ";" << std::endl;
    header << "    using isize = signed " << match_integer_size(sizeof(void*)) << ";" << std::endl;
    
    header << "}" << std::endl;
    
    std::ofstream header_file(outpath, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
    if (header_file.is_open())
    {
        std::string data = header.str();
        header_file.write(data.data(), data.size());
        header_file.close();
    }
    
    return 0;
}
