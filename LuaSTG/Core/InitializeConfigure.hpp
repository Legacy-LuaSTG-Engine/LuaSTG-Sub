#pragma once
#include <string>
#include <string_view>

namespace Core
{
    struct InitializeConfigure
    {
        std::string target_graphics_device;

        int canvas_width = 640;
        int canvas_height = 480;

        int target_window_width = 640;
        int target_window_height = 480;
        bool fullscreen_enable = false;
        bool vsync_enable = false;
        std::string window_title;
        bool window_cursor_enable = true;

        int target_frame_rate = 60;

        float music_channel_volume = 1.0f;
        float sound_effect_channel_volume = 1.0f;

        void reset();
        bool load(std::string_view const& source) noexcept;
        bool save(std::string& buffer);
        bool loadFromFile(std::string_view const& path) noexcept;
        bool saveToFile(std::string_view const& path) noexcept;
    };
}
