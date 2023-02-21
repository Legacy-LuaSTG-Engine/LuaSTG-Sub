#include "Platform/Shared.hpp"

#include "Platform/CommandLineArguments.hpp"

namespace Platform
{
	inline void wcstr_to_utf8(WCHAR const* wstr, std::string& u8str)
	{
		int const length = static_cast<int>(wcslen(wstr));
		if (length <= 0)
		{
			return; // empty string
		}
		int const count = WideCharToMultiByte(CP_UTF8, 0, wstr, length, NULL, 0, NULL, NULL);
		if (count <= 0)
		{
			return; // error
		}
		u8str.resize(static_cast<size_t>(count));
		int const write = WideCharToMultiByte(CP_UTF8, 0, wstr, length, u8str.data(), count, NULL, NULL);
		if (write != count)
		{
			u8str.clear();
			return; // error
		}
	}

	bool CommandLineArguments::Update()
	{
		m_args.clear();

		LPWSTR pCmdLine = GetCommandLineW();
		if (!pCmdLine)
		{
			return false;
		}

		int argc = 0;
		LPWSTR* argv = CommandLineToArgvW(pCmdLine, &argc);
		if (!argv)
		{
			return false;
		}

		m_args.resize(static_cast<size_t>(argc));
		for (int i = 0; i < argc; i += 1)
		{
			wcstr_to_utf8(argv[i], m_args[i]);
		}
	
		LocalFree(argv);

		return true;
	}
	bool CommandLineArguments::GetArguments(std::vector<std::string_view>& list)
	{
		if (m_args.empty())
		{
			if (!Update())
			{
				return false;
			}
		}

		list.resize(m_args.size());
		for (size_t i = 0; i < m_args.size(); i += 1)
		{
			list[i] = m_args[i];
		}

		return true;
	}
	bool CommandLineArguments::IsOptionExist(std::string_view option)
	{
		if (m_args.empty())
		{
			if (!Update())
			{
				return false;
			}
		}

		for (auto const& v : m_args)
		{
			if (v == option)
			{
				return true;
			}
		}

		return false;
	}

	CommandLineArguments::CommandLineArguments()
	{
	}
	CommandLineArguments::~CommandLineArguments()
	{
	}

	CommandLineArguments& CommandLineArguments::Get()
	{
		static CommandLineArguments instace;
		return instace;
	}
}

#include "Platform/DesktopWindowManager.hpp"

namespace Platform
{
	class ScopeRegKey
	{
	private:
		HKEY hKey{ NULL };
	public:
		HKEY Get() const noexcept { return hKey; }
		HKEY* GetAddressOf() noexcept { return &hKey; }
		void Close() { if (hKey) RegCloseKey(hKey); hKey = NULL; }
	public:
		ScopeRegKey() = default;
		~ScopeRegKey() { Close(); }
	};

	static BOOL _IsOverlayTestModeExists(DWORD option)
	{
		// 在 MPO 启用且支持工作的情况下，DWM 会专门分配一个 plane 来独立呈现
		// 但是 MPO 似乎因为各种问题给用户带来了困扰，NVIDIA 甚至专门给出了禁用方法
		// https://nvidia.custhelp.com/app/answers/detail/a_id/5157/~/after-updating-to-nvidia-game-ready-driver-461.09-or-newer%2C-some-desktop-apps
		// 禁用 MPO 后，DirectComposition 的延迟会大大增加，因为 DWM 会始终控制合成
		// 为了处理这个特例，我们需要检测该注册表项

		LSTATUS ls = ERROR_SUCCESS;

		ScopeRegKey hKey;
		ls = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\Dwm", 0, KEY_READ | option, hKey.GetAddressOf());
		if (ERROR_SUCCESS != ls) return FALSE;

		DWORD dwValue = 0;
		DWORD dwValueSize = 4;
		DWORD dwType = 0;
		ls = RegQueryValueExW(hKey.Get(), L"OverlayTestMode", NULL, &dwType, (BYTE*)&dwValue, &dwValueSize);
		if (ERROR_SUCCESS != ls) return FALSE;
		if (REG_DWORD != dwType) return FALSE;

		return 5 == dwValue;
	}

	BOOL DesktopWindowManager::IsOverlayTestModeExists()
	{
		if (_IsOverlayTestModeExists(0x0))
		{
			return TRUE; // 理论上在 64 位程序下会触发这个
		}

		if (_IsOverlayTestModeExists(KEY_WOW64_32KEY))
		{
			return TRUE; // 理论上不会触发，但是以防万一，省的微软妈又死了
		}

		if (_IsOverlayTestModeExists(KEY_WOW64_64KEY))
		{
			return TRUE; // 理论上在 32 位程序下会触发这个
		}

		return FALSE;
	}
}

#include "Platform/Direct3D11.hpp"

namespace Platform
{
	HRESULT Direct3D11::GetDeviceAdater(ID3D11Device* pDevice, IDXGIAdapter1** ppAdapter)
	{
		assert(pDevice);
		assert(ppAdapter);

		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<IDXGIDevice> dxgi_device;
		hr = pDevice->QueryInterface(IID_PPV_ARGS(&dxgi_device));
		if (FAILED(hr)) return hr;

		Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter;
		hr = dxgi_device->GetAdapter(&dxgi_adapter);
		if (FAILED(hr)) return hr;

		hr = dxgi_adapter->QueryInterface(IID_PPV_ARGS(ppAdapter));
		if (FAILED(hr)) return hr;

		return hr;
	}
	HRESULT Direct3D11::GetDeviceFactory(ID3D11Device* pDevice, IDXGIFactory2** ppFactory)
	{
		assert(pDevice);
		assert(ppFactory);

		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter;
		hr = GetDeviceAdater(pDevice, &dxgi_adapter);
		if (FAILED(hr)) return hr;

		hr = dxgi_adapter->GetParent(IID_PPV_ARGS(ppFactory));
		if (FAILED(hr)) return hr;

		return hr;
	}
}

