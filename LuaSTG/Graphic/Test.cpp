#include "Graphic/Test.h"
#include "Graphic/Device.h"
#include "Graphic/DearImGuiRenderer.h"
#include "imgui_impl_win32ex.h"

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
            slow::Graphic::Device::get().autoResizeSwapChain();
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
