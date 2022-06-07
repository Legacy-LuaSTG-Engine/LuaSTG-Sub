#define NOMINMAX
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

#include "platform/KnownDirectory.hpp"
#include "AppFrame.h"
#include "LuaWrapper/LuaWrapper.hpp"

#include <Xinput.h>
#include "platform/XInput.hpp"

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
        count = std::snprintf(buffer, 64, "%.2f KiB", (double)size / 1024.0);
    }
    else if (size < (1024 * 1024 * 1024)) // MB
    {
        count = std::snprintf(buffer, 64, "%.2f MiB", (double)size / 1048576.0);
    }
    else // GB
    {
        count = std::snprintf(buffer, 64, "%.2f GiB", (double)size / 1073741824.0);
    }
    return std::string(buffer, count);
}

static void showParticleSystemEditor(bool* p_open, LuaSTGPlus::ResParticle::ParticlePool* sys)
{
    struct EditorData
    {
        bool bContinuous = false;
        bool bSyncParticleLife = false;
        bool bSyncSpeed = false;
        bool bSyncGravity = false;
        bool bSyncRadialAccel = false;
        bool bSyncTangentialAccel = false;
        bool bSyncSize = false;
        bool bSyncSpin = false;
        bool bSyncAlpha = false;
        int nBlendMode = 0;
    };
    static std::unordered_map<void*, EditorData> g_EditorData;
    if (ImGui::Begin("Particle System Editor", p_open))
    {
        if (sys)
        {
            auto& info = sys->GetParticleSystemInfo();
            if (g_EditorData.find(&info) == g_EditorData.end())
            {
                g_EditorData.emplace(&info, EditorData{
                    .bContinuous = (info.fLifetime - (-1.0f)) < std::numeric_limits<float>::min(),
                    .bSyncParticleLife = false,
                    .bSyncSpeed = false,
                    .bSyncGravity = false,
                    .bSyncRadialAccel = false,
                    .bSyncTangentialAccel = false,
                    .bSyncSize = false,
                    .bSyncSpin = false,
                    .bSyncAlpha = false,
                    .nBlendMode = sys->GetBlendMode() == LuaSTGPlus::BlendMode::MulAdd ? 0 : 1,
                });
            }
            auto& data = g_EditorData[&info];

            auto widgetSyncMinMax = [](std::string_view name, float& minv, float& maxv, float a, float b, bool& sync) -> void {
                std::string checkbox_text = "Sync##"; checkbox_text.append(name);
                std::string min_text = "Min##"; min_text.append(name);
                std::string max_text = "Max##"; max_text.append(name);
                ImGui::AlignTextToFramePadding();
                ImGui::Text(name.data());
                ImGui::SameLine();
                if (ImGui::Checkbox(checkbox_text.data(), &sync))
                {
                    if (sync)
                    {
                        maxv = minv;
                    }
                }
                if (sync)
                {
                    if (ImGui::SliderFloat(min_text.data(), &minv, a, b))
                    {
                        maxv = minv;
                    }
                    if (ImGui::SliderFloat(max_text.data(), &maxv, a, b))
                    {
                        minv = maxv;
                    }
                }
                else
                {
                    ImGui::SliderFloat(min_text.data(), &minv, a, b);
                    ImGui::SliderFloat(max_text.data(), &maxv, a, b);
                }
            };

            auto widgetSyncStartEndVar = [](std::string_view name, float& begv, float& endv, float& varv, float a, float b, bool& sync) -> void {
                std::string checkbox_text = "Sync##"; checkbox_text.append(name);
                std::string beg_text = "Start##"; beg_text.append(name);
                std::string end_text = "End##"; end_text.append(name);
                std::string var_text = "Variation##"; var_text.append(name);
                ImGui::AlignTextToFramePadding();
                ImGui::Text(name.data());
                ImGui::SameLine();
                if (ImGui::Checkbox(checkbox_text.data(), &sync))
                {
                    if (sync)
                    {
                        endv = begv;
                    }
                }
                if (sync)
                {
                    if (ImGui::SliderFloat(beg_text.data(), &begv, a, b))
                    {
                        endv = begv;
                    }
                    if (ImGui::SliderFloat(end_text.data(), &endv, a, b))
                    {
                        begv = endv;
                    }
                }
                else
                {
                    ImGui::SliderFloat(beg_text.data(), &begv, a, b);
                    ImGui::SliderFloat(end_text.data(), &endv, a, b);
                }
                ImGui::SliderFloat(var_text.data(), &varv, 0.0f, 1.0f);
            };

            if (ImGui::CollapsingHeader("System Information"))
            {
                ImGui::LabelText("Particle Alive", "%u", (uint32_t)sys->GetAliveCount());
                ImGui::LabelText("FPS", "%.2f", LAPP.GetFPS());
            }

            if (ImGui::CollapsingHeader("System Parameter"))
            {
                if (!data.bContinuous)
                {
                    if (info.fLifetime < 0.0f)
                    {
                        info.fLifetime = 5.0f;
                    }
                    ImGui::SliderFloat("System Lifetime", &info.fLifetime, 0.0f, 10.0f);
                }
                else
                {
                    float fFake = 5.0f;
                    ImGui::SliderFloat("System Lifetime", &fFake, 0.0f, 10.0f);
                }
                ImGui::Checkbox("Continuous", &data.bContinuous);
                if (data.bContinuous)
                {
                    info.fLifetime = -1.0f;
                }
                ImGui::Separator();

                ImGui::SliderInt("Emission", &info.nEmission, 0, 1000, "%d (p/sec)");
                ImGui::Separator();

                widgetSyncMinMax("Particle Lifetime", info.fParticleLifeMin, info.fParticleLifeMax, 0.0f, 5.0f, data.bSyncParticleLife);
                ImGui::Separator();

                char const* const chBlendMode[2] = {
                    "Add",
                    "Alpha",
                };
                if (ImGui::Combo("Blend Mode", &data.nBlendMode, chBlendMode, 2))
                {
                    sys->SetBlendMode(data.nBlendMode == 0 ? LuaSTGPlus::BlendMode::MulAdd : LuaSTGPlus::BlendMode::MulAlpha);
                }
            }

            if (ImGui::CollapsingHeader("Particle Movement"))
            {
                if (info.bRelative)
                {
                    float fDir = info.fDirection;
                    ImGui::SliderAngle("Direction", &fDir, 0.0f, 360.0f);
                }
                else
                {
                    ImGui::SliderAngle("Direction", &info.fDirection, 0.0f, 360.0f);
                }
                ImGui::Checkbox("Relative", &info.bRelative);
                ImGui::Separator();

                ImGui::SliderAngle("Spread", &info.fSpread, 0.0f, 360.0f);
                ImGui::Separator();

                widgetSyncMinMax("Start Speed", info.fSpeedMin, info.fSpeedMax, -300.0f, 300.0f, data.bSyncSpeed);
                ImGui::Separator();

                widgetSyncMinMax("Gravity", info.fGravityMin, info.fGravityMax, -900.0f, 900.0f, data.bSyncGravity);
                ImGui::Separator();

                widgetSyncMinMax("Radial Acceleration", info.fRadialAccelMin, info.fRadialAccelMax, -900.0f, 900.0f, data.bSyncRadialAccel);
                ImGui::Separator();

                widgetSyncMinMax("Tangential Acceleration", info.fTangentialAccelMin, info.fTangentialAccelMax, -900.0f, 900.0f, data.bSyncGravity);
            }
            
            if (ImGui::CollapsingHeader("Particle Appearance"))
            {
                widgetSyncStartEndVar("Particle Size", info.fSizeStart, info.fSizeEnd, info.fSizeVar, 1.0f / 32.0f, 100.0f / 32.0f, data.bSyncSize);
                ImGui::Separator();

                widgetSyncStartEndVar("Particle Spin", info.fSpinStart, info.fSpinEnd, info.fSpinVar, -50.0f, 50.0f, data.bSyncSpin);
                ImGui::Separator();

                widgetSyncStartEndVar("Particle Alpha", info.colColorStart[3], info.colColorEnd[3], info.fAlphaVar, 0.0f, 1.0f, data.bSyncAlpha);
                ImGui::Separator();

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Particle Color");
                ImGui::ColorEdit3("Start##Particle Color", info.colColorStart);
                ImGui::ColorEdit3("End##Particle Color", info.colColorEnd);
                ImGui::SliderFloat("Variation##Particle Color", &info.fColorVar, 0.0f, 1.0f);
            }
        }
    }
    ImGui::End();
}

