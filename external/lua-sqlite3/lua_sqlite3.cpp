#include "lua_sqlite3.h"
#include <string_view>
#include <string>
#include <optional>
#include <lua.hpp>
#include <sqlite3.h>

using namespace std::string_view_literals;

namespace lua {
	class StackIndex {
	public:
		//inline explicit StackIndex(int32_t index) noexcept : value_(index) {}
		inline constexpr explicit StackIndex(int32_t index) noexcept : value_(index) {}
		inline int32_t value() const noexcept { return value_; }
	private:
		int32_t const value_;
	};

	constexpr auto arg1 = StackIndex(1);
	constexpr auto arg2 = StackIndex(2);
	constexpr auto arg3 = StackIndex(3);
	constexpr auto arg4 = StackIndex(4);
	constexpr auto arg5 = StackIndex(5);
	constexpr auto arg6 = StackIndex(6);
	constexpr auto arg7 = StackIndex(7);
	constexpr auto arg8 = StackIndex(8);
	constexpr auto arg9 = StackIndex(9);

	class StackBalancer {
	public:
		inline explicit StackBalancer(lua_State* L_) : L(L_), N(lua_gettop(L_)) {}
		inline ~StackBalancer() { lua_settop(L, N); }
	private:
		lua_State* const L;
		int32_t const N;
	};

	class Stack {
	public:
		template<typename T>
		[[nodiscard]] T getValue(StackIndex const index) const {
			if constexpr (std::is_same_v<int32_t, T>) {
				return static_cast<int32_t>(luaL_checkinteger(L, index.value()));
			}
			else if constexpr (std::is_same_v<std::string_view, T>) {
				size_t l{};
				char const* s = luaL_checklstring(L, index.value(), &l);
				return { s, l };
			}
			else {
				static_assert(false, "not implemented");
			}
		}
		template<typename T>
		void pushValue(T const& value) const {
			if constexpr (std::is_same_v<std::nullopt_t, T>) {
				lua_pushnil(L);
			}
			else if constexpr (std::is_same_v<StackIndex, T>) {
				lua_pushvalue(L, value.value());
			}
			else if constexpr (std::is_same_v<bool, T>) {
				lua_pushboolean(L, value ? 1 : 0);
			}
			else if constexpr (std::is_same_v<int32_t, T>) {
				lua_pushinteger(L, value);
			}
			else if constexpr (std::is_same_v<char const*, T>) {
				lua_pushstring(L, value);
			}
			else if constexpr (std::is_same_v<std::string_view, T>) {
				lua_pushlstring(L, value.data(), value.size());
			}
			else if constexpr (std::is_same_v<lua_CFunction, T>) {
				lua_pushcfunction(L, value);
			}
			else {
				static_assert(false, "not implemented");
			}
		}

		[[nodiscard]] StackIndex createMap(size_t cap = 0) const {
			lua_createtable(L, 0, static_cast<int>(cap));
			return StackIndex(lua_gettop(L));
		}

		template<typename T>
		void setMapValue(StackIndex const index, std::string_view const& key, T const& value) const {
			pushValue(key);
			pushValue(value);
			lua_settable(L, index.value());
		}

		[[nodiscard]] StackIndex createMetaTable(std::string_view const& name) const {
			luaL_newmetatable(L, name.data());
			return StackIndex(lua_gettop(L));
		}

		[[nodiscard]] StackIndex pushModule(std::string_view const& name) const {
			constexpr luaL_Reg empty[] = { {} };
			luaL_register(L, name.data(), empty);
			auto const index = lua_gettop(L);
			lua_pushnil(L);
			lua_setglobal(L, name.data());
			return StackIndex(index);
		}
	public:
		inline explicit Stack(lua_State* L_) : L(L_) {}
	private:
		lua_State* const L;
	};
}

namespace {
	struct Database {
		static const std::string_view class_name;

		static void registerClass(lua_State* L);
		static Database* create(lua_State* L);
		static Database* as(lua_State* L, int index);
		static bool is(lua_State* L, int index);

		sqlite3* database;
	};

	const std::string_view Database::class_name{ "sqlite3.Database"sv };

	struct DatabaseBinding : public Database {
		// meta methods