#include "Platform/DXGI.hpp"

namespace Platform
{
	BOOL DXGI::CheckFeatureSupportPresentAllowTearing(IDXGIFactory* pFactory)
	{
		Microsoft::WRL::ComPtr<IDXGIFactory> dxgi_factory = pFactory;
		Microsoft::WRL::ComPtr<IDXGIFactory5> dxgi_factory5;
		if (SUCCEEDED(dxgi_factory.As(&dxgi_factory5)))
		{
			BOOL feature_supported = FALSE;
			if (SUCCEEDED(dxgi_factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &feature_supported, sizeof(feature_supported))))
			{
				return feature_supported;
			}
		}
		return FALSE;
	}
	HRESULT DXGI::MakeSwapChainWindowAssociation(IDXGISwapChain* pSwapChain, UINT flags)
	{
		assert(pSwapChain);

		HRESULT hr = S_OK;

		HWND hwnd = NULL;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> pSwapChain1;
		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&pSwapChain1));
		if (SUCCEEDED(hr))
		{
			hr = pSwapChain1->GetHwnd(&hwnd);
			if (FAILED(hr)) return hr; // the swap chain might not be HWND-based
		}
		else
		{
			DXGI_SWAP_CHAIN_DESC desc = {};
			hr = pSwapChain->GetDesc(&desc);
			if (FAILED(hr)) return hr;
			hwnd = desc.OutputWindow;
		}

		Microsoft::WRL::ComPtr<IDXGIDevice> pDxgiDevice;
		hr = pSwapChain->GetDevice(IID_PPV_ARGS(&pDxgiDevice));
		if (FAILED(hr)) return hr;

		Microsoft::WRL::ComPtr<IDXGIAdapter> pDxgiAdapter;
		hr = pDxgiDevice->GetAdapter(&pDxgiAdapter);
		if (FAILED(hr)) return hr;

		Microsoft::WRL::ComPtr<IDXGIFactory> pDxgiFactory;
		hr = pDxgiAdapter->GetParent(IID_PPV_ARGS(&pDxgiFactory));
		if (FAILED(hr)) return hr;

		hr = pDxgiFactory->MakeWindowAssociation(hwnd, flags);
		return hr;
	}
	HRESULT DXGI::SetSwapChainMaximumFrameLatency(IDXGISwapChain* pSwapChain, UINT MaxLatency, HANDLE* pEvent)
	{
		assert(pSwapChain);
		assert(pEvent);

		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<IDXGISwapChain2> dxgi_swapchain2;
		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&dxgi_swapchain2));
		if (FAILED(hr)) return hr;

		hr = dxgi_swapchain2->SetMaximumFrameLatency(MaxLatency);
		if (FAILED(hr)) return hr;

		HANDLE event = dxgi_swapchain2->GetFrameLatencyWaitableObject();
		assert(event);
		if (!event) return E_FAIL;

		*pEvent = event;

		return hr;
	}
	HRESULT DXGI::SetDeviceMaximumFrameLatency(IDXGISwapChain* pSwapChain, UINT MaxLatency)
	{
		assert(pSwapChain);

		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<IDXGIDevice1> dxgi_device;
		hr = pSwapChain->GetDevice(IID_PPV_ARGS(&dxgi_device));
		if (FAILED(hr)) return hr;

		hr = dxgi_device->SetMaximumFrameLatency(MaxLatency);
		if (FAILED(hr)) return hr;

		return hr;
	}
}

#include "Platform/ModuleLoader.hpp"

namespace Platform
{
	HRESULT ModuleLoader::Load(std::string_view const name, bool search_only_in_system)
	{
		assert(!m_module);
		if (m_module)
		{
			FreeLibrary(m_module);
			m_module = NULL;
		}
		DWORD const flags = search_only_in_system ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0;
		m_module = LoadLibraryExW(to_wide(name).c_str(), NULL, flags);
		if (!m_module)
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
		return S_OK;
	}
	void* ModuleLoader::GetFunction(std::string_view const name)
	{
		assert(m_module);
		std::array<char, 256> buffer{};
		std::pmr::monotonic_buffer_resource resource(buffer.data(), buffer.size());
		std::pmr::string const name_c_str(name, &resource);
		return static_cast<void*>(GetProcAddress(m_module, name_c_str.c_str()));
	}

	ModuleLoader::ModuleLoader() = default;
	ModuleLoader::ModuleLoader(ModuleLoader&& right) noexcept
		: m_module(std::exchange(right.m_module, m_module))
	{
	}
	ModuleLoader::~ModuleLoader()
	{
		if (m_module)
		{
			FreeLibrary(m_module);
			m_module = NULL;
		}
	}

	bool ModuleLoader::IsSearchOnlyInSystemSupported()
	{
		ModuleLoader kernel32_loader;
		if (FAILED(kernel32_loader.Load("kernel32.dll", true)))
		{
			return false; // 一般到这一步就已经知道支不支持了
		}
		auto api_SetDefaultDllDirectories = kernel32_loader.GetFunction<decltype(SetDefaultDllDirectories)>("SetDefaultDllDirectories");
		auto api_AddDllDirectory = kernel32_loader.GetFunction<decltype(AddDllDirectory)>("AddDllDirectory");
		auto api_RemoveDllDirectory = kernel32_loader.GetFunction<decltype(RemoveDllDirectory)>("RemoveDllDirectory");
		if (!api_SetDefaultDllDirectories || !api_AddDllDirectory || !api_RemoveDllDirectory)
		{
			return false; // 以防万一，检测这三个函数
		}
		return true;
	}
}
