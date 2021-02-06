#pragma once

namespace native
{
    bool enableDpiAwareness();
    
    bool enableNonClientDpiScaling(void* window);
    
    unsigned int getDpiForMonitor(void* monitor);
    
    unsigned int getDpiForWindow(void* window);
    
    float getDpiScalingForMonitor(void* monitor);
    
    float getDpiScalingForWindow(void* window);
    
    namespace Windows
    {
        constexpr unsigned int WM_DPICHANGED_T = 0x02E0;
        
        struct RECT
        {
            long left;
            long top;
            long right;
            long bottom;
        };
        
        int AdjustWindowRectExForDpi(void* lpRect, unsigned long dwStyle, int bMenu, unsigned long dwExStyle, unsigned int dpi = 0);
    };
};
