#pragma once

namespace Platform
{
	class DisplayModeUpdater
	{
	private:
		DISPLAY_DEVICEW last_device{};
		DEVMODEW last_mode{};
		bool is_scope{ false };
	public:
		bool Enter(HWND window, UINT width, UINT height);
		void Leave();
	public:
		DisplayModeUpdater();
		~DisplayModeUpdater();
	};
}
