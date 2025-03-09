#pragma once

#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

namespace LuaSTGPlus
{
	class CLRHost 
	{
	private:
		hostfxr_initialize_for_runtime_config_fn _init_runtime = nullptr;
		hostfxr_get_runtime_delegate_fn _get_delegate = nullptr;
		hostfxr_close_fn _close_context = nullptr;

		load_assembly_and_get_function_pointer_fn _load_assembly_and_get_function_pointer = nullptr;
		load_assembly_fn _load_assembly = nullptr;
		get_function_pointer_fn _get_function_pointer = nullptr;

		const char_t* _config_path;

		void* load_library(const char_t* path);
		void* get_export(void* h, const char* name);

		bool init_hostfxr();
		bool get_dotnet_load_assembly_config(const char_t* config_path);
	public:
		CLRHost(const char_t* config_path);
		bool init();

		inline int load_assembly_and_get_function_pointer(
			const char_t* assembly_path, 
			const char_t* type_name, 
			const char_t* method_name, 
			const char_t* delegate_type_name, 
			/*out*/ void** delegate) const;
		inline int load_assembly(const char_t* assembly_path) const;
		inline int get_function_pointer(const char_t* type_name, 
			const char_t* method_name, 
			const char_t* delegate_type_name, 
			/*out*/ void** delegate);

		~CLRHost();
	};
}