#include "ImGuiExtension.h"
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_freetype.h"
#include "imgui_impl_win32ex.h"
#include "imgui_impl_dx11.h"
#include "implot.h"
#include <d3d11.h>

#include "lua.hpp"
#include "lua_imgui.hpp"
#include "lua_imgui_type.hpp"

#include "Common/SystemDirectory.hpp"

#include "AppFrame.h"

#define NOMINMAX
#define XINPUT_USE_9_1_0
#include <Xinput.h>

// lua imgui backend binding

static std::string bytes_count_to_string(DWORDLONG size)
{
    int count = 0;
    char buffer[64] = {};
    if (size < 1024) // B
    {
        count = std::snprintf(buffer, 64, "%u B", (unsigned int)size);
    }
    else if (size < (1024 * 1024)) // KB
    {
        count = std::snprintf(buffer, 64, "%.2f KB", (double)size / 1024.0);
    }
    else if (size < (1024 * 1024 * 1024)) // MB
    {
        count = std::snprintf(buffer, 64, "%.2f MB", (double)size / 1048576.0);
    }
    else // GB
    {
        count = std::snprintf(buffer, 64, "%.2f GB", (double)size / 1073741824.0);
    }
    return std::string(buffer, count);
}

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
static int lib_ShowMemoryUsageWindow(lua_State* L)
{
    bool v = (lua_gettop(L) >= 1) ? lua_toboolean(L, 1) : true;
    if (v)
    {
        if (ImGui::Begin("Memory Usage", &v))
        {
            MEMORYSTATUSEX info = { sizeof(MEMORYSTATUSEX) };
            BOOL bret = GlobalMemoryStatusEx(&info);
            f2dAdapterMemoryUsageStatistics gmuinfo = LAPP.GetRenderDev() ? LAPP.GetRenderDev()->GetAdapterMemoryUsageStatistics() : f2dAdapterMemoryUsageStatistics{};
            if (bret)
            {
                ImGui::Text("System Memory Usage: %u%%", info.dwMemoryLoad);
                ImGui::Text("Totoal Physical Memory: %s", bytes_count_to_string(info.ullTotalPhys).c_str());
                ImGui::Text("Avalid Physical Memory: %s", bytes_count_to_string(info.ullAvailPhys).c_str());
                ImGui::Text("Totoal Page File: %s", bytes_count_to_string(info.ullTotalPageFile).c_str());
                ImGui::Text("Avalid Page File: %s", bytes_count_to_string(info.ullAvailPageFile).c_str());
                ImGui::Text("Totoal User Mode Memory Space: %s", bytes_count_to_string(info.ullTotalVirtual).c_str());
                ImGui::Text("Avalid User Mode Memory Space: %s", bytes_count_to_string(info.ullAvailVirtual).c_str());
                ImGui::Text("Alloc* User Mode Memory Space: %s", bytes_count_to_string(info.ullTotalVirtual - info.ullAvailVirtual).c_str());

                ImGui::Text("Adapter Local Budget: %s", bytes_count_to_string(gmuinfo.local.budget).c_str());
                ImGui::Text("Adapter Local Usage: %s", bytes_count_to_string(gmuinfo.local.current_usage).c_str());
                ImGui::Text("Adapter Local Available For Reservation: %s", bytes_count_to_string(gmuinfo.local.available_for_reservation).c_str());
                ImGui::Text("Adapter Local Current Reservation: %s", bytes_count_to_string(gmuinfo.local.current_reservation).c_str());

                ImGui::Text("Adapter Non-Local Budget: %s", bytes_count_to_string(gmuinfo.non_local.budget).c_str());
                ImGui::Text("Adapter Non-Local Usage: %s", bytes_count_to_string(gmuinfo.non_local.current_usage).c_str());
                ImGui::Text("Adapter Non-Local Available For Reservation: %s", bytes_count_to_string(gmuinfo.non_local.available_for_reservation).c_str());
                ImGui::Text("Adapter Non-Local Current Reservation: %s", bytes_count_to_string(gmuinfo.non_local.current_reservation).c_str());

                if (ImGui::Button("Write To LOG File"))
                {
                    spdlog::info("[fancy2d] 系统内存使用情况：\n"
                        "    使用百分比：{}%\n"
                        "    总物理内存：{}\n"
                        "    剩余物理内存：{}\n"
                        "    当前进程可提交内存限制：{}\n"
                        "    当前进程剩余的可提交内存：{}\n"
                        "    当前进程用户模式内存空间限制*1：{}\n"
                        "    当前进程剩余的用户模式内存空间：{}\n"
                        "        *1 此项反映此程序实际上能用的最大内存，在 32 位应用程序上此项一般为 2 GB，修改 Windows 操作系统注册表后可能为 1 到 3 GB"
                        , info.dwMemoryLoad
                        , bytes_count_to_string(info.ullTotalPhys)
                        , bytes_count_to_string(info.ullAvailPhys)
                        , bytes_count_to_string(info.ullTotalPageFile)
                        , bytes_count_to_string(info.ullAvailPageFile)
                        , bytes_count_to_string(info.ullTotalVirtual)
                        , bytes_count_to_string(info.ullAvailVirtual)
                    );
                }
            }
        }
        ImGui::End();
    }
    lua_pushboolean(L, v);
    return 1;
}
static int lib_ShowFrameStatistics(lua_State* L)
{
    constexpr size_t arr_size = 3600;
    static bool is_init = false;
    static std::vector<double> arr_x(arr_size);
    static std::vector<double> arr_update_time(arr_size);
    static std::vector<double> arr_render_time(arr_size);
    static std::vector<double> arr_present_time(arr_size);
    static std::vector<double> arr_total_time(arr_size);
    static size_t arr_index = 0;
    static size_t record_range = 240;
    constexpr size_t record_range_min = 60;
    constexpr size_t record_range_max = 3600;
    static float height = 384.0f;
    static bool auto_fit = true;
    
    if (!is_init)
    {
        is_init = true;
        for (size_t x = 0; x < arr_size; x += 1)
        {
            arr_x[x] = (double)x;
        }
    }
    
    bool v = (lua_gettop(L) >= 1) ? lua_toboolean(L, 1) : true;
    if (v)
    {
        if (ImGui::Begin("Frame Statistics", &v))
        {
            f2dEngineFrameStatistics info = {};
            LuaSTGPlus::AppFrame::GetInstance().GetEngine()->GetFrameStatistics(info);
            
            ImGui::Text("Update : %.3fms", info.update_time  * 1000.0);
            ImGui::Text("Render : %.3fms", info.render_time  * 1000.0);
            ImGui::Text("Present: %.3fms", info.present_time * 1000.0);
            ImGui::Text("Total  : %.3fms", info.total_time   * 1000.0);
            
            ImGui::SliderScalar("Record Range", sizeof(size_t) == 8 ? ImGuiDataType_U64 : ImGuiDataType_U32, &record_range, &record_range_min, &record_range_max);
            record_range = std::clamp<size_t>(record_range, 2, record_range_max);
            ImGui::SliderFloat("Timeline Height", &height, 256.0f, 512.0f);
            ImGui::Checkbox("Auto-Fit Y Axis", &auto_fit);

            arr_update_time[arr_index] = 1000.0 * (info.update_time);
            arr_render_time[arr_index] = 1000.0 * (info.update_time + info.render_time);
            arr_present_time[arr_index] = 1000.0 * (info.update_time + info.render_time + info.present_time);
            arr_total_time[arr_index] = 1000.0 * (info.total_time);
            arr_index = (arr_index + 1) % record_range;

            if (ImPlot::BeginPlot("##Frame Statistics", ImVec2(-1, height), 0))
            {
                //ImPlot::SetupAxes("Frame", "Time", flags, flags);
                ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, (double)(record_range - 1), ImGuiCond_Always);
                //ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 1000.0 / 18.0, ImGuiCond_Always);
                if (auto_fit)
                    ImPlot::SetupAxes(NULL, NULL, ImPlotAxisFlags_None, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);
                else
                    ImPlot::SetupAxes(NULL, NULL);

                ImPlot::SetupLegend(ImPlotLocation_North, ImPlotLegendFlags_Horizontal | ImPlotLegendFlags_Outside);

                ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.5f);
                ImPlot::PlotShaded("Total", arr_x.data(), arr_present_time.data(), arr_total_time.data(), (int)record_range);
                ImPlot::PlotShaded("Present", arr_x.data(), arr_render_time.data(), arr_present_time.data(), (int)record_range);
                ImPlot::PlotShaded("Render", arr_x.data(), arr_update_time.data(), arr_render_time.data(), (int)record_range);
                ImPlot::PlotShaded("Update", arr_x.data(), arr_update_time.data(), (int)record_range);
                ImPlot::PopStyleVar();

                ImPlot::PlotLine("Total", arr_total_time.data(), (int)record_range);
                ImPlot::PlotLine("Present", arr_present_time.data(), (int)record_range);
                ImPlot::PlotLine("Render", arr_render_time.data(), (int)record_range);
                ImPlot::PlotLine("Update", arr_update_time.data(), (int)record_range);
                
                static double arr_ms[] = {
                    1000.0 / 60.0,
                    1000.0 / 30.0,
                    1000.0 / 20.0,
                };
                ImPlot::SetNextLineStyle(ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
                ImPlot::PlotHLines("##60 FPS", arr_ms, 1);
                //ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.2f, 0.2f, 1.0f));
                //ImPlot::PlotHLines("##30 FPS", arr_ms + 1, 1);
                //ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                //ImPlot::PlotHLines("##20 FPS", arr_ms + 2, 1);

                double mark = (double)arr_index;
                ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                ImPlot::PlotVLines("##Current Time", &arr_index, 1);

                ImPlot::EndPlot();
            }
        }
        ImGui::End();
    }
    lua_pushboolean(L, v);
    return 1;
}

