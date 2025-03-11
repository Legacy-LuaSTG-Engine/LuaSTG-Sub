#pragma once

#include "CLRBinding/CLRHost.hpp"

namespace LuaSTGPlus
{
	typedef void (CORECLR_DELEGATE_CALLTYPE* Log_fn)();
	typedef void (CORECLR_DELEGATE_CALLTYPE* game_callback_fn)();
	typedef bool (CORECLR_DELEGATE_CALLTYPE* game_loop_fn)();

	struct CLRInitPayload
	{
		Log_fn log;
	};

	struct CLRFunctions 
	{
		game_callback_fn GameInit;
		game_loop_fn FrameFunc;
		game_callback_fn RenderFunc;
		game_callback_fn GameExit;
		game_callback_fn FocusLoseFunc;
		game_callback_fn FocusGainFunc;
	};

	typedef void (CORECLR_DELEGATE_CALLTYPE* entry_point_fn)(CLRInitPayload);

	void CallCLRStartUp(const entry_point_fn& entry_point);

	bool InitCLRBinding(const CLRHost* host, CLRFunctions* functions);
}