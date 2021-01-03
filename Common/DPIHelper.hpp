#pragma once

namespace native
{
    bool enableDpiAwareness();
    
    bool enableNonClientDpiScaling(void* window);
    
    float getDpiScalingForMonitor(void* monitor);
    
    float getDpiScalingForWindow(void* window);
};