		static int __gc(lua_State* L) {
			auto* self = as(L, 1);
			if (self->database) {
				if (auto const result = sqlite3_close_v2(self->database); result == SQLITE_OK) {
					self->database = nullptr;
				}
			}
			return 0;
		}
		static int __tostring(lua_State* L) {
			lua::Stack S(L);
			[[maybe_unused]] auto* self = as(L, 1);
			S.pushValue(class_name);
			return 1;
		}

		// instance methods

		static int close(lua_State* L) {
			lua::Stack S(L);
			auto* self = as(L, 1);
			if (self->database) {
				if (auto const result = sqlite3_close_v2(self->database); result == SQLITE_OK) {
					self->database = nullptr;
				}
				else {
					S.pushValue(false);
					S.pushValue(sqlite3_errmsg(self->database));
					S.pushValue(result);
					return 3;
				}
			}
			S.pushValue(true);
			return 1;
		}

		// static methods

		static int open(lua_State* L) {
			lua::Stack S(L);
			auto const file_name = S.getValue<std::string_view>(lua::arg1);
			auto const flags = S.getValue<int32_t>(lua::arg2);
			auto* self = create(L);
			if (auto const result = sqlite3_open_v2(file_name.data(), &self->database, flags, nullptr); result != SQLITE_OK) {
				S.pushValue(std::nullopt);
				S.pushValue(sqlite3_errmsg(self->database));
				S.pushValue(result);
				return 3;
			}
			return 1;
		}
	};

	void Database::registerClass(lua_State* L) {
		[[maybe_unused]] lua::StackBalancer SB(L);
		lua::Stack S(L);

		auto const class_table = S.pushModule(class_name);
		S.setMapValue(class_table, "close"sv, &DatabaseBinding::close);
		S.setMapValue(class_table, "open"sv, &DatabaseBinding::open);

		auto const meta_table = S.createMetaTable(class_name);
		S.setMapValue(meta_table, "__gc"sv, &DatabaseBinding::__gc);
		S.setMapValue(meta_table, "__tostring"sv, &DatabaseBinding::__tostring);
		S.setMapValue(meta_table, "__index"sv, class_table);
	}
	Database* Database::create(lua_State* L) {
		auto* self = static_cast<Database*>(lua_newuserdata(L, sizeof(Database)));
		self->database = nullptr;
		luaL_setmetatable(L, class_name.data());
		return self;
	}
	Database* Database::as(lua_State* L, int index) {
		return static_cast<Database*>(luaL_checkudata(L, index, class_name.data()));
	}
	bool Database::is(lua_State* L, int index) {
		return luaL_testudata(L, index, class_name.data()) != nullptr;
	}
}

extern "C" int luaopen_sqlite3(lua_State* L) {
	[[maybe_unused]] lua::StackBalancer SB(L);
	lua::Stack S(L);
	
	Database::registerClass(L);

	auto const module_table = S.pushModule("sqlite"sv);

#define CODE(X) S.setMapValue(module_table, "" #X ""sv, SQLITE_##X)

	CODE(OK);

	CODE(OPEN_READONLY     );
	CODE(OPEN_READWRITE    );
	CODE(OPEN_CREATE       );
	CODE(OPEN_DELETEONCLOSE);
	CODE(OPEN_EXCLUSIVE    );
	CODE(OPEN_AUTOPROXY    );
	CODE(OPEN_URI          );
	CODE(OPEN_MEMORY       );
	CODE(OPEN_MAIN_DB      );
	CODE(OPEN_TEMP_DB      );
	CODE(OPEN_TRANSIENT_DB );
	CODE(OPEN_MAIN_JOURNAL );
	CODE(OPEN_TEMP_JOURNAL );
	CODE(OPEN_SUBJOURNAL   );
	CODE(OPEN_SUPER_JOURNAL);
	CODE(OPEN_NOMUTEX      );
	CODE(OPEN_FULLMUTEX    );
	CODE(OPEN_SHAREDCACHE  );
	CODE(OPEN_PRIVATECACHE );
	CODE(OPEN_WAL          );
	CODE(OPEN_NOFOLLOW     );
	CODE(OPEN_EXRESCODE    );

	return 1;
}
