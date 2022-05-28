#pragma once
#include "f2dInterface.h"
#include "f2dWindow.h"
#include "f2dSoundSys.h"
#include "f2dRenderer.h"

struct f2dEngineRenderWindowParam
{
	fcStrW title;
	fBool windowed;
	fBool vsync;
	f2dDisplayMode mode;
	fcStrW gpu;
	void* appmodel = nullptr;
};

struct f2dEngine : f2dInterface
{
	// --- 获得组件 ---

	virtual f2dWindow* GetMainWindow()=0;
	virtual f2dSoundSys* GetSoundSys()=0;
	virtual f2dRenderer* GetRenderer()=0;

	// --- 内部使用 ---

	virtual void* GetAppModel() = 0;
};
