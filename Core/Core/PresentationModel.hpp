#pragma once
#include "Core/Type.hpp"

namespace Core
{
	struct IPresentationModel : public IObject
	{
		virtual bool CheckSupport(HWND window, ID3D11Device* device, ID2D1DeviceContext* renderer) = 0;
		virtual bool AttachDevice(HWND window, ID3D11Device* device, ID2D1DeviceContext* renderer) = 0;
		virtual void DetachDevice() = 0;
		virtual bool OnWindowSize(Vector2U size) = 0;
		virtual Vector2U GetWindowSize() = 0;
		virtual void OnWindowActive(bool active) = 0;
		virtual void OnWindowFullscreenStateChange(bool fullscreen) = 0;
		virtual bool SetCanvasSize(Vector2U size) = 0;
		virtual Vector2U GetCanvasSize() = 0;
		virtual ID3D11RenderTargetView* GetCanvasRenderTargetView() = 0;
		virtual ID3D11DepthStencilView* GetCanvasDepthStencilView() = 0;
		virtual ID2D1Bitmap1* GetCanvasBitmapTarget() = 0;
		virtual bool Present(bool vsync) = 0;
	};

	enum class PresentationModelType
	{
		LegacyWindowAndExclusiveFullscreen,
		LegacyWindow,
		Window,
		DirectComposition,
	};

	struct IPresentationModelManager : public IPresentationModel
	{
		virtual bool SetModelType(PresentationModelType type) = 0;
		virtual PresentationModelType GetModelType() = 0;
	};

	bool CreatePresentationModelManager(IPresentationModelManager** pp_manager);
	bool CreatePresentationModelManagerWithCanvasSize(Vector2U size, IPresentationModelManager** pp_manager);
}
