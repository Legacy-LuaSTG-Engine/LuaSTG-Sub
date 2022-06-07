#pragma once
#include "f2dInterface.h"
#include "f2dSoundSys.h"

struct f2dEngine : f2dInterface
{
	virtual f2dSoundSys* GetSoundSys()=0;
};

fResult CreateF2DEngineAndInit(f2dEngine** pOut);
