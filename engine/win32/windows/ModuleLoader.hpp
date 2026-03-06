#pragma once
#include <string_view>

namespace Platform
{
	class ModuleLoader
	{
	private:
	#ifdef _MINWINDEF_
		HMODULE m_module{};
	#else
		void* m_module{};
	#endif
	public:
	#ifdef _MINWINDEF_
		HRESULT Load(std::string_view const name, bool search_only_in_system);
	#else
		long Load(std::string_view const name, bool search_only_in_system);
	#endif
		void* GetFunction(std::string_view const name);
		template<typename F>
		F* GetFunction(std::string_view const name)
		{
			return reinterpret_cast<F*>(GetFunction(name));
		}
	public:
		ModuleLoader();
		ModuleLoader(ModuleLoader const&) = delete;
		ModuleLoader(ModuleLoader&& right) noexcept;
		~ModuleLoader();
	public:
		static bool IsSearchOnlyInSystemSupported();
	};
}
