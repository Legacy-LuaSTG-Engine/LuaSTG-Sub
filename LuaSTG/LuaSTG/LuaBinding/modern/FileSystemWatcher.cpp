#include "FileSystemWatcher.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "lua/plus.hpp"
#include "utf8.hpp"
#include <thread>
#include <mutex>
#include <list>
#include <windows.h>
#include <wil/resource.h>

using std::string_view_literals::operator ""sv;

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

		bool open(std::string_view const& path, uint32_t const filter = default_filter){
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
					spdlog::error("core::MessageQueueBasedFileSystemWatcher::worker (ResetEvent)");
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
					spdlog::error("core::MessageQueueBasedFileSystemWatcher::worker (ReadDirectoryChangesW)");
					return;
				}

				HANDLE const wait_events[2]{ self->m_exit_event.get() , self->m_complete_event.get() };
				DWORD const wait_result = WaitForMultipleObjects(2, wait_events, FALSE, INFINITE);
				if (wait_result == WAIT_FAILED || wait_result == WAIT_TIMEOUT || wait_result == WAIT_ABANDONED) {
					spdlog::error("core::MessageQueueBasedFileSystemWatcher::worker (WaitForMultipleObjects: WAIT_FAILED|WAIT_TIMEOUT|WAIT_ABANDONED)");
					return;
				}
				if (wait_result == WAIT_OBJECT_0) {
					return;
				}
				if (wait_result != (WAIT_OBJECT_0 + 1)) {
					spdlog::error("core::MessageQueueBasedFileSystemWatcher::worker (WaitForMultipleObjects: UNKNOWN)");
					return;
				}

				DWORD transferred_bytes{};
				if (!GetOverlappedResult(self->m_file.get(), &overlapped, &transferred_bytes, TRUE)) {
					spdlog::error("core::MessageQueueBasedFileSystemWatcher::worker (GetOverlappedResult)");
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
					IImmutableString::create(file_name, &info.file_name);
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
}

namespace luastg::binding {

	std::string_view const FileSystemWatcher::class_name{ "lstg.FileSystemWatcher"sv };

	struct FileSystemWatcherBinding : FileSystemWatcher {

		// meta methods

		// NOLINTBEGIN(*-reserved-identifier)

		static int __gc(lua_State* const vm) {
			if (auto const self = as(vm, 1); self->object != nullptr) {
				self->object->release();
				self->object = nullptr;
			}
			return 0;
		}

		static int __tostring(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			[[maybe_unused]] auto const self = as(vm, 1);
			ctx.push_value(class_name);
			return 1;
		}

		static int __eq(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (is(vm, 2)) {
				auto const other = as(vm, 2);
				ctx.push_value(self->object == other->object);
			}
			else {
				ctx.push_value(false);
			}
			return 1;
		}

		// NOLINTEND(*-reserved-identifier)

		// instance methods

		static int next(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (self->object == nullptr) {
				ctx.push_value(false);
				return 1;
			}
			if (!ctx.is_table(1 + 1)) {
				return luaL_typerror(vm, 2, "table");
			}

			core::FileNotifyInformation info;
			if (self->object->next(&info)) {
				constexpr lua::stack_index_t info_table(1 + 1);
				ctx.set_map_value(info_table, "file_name"sv, std::string_view(info.file_name->c_str(), info.file_name->length()));
				ctx.set_map_value(info_table, "action"sv, static_cast<int32_t>(info.action));
				ctx.push_value(true);
			}
			else {
				ctx.push_value(false);
			}

			return 1;
		}
		static int close(lua_State* const vm) {
			if (auto const self = as(vm, 1); self->object != nullptr) {
				self->object->release();
				self->object = nullptr;
			}
			return 0;
		}

		// static methods

		static int create(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const path = ctx.get_value<std::string_view>(1);
			auto const filter = ctx.get_value<uint32_t>(2, core::MessageQueueBasedFileSystemWatcher::default_filter);

			core::SmartReference<core::MessageQueueBasedFileSystemWatcher> object;
			object.attach(new core::MessageQueueBasedFileSystemWatcher);
			if (!object->open(path, filter)) {
				ctx.push_value(std::nullopt);
				return 1;
			}

			auto const self = FileSystemWatcher::create(vm);
			self->object = object.detach();
			return 1;
		}

	};

	bool FileSystemWatcher::is(lua_State* const vm, int const index) {
		return nullptr != luaL_testudata(vm, index, class_name.data());
	}

	FileSystemWatcher* FileSystemWatcher::as(lua_State* const vm, int const index) {
		return static_cast<FileSystemWatcher*>(luaL_checkudata(vm, index, class_name.data()));
	}

	FileSystemWatcher* FileSystemWatcher::create(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<FileSystemWatcher>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->object = nullptr;
		return self;
	}

	void FileSystemWatcher::registerClass(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		// method

		auto const method_table = ctx.create_module(class_name);
		ctx.set_map_value(method_table, "read", &FileSystemWatcherBinding::next);
		ctx.set_map_value(method_table, "close", &FileSystemWatcherBinding::close);
		ctx.set_map_value(method_table, "create", &FileSystemWatcherBinding::create);

		// metatable

		auto const metatable = ctx.create_metatable(class_name);
		ctx.set_map_value(metatable, "__gc", &FileSystemWatcherBinding::__gc);
		ctx.set_map_value(metatable, "__tostring", &FileSystemWatcherBinding::__tostring);
		ctx.set_map_value(metatable, "__eq", &FileSystemWatcherBinding::__eq);
		ctx.set_map_value(metatable, "__index", method_table);
	}

}
