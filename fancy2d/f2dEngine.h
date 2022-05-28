#pragma once
#include "f2dInterface.h"
#include "f2dSoundSys.h"
#include "f2dRenderer.h"

struct f2dEngineRenderWindowParam
{
	void* appmodel = nullptr;
};

struct f2dEngine : f2dInterface
{
	// --- 获得组件 ---

	virtual f2dSoundSys* GetSoundSys()=0;
	virtual f2dRenderer* GetRenderer()=0;

	// --- 内部使用 ---

	virtual void* GetAppModel() = 0;
};

fResult CreateF2DEngineAndInit(f2dEngineRenderWindowParam* RenderWindowParam, f2dEngine** pOut);
