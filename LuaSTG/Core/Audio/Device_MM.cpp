#include "Core/Audio/Device_XAUDIO2.hpp"

#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include "utf8.hpp"

namespace core::Audio
{
	bool Device_XAUDIO2::refreshAudioDeviceList()
	{
		m_audio_device_list.clear();

		try
		{
			win32::com_ptr<IMMDeviceEnumerator> device_enumerator;
			win32::check_hresult_throw_if_failed(win32::create_instance<MMDeviceEnumerator, IMMDeviceEnumerator>(CLSCTX_ALL, device_enumerator.put()));
			
			win32::com_ptr<IMMDeviceCollection> device_list;
			win32::check_hresult_throw_if_failed(device_enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, device_list.put()));

			UINT device_count = 0;
			win32::check_hresult_throw_if_failed(device_list->GetCount(&device_count));

			for (UINT index = 0; index < device_count; index += 1)
			{
				win32::com_ptr<IMMDevice> device;
				win32::check_hresult_throw_if_failed(device_list->Item(index, device.put()));

				wil::unique_cotaskmem_string id;
				win32::check_hresult_throw_if_failed(device->GetId(id.put()));

				win32::com_ptr<IPropertyStore> prop;
				win32::check_hresult_throw_if_failed(device->OpenPropertyStore(STGM_READ, prop.put()));
				assert(id.get());

				wil::unique_prop_variant name;
				win32::check_hresult_throw_if_failed(prop->GetValue(PKEY_Device_FriendlyName, name.addressof()));
				assert(name.vt == VT_LPWSTR);

				m_audio_device_list.emplace_back(AudioDeviceInfo{
					utf8::to_string(id.get()),
					name.vt == VT_LPWSTR ? utf8::to_string(name.pwszVal) : std::string("Device"),
				});
			}

			return true;
		}
		catch (win32::hresult_error const& e)
		{
			spdlog::error("[core] <win32::hresult_error> {}", e.message());
		}
		catch (std::exception const& e)
		{
			spdlog::error("[core] <std::exception> {}", e.what());
		}

		return false;
	}
}
