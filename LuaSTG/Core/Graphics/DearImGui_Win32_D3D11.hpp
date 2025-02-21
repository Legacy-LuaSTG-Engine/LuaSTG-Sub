#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/Window_Win32.hpp"
#include "Core/Graphics/Device_D3D11.hpp"

namespace Core::Graphics
{
	class DearImGui_Backend
		: public Object<IObject>
		, public IWindowEventListener
		, public IDeviceEventListener
	{
	private:
		struct Win32Message
		{
			HWND   hWnd;
			UINT   uMsg;
			WPARAM wParam;
			LPARAM lParam;
		};
		struct Win32MessageQueueSafe
		{
			size_t const size = 256;
			Win32Message data[256] = {};
			size_t writer_index = 0;
			size_t reader_index = 0;
			HANDLE semaphore_space = NULL;
			HANDLE semaphore_data = NULL;

			bool write(Win32Message const& v)
			{
				if (::WaitForSingleObject(semaphore_space, 0) == WAIT_OBJECT_0)
				{
					data[writer_index] = v;
					writer_index = (writer_index + 1) % size;
					::ReleaseSemaphore(semaphore_data, 1, NULL);
					return true;
				}
				return false;
			}
			bool read(Win32Message& v)
			{
				if (::WaitForSingleObject(semaphore_data, 0) == WAIT_OBJECT_0)
				{
					v = data[reader_index];
					reader_index = (reader_index + 1) % size;
					::ReleaseSemaphore(semaphore_space, 1, NULL);
					return true;
				}
				return false;
			}

			Win32MessageQueueSafe()
			{
				LONG const value = (LONG)size;
				writer_index = 0;
				reader_index = 0;
				semaphore_space = ::CreateSemaphoreExW(NULL, value, value, NULL, 0, SEMAPHORE_ALL_ACCESS);
				semaphore_data = ::CreateSemaphoreExW(NULL, 0, value, NULL, 0, SEMAPHORE_ALL_ACCESS);
				if (semaphore_space == NULL || semaphore_data == NULL)
					throw;
			}
			~Win32MessageQueueSafe()
			{
				writer_index = 0;
				reader_index = 0;
				if (semaphore_space) ::CloseHandle(semaphore_space); semaphore_space = NULL;
				if (semaphore_data) ::CloseHandle(semaphore_data); semaphore_data = NULL;
			}
		};
		struct Win32MessageQueue
		{
			Win32Message data[0x100]{};
			std::atomic_int cave[0x100]{};
			size_t writer_index{ 0 };
			size_t reader_index{ 0 };
			size_t const size = 0x100u;

			bool write(Win32Message const& v)
			{
				int const state_ = cave[writer_index].exchange(0);
				if (state_ == 1)
				{
					data[writer_index] = v;
					cave[writer_index].exchange(2);
					writer_index = (writer_index + 1) % 0x100;
					return true;
				}
				return false;
			}
			bool read(Win32Message& v)
			{
				int const state_ = cave[reader_index].exchange(0);
				if (state_ == 2)
				{
					v = data[reader_index];
					cave[reader_index].exchange(1);
					reader_index = (reader_index + 1) % 0x100;
					return true;
				}
				return false;
			}
			void clear()
			{
				for (auto& v : cave)
				{
					v.store(1);
				}
			}

			Win32MessageQueue()
			{
				clear();
			}
		};
	private:
		ScopeObject<Window_Win32> m_window;
		ScopeObject<Direct3D11::Device> m_device;
		LARGE_INTEGER     m_time = {};
		LARGE_INTEGER     m_freq = {};
		HWND              m_mouse_window = NULL;
		bool              m_is_mouse_tracked = false;
		int               m_mouse_button_down = 0;
		int               m_last_cursor = 0;
		Win32MessageQueue m_queue;
	private:
		bool createWindowResources();
		void onWindowCreate();
		void onWindowDestroy();
		NativeWindowMessageResult onNativeWindowMessage(void* arg1, uint32_t arg2, uintptr_t arg3, intptr_t arg4);
	private:
		void processMessage();
		bool updateMouseCursor();
	public:
		void update();
	public:
		DearImGui_Backend(Window_Win32* p_window, Direct3D11::Device* p_device);
		~DearImGui_Backend();
	};
}
