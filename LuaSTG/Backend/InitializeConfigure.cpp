#include "Core/InitializeConfigure.hpp"
#include <fstream>
#include <filesystem>
#include "nlohmann/json.hpp"
#include "utility/encoding.hpp"

namespace LuaSTG::Core
{
    void InitializeConfigure::reset()
    {
        gpu.clear();
        width = 640;
        height = 480;
        refresh_rate_numerator = 0;
        refresh_rate_denominator = 0;
        windowed = true;
        vsync = false;
        dgpu_trick = false;
    }
    bool InitializeConfigure::load(std::string_view const& source) noexcept
    {
        try
        {
            nlohmann::json json = nlohmann::json::parse(source);
            gpu = json["gpu"].get<std::string>();
            width = json["width"].get<int>();
            height = json["height"].get<int>();
            refresh_rate_numerator = json["refresh_rate_numerator"].get<int>();
            refresh_rate_denominator = json["refresh_rate_denominator"].get<int>();
            windowed = json["windowed"].get<bool>();
            vsync = json["vsync"].get<bool>();
            dgpu_trick = json["dgpu_trick"].get<bool>();
            return true;
        }
        catch (...)
        {
            reset();
            return false;
        }
    }
    bool InitializeConfigure::save(std::string& buffer)
    {
        try
        {
            nlohmann::json json;
            json["gpu"] = gpu;
            json["width"] = width;
            json["height"] = height;
            json["refresh_rate_numerator"] = refresh_rate_numerator;
            json["refresh_rate_denominator"] = refresh_rate_denominator;
            json["windowed"] = windowed;
            json["vsync"] = vsync;
            json["dgpu_trick"] = dgpu_trick;
            buffer = std::move(json.dump());
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    bool InitializeConfigure::loadFromFile(std::string_view const& path) noexcept
    {
        try
        {
            std::wstring wpath(utility::encoding::to_wide(path));
            if (!std::filesystem::is_regular_file(wpath))
            {
                return false;
            }
            std::ifstream file(wpath, std::ios::in | std::ios::binary);
            if (!file.is_open())
            {
                return false;
            }
            nlohmann::json json;
            file >> json;
            gpu = json["gpu"].get<std::string>();
            width = json["width"].get<int>();
            height = json["height"].get<int>();
            refresh_rate_numerator = json["refresh_rate_numerator"].get<int>();
            refresh_rate_denominator = json["refresh_rate_denominator"].get<int>();
            windowed = json["windowed"].get<bool>();
            vsync = json["vsync"].get<bool>();
            dgpu_trick = json["dgpu_trick"].get<bool>();
            return true;
        }
        catch (...)
        {
            reset();
            return false;
        }
    }
    bool InitializeConfigure::saveToFile(std::string_view const& path) noexcept
    {
        try
        {
            nlohmann::json json;
            json["gpu"] = gpu;
            json["width"] = width;
            json["height"] = height;
            json["refresh_rate_numerator"] = refresh_rate_numerator;
            json["refresh_rate_denominator"] = refresh_rate_denominator;
            json["windowed"] = windowed;
            json["vsync"] = vsync;
            json["dgpu_trick"] = dgpu_trick;
            std::wstring wpath(utility::encoding::to_wide(path));
            std::ofstream file(wpath, std::ios::out | std::ios::binary | std::ios::trunc);
            if (!file.is_open())
            {
                return false;
            }
            file << json;
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
}
