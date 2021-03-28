#include "Graphic/Test.h"
#include "Graphic/Device.h"
#include "Graphic/DearImGuiRenderer.h"
#include "imgui_impl_win32ex.h"
#include "AppFrame.h"

namespace slow::Graphic
{
    static bool _test = false;
    static bool _ok = false;
    
    bool _bindEngine(void* window)
    {
        if (_test)
        {
            if (!slow::Graphic::Device::get().bind((handle_t)window))
                return false;
            if (!slow::Graphic::DearImGuiRenderer::get().bind(slow::Graphic::Device::get()))
                return false;
            Pointer<ITexture2D> tex_;
            Device::get().createTexture2D(u8"res/lenna.png", ~tex_);
            
            _ok = true;
        }
        return true;
    };
    void _unbindEngine()
    {
        _ok = false;
        slow::Graphic::DearImGuiRenderer::get().unbind();
        slow::Graphic::Device::get().unbind();
    };
    bool _update()
    {
        if (_ok)
        {
            if (ImGui::IsKeyPressed(/*VK_N1*/0x31))
            {
                LuaSTGPlus::AppFrame::GetInstance().GetWindow()->SetClientRect(fcyRect(0, 0, 1280, 720));
                LuaSTGPlus::AppFrame::GetInstance().GetWindow()->MoveToCenter();
                Device::get().resizeSwapChain(1280, 720);
            }
            else if (ImGui::IsKeyPressed(/*VK_N2*/0x32))
            {
                LuaSTGPlus::AppFrame::GetInstance().GetWindow()->SetClientRect(fcyRect(0, 0, 2560, 1440));
                LuaSTGPlus::AppFrame::GetInstance().GetWindow()->MoveToCenter();
                Device::get().resizeSwapChain(2560, 1440);
            }
            else
            {
                slow::Graphic::Device::get().autoResizeSwapChain();
            }
            
            slow::Graphic::DearImGuiRenderer::get().update();
            ImGui_ImplWin32Ex_NewFrame();
            
            ImGui::NewFrame();
            ImGui::ShowDemoWindow();
            ImGui::EndFrame();
            ImGui::Render();
        }
        return true;
    };
    bool _draw()
    {
        if (_ok)
        {
            
            slow::Graphic::Device::get().setSwapChain();
            slow::Graphic::Device::get().clearRenderTarget();
            slow::Graphic::Device::get().clearDepthBuffer();
            
            slow::Graphic::DearImGuiRenderer::get().draw();
            
            slow::Graphic::Device::get().updateSwapChain();
            
            return true;
        }
        return false;
    };
};
