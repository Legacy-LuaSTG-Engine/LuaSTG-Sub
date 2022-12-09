#pragma once
#include <string>
#include <string_view>

namespace Core
{
    struct InitializeConfigure
    {
        std::string gpu;
        int width = 640;
        int height = 480;
        int refresh_rate_numerator = 0;
        int refresh_rate_denominator = 0;
        bool windowed = true;
        bool vsync = false;

        void reset();
        bool load(std::string_view const& source) noexcept;
        bool save(std::string& buffer);
        bool loadFromFile(std::string_view const& path) noexcept;
        bool saveToFile(std::string_view const& path) noexcept;
    };
}