static int lib_NewFrame(lua_State* L)
{
    std::ignore = L;
    imgui::updateEngine();
    return 0;
}
static int lib_RenderDrawData(lua_State* L)
{
    std::ignore = L;
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
            auto gmuinfo = LAPP.GetAppModel()->getDevice()->getMemoryUsageStatistics();
            if (bret)
            {
                static bool more_info = false;
                ImGui::Checkbox("More Informations", &more_info);

                if (more_info) ImGui::Text("System Memory Usage: %u%%", info.dwMemoryLoad);
                if (more_info) ImGui::Text("Totoal Physical Memory: %s", bytes_count_to_string(info.ullTotalPhys).c_str());
                if (more_info) ImGui::Text("Avalid Physical Memory: %s", bytes_count_to_string(info.ullAvailPhys).c_str());
                if (more_info) ImGui::Text("Totoal Page File: %s", bytes_count_to_string(info.ullTotalPageFile).c_str());
                if (more_info) ImGui::Text("Avalid Page File: %s", bytes_count_to_string(info.ullAvailPageFile).c_str());
                if (more_info) ImGui::Text("Totoal User Mode Memory Space: %s", bytes_count_to_string(info.ullTotalVirtual).c_str());
                ImGui::Text("Avalid User Mode Memory Space: %s", bytes_count_to_string(info.ullAvailVirtual).c_str());
                ImGui::Text("Alloc* User Mode Memory Space: %s", bytes_count_to_string(info.ullTotalVirtual - info.ullAvailVirtual).c_str());

                if (more_info) ImGui::Text("Adapter Local Budget: %s", bytes_count_to_string(gmuinfo.local.budget).c_str());
                ImGui::Text("Adapter Local Usage: %s", bytes_count_to_string(gmuinfo.local.current_usage).c_str());
                if (more_info) ImGui::Text("Adapter Local Available For Reservation: %s", bytes_count_to_string(gmuinfo.local.available_for_reservation).c_str());
                if (more_info) ImGui::Text("Adapter Local Current Reservation: %s", bytes_count_to_string(gmuinfo.local.current_reservation).c_str());

                if (more_info) ImGui::Text("Adapter Non-Local Budget: %s", bytes_count_to_string(gmuinfo.non_local.budget).c_str());
                ImGui::Text("Adapter Non-Local Usage: %s", bytes_count_to_string(gmuinfo.non_local.current_usage).c_str());
                if (more_info) ImGui::Text("Adapter Non-Local Available For Reservation: %s", bytes_count_to_string(gmuinfo.non_local.available_for_reservation).c_str());
                if (more_info) ImGui::Text("Adapter Non-Local Current Reservation: %s", bytes_count_to_string(gmuinfo.non_local.current_reservation).c_str());

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
            // TODO:

            auto info = LAPP.GetAppModel()->getFrameStatistics();
            
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
static int lib_ShowResourceManagerDebugWindow(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        bool v = lua_toboolean(L, 1);
        LRES.ShowResourceManagerDebugWindow(&v);
        lua_pushboolean(L, v);
        return 1;
    }
    else
    {
        LRES.ShowResourceManagerDebugWindow();
        return 0;
    }
}
static int lib_ShowParticleSystemEditor(lua_State* L)
{
    if (lua_gettop(L) >= 2)
    {
        bool v = lua_toboolean(L, 1);
        auto p = LuaSTGPlus::LuaWrapper::ParticleSystemWrapper::Cast(L, 2);
        showParticleSystemEditor(&v, p->ptr);
        lua_pushboolean(L, v);
        return 1;
    }
    else
    {
        auto p = LuaSTGPlus::LuaWrapper::ParticleSystemWrapper::Cast(L, 1);
        showParticleSystemEditor(nullptr, p->ptr);
        return 0;
    }
}

void imgui_binding_lua_register_backend(lua_State* L)
{
    const luaL_Reg lib_fun[] = {
        {"NewFrame", &lib_NewFrame},
        {"RenderDrawData", &lib_RenderDrawData},
        {"ShowTestInputWindow", &lib_ShowTestInputWindow},
        {"ShowMemoryUsageWindow", &lib_ShowMemoryUsageWindow},
        {"ShowFrameStatistics", &lib_ShowFrameStatistics},
        {"ShowResourceManagerDebugWindow", &lib_ShowResourceManagerDebugWindow},
        {"ShowParticleSystemEditor", &lib_ShowParticleSystemEditor},
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

    class ImGuiBackendEventListener
        : public LuaSTG::Core::Graphics::IDeviceEventListener
        , public LuaSTG::Core::Graphics::IWindowEventListener
    {
    public:
        void onDeviceDestroy()
        {
            g_ImGuiTexIDValid = false;
            ImGui_ImplDX11_Shutdown();
        }
        void onDeviceCreate()
        {
            g_ImGuiTexIDValid = false;
            ID3D11Device* device = (ID3D11Device*)APP.GetAppModel()->getDevice()->getNativeHandle();
            ID3D11DeviceContext* context = NULL;
            device->GetImmediateContext(&context);
            ImGui_ImplDX11_Init(device, context);
            context->Release();
        }
        NativeWindowMessageResult onNativeWindowMessage(void* hwnd, uint32_t msg, uintptr_t wparam, intptr_t lparam)
        {
            LRESULT lresult = ImGui_ImplWin32Ex_WndProcHandler((HWND)hwnd, msg, wparam, lparam);
            if (lresult)
                return NativeWindowMessageResult(lresult, true);
            else
                return {};
        }
    };
    static ImGuiBackendEventListener g_ImGuiRenderDeviceEventListener;

    void loadConfig()
    {
        if (ImGui::GetCurrentContext())
        {
            ImGuiIO& io = ImGui::GetIO();
            // handle imgui config data
            io.IniFilename = NULL;
            std::wstring path;
            #ifdef USING_SYSTEM_DIRECTORY
                if (platform::KnownDirectory::makeAppDataW(APP_COMPANY, APP_PRODUCT, path))
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
                if (platform::KnownDirectory::makeAppDataW(APP_COMPANY, APP_PRODUCT, path))
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
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
        
        ImGuiStyle style;
        ImGui::StyleColorsDark(&style);
        
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
        
        style.ScaleAllSizes(APP.GetAppModel()->getWindow()->getDPIScaling());

        ImGui::GetStyle() = style;

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
                    16.0f * APP.GetAppModel()->getWindow()->getDPIScaling(),
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
        auto* window = APP.GetAppModel()->getWindow();
        auto* device = APP.GetAppModel()->getDevice();
        auto* L = APP.GetLuaEngine();
        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        
        setConfig();
        loadConfig();
        
        ImGui_ImplWin32Ex_Init((void*)window->getNativeHandle());
        window->addEventListener(&g_ImGuiRenderDeviceEventListener);
        
        g_ImGuiRenderDeviceEventListener.onDeviceCreate();
        device->addEventListener(&g_ImGuiRenderDeviceEventListener);
        
        luaopen_imgui(L);
        imgui_binding_lua_register_backend(L);
        lua_pop(L, 1);
        
        g_ImGuiBindEngine = true;
    }
    void unbindEngine()
    {
        if (g_ImGuiBindEngine)
        {
            auto& io = ImGui::GetIO();
            if (io.WantSaveIniSettings)
                saveConfig();
        }
        
        g_ImGuiBindEngine = false;
        g_ImGuiTexIDValid = false;
        
        auto* window = APP.GetAppModel()->getWindow();
        auto* device = APP.GetAppModel()->getDevice();
        
        device->removeEventListener(&g_ImGuiRenderDeviceEventListener);
        g_ImGuiRenderDeviceEventListener.onDeviceDestroy();
        
        window->removeEventListener(&g_ImGuiRenderDeviceEventListener);
        ImGui_ImplWin32Ex_Shutdown();
        
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

    void cancelSetCursor()
    {
        if (g_ImGuiBindEngine)
        {
            auto& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        }
    }
    void updateEngine()
    {
        if (g_ImGuiBindEngine)
        {
            constexpr int const mask = (~((int)ImGuiConfigFlags_NoMouseCursorChange));
            auto& io = ImGui::GetIO();
            io.ConfigFlags &= mask;
            if (io.WantSaveIniSettings)
                saveConfig();
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
            engine.GetAppModel()->getRenderer()->endBatch();
            
            // 绘制GUI数据
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            
            // 重启渲染过程
            engine.GetAppModel()->getRenderer()->beginBatch();
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
            static bool force_update = false;
            
            XINPUT_STATE xstate[4];
            ZeroMemory(xstate, sizeof(xstate));
            bool bxstate[4] = { false, false, false, false };
            DWORD xdevice = 0;
            for (size_t i = 0; i < 4; i += 1)
            {
                auto hr = platform::XInput::getState(i, xstate);
                if (hr == ERROR_SUCCESS)
                {
                    bxstate[i] = true;
                    xdevice += 1;
                }
            }
            
            ImGui::Checkbox("Force Update", &force_update);
            if (force_update)
            {
                dinput.update();
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
                        
                        platform::DirectInput::RawState state;
                        if (dinput.getRawState(current_didx, &state))
                        {
                            platform::DirectInput::AxisRange range;
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
                                if constexpr ((I) < 10) ImGui::Checkbox("Button 0" #I, &bcache);\
                                else ImGui::Checkbox("Button " #I, &bcache);\
                                if constexpr (B) ImGui::SameLine();
                            
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
                        
                        platform::DirectInput::State state;
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
                                if constexpr ((L) != 0) ImGui::SameLine();
                            
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
                        
                        if (current_xidx < (int)xdevice)
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
                                if constexpr ((L) != 0) ImGui::SameLine();
                            
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
