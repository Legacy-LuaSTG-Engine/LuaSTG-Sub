#include <string>
#include <string_view>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
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
        if (std::string_view(argv[c]) == "-o")
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
