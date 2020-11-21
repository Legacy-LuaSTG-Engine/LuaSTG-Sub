#include "ImGuiExtension.h"
#include <fstream>

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_freetype.h"
#include "imgui_impl_win32_workingthread.h"
#include "imgui_impl_dx9.h"
#include "LuaWrapper/imgui/lua_imgui.hpp"
#include "LuaWrapper/imgui/lua_imgui_type.hpp"

#include "Config.h"
#include "SystemDirectory.hpp"
#include "AppFrame.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32WorkingThread_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace imgui
{
    static bool g_ImGuiBindEngine = false;
    
    class ImGuiRenderDeviceEventListener : public f2dRenderDeviceEventListener
    {
    public:
        void OnRenderDeviceLost()
        {
            ImGui_ImplDX9_Shutdown();
        }
        void OnRenderDeviceReset()
        {
            IDirect3DDevice9* device = (IDirect3DDevice9*)LuaSTGPlus::AppFrame::GetInstance().GetRenderDev()->GetHandle();
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
    }
    
    void bindEngine()
    {
        auto* window = LuaSTGPlus::AppFrame::GetInstance().GetEngine()->GetMainWindow();
        auto* device = LuaSTGPlus::AppFrame::GetInstance().GetRenderDev();
        auto* L = LuaSTGPlus::AppFrame::GetInstance().GetLuaEngine();
        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        
        setConfig();
        loadConfig();
        
        ImGui_ImplWin32WorkingThread_Init((void*)window->GetHandle());
        window->SetNativeMessageProcess((void*)&ImGui_ImplWin32WorkingThread_WndProcHandler);
        
        ImGui_ImplDX9_Init((IDirect3DDevice9*)device->GetHandle());
        device->AttachListener(&g_ImGuiRenderDeviceEventListener);
        
        luaopen_imgui(L);
        lua_pop(L, 1);
        
        g_ImGuiBindEngine = true;
    }
    void unbindEngine()
    {
        g_ImGuiBindEngine = false;
        
        auto* window = LuaSTGPlus::AppFrame::GetInstance().GetEngine()->GetMainWindow();
        auto* device = LuaSTGPlus::AppFrame::GetInstance().GetRenderDev();
        auto* L = LuaSTGPlus::AppFrame::GetInstance().GetLuaEngine();
        
        device->RemoveListener(&g_ImGuiRenderDeviceEventListener);
        ImGui_ImplDX9_Shutdown();
        
        window->SetNativeMessageProcess(NULL);
        ImGui_ImplWin32WorkingThread_Shutdown();
        
        saveConfig();
        ImGui::DestroyContext();
    }
    
    void updateEngine()
    {
        if (g_ImGuiBindEngine)
        {
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32WorkingThread_NewFrame();
        }
    }
    void drawEngine()
    {
        if (g_ImGuiBindEngine)
        {
            auto& engine = LuaSTGPlus::AppFrame::GetInstance();
            
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
};
