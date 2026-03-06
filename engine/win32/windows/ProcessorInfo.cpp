#include "Shared.hpp"
#include "ProcessorInfo.hpp"

namespace Platform {
	std::string ProcessorInfo::name() {
		DWORD type{};
		WCHAR buffer[256]{};
		DWORD size{ sizeof(buffer) };
		const LSTATUS result = RegGetValueW(
			HKEY_LOCAL_MACHINE,
			LR"(HARDWARE\DESCRIPTION\System\CentralProcessor\0)",
			L"ProcessorNameString",
			RRF_RT_REG_SZ,
			&type,
			buffer,
			&size
		);
		if (ERROR_SUCCESS == result) {
			return to_utf8(buffer);
		}
		return "Unknown";
	}
}
