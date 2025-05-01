#include "LuaBinding/LuaWrapper.hpp"
#include "core/FileSystem.hpp"
#include "utility/path.hpp"
#include "AppFrame.h"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace luastg::binding {

	struct Archive::Wrapper {
		core::IFileSystemArchive* data{};
	};

	void Archive::Register(lua_State* L)noexcept {
		struct Function {
			static int IsValid(lua_State* L) {
				lua::stack_t const vm(L);
				auto const self = as(L, 1);
				vm.push_value(self->data != nullptr);
				return 1;
			}
			static int EnumFiles(lua_State* L) {
				lua::stack_t const vm(L);
				auto const self = as(L, 1);

				//                                              self directory
				auto const result_table = vm.create_array(); // self directory results
				if (self->data == nullptr) {
					return 1;
				}

				auto const directory = vm.get_value<std::string_view>(1 + 1);
				core::SmartReference<core::IFileSystemEnumerator> enumerator;
				if (!self->data->createEnumerator(enumerator.put(), directory, false)) {
					return 1;
				}

				int32_t i{ 1 };
				while (enumerator->next()) {
					auto const node_table = vm.create_array(); // self directory results node
					vm.set_array_value(node_table, 1, enumerator->getName());
					vm.set_array_value(node_table, 2, enumerator->getNodeType() == core::FileSystemNodeType::directory);
					vm.set_array_value(result_table, i, node_table);
					vm.pop_value(); // self directory results
					++i;
				}

				return 1;
			}
			static int ListFiles(lua_State* L) {
				lua::stack_t const vm(L);
				auto const self = as(L, 1);

				//                                              self directory
				auto const result_table = vm.create_array(); // self directory results
				if (self->data == nullptr) {
					return 1;
				}

				auto const directory = vm.get_value<std::string_view>(1 + 1);
				core::SmartReference<core::IFileSystemEnumerator> enumerator;
				if (!self->data->createEnumerator(enumerator.put(), directory, true)) {
					return 1;
				}

				int32_t i{ 1 };
				while (enumerator->next()) {
					auto const node_table = vm.create_array(); // self directory results node
					vm.set_array_value(node_table, 1, enumerator->getName());
					vm.set_array_value(node_table, 2, enumerator->getNodeType() == core::FileSystemNodeType::directory);
					vm.set_array_value(result_table, i, node_table);
					vm.pop_value(); // self directory results
					++i;
				}

				return 1;
			}
			static int FileExist(lua_State* L) {
				lua::stack_t const vm(L);
				auto const self = as(L, 1);
				if (self->data == nullptr) {
					vm.push_value(false);
					return 1;
				}

				auto const path = vm.get_value<std::string_view>(1 + 1);
				vm.push_value(self->data->hasFile(path));
				return 1;
			}
			static int GetName(lua_State* L) {
				lua::stack_t const vm(L);
				auto const self = as(L, 1);
				if (self->data == nullptr) {
					vm.push_value(std::nullopt);
					return 1;
				}
				vm.push_value(self->data->getArchivePath());
				return 1;
			}
			static int GetPriority(lua_State* L) {
				[[maybe_unused]] auto const self = as(L, 1);
				lua_pushinteger(L, 0);
				return 1;
			}
			static int SetPriority(lua_State* L) {
				[[maybe_unused]] auto const self = as(L, 1);
				return 0;
			}

			static int Meta_ToString(lua_State* L)noexcept {
				lua::stack_t const vm(L);
				if (auto const self = as(L, 1); self->data != nullptr) {
					vm.push_value(std::format("lstg.Archive(\"{}\")", self->data->getArchivePath()));
				}
				else {
					vm.push_value("lstg.Archive(null)"sv);
				}
				return 1;
			}
			static int Meta_GC(lua_State* L)noexcept {
				if (auto const self = as(L, 1); self->data != nullptr) {
					self->data->release();
					self->data = nullptr;
				}
				return 0;
			}

			static Archive::Wrapper* as(lua_State* L, int const i) {
				return static_cast<Archive::Wrapper*>(luaL_checkudata(L, i, LUASTG_LUA_TYPENAME_ARCHIVE));
			}
		};

		luaL_Reg tMethods[] = {
			{ "IsValid", &Function::IsValid },
			{ "EnumFiles", &Function::EnumFiles },
			{ "ListFiles", &Function::ListFiles },
			{ "FileExist", &Function::FileExist },
			{ "GetName", &Function::GetName },
			{ "GetPriority", &Function::GetPriority },
			{ "SetPriority", &Function::SetPriority },
			{},
		};

		luaL_Reg tMetaTable[] = {
			{ "__tostring", &Function::Meta_ToString },
			{ "__gc", &Function::Meta_GC },
			{},
		};

		RegisterMethodD(L, LUASTG_LUA_TYPENAME_ARCHIVE, tMethods, tMetaTable);
	}

	void Archive::CreateAndPush(lua_State* L, core::IFileSystemArchive* const archive)noexcept {
		Archive::Wrapper* p = static_cast<Archive::Wrapper*>(lua_newuserdata(L, sizeof(Archive::Wrapper))); // udata
		p->data = archive;
		if (p->data) {
			p->data->retain();
		}
		luaL_getmetatable(L, LUASTG_LUA_TYPENAME_ARCHIVE); // udata mt
		lua_setmetatable(L, -2); // udata 
	}
}
