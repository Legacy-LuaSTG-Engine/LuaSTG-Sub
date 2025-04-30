#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/Type.hpp"
#include "lua.hpp"

namespace core {
	enum class FileAction : int32_t {  // NOLINT(performance-enum-size)
		unknown = 0,
		added = 1,
		removed = 2,
		modified = 3,
		renamed_old_name = 4,
		renamed_new_name = 5,
	};

	struct FileNotifyInformation {
		IImmutableString* file_name{};
		FileAction action{};

		FileNotifyInformation() = default;
		FileNotifyInformation(FileNotifyInformation const& other) : file_name(other.file_name), action(other.action) {
			if (file_name != nullptr) {
				file_name->retain();
			}
		}
		FileNotifyInformation(FileNotifyInformation&& other) noexcept : file_name(std::exchange(other.file_name, nullptr)), action(std::exchange(other.action, FileAction::unknown)) {
		}
		~FileNotifyInformation() {
			reset();
		}

		FileNotifyInformation& operator=(FileNotifyInformation const& other) {
			if (this == &other) {
				return *this;
			}

			reset();

			assignFileName(other.file_name);
			action = other.action;

			return *this;
		}
		FileNotifyInformation& operator=(FileNotifyInformation&& other) noexcept {
			if (this == &other) {
				return *this;
			}
			reset();
			std::swap(file_name, other.file_name);
			std::swap(action, other.action);
			return *this;
		}

		void assignFileName(IImmutableString* const new_file_name) {
			if (file_name != nullptr) {
				file_name->release();
				file_name = nullptr;
			}
			this->file_name = new_file_name;
			if (file_name != nullptr) {
				file_name->retain();
			}
		}
		void reset() {
			if (file_name != nullptr) {
				file_name->release();
				file_name = nullptr;
			}
			action = FileAction::unknown;
		}
	};

	struct CORE_NO_VIRTUAL_TABLE IMessageQueueBasedFileSystemWatcher : IReferenceCounted {
		virtual bool next(FileNotifyInformation* info) = 0;
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IMessageQueueBasedFileSystemWatcher
	template<> constexpr InterfaceId getInterfaceId<IMessageQueueBasedFileSystemWatcher>() { return UUID::parse("30b35341-08d1-5144-b3fe-7f96bd29978b"); }
}

namespace luastg::binding {

	struct FileSystemWatcher {

		static std::string_view const class_name;

		[[maybe_unused]] core::IMessageQueueBasedFileSystemWatcher* object{};

		static bool is(lua_State* vm, int index);

		static FileSystemWatcher* as(lua_State* vm, int index);

		static FileSystemWatcher* create(lua_State* vm);

		static void registerClass(lua_State* vm);

	};

}
