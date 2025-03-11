#include "CLRBinding.hpp"

void Log()
{
	spdlog::level::level_enum const level = spdlog::level::level_enum::debug;
	std::string_view message = "FromCLR";
	spdlog::log(level, "[lua] {}", message);
}

void LuaSTGPlus::CallCLRStartUp(const entry_point_fn& entry_point)
{
	CLRInitPayload payload{};
	payload.log = Log;
	entry_point(payload);
}

bool LuaSTGPlus::InitCLRBinding(const CLRHost* host, CLRFunctions* functions)
{
	void* fn = nullptr;
	if (host->load_assembly_and_get_function_pointer(
		L".\\Managed\\net8.0\\LuaSTG.dll",
		L"LuaSTG.LuaSTGAPI, LuaSTG",
		L"StartUp",
		UNMANAGEDCALLERSONLY_METHOD,
		&fn) || !fn)
	{
		return false;
	}

	CLRInitPayload payload{};
	payload.log = Log;
	((entry_point_fn)fn)(payload);

	fn = nullptr;
	if (host->get_function_pointer(
		L"LuaSTG.LuaSTGAPI, LuaSTG",
		L"GameInit",
		L"LuaSTG.LuaSTGAPI+GameInit, LuaSTG",
		&fn) || !fn)
	{
		return false;
	}
	functions->GameInit = (game_callback_fn)fn;

	return true;
}
