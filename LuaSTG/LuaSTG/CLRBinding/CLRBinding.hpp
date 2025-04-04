#pragma once

#include "CLRBinding/CLRHost.hpp"

namespace LuaSTGPlus
{
	struct UnmanagedAPI
	{
		void (CORECLR_DELEGATE_CALLTYPE* Log)(int32_t, intptr_t);

		intptr_t (CORECLR_DELEGATE_CALLTYPE* GameObject_New)();
		uint64_t (CORECLR_DELEGATE_CALLTYPE* GameObject_GetID)(intptr_t);

		void (CORECLR_DELEGATE_CALLTYPE* BeginScene)();
		void (CORECLR_DELEGATE_CALLTYPE* EndScene)();
		void (CORECLR_DELEGATE_CALLTYPE* RenderClear)(uint8_t, uint8_t, uint8_t, uint8_t);
	};

	struct ManagedAPI 
	{
#pragma region GameLoop
		void (CORECLR_DELEGATE_CALLTYPE* GameInit)();
		bool (CORECLR_DELEGATE_CALLTYPE* FrameFunc)();
		void (CORECLR_DELEGATE_CALLTYPE* RenderFunc)();
		void (CORECLR_DELEGATE_CALLTYPE* GameExit)();
		void (CORECLR_DELEGATE_CALLTYPE* FocusLoseFunc)();
		void (CORECLR_DELEGATE_CALLTYPE* FocusGainFunc)();
#pragma endregion
		void (CORECLR_DELEGATE_CALLTYPE* DetachGameObject)(uint64_t);
		void (CORECLR_DELEGATE_CALLTYPE* CreateLuaGameObject)(intptr_t);
	};

	typedef int (CORECLR_DELEGATE_CALLTYPE* entry_point_fn)(UnmanagedAPI*, ManagedAPI*);

	bool InitCLRBinding(const CLRHost* host, ManagedAPI* functions);
}