#pragma once

#include "CLRBinding/CLRHost.hpp"

namespace luastg
{
#define BINDING_METHODS \
    DECLARE_CLR_API(void, Log, (int32_t, intptr_t)) \
	\
    DECLARE_CLR_API(intptr_t, GameObject_New, (uint32_t)) \
    DECLARE_CLR_API(int32_t, GameObject_GetID, (intptr_t)) \
    DECLARE_CLR_API(void, GameObject_DefaultRenderFunc, (intptr_t)) \
	\
    DECLARE_CLR_API(void, Del, (intptr_t, bool)) \
	\
    DECLARE_CLR_API(int32_t, FirstObject, (int64_t)) \
    DECLARE_CLR_API(int32_t, NextObject, (int64_t, int32_t)) \
	\
    DECLARE_CLR_API(void, BeginScene, ()) \
    DECLARE_CLR_API(void, EndScene, ()) \
    DECLARE_CLR_API(void, RenderClear, (uint8_t, uint8_t, uint8_t, uint8_t))

	class CLRBinding {
	public:
#define DECLARE_CLR_API(ReturnType, Name, Params) static ReturnType Name Params;
		BINDING_METHODS
#undef DECLARE_CLR_API
	};

	struct UnmanagedAPI {
#define DECLARE_CLR_API(ReturnType, Name, Params) ReturnType (CORECLR_DELEGATE_CALLTYPE* Name) Params;
		BINDING_METHODS
#undef DECLARE_CLR_API

		UnmanagedAPI() {
#define DECLARE_CLR_API(ReturnType, Name, Params) Name = CLRBinding::Name;
			BINDING_METHODS
#undef DECLARE_CLR_API
		}
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
		void (CORECLR_DELEGATE_CALLTYPE* CallOnFrame)(uint64_t);
		void (CORECLR_DELEGATE_CALLTYPE* CallOnRender)(uint64_t);
		void (CORECLR_DELEGATE_CALLTYPE* CallOnDestroy)(uint64_t, int32_t);
		void (CORECLR_DELEGATE_CALLTYPE* CallOnColli)(uint64_t, uint64_t);
	};

	const int CLR_DESTROY_BOUNDS = 0;
	const int CLR_DESTROY_DEL = 1;
	const int CLR_DESTROY_KILL = 2;

	typedef int (CORECLR_DELEGATE_CALLTYPE* entry_point_fn)(UnmanagedAPI*, ManagedAPI*);

	bool InitCLRBinding(const CLRHost* host, ManagedAPI* functions);
}