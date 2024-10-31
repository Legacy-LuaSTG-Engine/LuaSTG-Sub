#pragma once
#include <cstdint>
#include <string_view>
#include <string>
#include <unordered_map>
#include "lua.hpp"

namespace http {
	enum class RequestMethod : int32_t {
		get = 1,
		head,
		post,
		put,
		del, // delete is a c/c++ keyword
		patch,
	};

	struct Request {
		static std::string_view class_name;

		// MEMBER BEGIN
		RequestMethod request_method{ RequestMethod::get };
		std::string url;
		std::unordered_map<std::string, std::string> headers;
		std::string body;
		int32_t resolve_timeout{};
		int32_t connect_timeout{};
		int32_t send_timeout{};
		int32_t receive_timeout{};

		Request() = default;

		~Request() = default;

		// MEMBER END

		static bool is(lua_State* L, int index);

		static Request* as(lua_State* L, int index);

		static Request* create(lua_State* L);

		static void registerClass(lua_State* L);
	};

	struct ResponseEntity {
		static std::string_view class_name;

		// MEMBER BEGIN
		std::unordered_map<std::string, std::string> headers;
		std::string body;

		ResponseEntity() = default;

		~ResponseEntity() = default;

		// MEMBER END

		static bool is(lua_State* L, int index);

		static ResponseEntity* as(lua_State* L, int index);

		static ResponseEntity* create(lua_State* L);

		static void registerClass(lua_State* L);
	};
}
