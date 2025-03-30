#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include "windows.h"

#include "CLRHost.hpp"

void* LuaSTGPlus::CLRHost::load_library(const char_t* path)
{
	HMODULE h = ::LoadLibraryW(path);
	assert(h != nullptr);
	return (void*)h;
}

void* LuaSTGPlus::CLRHost::get_export(void* h, const char* name)
{
	void* f = ::GetProcAddress((HMODULE)h, name);
	assert(f != nullptr);
	return f;
}

bool LuaSTGPlus::CLRHost::init_hostfxr()
{
	// Pre-allocate a large buffer for the path to hostfxr
	char_t buffer[MAX_PATH];
	size_t buffer_size = sizeof(buffer) / sizeof(char_t);
	int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);
	if (rc != 0)
		return false;

	// Load hostfxr and get desired exports
	void* lib = load_library(buffer);
	_init_runtime = (hostfxr_initialize_for_runtime_config_fn)get_export(lib, "hostfxr_initialize_for_runtime_config");
	_get_delegate = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
	_close_context = (hostfxr_close_fn)get_export(lib, "hostfxr_close");

	return (_init_runtime && _get_delegate && _close_context);
}

bool LuaSTGPlus::CLRHost::get_dotnet_load_assembly_config(const char_t* config_path)
{
	hostfxr_handle cxt = nullptr;
	int rc = _init_runtime(config_path, nullptr, &cxt);
	if (rc != 0 || cxt == nullptr)
	{
		//std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
		_close_context(cxt);
		return false;
	}

	void* load_assembly_and_get_function_pointer = nullptr;
	void* load_assembly = nullptr;
	void* get_function_pointer = nullptr;

	rc = _get_delegate(
		cxt,
		hdt_load_assembly_and_get_function_pointer,
		&load_assembly_and_get_function_pointer);
	if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
	{
		//std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;
		return false;
	}

	rc = _get_delegate(
		cxt,
		hdt_load_assembly,
		&load_assembly);
	if (rc != 0 || load_assembly == nullptr)
	{
		//std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;
		return false;
	}

	rc = _get_delegate(
		cxt,
		hdt_get_function_pointer,
		&get_function_pointer);
	if (rc != 0 || get_function_pointer == nullptr)
	{
		//std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;
		return false;
	}

	_close_context(cxt);

	_load_assembly_and_get_function_pointer = (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
	_load_assembly = (load_assembly_fn)load_assembly;
	_get_function_pointer = (get_function_pointer_fn)get_function_pointer;
	return true;
}

LuaSTGPlus::CLRHost::CLRHost(const char_t* config_path)
{
	_init_runtime = nullptr;
	_get_delegate = nullptr;
	_close_context = nullptr;

	_load_assembly_and_get_function_pointer = nullptr;
	_load_assembly = nullptr;
	_get_function_pointer = nullptr;

	_config_path = config_path;
}

bool LuaSTGPlus::CLRHost::init()
{
	return init_hostfxr() && get_dotnet_load_assembly_config(_config_path);
}

int LuaSTGPlus::CLRHost::load_assembly_and_get_function_pointer(
	const char_t* assembly_path, 
	const char_t* type_name, 
	const char_t* method_name, 
	const char_t* delegate_type_name, 
	/*out*/ void** delegate) const
{
	return _load_assembly_and_get_function_pointer(
		assembly_path,
		type_name, 
		method_name, 
		delegate_type_name, 
		nullptr, 
		delegate
	);
}

int LuaSTGPlus::CLRHost::load_assembly(const char_t* assembly_path) const
{
	return _load_assembly(
		assembly_path,
		nullptr,
		nullptr
	);
}

int LuaSTGPlus::CLRHost::get_function_pointer(
	const char_t* type_name, 
	const char_t* method_name, 
	const char_t* delegate_type_name, 
	/*out*/ void** delegate) const
{
	return _get_function_pointer(
		type_name,
		method_name,
		delegate_type_name,
		nullptr,
		nullptr,
		delegate
	);
}

LuaSTGPlus::CLRHost::~CLRHost()
{
}