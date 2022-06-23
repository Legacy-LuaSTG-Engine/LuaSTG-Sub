#pragma once

namespace imgui
{
    void loadConfig();
    void saveConfig();
    
    void bindEngine();
    void unbindEngine();
    
    void cancelSetCursor();
    void updateEngine();
    void drawEngine();

    void showTestInputWindow(bool* p_open = nullptr);
};
