#include "ImGuiExtension.h"
#include <string>
#include <fstream>
#include <filesystem>

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_freetype.h"
#include "imgui_impl_win32ex.h"
#include "imgui_impl_dx9.h"

#include "lua_imgui.hpp"
#include "lua_imgui_type.hpp"

#include "Common/DPIHelper.hpp"

#include "Config.h"
#include "SystemDirectory.hpp"
#include "AppFrame.h"

#define XINPUT_USE_9_1_0
#include <Xinput.h>

// lua imgui backend binding

static int lib_NewFrame(lua_State* L)
{
    imgui::updateEngine();
    return 0;
}
static int lib_RenderDrawData(lua_State* L)
{
    imgui::drawEngine();
    return 0;
}

static int lib_ShowTestInputWindow(lua_State* L)
{
    if(lua_gettop(L) >= 1)
    {
        bool v = lua_toboolean(L, 1);
        imgui::showTestInputWindow(&v);
        lua_pushboolean(L, v);
        return 1;
    }
    else
    {
        imgui::showTestInputWindow();
        return 0;
    }
}

void imgui_binding_lua_register_backend(lua_State* L)
{
    const luaL_Reg lib_fun[] = {
        {"NewFrame", &lib_NewFrame},
        {"RenderDrawData", &lib_RenderDrawData},
        {"ShowTestInputWindow", &lib_ShowTestInputWindow},
        {NULL, NULL},
    };
    const auto lib_func = (sizeof(lib_fun) / sizeof(luaL_Reg)) - 1;
    
    //                                      // ? m
    lua_pushstring(L, "backend");           // ? m k
    lua_createtable(L, 0, lib_func);        // ? m k t
    luaL_setfuncs(L, lib_fun, 0);           // ? m k t
    lua_settable(L, -3);                    // ? m
}

// imgui backend binding

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32Ex_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define APP LuaSTGPlus::AppFrame::GetInstance()

namespace imgui
{
    static bool g_ImGuiBindEngine = false;
    static HMODULE g_hXinput = NULL;
    static DWORD (WINAPI *g_fXInputGetState)(DWORD, XINPUT_STATE*) = NULL;
    
    class ImGuiRenderDeviceEventListener : public f2dRenderDeviceEventListener
    {
    public:
        void OnRenderDeviceLost()
        {
            ImGui_ImplDX9_Shutdown();
        }
        void OnRenderDeviceReset()
        {
            IDirect3DDevice9* device = (IDirect3DDevice9*)APP.GetRenderDev()->GetHandle();
            ImGui_ImplDX9_Init(device);
        }
    };
    static ImGuiRenderDeviceEventListener g_ImGuiRenderDeviceEventListener;
    
    void loadConfig()
    {
        if (ImGui::GetCurrentContext())
        {
            ImGuiIO& io = ImGui::GetIO();
            // handle imgui config data
            io.IniFilename = NULL;
            std::wstring path;
            #ifdef USING_SYSTEM_DIRECTORY
                if (app::makeApplicationRoamingAppDataDirectory(APP_COMPANY, APP_PRODUCT, path))
                {
                    path.push_back(L'\\');
                }
            #endif
            path.append(L"imgui.ini");
            std::ifstream file(path, std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                file.seekg(0, std::ios::end);
                auto p2 = file.tellg();
                file.seekg(0, std::ios::beg);
                auto p1 = file.tellg();
                auto dp = p2 - p1;
                std::string buffer;
                buffer.resize((size_t)dp);
                file.read((char*)buffer.data(), dp);
                file.close();
                ImGui::LoadIniSettingsFromMemory(buffer.data(), (size_t)dp);
            }
        }
    }
    void saveConfig()
    {
        if (ImGui::GetCurrentContext())
        {
            std::wstring path;
            #ifdef USING_SYSTEM_DIRECTORY
                if (app::makeApplicationRoamingAppDataDirectory(APP_COMPANY, APP_PRODUCT, path))
                {
                    path.push_back(L'\\');
                }
            #endif
            path.append(L"imgui.ini");
            std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
            if (file.is_open())
            {
                size_t length = 0;
                const char* buffer = ImGui::SaveIniSettingsToMemory(&length);
                file.write(buffer, (std::streamsize)length);
                file.close();
            }
        }
    }
    
