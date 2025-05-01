#include "core/FileSystemWatcher.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "core/Logger.hpp"
#include "core/FileSystemCommon.hpp"
#include "utf8.hpp"
#include <cassert>
#include <mutex>
#include <thread>
#include <list>
#include <windows.h>
#include <wil/resource.h>

namespace core {
	class MessageQueueBasedFileSystemWatcher final : public implement::ReferenceCounted<IMessageQueueBasedFileSystemWatcher> {
	public:
		// IMessageQueueBasedFileSystemWatcher

		bool next(FileNotifyInformation* info) override {
			assert(info != nullptr);
			if (info == nullptr) {
				return false;
			}
			std::lock_guard notify_lock(m_notify_mutex);
			if (m_notify.empty()) {
				return false;
			}
			*info = m_notify.front();
			m_notify.pop_front();
			return true;
		}

		// MessageQueueBasedFileSystemWatcher

		MessageQueueBasedFileSystemWatcher() = default;
		MessageQueueBasedFileSystemWatcher(MessageQueueBasedFileSystemWatcher const&) = delete;
		MessageQueueBasedFileSystemWatcher(MessageQueueBasedFileSystemWatcher&&) = delete;
		~MessageQueueBasedFileSystemWatcher() override {
			SetEvent(m_exit_event.get());
			if (m_worker.joinable()) {
				m_worker.join();
			}
		}

		MessageQueueBasedFileSystemWatcher& operator=(MessageQueueBasedFileSystemWatcher const&) = delete;
		MessageQueueBasedFileSystemWatcher& operator=(MessageQueueBasedFileSystemWatcher&&) = delete;

		static constexpr uint32_t default_filter = FILE_NOTIFY_CHANGE_FILE_NAME
			| FILE_NOTIFY_CHANGE_DIR_NAME
			| FILE_NOTIFY_CHANGE_SIZE
			| FILE_NOTIFY_CHANGE_LAST_WRITE
			| FILE_NOTIFY_CHANGE_CREATION;

		bool open(std::string_view const& path, uint32_t const filter = default_filter) {
			m_exit_event.reset(CreateEventExW(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS));
			if (!m_exit_event.is_valid()) {
				return false;
			}

			m_complete_event.reset(CreateEventExW(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS));
			if (!m_complete_event.is_valid()) {
				return false;
			}

			auto const path_w = utf8::to_wstring(path);
			m_file.reset(CreateFileW(
				path_w.c_str(),
				FILE_LIST_DIRECTORY | GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				nullptr,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
				nullptr
			));
			if (!m_file.is_valid()) {
				return false;
			}

			m_notify_filter = filter;

			m_worker = std::thread(&worker, this);
			return true;
		}

		static void worker(MessageQueueBasedFileSystemWatcher* self) {
			std::array<DWORD, 1024> buffer;

			for (;;) {
				buffer.fill(0);
				if (!ResetEvent(self->m_complete_event.get())) {
					Logger::error("core::MessageQueueBasedFileSystemWatcher::worker (ResetEvent)");
					return;
				}

				OVERLAPPED overlapped{};
				overlapped.hEvent = self->m_complete_event.get();
				if (!ReadDirectoryChangesW(
					self->m_file.get(),
					buffer.data(),
					sizeof(buffer),
					TRUE,
					self->m_notify_filter,
					nullptr,
					&overlapped,
					nullptr
				)) {
					Logger::error("core::MessageQueueBasedFileSystemWatcher::worker (ReadDirectoryChangesW)");
					return;
				}

				HANDLE const wait_events[2]{ self->m_exit_event.get() , self->m_complete_event.get() };
				DWORD const wait_result = WaitForMultipleObjects(2, wait_events, FALSE, INFINITE);
				if (wait_result == WAIT_FAILED || wait_result == WAIT_TIMEOUT || wait_result == WAIT_ABANDONED) {
					Logger::error("core::MessageQueueBasedFileSystemWatcher::worker (WaitForMultipleObjects: WAIT_FAILED|WAIT_TIMEOUT|WAIT_ABANDONED)");
					return;
				}
				if (wait_result == WAIT_OBJECT_0) {
					return;
				}
				if (wait_result != (WAIT_OBJECT_0 + 1)) {
					Logger::error("core::MessageQueueBasedFileSystemWatcher::worker (WaitForMultipleObjects: UNKNOWN)");
					return;
				}

				DWORD transferred_bytes{};
				if (!GetOverlappedResult(self->m_file.get(), &overlapped, &transferred_bytes, TRUE)) {
					Logger::error("core::MessageQueueBasedFileSystemWatcher::worker (GetOverlappedResult)");
					return;
				}

				auto const begin = reinterpret_cast<uint8_t*>(buffer.data());
				auto const end = begin + transferred_bytes;
				auto ptr = begin;
				while (ptr < end) {
					auto const cur = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(ptr);
					ptr += cur->NextEntryOffset;

					FileNotifyInformation info{};
					auto const file_name = utf8::to_string(std::wstring_view(cur->FileName));
					auto const normalized = normalizePath(file_name);
					IImmutableString::create(getStringView(normalized), &info.file_name);
					info.action = static_cast<FileAction>(cur->Action);

					{
						std::lock_guard notify_lock(self->m_notify_mutex);
						self->m_notify.emplace_back(info);
					}

					if (cur->NextEntryOffset == 0) {
						break;
					}
				}
			}
		}

	private:
		wil::unique_event m_exit_event;
		wil::unique_event m_complete_event;
		wil::unique_hfile m_file;
		std::thread m_worker;
		std::list<FileNotifyInformation> m_notify;
		std::recursive_mutex m_notify_mutex;
		DWORD m_notify_filter{};
	};

	bool IMessageQueueBasedFileSystemWatcher::create(std::string_view const& path, IMessageQueueBasedFileSystemWatcher** const object) {
		assert(object != nullptr);
		if (object == nullptr) {
			return false;
		}
		SmartReference<MessageQueueBasedFileSystemWatcher> temp;
		temp.attach(new MessageQueueBasedFileSystemWatcher);
		if (!temp->open(path, MessageQueueBasedFileSystemWatcher::default_filter)) {
			return false;
		}
		*object = temp.detach();
		return true;
	}
}
