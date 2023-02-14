#include "Core/Audio/Device_XAUDIO2.hpp"

#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

namespace Core::Audio
{
	bool Device_XAUDIO2::refreshAudioDeviceList()
	{
		m_audio_device_list.clear();

		try
		{
			winrt::com_ptr<IMMDeviceEnumerator> device_enumerator = winrt::create_instance<IMMDeviceEnumerator>(__uuidof(MMDeviceEnumerator), CLSCTX_ALL);

			winrt::com_ptr<IMMDeviceCollection> device_list;
			winrt::check_hresult(device_enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, device_list.put()));

			UINT device_count = 0;
			winrt::check_hresult(device_list->GetCount(&device_count));

			for (UINT index = 0; index < device_count; index += 1)
			{
				winrt::com_ptr<IMMDevice> device;
				winrt::check_hresult(device_list->Item(index, device.put()));

				wil::unique_cotaskmem_string id;
				winrt::check_hresult(device->GetId(id.put()));

				winrt::com_ptr<IPropertyStore> prop;
				winrt::check_hresult(device->OpenPropertyStore(STGM_READ, prop.put()));
				assert(id.get());

				wil::unique_prop_variant name;
				winrt::check_hresult(prop->GetValue(PKEY_Device_FriendlyName, name.addressof()));
				assert(name.vt == VT_LPWSTR);

				winrt::hstring name_default = winrt::hstring(L"Device") + winrt::to_hstring(index);

				m_audio_device_list.emplace_back(AudioDeviceInfo{
					winrt::to_string(id.get()),
					winrt::to_string(name.vt == VT_LPWSTR ? name.pwszVal : name_default),
				});
			}

			return true;
		}
		catch (winrt::hresult_error const& e)
		{
			spdlog::error("[core] <winrt::hresult_error> {}", winrt::to_string(e.message()));
		}
		catch (std::exception const& e)
		{
			spdlog::error("[core] <std::exception> {}", e.what());
		}

		return false;
	}
}
