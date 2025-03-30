#pragma once

#include "CLRBinding/CLRHost.hpp"

namespace LuaSTGPlus
{
	struct CLRInitPayload
	{
		void (CORECLR_DELEGATE_CALLTYPE* Log)(int32_t, intptr_t);

		void (CORECLR_DELEGATE_CALLTYPE* BeginScene)();
		void (CORECLR_DELEGATE_CALLTYPE* EndScene)();
		void (CORECLR_DELEGATE_CALLTYPE* RenderClear)(uint8_t, uint8_t, uint8_t, uint8_t);
	};

	struct CLRFunctions 
	{
#pragma region GameLoop
		void (CORECLR_DELEGATE_CALLTYPE* GameInit)();
		bool (CORECLR_DELEGATE_CALLTYPE* FrameFunc)();
		void (CORECLR_DELEGATE_CALLTYPE* RenderFunc)();
		void (CORECLR_DELEGATE_CALLTYPE* GameExit)();
		void (CORECLR_DELEGATE_CALLTYPE* FocusLoseFunc)();
		void (CORECLR_DELEGATE_CALLTYPE* FocusGainFunc)();
#pragma endregion
	};

	typedef int (CORECLR_DELEGATE_CALLTYPE* entry_point_fn)(CLRInitPayload*, CLRFunctions*);

	bool InitCLRBinding(const CLRHost* host, CLRFunctions* functions);
}