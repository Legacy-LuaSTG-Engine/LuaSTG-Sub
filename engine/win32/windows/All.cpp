#include "Shared.hpp"
#include "RuntimeLoader/DXGI.hpp"
#include "RuntimeLoader/Direct3D11.hpp"

#include "DesktopWindowManager.hpp"

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

#include "Direct3D11.hpp"

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
	BOOL Direct3D11::HasDevice(UINT Flags, D3D_FEATURE_LEVEL TargetFeatureLevel)
	{
		HRESULT hr = S_OK;
		RuntimeLoader::DXGI dxgi;
		RuntimeLoader::Direct3D11 d3d11;
		Microsoft::WRL::ComPtr<IDXGIFactory1> dxgi_factory;
		hr = dxgi.CreateFactory(IID_PPV_ARGS(&dxgi_factory));
		if (FAILED(hr)) return FALSE;
		Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter;
		UINT count = 0;
		for (UINT index = 0; SUCCEEDED(dxgi_factory->EnumAdapters1(index, &dxgi_adapter)); index += 1)
		{
			DXGI_ADAPTER_DESC1 dxgi_adapter_info{};
			hr = dxgi_adapter->GetDesc1(&dxgi_adapter_info);
			if (FAILED(hr)) continue;
			// skip software device
			if (dxgi_adapter_info.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
			if (dxgi_adapter_info.Flags & DXGI_ADAPTER_FLAG_REMOTE) continue;
			// test feature level
			hr = d3d11.CreateDeviceFromAdapter(dxgi_adapter.Get(), Flags, TargetFeatureLevel,
				NULL, NULL, NULL);
			if (SUCCEEDED(hr)) count += 1;
		}
		return count > 0 ? TRUE : FALSE;
	}
}

#include "DXGI.hpp"

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

#include "ModuleLoader.hpp"

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

#include "ApplicationSingleInstance.hpp"

namespace Platform
{
	static constexpr std::string_view error_message_en_us_uuid{"UUID cannot be empty."};
	static constexpr std::string_view error_message_zh_cn_uuid{"UUID不能为空。"};
	static constexpr std::string_view error_message_en_us_convert{"Convert string encoding failed."};
	static constexpr std::string_view error_message_zh_cn_convert{"转换字符串编码失败。"};
	static constexpr std::string_view error_message_en_us_initialize{"Single instance application model has been enabled."};
	static constexpr std::string_view error_message_zh_cn_initialize{"单实例应用程序模型已被启用。"};
	static constexpr std::string_view error_message_en_us_create{"Launch single application instance failed."};
	static constexpr std::string_view error_message_zh_cn_create{"启动单实例应用程序失败。"};
	static constexpr std::string_view error_message_en_us_test{"Launch multiple application instances is not allowed, please terminate the existing instance."};
	static constexpr std::string_view error_message_zh_cn_test{"无法启动多个应用程序实例，请终止已存在的实例。"};

	inline bool IsChineseSimplified()
	{
		LANGID const language_chinese_simplified = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
		// TODO: This is a deprecated API
	#pragma warning(push)
	#pragma warning(disable: 6387)
		LANGID const language_user_default = LANGIDFROMLCID(LocaleNameToLCID(LOCALE_NAME_USER_DEFAULT, 0));
	#pragma warning(pop)
		return language_user_default == language_chinese_simplified;
	}
	inline std::wstring Utf8ToWide(std::string_view const str)
	{
		std::wstring wstr;
		int const length = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
		if (length <= 0) {
			return wstr;
		}
		wstr.resize(static_cast<size_t>(length));
		int const result = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), wstr.data(), length);
		if (length != result) {
			wstr.clear();
		}
		return wstr;
	}
	inline std::string_view LocalizeString(std::string_view const en_us, std::string_view const zh_cn)
	{
		return IsChineseSimplified() ? zh_cn : en_us;
	}

	void ApplicationSingleInstance::ShowErrorWindow(std::string_view const text)
	{
		std::wstring wide_text(Utf8ToWide(text));
		std::wstring wide_title(Utf8ToWide(m_title));
		MessageBoxW(
			nullptr,
			wide_text.c_str(),
			wide_title.c_str(),
			MB_ICONERROR | MB_OK
		);
	}
	void ApplicationSingleInstance::Clear()
	{
		if (m_event) {
			CloseHandle(m_event);
			m_event = nullptr;
		}
	}

	void ApplicationSingleInstance::SetErrorWindowTitle(std::string_view const title)
	{
		m_title = title;
	}
	void ApplicationSingleInstance::Initialize(std::string_view const uuid)
	{
		if (m_event != nullptr) {
			ShowErrorWindow(LocalizeString(error_message_en_us_initialize, error_message_zh_cn_initialize));
			std::exit(EXIT_FAILURE);
		}
		if (uuid.empty()) {
			ShowErrorWindow(LocalizeString(error_message_en_us_uuid, error_message_zh_cn_uuid));
			std::exit(EXIT_FAILURE);
		}
		std::string name;
		name.append("Local\\ApplicationInstance-");
		name.append(uuid);
		std::wstring wide_name(Utf8ToWide(name));
		if (wide_name.empty()) {
			// unlikely
			ShowErrorWindow(LocalizeString(error_message_en_us_convert, error_message_zh_cn_convert));
			std::exit(EXIT_FAILURE);
		}
		SetLastError(0);
		m_event = CreateEventExW(nullptr, wide_name.c_str(), 0, EVENT_ALL_ACCESS);
		if (nullptr == m_event) {
			ShowErrorWindow(LocalizeString(error_message_en_us_create, error_message_zh_cn_create));
			std::exit(EXIT_FAILURE);
		}
		if (ERROR_ALREADY_EXISTS == GetLastError()) {
			Clear();
			ShowErrorWindow(LocalizeString(error_message_en_us_test, error_message_zh_cn_test));
			std::exit(EXIT_FAILURE);
		}
	}

	ApplicationSingleInstance::ApplicationSingleInstance() = default;
	ApplicationSingleInstance::ApplicationSingleInstance(std::string_view const title) : m_title(title) {}
	ApplicationSingleInstance::ApplicationSingleInstance(std::string_view const title, std::string_view const uuid) : m_title(title)
	{
		Initialize(uuid);
	}
	ApplicationSingleInstance::~ApplicationSingleInstance()
	{
		Clear();
	}
}