    void setConfig()
    {
        ImGuiIO& io = ImGui::GetIO();
        
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
        
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        
        style.WindowRounding = 0.0f;
        style.ChildRounding = 0.0f;
        style.PopupRounding = 0.0f;
        style.FrameRounding = 0.0f;
        style.ScrollbarRounding = 0.0f;
        style.GrabRounding = 0.0f;
        style.TabRounding = 0.0f;
        
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.TabBorderSize = 1.0f;
        
        if (true)
        {
            ImFontConfig cfg;
            cfg.OversampleH = 1;
            std::string fontpath = "C:\\Windows\\Fonts\\msyh.ttc";
            if (!std::filesystem::is_regular_file(fontpath))
            {
                fontpath = "C:\\Windows\\Fonts\\msyh.ttf"; // Windows 7
            }
            if (std::filesystem::is_regular_file(fontpath))
            {
                io.Fonts->AddFontFromFileTTF(
                    fontpath.c_str(),
                    16.0f * native::getDpiScalingForWindow((void*)APP.GetEngine()->GetMainWindow()->GetHandle()),
                    &cfg,
                    io.Fonts->GetGlyphRangesDefault());
                io.Fonts->AddFontDefault();
            }
            else
            {
                io.Fonts->AddFontDefault(); // fallback
            }
        }
    }
    
    void bindEngine()
    {
        auto* window = APP.GetEngine()->GetMainWindow();
        auto* device = APP.GetRenderDev();
        auto* L = APP.GetLuaEngine();
        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        
        setConfig();
        loadConfig();
        
        ImGui_ImplWin32Ex_Init((void*)window->GetHandle());
        window->AddNativeMessageCallback((ptrdiff_t)&ImGui_ImplWin32Ex_WndProcHandler);
        
        ImGui_ImplDX9_Init((IDirect3DDevice9*)device->GetHandle());
        device->AttachListener(&g_ImGuiRenderDeviceEventListener);
        
        luaopen_imgui(L);
        imgui_binding_lua_register_backend(L);
        lua_pop(L, 1);
        
        typedef DWORD (__stdcall *f_XInputGetState)(DWORD, XINPUT_STATE*);
        
        g_hXinput = LoadLibraryW(L"xinput9_1_0.dll");
        if (g_hXinput) g_fXInputGetState = (f_XInputGetState)GetProcAddress(g_hXinput, "XInputGetState");
        
        g_ImGuiBindEngine = true;
    }
    void unbindEngine()
    {
        g_ImGuiBindEngine = false;
        
        g_fXInputGetState = NULL;
        if (g_hXinput) { FreeLibrary(g_hXinput); g_hXinput = NULL; }
        
        auto* window = APP.GetEngine()->GetMainWindow();
        auto* device = APP.GetRenderDev();
        auto* L = APP.GetLuaEngine();
        
        device->RemoveListener(&g_ImGuiRenderDeviceEventListener);
        ImGui_ImplDX9_Shutdown();
        
        window->RemoveNativeMessageCallback((ptrdiff_t)&ImGui_ImplWin32Ex_WndProcHandler);
        ImGui_ImplWin32Ex_Shutdown();
        
        saveConfig();
        ImGui::DestroyContext();
    }
    
