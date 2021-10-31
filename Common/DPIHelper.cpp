#include "Common/DPIHelper.hpp"
#include <Windows.h>

#define HAVE_MANIFEST 1

namespace native
{
    namespace Windows
    {
        // -- Windows XP or early -- //
        
        // HDC
        
        // user32.dll
        // MonitorFromWindow
        
        // user32.dll
        // GetDC
        
        // gdi32.dll
        typedef int (WINAPI *function_GetDeviceCaps)(HDC hdc, int index);
        
        // -- Windows Vista -- //
        
        // user32.dll
        typedef BOOL (WINAPI *function_SetProcessDPIAware)();
        
        // -- Windows 8.1
        
        enum PROCESS_DPI_AWARENESS {
            PROCESS_DPI_UNAWARE,
            PROCESS_SYSTEM_DPI_AWARE,
            PROCESS_PER_MONITOR_DPI_AWARE,
        };
        
        enum MONITOR_DPI_TYPE {
            MDT_EFFECTIVE_DPI,
            MDT_ANGULAR_DPI,
            MDT_RAW_DPI,
            MDT_DEFAULT,
        };
        
        // SHCore.dll
        typedef HRESULT (WINAPI *function_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS value);
        
        // SHCore.dll
        typedef HRESULT (WINAPI *function_GetDpiForMonitor)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY);
        
        // -- Windows 10 -- //
        
        DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
        
        const DPI_AWARENESS_CONTEXT handle_DPI_AWARENESS_CONTEXT_UNAWARE              = ((DPI_AWARENESS_CONTEXT)-1);
        const DPI_AWARENESS_CONTEXT handle_DPI_AWARENESS_CONTEXT_SYSTEM_AWARE         = ((DPI_AWARENESS_CONTEXT)-2);
        const DPI_AWARENESS_CONTEXT handle_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    = ((DPI_AWARENESS_CONTEXT)-3);
        const DPI_AWARENESS_CONTEXT handle_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = ((DPI_AWARENESS_CONTEXT)-4);
        const DPI_AWARENESS_CONTEXT handle_DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED    = ((DPI_AWARENESS_CONTEXT)-5);
        
        // Windows 10 1607 user32.dll
        typedef DPI_AWARENESS_CONTEXT (WINAPI *function_SetThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT dpiContext);
        
        // Windows 10 1703 user32.dll
        typedef BOOL (WINAPI *function_SetProcessDpiAwarenessContext)(DPI_AWARENESS_CONTEXT value);
        
        // Windows 10 1607 user32.dll
        typedef BOOL (WINAPI *function_EnableNonClientDpiScaling)(HWND hwnd);
        
        // Windows 10 1607 user32.dll
        typedef UINT (WINAPI *function_GetDpiForWindow)(HWND hwnd);
        
