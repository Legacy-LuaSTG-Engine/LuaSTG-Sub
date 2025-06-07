#include "backend/AudioEngineXAudio2.hpp"
#include "core/Logger.hpp"
#include <windows.h>
#include <wil/resource.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include "utf8.hpp"
#include "win32/base.hpp"

using std::string_view_literals::operator ""sv;

namespace core {
	bool AudioEngineXAudio2::refreshAudioEndpoint() {
		m_endpoints.clear();

		win32::com_ptr<IMMDeviceEnumerator> device_enumerator;
		if (!win32::check_hresult_as_boolean(
			win32::create_instance<MMDeviceEnumerator, IMMDeviceEnumerator>(CLSCTX_ALL, device_enumerator.put()),
			"CoCreateInstance->IMMDeviceEnumerator"sv
		)) {
			return false;
		}

		win32::com_ptr<IMMDeviceCollection> device_list;
		if (!win32::check_hresult_as_boolean(
			device_enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, device_list.put()),
			"IMMDeviceEnumerator::EnumAudioEndpoints"sv
		)) {
			return false;
		}

		UINT device_count = 0;
		if (!win32::check_hresult_as_boolean(device_list->GetCount(&device_count), "IMMDeviceCollection::GetCount"sv)) {
			return false;
		}

		for (UINT index = 0; index < device_count; index += 1) {
			win32::com_ptr<IMMDevice> device;
			if (!win32::check_hresult_as_boolean(device_list->Item(index, device.put()), "IMMDeviceCollection::Item"sv)) {
				return false;
			}

			wil::unique_cotaskmem_string id;
			if (!win32::check_hresult_as_boolean(device->GetId(id.put()), "IMMDevice::GetId"sv)) {
				return false;
			}
			assert(id.get() != nullptr);

			win32::com_ptr<IPropertyStore> prop;
			if (!win32::check_hresult_as_boolean(device->OpenPropertyStore(STGM_READ, prop.put()), "IMMDevice::OpenPropertyStore"sv)) {
				return false;
			}

			wil::unique_prop_variant name;
			if (!win32::check_hresult_as_boolean(prop->GetValue(PKEY_Device_FriendlyName, name.addressof()), "IPropertyStore::GetValue(PKEY_Device_FriendlyName)"sv)) {
				return false;
			}
			assert(name.vt == VT_LPWSTR);

			m_endpoints.emplace_back(
				AudioEndpointInfo{
				.id = utf8::to_string(id.get()),
				.name = name.vt == VT_LPWSTR ? utf8::to_string(name.pwszVal) : std::format("Audio Endpoint {}", index),
				});
		}

		return true;
	}
}