    void updateEngine()
    {
        if (g_ImGuiBindEngine)
        {
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32Ex_NewFrame();
        }
    }
    void drawEngine()
    {
        if (g_ImGuiBindEngine)
        {
            auto& engine = APP;
            
            // 终止渲染过程
            bool bRestartRenderPeriod = false;
            switch (engine.GetGraphicsType())
            {
            case LuaSTGPlus::GraphicsType::Graph2D:
                if (engine.GetGraphics2D()->IsInRender())
                {
                    bRestartRenderPeriod = true;
                    engine.GetGraphics2D()->End();
                }
                break;
            case LuaSTGPlus::GraphicsType::Graph3D:
                if (engine.GetGraphics3D()->IsInRender())
                {
                    bRestartRenderPeriod = true;
                    engine.GetGraphics3D()->End();
                }
                break;
            }
            
            // 绘制GUI数据
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            
            // 重启渲染过程
            if (bRestartRenderPeriod)
            {
                switch (engine.GetGraphicsType())
                {
                case LuaSTGPlus::GraphicsType::Graph2D:
                    engine.GetGraphics2D()->Begin();
                    break;
                case LuaSTGPlus::GraphicsType::Graph3D:
                    engine.GetGraphics3D()->Begin();
                    break;
                }
            }
        }
    }
    
