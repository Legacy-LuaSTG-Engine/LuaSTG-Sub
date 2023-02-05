#include "Core/InitializeConfigure.hpp"
#include <fstream>
#include <filesystem>
#include "nlohmann/json.hpp"
#include "utility/encoding.hpp"

namespace Core
{
    void to_json(nlohmann::json& j, InitializeConfigure const& p)
    {
        j["target_graphics_device"] = p.target_graphics_device;

        j["canvas_width"] = p.canvas_width;
        j["canvas_height"] = p.canvas_height;

        j["target_window_width"] = p.target_window_width;
        j["target_window_height"] = p.target_window_height;
        j["fullscreen_enable"] = p.fullscreen_enable;
        j["vsync_enable"] = p.vsync_enable;
        j["window_title"] = p.window_title;
        j["window_cursor_enable"] = p.window_cursor_enable;

        j["target_frame_rate"] = p.target_frame_rate;

        j["music_channel_volume"] = p.music_channel_volume;
        j["sound_effect_channel_volume"] = p.sound_effect_channel_volume;
    }
    void from_json(nlohmann::json const& j, InitializeConfigure& p)
    {
        j.at("target_graphics_device").get_to(p.target_graphics_device);

        j.at("canvas_width").get_to(p.canvas_width);
        j.at("canvas_height").get_to(p.canvas_height);

        j.at("target_window_width").get_to(p.target_window_width);
        j.at("target_window_height").get_to(p.target_window_height);
        j.at("fullscreen_enable").get_to(p.fullscreen_enable);
        j.at("vsync_enable").get_to(p.vsync_enable);
        j.at("window_title").get_to(p.window_title);
        j.at("window_cursor_enable").get_to(p.window_cursor_enable);

        j.at("target_frame_rate").get_to(p.target_frame_rate);

        j.at("music_channel_volume").get_to(p.music_channel_volume);
        j.at("sound_effect_channel_volume").get_to(p.sound_effect_channel_volume);
    }

    void InitializeConfigure::reset()
    {
        target_graphics_device.clear();

        canvas_width = 640;
        canvas_height = 480;

        target_window_width = 640;
        target_window_height = 480;
        fullscreen_enable = false;
        vsync_enable = false;
        window_title.clear();
        window_cursor_enable = true;

        target_frame_rate = 60;

        music_channel_volume = 1.0f;
        sound_effect_channel_volume = 1.0f;
    }
    bool InitializeConfigure::load(std::string_view const& source) noexcept
    {
        try
        {
            nlohmann::json json = nlohmann::json::parse(source);
            from_json(json, *this);
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
            to_json(json, *this);
            buffer = std::move(json.dump(2));
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
            from_json(json, *this);
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
            to_json(json, *this);
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