        // Windows 10 1607 user32.dll
        typedef BOOL (WINAPI *function_AdjustWindowRectExForDpi)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);
        
        // -- host -- //
        
        #ifndef USER_DEFAULT_SCREEN_DPI
        #define USER_DEFAULT_SCREEN_DPI 96
        #endif
        
        struct DpiHelper
        {
            HMODULE user32;
            HMODULE SHCore;
            HMODULE gdi32;
            
            function_SetProcessDPIAware             SetProcessDPIAware;
            function_SetProcessDpiAwareness         SetProcessDpiAwareness;
            function_SetProcessDpiAwarenessContext  SetProcessDpiAwarenessContext;
            
            function_EnableNonClientDpiScaling      EnableNonClientDpiScaling;
            
            function_GetDpiForWindow                GetDpiForWindow;
            function_GetDpiForMonitor               GetDpiForMonitor;
            function_GetDeviceCaps                  GetDeviceCaps;
            function_AdjustWindowRectExForDpi       AdjustWindowRectExForDpi;
            
            BOOL lstate;
            BOOL estate;
            
            void load()
            {
                if (lstate == FALSE)
                {
                    user32 = ::LoadLibraryA("user32.dll");
                    if (user32 == NULL) user32 = ::LoadLibraryA("User32.dll"); // ?
                    SHCore = ::LoadLibraryA("SHCore.dll");
                    if (SHCore == NULL) SHCore = ::LoadLibraryA("Shcore.dll"); // ?
                    gdi32 = ::LoadLibraryA("gdi32.dll");
                    if (user32)
                    {
                        SetProcessDPIAware = (function_SetProcessDPIAware)
                        GetProcAddress(user32, "SetProcessDPIAware");
                        SetProcessDpiAwarenessContext = (function_SetProcessDpiAwarenessContext)
                        GetProcAddress(user32, "SetProcessDpiAwarenessContext");
                        
                        EnableNonClientDpiScaling = (function_EnableNonClientDpiScaling)
                        GetProcAddress(user32, "EnableNonClientDpiScaling");
                        
                        GetDpiForWindow = (function_GetDpiForWindow)
                        GetProcAddress(user32, "GetDpiForWindow");
                        AdjustWindowRectExForDpi = (function_AdjustWindowRectExForDpi)
                        GetProcAddress(user32, "AdjustWindowRectExForDpi");
                    }
                    if (SHCore)
                    {
                        SetProcessDpiAwareness = (function_SetProcessDpiAwareness)
                        GetProcAddress(SHCore, "SetProcessDpiAwareness");
                        
                        GetDpiForMonitor = (function_GetDpiForMonitor)
                        GetProcAddress(SHCore, "GetDpiForMonitor");
                    }
                    if (gdi32)
                    {
                        GetDeviceCaps = (function_GetDeviceCaps)
                        GetProcAddress(gdi32, "GetDeviceCaps");
                    }
                    lstate = TRUE;
                }
            };
            
            bool enableDpiAwareness()
            {
                load();
                if (estate == TRUE) return true;
                if (SetProcessDpiAwarenessContext)
                {
                    if (FALSE != SetProcessDpiAwarenessContext(handle_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
                    {
                        estate = TRUE;
                        return true;
                    }
                    if (ERROR_ACCESS_DENIED == ::GetLastError())
                    {
                        // is dpi awareness
                        estate = TRUE;
                        return true;
                    }
                }
                if (SetProcessDpiAwareness)
                {
                    HRESULT hr = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
                    if (hr == S_OK || hr == E_ACCESSDENIED)
                    {
                        estate = TRUE;
                        return true;
                    }
                }
                if (SetProcessDPIAware)
                {
                    if (FALSE != SetProcessDPIAware())
                    {
                        estate = TRUE;
                        return true;
                    }
                }
                return false;
            };
            bool enableNonClientDpiScaling(HWND window)
            {
                load();
                if (EnableNonClientDpiScaling)
                {
                    if (FALSE != EnableNonClientDpiScaling(window))
                    {
                        return true;
                    }
                }
                return false;
            }
            unsigned int getDpiForMonitor(HMONITOR monitor)
            {
                #ifndef HAVE_MANIFEST
                if (estate != TRUE) return USER_DEFAULT_SCREEN_DPI;
                #endif
                load();
                // Windows 8.1 or later
                if (GetDpiForMonitor)
                {
                    UINT dpi_x = 0, dpi_y = 0;
                    // will dpi_x < 0 happen? I don't known
                    if (S_OK == GetDpiForMonitor(monitor, MDT_DEFAULT, &dpi_x, &dpi_y) && dpi_x > 0)
                    {
                        return dpi_x; // will dpi_x != dpi_y happen? I don't known
                    }
                }
                // Windows 2000, XP, Vista, 7
                if (GetDeviceCaps)
                {
                    HDC dc = ::GetDC(NULL);
                    if (dc)
                    {
                        int dpi_x = ::GetDeviceCaps(dc, LOGPIXELSX);
                        // int dpi_y = ::GetDeviceCaps(dc, LOGPIXELSY);
                        ::ReleaseDC(NULL, dc);
                        return (unsigned int)dpi_x; // will dpi_x != dpi_y happen? I don't known
                    }
                }
                // WTF???
                return USER_DEFAULT_SCREEN_DPI;
            };
            unsigned int getDpiForWindow(HWND window)
            {
                #ifndef HAVE_MANIFEST
                if (estate != TRUE) return USER_DEFAULT_SCREEN_DPI;
                #endif
                load();
                // Windows 10 1607 or later
                if (GetDpiForWindow)
                {
                    UINT dpi = GetDpiForWindow(window);
                    if (dpi > 0)
                    {
                        return dpi;
                    }
                }
                // Try another
                HMONITOR monitor = ::MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
                if (monitor)
                {
                    return getDpiForMonitor(monitor);
                }
                // WTF???
                return USER_DEFAULT_SCREEN_DPI;
            }
            float getDpiScalingForMonitor(HMONITOR monitor)
            {
                #ifndef HAVE_MANIFEST
                if (estate != TRUE) return 1.0f;
                #endif
                return (float)getDpiForMonitor(monitor) / (float)USER_DEFAULT_SCREEN_DPI;
            };
            float getDpiScalingForWindow(HWND window)
            {
                #ifndef HAVE_MANIFEST
                if (estate != TRUE) return 1.0f;
                #endif
                return (float)getDpiForWindow(window) / (float)USER_DEFAULT_SCREEN_DPI;
            }
            
            DpiHelper()
            {
                user32 = NULL;
                SHCore = NULL;
                gdi32  = NULL;
                
                SetProcessDPIAware            = NULL;
                SetProcessDpiAwareness        = NULL;
                SetProcessDpiAwarenessContext = NULL;
                EnableNonClientDpiScaling     = NULL;
                GetDpiForWindow               = NULL;
                GetDpiForMonitor              = NULL;
                GetDeviceCaps                 = NULL;
                AdjustWindowRectExForDpi      = NULL;
                
                lstate = FALSE;
                estate = FALSE;
            };
            ~DpiHelper()
            {
                lstate = FALSE;
                estate = FALSE;
                
                SetProcessDPIAware            = NULL;
                SetProcessDpiAwareness        = NULL;
                SetProcessDpiAwarenessContext = NULL;
                EnableNonClientDpiScaling     = NULL;
                GetDpiForWindow               = NULL;
                GetDpiForMonitor              = NULL;
                GetDeviceCaps                 = NULL;
                AdjustWindowRectExForDpi      = NULL;
                
                if (user32) ::FreeLibrary(user32); user32 = NULL;
                if (SHCore) ::FreeLibrary(SHCore); SHCore = NULL;
                if (gdi32 ) ::FreeLibrary(gdi32 ); gdi32  = NULL;
            };
        };
        
        static DpiHelper global_DPIAwarenessHelper;
    };
    
    // call it at the begin of "main" or "WinMain/wWinMain" function
    bool enableDpiAwareness()
    {
        return Windows::global_DPIAwarenessHelper.enableDpiAwareness();
    };
    
    // call it when recive WM_NCCREATE message and remember to call DefWindowProc
    bool enableNonClientDpiScaling(void* window)
    {
        return Windows::global_DPIAwarenessHelper.enableNonClientDpiScaling((HWND)window);
    };
    
    // HMONITOR monitor
    unsigned int getDpiForMonitor(void* monitor)
    {
        return Windows::global_DPIAwarenessHelper.getDpiForMonitor((HMONITOR)monitor);
    };
    
    // HWND window
    unsigned int getDpiForWindow(void* window)
    {
        return Windows::global_DPIAwarenessHelper.getDpiForWindow((HWND)window);
    };
    
    // HMONITOR monitor
    float getDpiScalingForMonitor(void* monitor)
    {
        return Windows::global_DPIAwarenessHelper.getDpiScalingForMonitor((HMONITOR)monitor);
    };
    
    // HWND window
    float getDpiScalingForWindow(void* window)
    {
        return Windows::global_DPIAwarenessHelper.getDpiScalingForWindow((HWND)window);
    };
    
    namespace Windows
    {
        int AdjustWindowRectExForDpi(void* lpRect, unsigned long dwStyle, int bMenu, unsigned long dwExStyle, unsigned int dpi)
        {
            global_DPIAwarenessHelper.load();
            if (dpi == 0)
                dpi = USER_DEFAULT_SCREEN_DPI;
            if (global_DPIAwarenessHelper.AdjustWindowRectExForDpi)
                return global_DPIAwarenessHelper.AdjustWindowRectExForDpi((::RECT*)lpRect, dwStyle, bMenu, dwExStyle, dpi);
            else
                return ::AdjustWindowRectEx((::RECT*)lpRect, dwStyle, bMenu, dwExStyle);
        }
    };
    
    #ifndef WM_DPICHANGED
    #define WM_DPICHANGED 0x02E0
    #endif
};