    void showTestInputWindow(bool* p_open)
    {
        auto* __p = APP.GetDInput();
        if (__p == nullptr) return;
        auto& dinput = *__p;
        
        static bool _first_set_size = false;
        if (!_first_set_size)
        {
            ImGui::SetNextWindowContentSize(ImVec2(640.0f, 480.0f));
            _first_set_size = true;
        }
        bool show = ImGui::Begin("InputTest##80FF", p_open);
        if (show)
        {
            static char buffer[1024] = { 0 };
            static wchar_t wbuffer[1024] = { 0 };
            static std::vector<std::string> combo_str;
            static std::vector<char*> combo_data;
            static int current_didx = 0;
            static int current_xidx = 0;
            
            XINPUT_STATE xstate[4];
            ZeroMemory(xstate, sizeof(xstate));
            bool bxstate[4] = { false, false, false, false };
            DWORD xdevice = 0;
            for (size_t i = 0; i < 4; i += 1)
            {
                auto hr = g_fXInputGetState(i, xstate);
                if (hr == ERROR_SUCCESS)
                {
                    bxstate[i] = true;
                    xdevice += 1;
                }
            }
            
            if (ImGui::BeginTabBar("##8010"))
            {
                if (ImGui::BeginTabItem(u8"DirectInput##8011"))
                {
                    {
                        const auto cnt = dinput.count();
                        combo_str.resize(cnt);
                        combo_data.resize(cnt);
                        for (size_t i = 0; i < cnt; i += 1)
                        {
                            auto s1 = dinput.getDeviceName(i);
                            auto s2 = dinput.getProductName(i);
                            swprintf(wbuffer, 1023, L"%u. %s (%s)", i + 1, s1 ? s1 : L"<null>", s2 ? s2 : L"<null>");
                            int need = WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, NULL, 0, NULL, NULL);
                            combo_str[i].resize(need);
                            WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, (LPSTR)combo_str[i].data(), need, NULL, NULL);
                            combo_data[i] = (char*)combo_str[i].c_str();
                        }
                        
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                        ImGui::Combo(u8"Devices", &current_didx, combo_data.data(), combo_data.size());
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
                        if (ImGui::Button(u8"Refresh"))
                        {
                            dinput.refresh();
                        }
                        
                        native::DirectInput::RawState state;
                        if (dinput.getRawState(current_didx, &state))
                        {
                            native::DirectInput::AxisRange range;
                            dinput.getAxisRange(current_didx, &range);
                            
                            int cache = 0;
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.lX;
                            ImGui::SliderInt(u8"Axis X", &cache, range.XMin, range.XMax);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.lY;
                            ImGui::SliderInt(u8"Axis Y", &cache, range.YMin, range.YMax);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.lZ;
                            ImGui::SliderInt(u8"Axis Z", &cache, range.ZMin, range.ZMax);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.lRx;
                            ImGui::SliderInt(u8"Axis RX", &cache, range.RxMin, range.RxMax);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.lRy;
                            ImGui::SliderInt(u8"Axis RY", &cache, range.RyMin, range.RyMax);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.lRz;
                            ImGui::SliderInt(u8"Axis RZ", &cache, range.RzMin, range.RzMax);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.rglSlider[0];
                            ImGui::SliderInt(u8"Slider 1", &cache, range.Slider0Min, range.Slider0Max);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.rglSlider[1];
                            ImGui::SliderInt(u8"Slider 2", &cache, range.Slider1Min, range.Slider1Max);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = (state.rgdwPOV[0] <= 36000) ? state.rgdwPOV[0] : 0;
                            ImGui::SliderInt(u8"POV 1", &cache, 0, (state.rgdwPOV[0] <= 36000) ? 36000 : 0);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = (state.rgdwPOV[1] <= 36000) ? state.rgdwPOV[1] : 0;
                            ImGui::SliderInt(u8"POV 2", &cache, 0, (state.rgdwPOV[1] <= 36000) ? 36000 : 0);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = (state.rgdwPOV[2] <= 36000) ? state.rgdwPOV[2] : 0;
                            ImGui::SliderInt(u8"POV 3", &cache, 0, (state.rgdwPOV[2] <= 36000) ? 36000 : 0);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = (state.rgdwPOV[3] <= 36000) ? state.rgdwPOV[3] : 0;
                            ImGui::SliderInt(u8"POV 4", &cache, 0, (state.rgdwPOV[3] <= 36000) ? 36000 : 0);
                            
                            bool bcache = false;
                            
                            #define SHOWKEY(I, B) \
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.125f);\
                                bcache = (state.rgbButtons[I - 1] != 0);\
                                if ((I) < 10) ImGui::Checkbox(u8"Button 0" u8#I, &bcache);\
                                else ImGui::Checkbox(u8"Button " u8#I, &bcache);\
                                if (B) ImGui::SameLine();
                            
                            SHOWKEY( 1, 1); SHOWKEY( 2, 1); SHOWKEY( 3, 1); SHOWKEY( 4, 1); SHOWKEY( 5, 1); SHOWKEY( 6, 1); SHOWKEY( 7, 1); SHOWKEY( 8, 0);
                            SHOWKEY( 9, 1); SHOWKEY(10, 1); SHOWKEY(11, 1); SHOWKEY(12, 1); SHOWKEY(13, 1); SHOWKEY(14, 1); SHOWKEY(15, 1); SHOWKEY(16, 0);
                            SHOWKEY(17, 1); SHOWKEY(18, 1); SHOWKEY(19, 1); SHOWKEY(20, 1); SHOWKEY(21, 1); SHOWKEY(22, 1); SHOWKEY(23, 1); SHOWKEY(24, 0);
                            SHOWKEY(25, 1); SHOWKEY(26, 1); SHOWKEY(27, 1); SHOWKEY(28, 1); SHOWKEY(29, 1); SHOWKEY(30, 1); SHOWKEY(31, 1); SHOWKEY(32, 0);
                            
                            #undef SHOWKEY
                        }
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(u8"DirectInput to XInput##8012"))
                {
                    {
                        const auto cnt = dinput.count();
                        combo_str.resize(cnt);
                        combo_data.resize(cnt);
                        for (size_t i = 0; i < cnt; i += 1)
                        {
                            auto s1 = dinput.getDeviceName(i);
                            auto s2 = dinput.getProductName(i);
                            swprintf(wbuffer, 1023, L"%u. %s (%s)", i + 1, s1 ? s1 : L"<null>", s2 ? s2 : L"<null>");
                            int need = WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, NULL, 0, NULL, NULL);
                            combo_str[i].resize(need);
                            WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, (LPSTR)combo_str[i].data(), need, NULL, NULL);
                            combo_data[i] = (char*)combo_str[i].c_str();
                        }
                        
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                        ImGui::Combo(u8"Devices", &current_didx, combo_data.data(), combo_data.size());
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
                        if (ImGui::Button(u8"Refresh"))
                        {
                            dinput.refresh();
                        }
                        
                        native::DirectInput::State state;
                        if (dinput.getState(current_didx, &state))
                        {
                            SHORT cache[2];
                            SHORT minv = -32768, maxv = 32767;
                            BYTE bcache = 0;
                            BYTE bminv = 0, bmaxv = 255;
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache[0] = state.sThumbLX; cache[1] = state.sThumbLY;
                            ImGui::SliderScalarN(u8"Left Joystick (LJ)", ImGuiDataType_S16, cache, 2, &minv, &maxv);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache[0] = state.sThumbRX; cache[1] = state.sThumbRY;
                            ImGui::SliderScalarN(u8"Right Joystick (RJ)", ImGuiDataType_S16, cache, 2, &minv, &maxv);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            bcache = state.bLeftTrigger;
                            ImGui::SliderScalar(u8"Left Trigger（LT）", ImGuiDataType_U8, &bcache, &bminv, &bmaxv);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            bcache = state.bRightTrigger;
                            ImGui::SliderScalar(u8"Right Trigger（RT）", ImGuiDataType_U8, &bcache, &bminv, &bmaxv);
                            
                            bool _bstate = false;
                            
                            #define SHOWKEY(L, NAME, C) \
                                _bstate = ((state.wButtons & C) != 0);\
                                ImGui::Checkbox(u8#NAME, &_bstate);\
                                if ((L) != 0) ImGui::SameLine();
                            
                            SHOWKEY(1,    UP, XINPUT_GAMEPAD_DPAD_UP       );
                            SHOWKEY(1,  DOWN, XINPUT_GAMEPAD_DPAD_DOWN     );
                            SHOWKEY(1,  LEFT, XINPUT_GAMEPAD_DPAD_LEFT     );
                            SHOWKEY(0, RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT    );
                            
                            SHOWKEY(1, START, XINPUT_GAMEPAD_START         );
                            SHOWKEY(0,  BACK, XINPUT_GAMEPAD_BACK          );
                            
                            SHOWKEY(1,  Left Thumb (LJB), XINPUT_GAMEPAD_LEFT_THUMB    );
                            SHOWKEY(0, Right Thumb (RJB), XINPUT_GAMEPAD_RIGHT_THUMB   );
                            
                            SHOWKEY(1,  Left Shoulder (LB), XINPUT_GAMEPAD_LEFT_SHOULDER );
                            SHOWKEY(0, Right Shoulder (RB), XINPUT_GAMEPAD_RIGHT_SHOULDER);
                            
                            SHOWKEY(1, A, XINPUT_GAMEPAD_A             );
                            SHOWKEY(1, B, XINPUT_GAMEPAD_B             );
                            SHOWKEY(1, X, XINPUT_GAMEPAD_X             );
                            SHOWKEY(0, Y, XINPUT_GAMEPAD_Y             );
                            
                            #undef SHOWKEY
                        }
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(u8"XInput##8013"))
                {
                    {
                        combo_str.resize(xdevice);
                        combo_data.resize(xdevice);
                        for (size_t i = 0; i < xdevice; i += 1)
                        {
                            snprintf(buffer, 1023, u8"%u. %s", i + 1, u8"XBox Controller & XInput Compatible Controller");
                            combo_str[i] = buffer;
                            combo_data[i] = (char*)combo_str[i].c_str();
                        }
                        
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                        ImGui::Combo(u8"Devices", &current_xidx, combo_data.data(), combo_data.size());
                        
                        if (current_xidx < xdevice)
                        {
                            auto& state = xstate[current_xidx].Gamepad;
                            
                            SHORT cache[2];
                            SHORT minv = -32768, maxv = 32767;
                            BYTE bcache = 0;
                            BYTE bminv = 0, bmaxv = 255;
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache[0] = state.sThumbLX; cache[1] = state.sThumbLY;
                            ImGui::SliderScalarN(u8"Left Joystick (LJ)", ImGuiDataType_S16, cache, 2, &minv, &maxv);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache[0] = state.sThumbRX; cache[1] = state.sThumbRY;
                            ImGui::SliderScalarN(u8"Right Joystick (RJ)", ImGuiDataType_S16, cache, 2, &minv, &maxv);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            bcache = state.bLeftTrigger;
                            ImGui::SliderScalar(u8"Left Trigger（LT）", ImGuiDataType_U8, &bcache, &bminv, &bmaxv);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            bcache = state.bRightTrigger;
                            ImGui::SliderScalar(u8"Right Trigger（RT）", ImGuiDataType_U8, &bcache, &bminv, &bmaxv);
                            
                            bool _bstate = false;
                            
                            #define SHOWKEY(L, NAME, C) \
                                _bstate = ((state.wButtons & C) != 0);\
                                ImGui::Checkbox(u8#NAME, &_bstate);\
                                if ((L) != 0) ImGui::SameLine();
                            
                            SHOWKEY(1,    UP, XINPUT_GAMEPAD_DPAD_UP       );
                            SHOWKEY(1,  DOWN, XINPUT_GAMEPAD_DPAD_DOWN     );
                            SHOWKEY(1,  LEFT, XINPUT_GAMEPAD_DPAD_LEFT     );
                            SHOWKEY(0, RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT    );
                            
                            SHOWKEY(1, START, XINPUT_GAMEPAD_START         );
                            SHOWKEY(0,  BACK, XINPUT_GAMEPAD_BACK          );
                            
                            SHOWKEY(1,  Left Thumb (LJB), XINPUT_GAMEPAD_LEFT_THUMB    );
                            SHOWKEY(0, Right Thumb (RJB), XINPUT_GAMEPAD_RIGHT_THUMB   );
                            
                            SHOWKEY(1,  Left Shoulder (LB), XINPUT_GAMEPAD_LEFT_SHOULDER );
                            SHOWKEY(0, Right Shoulder (RB), XINPUT_GAMEPAD_RIGHT_SHOULDER);
                            
                            SHOWKEY(1, A, XINPUT_GAMEPAD_A             );
                            SHOWKEY(1, B, XINPUT_GAMEPAD_B             );
                            SHOWKEY(1, X, XINPUT_GAMEPAD_X             );
                            SHOWKEY(0, Y, XINPUT_GAMEPAD_Y             );
                            
                            #undef SHOWKEY
                        }
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(u8"Keyboard##8014"))
                {
                    for (int i = 0; i < 256; i += 1)
                    {
                        if (dinput.getKeyboardKeyState(i))
                        {
                            ImGui::Text(u8"[%d]", i);
                        }
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(u8"Mouse##8014"))
                {
                    const auto x = dinput.getMouseMoveDeltaX();
                    const auto y = dinput.getMouseMoveDeltaY();
                    int xy[2] = { x, y };
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                    ImGui::DragInt2(u8"Move Delta", xy);
                    
                    static int g_x = 0;
                    static int g_y = 0;
                    g_x += x;
                    g_y += y;
                    int g_xy[2] = { g_x, g_y };
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                    ImGui::DragInt2(u8"Total Move", g_xy);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
                    if (ImGui::Button(u8"Reset##1"))
                    {
                        g_x = 0;
                        g_y = 0;
                    }
                    
                    int z = dinput.getMouseWheelDelta();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                    ImGui::DragInt(u8"Wheel Delta", &z);
                    
                    static int g_z = 0;
                    g_z += z;
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                    int _g_z = g_z;
                    ImGui::DragInt(u8"Total Wheel", &_g_z);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
                    if (ImGui::Button(u8"Reset##2"))
                    {
                        g_z = 0;
                    }
                    
                    for (int i = 0; i < 8; i += 1)
                    {
                        if (dinput.getMouseKeyState(i))
                        {
                            ImGui::Text(u8"[%d]", i);
                        }
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
};
