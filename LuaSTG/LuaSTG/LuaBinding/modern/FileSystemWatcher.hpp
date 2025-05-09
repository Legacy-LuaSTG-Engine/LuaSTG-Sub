#pragma once
#include "core/FileSystemWatcher.hpp"
#include "lua.hpp"

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
