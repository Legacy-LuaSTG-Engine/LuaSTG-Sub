#pragma once

namespace xinput
{
	DWORD getState(DWORD dwUserIndex, XINPUT_STATE* pState);
	DWORD setState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);
	DWORD getCapabilities(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities);
}