void imgui_binding_lua_register_backend(lua_State* L)
{
    const luaL_Reg lib_fun[] = {
        {"NewFrame", &lib_NewFrame},
        {"RenderDrawData", &lib_RenderDrawData},
        {"ShowTestInputWindow", &lib_ShowTestInputWindow},
        {"ShowMemoryUsageWindow", &lib_ShowMemoryUsageWindow},
        {"ShowFrameStatistics", &lib_ShowFrameStatistics},
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
    static bool g_ImGuiTexIDValid = false;
    static HMODULE g_hXinput = NULL;
    static DWORD (WINAPI *g_fXInputGetState)(DWORD, XINPUT_STATE*) = NULL;
    
    class ImGuiRenderDeviceEventListener : public f2dRenderDeviceEventListener
    {
    public:
        void OnRenderDeviceLost()
        {
            g_ImGuiTexIDValid = false;
            ImGui_ImplDX11_Shutdown();
        }
        void OnRenderDeviceReset()
        {
            g_ImGuiTexIDValid = false;
            ID3D11Device* device = (ID3D11Device*)APP.GetRenderDev()->GetHandle();
            ID3D11DeviceContext* context = NULL;
            device->GetImmediateContext(&context);
            ImGui_ImplDX11_Init(device, context);
            context->Release();
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
                if (windows::makeApplicationRoamingAppDataDirectory(APP_COMPANY, APP_PRODUCT, path))
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
                if (windows::makeApplicationRoamingAppDataDirectory(APP_COMPANY, APP_PRODUCT, path))
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
            //std::string fontpath = "C:\\Windows\\Fonts\\msyh.ttc";
            //if (!std::filesystem::is_regular_file(fontpath))
            //{
            //    fontpath = "C:\\Windows\\Fonts\\msyh.ttf"; // Windows 7
            //}
            std::string fontpath = "C:\\Windows\\Fonts\\consola.ttf";
            if (std::filesystem::is_regular_file(fontpath))
            {
                io.Fonts->AddFontFromFileTTF(
                    fontpath.c_str(),
                    16.0f * APP.GetEngine()->GetMainWindow()->GetDPIScaling(),
                    &cfg
                    //, io.Fonts->GetGlyphRangesChineseFull()
                );
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
        ImPlot::CreateContext();
        
        setConfig();
        loadConfig();
        
        ImGui_ImplWin32Ex_Init((void*)window->GetHandle());
        window->AddNativeMessageCallback((ptrdiff_t)&ImGui_ImplWin32Ex_WndProcHandler);
        
        g_ImGuiRenderDeviceEventListener.OnRenderDeviceReset();
        device->AttachListener(&g_ImGuiRenderDeviceEventListener);
        
        luaopen_imgui(L);
        imgui_binding_lua_register_backend(L);
        lua_pop(L, 1);
        
        typedef DWORD (WINAPI *f_XInputGetState)(DWORD, XINPUT_STATE*);
        
        g_hXinput = LoadLibraryW(L"xinput9_1_0.dll");
        if (g_hXinput) g_fXInputGetState = (f_XInputGetState)GetProcAddress(g_hXinput, "XInputGetState");
        
        g_ImGuiBindEngine = true;
    }
    void unbindEngine()
    {
        g_ImGuiBindEngine = false;
        g_ImGuiTexIDValid = false;
        
        g_fXInputGetState = NULL;
        if (g_hXinput) { FreeLibrary(g_hXinput); g_hXinput = NULL; }
        
        auto* window = APP.GetEngine()->GetMainWindow();
        auto* device = APP.GetRenderDev();
        auto* L = APP.GetLuaEngine();
        
        device->RemoveListener(&g_ImGuiRenderDeviceEventListener);
        g_ImGuiRenderDeviceEventListener.OnRenderDeviceLost();
        
        window->RemoveNativeMessageCallback((ptrdiff_t)&ImGui_ImplWin32Ex_WndProcHandler);
        ImGui_ImplWin32Ex_Shutdown();
        
        saveConfig();

        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }
    
    void updateEngine()
    {
        if (g_ImGuiBindEngine)
        {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32Ex_NewFrame();
            g_ImGuiTexIDValid = true;
        }
    }
    void drawEngine()
    {
        if (g_ImGuiBindEngine && g_ImGuiTexIDValid)
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
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            
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
                if (ImGui::BeginTabItem("DirectInput##8011"))
                {
                    {
                        const auto cnt = dinput.count();
                        combo_str.resize(cnt);
                        combo_data.resize(cnt);
                        for (uint32_t i = 0; i < cnt; i += 1)
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
                        ImGui::Combo("Devices", &current_didx, combo_data.data(), combo_data.size());
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
                        if (ImGui::Button("Refresh"))
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
                            ImGui::SliderInt("Axis X", &cache, range.XMin, range.XMax);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.lY;
                            ImGui::SliderInt("Axis Y", &cache, range.YMin, range.YMax);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.lZ;
                            ImGui::SliderInt("Axis Z", &cache, range.ZMin, range.ZMax);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.lRx;
                            ImGui::SliderInt("Axis RX", &cache, range.RxMin, range.RxMax);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.lRy;
                            ImGui::SliderInt("Axis RY", &cache, range.RyMin, range.RyMax);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.lRz;
                            ImGui::SliderInt("Axis RZ", &cache, range.RzMin, range.RzMax);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.rglSlider[0];
                            ImGui::SliderInt("Slider 1", &cache, range.Slider0Min, range.Slider0Max);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = state.rglSlider[1];
                            ImGui::SliderInt("Slider 2", &cache, range.Slider1Min, range.Slider1Max);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = (state.rgdwPOV[0] <= 36000) ? state.rgdwPOV[0] : 0;
                            ImGui::SliderInt("POV 1", &cache, 0, (state.rgdwPOV[0] <= 36000) ? 36000 : 0);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = (state.rgdwPOV[1] <= 36000) ? state.rgdwPOV[1] : 0;
                            ImGui::SliderInt("POV 2", &cache, 0, (state.rgdwPOV[1] <= 36000) ? 36000 : 0);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = (state.rgdwPOV[2] <= 36000) ? state.rgdwPOV[2] : 0;
                            ImGui::SliderInt("POV 3", &cache, 0, (state.rgdwPOV[2] <= 36000) ? 36000 : 0);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache = (state.rgdwPOV[3] <= 36000) ? state.rgdwPOV[3] : 0;
                            ImGui::SliderInt("POV 4", &cache, 0, (state.rgdwPOV[3] <= 36000) ? 36000 : 0);
                            
                            bool bcache = false;
                            
                            #define SHOWKEY(I, B) \
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.125f);\
                                bcache = (state.rgbButtons[I - 1] != 0);\
                                if ((I) < 10) ImGui::Checkbox("Button 0" #I, &bcache);\
                                else ImGui::Checkbox("Button " #I, &bcache);\
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
                if (ImGui::BeginTabItem("DirectInput to XInput##8012"))
                {
                    {
                        const auto cnt = dinput.count();
                        combo_str.resize(cnt);
                        combo_data.resize(cnt);
                        for (uint32_t i = 0; i < cnt; i += 1)
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
                        ImGui::Combo("Devices", &current_didx, combo_data.data(), combo_data.size());
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
                        if (ImGui::Button("Refresh"))
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
                            ImGui::SliderScalarN("Left Joystick (LJ)", ImGuiDataType_S16, cache, 2, &minv, &maxv);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache[0] = state.sThumbRX; cache[1] = state.sThumbRY;
                            ImGui::SliderScalarN("Right Joystick (RJ)", ImGuiDataType_S16, cache, 2, &minv, &maxv);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            bcache = state.bLeftTrigger;
                            ImGui::SliderScalar("Left Trigger（LT）", ImGuiDataType_U8, &bcache, &bminv, &bmaxv);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            bcache = state.bRightTrigger;
                            ImGui::SliderScalar("Right Trigger（RT）", ImGuiDataType_U8, &bcache, &bminv, &bmaxv);
                            
                            bool _bstate = false;
                            
                            #define SHOWKEY(L, NAME, C) \
                                _bstate = ((state.wButtons & C) != 0);\
                                ImGui::Checkbox(#NAME, &_bstate);\
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
                if (ImGui::BeginTabItem("XInput##8013"))
                {
                    {
                        combo_str.resize(xdevice);
                        combo_data.resize(xdevice);
                        for (uint32_t i = 0; i < xdevice; i += 1)
                        {
                            snprintf(buffer, 1023, "%u. %s", i + 1, "XBox Controller & XInput Compatible Controller");
                            combo_str[i] = buffer;
                            combo_data[i] = (char*)combo_str[i].c_str();
                        }
                        
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                        ImGui::Combo("Devices", &current_xidx, combo_data.data(), combo_data.size());
                        
                        if (current_xidx < xdevice)
                        {
                            auto& state = xstate[current_xidx].Gamepad;
                            
                            SHORT cache[2];
                            SHORT minv = -32768, maxv = 32767;
                            BYTE bcache = 0;
                            BYTE bminv = 0, bmaxv = 255;
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache[0] = state.sThumbLX; cache[1] = state.sThumbLY;
                            ImGui::SliderScalarN("Left Joystick (LJ)", ImGuiDataType_S16, cache, 2, &minv, &maxv);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            cache[0] = state.sThumbRX; cache[1] = state.sThumbRY;
                            ImGui::SliderScalarN("Right Joystick (RJ)", ImGuiDataType_S16, cache, 2, &minv, &maxv);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            bcache = state.bLeftTrigger;
                            ImGui::SliderScalar("Left Trigger（LT）", ImGuiDataType_U8, &bcache, &bminv, &bmaxv);
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                            bcache = state.bRightTrigger;
                            ImGui::SliderScalar("Right Trigger（RT）", ImGuiDataType_U8, &bcache, &bminv, &bmaxv);
                            
                            bool _bstate = false;
                            
                            #define SHOWKEY(L, NAME, C) \
                                _bstate = ((state.wButtons & C) != 0);\
                                ImGui::Checkbox(#NAME, &_bstate);\
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
                if (ImGui::BeginTabItem("Keyboard##8014"))
                {
                    for (int i = 0; i < 256; i += 1)
                    {
                        if (dinput.getKeyboardKeyState(i))
                        {
                            ImGui::Text("[%d]", i);
                        }
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Mouse##8014"))
                {
                    const auto x = dinput.getMouseMoveDeltaX();
                    const auto y = dinput.getMouseMoveDeltaY();
                    int xy[2] = { x, y };
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                    ImGui::DragInt2("Move Delta", xy);
                    
                    static int g_x = 0;
                    static int g_y = 0;
                    g_x += x;
                    g_y += y;
                    int g_xy[2] = { g_x, g_y };
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                    ImGui::DragInt2("Total Move", g_xy);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
                    if (ImGui::Button("Reset##1"))
                    {
                        g_x = 0;
                        g_y = 0;
                    }
                    
                    int z = dinput.getMouseWheelDelta();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                    ImGui::DragInt("Wheel Delta", &z);
                    
                    static int g_z = 0;
                    g_z += z;
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
                    int _g_z = g_z;
                    ImGui::DragInt("Total Wheel", &_g_z);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
                    if (ImGui::Button("Reset##2"))
                    {
                        g_z = 0;
                    }
                    
                    for (int i = 0; i < 8; i += 1)
                    {
                        if (dinput.getMouseKeyState(i))
                        {
                            ImGui::Text("[%d]", i);
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
