// dear imgui: Platform Backend for Windows (standard windows API for 32 and 64 bits applications)
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3, Vulkan..)

// Implemented features:
//  [X] Platform: Clipboard support (for Win32 this is actually part of core dear imgui)
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Keyboard arrays indexed using VK_* Virtual Key Codes, e.g. ImGui::IsKeyPressed(VK_SPACE).
//  [X] Platform: Gamepad support. Enabled with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.

// Experimental features:
//  [X] Platform: It's safety to using this implement on a thread different to "Win32 GUI thread"
//      (or named "Win32 Window message thread").
//  [X] Platform: Dynamic link to XInput library (LoadLibrary and GetProcAddress), support multi XInput versions

// Warning:
//  1. This is a experimental implement and may have some bugs.
//  2. Mouse capture (Win32 API SetCapture, GetCapture and ReleaseCapture) may not working in some case
//     because it is asynchronous (execution order is not guaranteed).
//  3. Input latency may be large.
//  4. Support ImGuiBackendFlags_HasSetMousePos, but the reason same as (2), it may not working in some case.

// Change logs:
//  2020-11-01: First implement and add a example ("example_win32_workingthread_directx11").
//  2020-11-01: Fixed the wrong IME candidate list position bug.
//  2020-12-09: Rename backend to "imgui_impl_win32ex" and rename all methods to "ImGui_ImplWin32Ex"
//  2020-12-09: Rewrite the way exchanging messages, using Win32 API (SendMessage) to sync
//  2020-12-09: Rewrite the way linking to XInput library (LoadLibrary and GetProcAddress)
//  2020-12-09: Rename example to "example_win32ex_directx11"

// You can copy and use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#pragma once
#include "imgui.h"      // IMGUI_IMPL_API

struct ImGui_ImplWin32Ex_FrameData
{
	ImVec2 view_size;
	ImVec2 mouse_offset;
	ImVec2 mouse_scale;
};

IMGUI_IMPL_API bool ImGui_ImplWin32Ex_Init(void* window);
IMGUI_IMPL_API void ImGui_ImplWin32Ex_Shutdown();
IMGUI_IMPL_API void ImGui_ImplWin32Ex_NewFrame(ImGui_ImplWin32Ex_FrameData* frame_data = nullptr);

// Win32 user define message
//  - If it conflicts with yours, define a new one to override it.
#ifndef IMGUI_IMPL_WIN32EX_WM_USER
#define IMGUI_IMPL_WIN32EX_WM_USER 0x0400 // WM_USER
#endif

// Win32 message handler your application need to call.
// - Intentionally commented out in a '#if 0' block to avoid dragging dependencies on <windows.h> from this helper.
// - You should COPY the line below into your .cpp code to forward declare the function and then you can call it.
#if 0
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32Ex_WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
