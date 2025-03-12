#pragma once

#include "CLRBinding/CLRHost.hpp"

#define CORECLR_BINDING(ret, name, parameter) typedef ret (CORECLR_DELEGATE_CALLTYPE* t_##name)(parameter);\
t_##name name;

namespace LuaSTGPlus
{
	struct CLRInitPayload
	{
		void (CORECLR_DELEGATE_CALLTYPE* BeginScene)();
		void (CORECLR_DELEGATE_CALLTYPE* EndScene)();
		void (CORECLR_DELEGATE_CALLTYPE* RenderClear)(unsigned char, unsigned char, unsigned char, unsigned char);
	};

	struct CLRFunctions 
	{
		void (CORECLR_DELEGATE_CALLTYPE* GameInit)();
		bool (CORECLR_DELEGATE_CALLTYPE* FrameFunc)();
		void (CORECLR_DELEGATE_CALLTYPE* RenderFunc)();
		void (CORECLR_DELEGATE_CALLTYPE* GameExit)();
		void (CORECLR_DELEGATE_CALLTYPE* FocusLoseFunc)();
		void (CORECLR_DELEGATE_CALLTYPE* FocusGainFunc)();
	};

	typedef int (CORECLR_DELEGATE_CALLTYPE* entry_point_fn)(CLRInitPayload*, CLRFunctions*);

	bool InitCLRBinding(const CLRHost* host, CLRFunctions* functions);
}