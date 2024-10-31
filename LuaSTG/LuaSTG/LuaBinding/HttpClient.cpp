#include "HttpClient.hpp"

#include <cassert>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#define NOSERVICE
#define NOMCX
#define NOIME
#include <Windows.h>
#include <winhttp.h>
#include <winrt/base.h>
#include <wil/resource.h>
#include <wil/result_macros.h>

#pragma comment(lib, "winhttp.lib")

static void setHeaders(HINTERNET const handle, std::unordered_map<std::string, std::string> const& headers) {
	assert(handle != nullptr);
	if (headers.empty()) {
		return;
	}
	std::string headers_buffer;
	for (auto const& [name, value] : headers) {
		headers_buffer.append(name);
		headers_buffer.append(": ");
		headers_buffer.append(value);
		headers_buffer.append("\r\n");
	}
	if (headers_buffer.ends_with("\r\n")) {
		headers_buffer.pop_back();
		headers_buffer.pop_back();
	}

	auto const headers_wide = winrt::to_hstring(headers_buffer);
	THROW_IF_WIN32_BOOL_FALSE(WinHttpAddRequestHeaders(
		handle,
		headers_wide.c_str(),
		headers_wide.size(),
		0
	));
}

static std::string_view trim(std::string_view const& input) {
	auto const begin = input.find_first_not_of(' ');
	auto const end = input.find_last_not_of(' ');
	if (begin == std::string_view::npos || end == std::string_view::npos) {
		return {};
	}
	return input.substr(begin, end - begin + 1);
}

static void parseResponseHeaders(std::string const& response_headers, std::unordered_map<std::string, std::string>& headers) {
	if (response_headers.empty()) {
		return;
	}

	std::vector<std::string_view> header_lines; {
		constexpr std::string_view line_break("\r\n");
		std::string_view view(response_headers);
		while (!view.empty()) {
			auto const next_line_break_index = view.find_first_of(line_break);
			if (next_line_break_index == std::string_view::npos) {
				header_lines.emplace_back(view);
				break;
			}
			if (next_line_break_index == 0) {
				view = view.substr(next_line_break_index + 2); // skip \r\n
				continue;
			}
			auto const current_line = view.substr(0, next_line_break_index);
			view = view.substr(next_line_break_index + 2); // skip \r\n
			if (!current_line.empty() && current_line != line_break) {
				header_lines.emplace_back(current_line);
			}
		}
	}
	std::ignore = nullptr;

	for (auto const& line : header_lines) {
		if (line.starts_with("HTTP/")) {
			continue; // status line
		}
		auto const separator_index = line.find_first_of(':');
		if (separator_index == std::string_view::npos) {
			continue;
		}
		auto const name = line.substr(0, separator_index);
		auto const value = line.substr(separator_index + 1);
		headers.emplace(trim(name), trim(value));
	}
}

static int verifyUrl(lua_State* L, std::string_view const& url) {
	URL_COMPONENTS url_components{};
	url_components.dwStructSize = sizeof(url_components);
	url_components.dwSchemeLength = static_cast<DWORD>(-1);
	url_components.dwHostNameLength = static_cast<DWORD>(-1);
	url_components.dwUserNameLength = static_cast<DWORD>(-1);
	url_components.dwPasswordLength = static_cast<DWORD>(-1);
	url_components.dwUrlPathLength = static_cast<DWORD>(-1);
	url_components.dwExtraInfoLength = static_cast<DWORD>(-1);
	auto const url_wide = winrt::to_hstring(url);
	auto const result = WinHttpCrackUrl(
		url_wide.c_str(), url_wide.size(), 0, &url_components);
	if (!result) {
		switch (GetLastError()) {
		case ERROR_WINHTTP_INVALID_URL:
			return luaL_error(L, "invalid url");
		case ERROR_WINHTTP_UNRECOGNIZED_SCHEME:
			return luaL_error(L, "unsupported scheme");
		default:
			return luaL_error(L, "unknown error");
		}
	}
	return 0x0d000721;
}

namespace lua {
	struct StackIndex {
		int32_t value{};

		explicit StackIndex(int32_t const value) noexcept : value(value) {
		}
	};

	class StackBalancer {
	public:
		explicit StackBalancer(lua_State* L) : L(L), N(lua_gettop(L)) {
		}

		~StackBalancer() { lua_settop(L, N); }

	private:
		lua_State* L;
		int32_t N;
	};

	class Stack {
	public:
		explicit Stack(lua_State* L) noexcept : L(L) {
		}

		[[nodiscard]] StackIndex indexOfTop() const {
			return StackIndex(lua_gettop(L));
		}

		void pushValue(std::nullopt_t const) const {
			lua_pushnil(L);
		}

		void pushValue(bool const value) const {
			lua_pushboolean(L, value ? 1 : 0);
		}

		void pushValue(std::string_view const& value) const {
			lua_pushlstring(L, value.data(), value.length());
		}

		void pushValue(StackIndex const value) const {
			lua_pushvalue(L, value.value);
		}

		template<typename T>
		[[nodiscard]] T getValue(StackIndex index) const;

		template<typename T>
		[[nodiscard]] T getValue(int32_t const index) const {
			return getValue<T>(StackIndex(index));
		}

		// module and class system

		[[nodiscard]] StackIndex pushModule(std::string_view const& name) const {
			constexpr luaL_Reg empty[] = { {} };
			luaL_register(L, name.data(), empty);
			auto const index = lua_gettop(L);
			lua_pushnil(L);
			lua_setglobal(L, name.data());
			return StackIndex(index);
		}

		[[nodiscard]] StackIndex createMetaTable(std::string_view const& name) const {
			luaL_newmetatable(L, name.data());
			return StackIndex(lua_gettop(L));
		}

		void setMetaTable(StackIndex const index, std::string_view const& name) const {
			luaL_getmetatable(L, name.data());
			lua_setmetatable(L, index.value);
		}

		template<typename T>
		[[nodiscard]] T* createUserData() const {
			return static_cast<T*>(lua_newuserdata(L, sizeof(T)));
		}

		template<typename T>
		[[nodiscard]] T* createUserDataWithNew() const {
			auto p = static_cast<T*>(lua_newuserdata(L, sizeof(T)));
			return new(p) T();
		}

		// map

		[[nodiscard]] StackIndex createMap(size_t const reserve = 0) const {
			lua_createtable(L, 0, static_cast<int>(reserve));
			return indexOfTop();
		}

		void setMapValue(StackIndex const index, std::string_view const& key, lua_CFunction const value) const {
			lua_pushcfunction(L, value);
			lua_setfield(L, index.value, key.data());
		}

		void setMapValue(StackIndex const index, std::string_view const& key, StackIndex const value_index) const {
			lua_pushvalue(L, value_index.value);
			lua_setfield(L, index.value, key.data());
		}

	private:
		lua_State* L{};
	};

	template<>
	[[nodiscard]] int32_t Stack::getValue<int32_t>(StackIndex const index) const {
		return static_cast<int32_t>(luaL_checkinteger(L, index.value));
	}

	template<>
	[[nodiscard]] std::string_view Stack::getValue<std::string_view>(StackIndex const index) const {
		size_t len{};
		auto str = luaL_checklstring(L, index.value, &len);
		// managed by lua VM
		// ReSharper disable once CppDFALocalValueEscapesFunction
		return { str, len };
	}

	template<>
	[[nodiscard]] std::string Stack::getValue<std::string>(StackIndex const index) const {
		size_t len{};
		auto str = luaL_checklstring(L, index.value, &len);
		// managed by lua VM
		// ReSharper disable once CppDFALocalValueEscapesFunction
		return { str, len };
	}

	static_assert(sizeof(Stack) == sizeof(lua_State*));
	static_assert(alignof(Stack) == alignof(lua_State*));
}

namespace http {
	std::string_view Request::class_name{ "http.Request" };

	struct RequestBinding : Request {
		// meta methods

		static int /* NOLINT(*-reserved-identifier) */ __gc(lua_State* L) {
			auto const self = as(L, 1);
			self->~Request();
			return 0;
		}

		static int /* NOLINT(*-reserved-identifier) */ __tostring(lua_State* L) {
			lua::Stack const S(L);
			[[maybe_unused]] auto const self = as(L, 1);
			S.pushValue(class_name);
			return 1;
		}

		// request methods

		static int setResolveTimeout(lua_State* L) {
			lua::Stack const S(L);
			auto const self = as(L, 1);
			auto const timeout = S.getValue<int32_t>(2);
			self->resolve_timeout = timeout;
			S.pushValue(lua::StackIndex(1)); // return self
			return 1;
		}

		static int setConnectTimeout(lua_State* L) {
			lua::Stack const S(L);
			auto const self = as(L, 1);
			auto const timeout = S.getValue<int32_t>(2);
			self->connect_timeout = timeout;
			S.pushValue(lua::StackIndex(1)); // return self
			return 1;
		}

		static int setSendTimeout(lua_State* L) {
			lua::Stack const S(L);
			auto const self = as(L, 1);
			auto const timeout = S.getValue<int32_t>(2);
			self->send_timeout = timeout;
			S.pushValue(lua::StackIndex(1)); // return self
			return 1;
		}

		static int setReceiveTimeout(lua_State* L) {
			lua::Stack const S(L);
			auto const self = as(L, 1);
			auto const timeout = S.getValue<int32_t>(2);
			self->receive_timeout = timeout;
			S.pushValue(lua::StackIndex(1)); // return self
			return 1;
		}

		static int addHeader(lua_State* L) {
			lua::Stack const S(L);
			auto const self = as(L, 1);
			auto const name = S.getValue<std::string_view>(2);
			auto const value = S.getValue<std::string_view>(3);
			self->headers.emplace(name, value);
			S.pushValue(lua::StackIndex(1)); // return self
			return 1;
		}

		static int body(lua_State* L) {
			lua::Stack const S(L);
			auto const self = as(L, 1);
			auto const body = S.getValue<std::string_view>(2);
			self->body = body;
			S.pushValue(lua::StackIndex(1)); // return self
			return 1;
		}

		static int execute(lua_State* L) {
			lua::Stack const S(L);
			auto const self = as(L, 1);

			try {
				BOOL br{};

				// decode url

				URL_COMPONENTS url_components{};
				url_components.dwStructSize = sizeof(url_components);
				url_components.dwSchemeLength = static_cast<DWORD>(-1);
				url_components.dwHostNameLength = static_cast<DWORD>(-1);
				url_components.dwUserNameLength = static_cast<DWORD>(-1);
				url_components.dwPasswordLength = static_cast<DWORD>(-1);
				url_components.dwUrlPathLength = static_cast<DWORD>(-1);
				url_components.dwExtraInfoLength = static_cast<DWORD>(-1);
				auto const url_wide = winrt::to_hstring(self->url);
				br = WinHttpCrackUrl(url_wide.c_str(), url_wide.size(), 0, &url_components);
				THROW_IF_WIN32_BOOL_FALSE_MSG(br, "WinHttpCrackUrl failed");

				// open session

				wil::unique_winhttp_hinternet session;
				session.reset(WinHttpOpen(
					nullptr,
					WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
					// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
					WINHTTP_NO_PROXY_NAME,
					// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
					WINHTTP_NO_PROXY_BYPASS,
					0
				));
				THROW_LAST_ERROR_IF_NULL_MSG(session.get(), "WinHttpOpen failed");

				br = WinHttpSetTimeouts(
					session.get(),
					self->resolve_timeout,
					self->connect_timeout,
					self->send_timeout,
					self->receive_timeout);
				THROW_IF_WIN32_BOOL_FALSE_MSG(br, "WinHttpSetTimeouts failed");

				// open connect

				std::wstring schema(url_components.lpszScheme, url_components.dwSchemeLength);
				std::wstring host_name(url_components.lpszHostName, url_components.dwHostNameLength);
				wil::unique_winhttp_hinternet connect;
				connect.reset(WinHttpConnect(
					session.get(),
					host_name.c_str(),
					schema == L"https" ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT,
					0
				));
				THROW_LAST_ERROR_IF_NULL_MSG(connect.get(), "WinHttpConnect failed");

				// open request

				std::wstring path(url_components.lpszUrlPath);
				if (path.empty()) {
					path.push_back(L'/');
				}
				wil::unique_winhttp_hinternet request;
				request.reset(WinHttpOpenRequest(
					connect.get(),
					self->request_method == RequestMethod::post ? L"POST" : L"GET",
					path.c_str(),
					nullptr,
					// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
					WINHTTP_NO_REFERER,
					// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
					WINHTTP_DEFAULT_ACCEPT_TYPES,
					schema == L"https" ? WINHTTP_FLAG_SECURE : 0
				));
				THROW_LAST_ERROR_IF_NULL_MSG(request.get(), "WinHttpOpenRequest failed");

				// send request

				setHeaders(request.get(), self->headers);

				br = WinHttpSendRequest(
					request.get(),
					// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
					WINHTTP_NO_ADDITIONAL_HEADERS, 0,
					// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
					WINHTTP_NO_REQUEST_DATA, 0,
					0, 0
				);
				THROW_IF_WIN32_BOOL_FALSE_MSG(br, "WinHttpSendRequest failed");

				// receive response

				THROW_IF_WIN32_BOOL_FALSE(WinHttpReceiveResponse(request.get(), nullptr));

				DWORD response_headers_size{};
				SetLastError(ERROR_SUCCESS);
				br = WinHttpQueryHeaders(
					request.get(),
					WINHTTP_QUERY_RAW_HEADERS_CRLF,
					// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
					WINHTTP_HEADER_NAME_BY_INDEX,
					// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
					WINHTTP_NO_OUTPUT_BUFFER,
					&response_headers_size,
					// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
					WINHTTP_NO_HEADER_INDEX
				);
				if (!br && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
					THROW_IF_WIN32_BOOL_FALSE_MSG(br, "WinHttpQueryHeaders failed");
				}

				std::wstring response_headers_buffer((response_headers_size + 1) / sizeof(std::wstring::value_type),
					L'\0');
				SetLastError(ERROR_SUCCESS);
				br = WinHttpQueryHeaders(
					request.get(),
					WINHTTP_QUERY_RAW_HEADERS_CRLF,
					// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
					WINHTTP_HEADER_NAME_BY_INDEX,
					// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
					response_headers_buffer.data(),
					&response_headers_size,
					// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
					WINHTTP_NO_HEADER_INDEX
				);
				THROW_IF_WIN32_BOOL_FALSE_MSG(br, "WinHttpQueryHeaders failed");

				response_headers_buffer.resize(response_headers_size / sizeof(std::wstring::value_type));
				auto const response_headers = winrt::to_string(response_headers_buffer);
				self->headers.clear();
				parseResponseHeaders(response_headers, self->headers);

				std::string buffer;
				for (;;) {
					DWORD bytes_available{};
					br = WinHttpQueryDataAvailable(request.get(), &bytes_available);
					THROW_IF_WIN32_BOOL_FALSE_MSG(br, "WinHttpQueryDataAvailable failed");
					if (bytes_available == 0) {
						break;
					}

					std::string temp_buffer(bytes_available, '\0');
					DWORD bytes_read{};
					br = WinHttpReadData(request.get(), temp_buffer.data(), bytes_available, &bytes_read);
					THROW_IF_WIN32_BOOL_FALSE_MSG(br, "WinHttpReadData failed");
					if (bytes_read == 0) {
						break;
					}
					temp_buffer.resize(bytes_read);
					buffer.append(temp_buffer);
				}

				self->body = buffer;
			}
			catch (wil::ResultException const& e) {
				std::wstring message_buffer(65536, L'\0');
				wil::GetFailureLogString(message_buffer.data(), message_buffer.size(), e.GetFailureInfo());
				auto const message = winrt::to_string(message_buffer);
				return luaL_error(L, message.c_str());
			}
			catch (std::exception const& e) {
				return luaL_error(L, e.what());
			}

			auto response_entity = ResponseEntity::create(L);
			response_entity->headers = self->headers;
			response_entity->body = self->body;
			return 1;
		}

		// static methods

		static int get(lua_State* L) {
			lua::Stack const S(L);
			auto const self = create(L);
			auto const url = S.getValue<std::string_view>(1);
			verifyUrl(L, url);
			self->request_method = RequestMethod::get;
			self->url = url;
			return 1;
		}

		static int post(lua_State* L) {
			lua::Stack const S(L);
			auto const self = create(L);
			auto const url = S.getValue<std::string_view>(1);
			verifyUrl(L, url);
			self->request_method = RequestMethod::post;
			self->url = url;
			return 1;
		}
	};

	bool Request::is(lua_State* L, int const index) {
		if (lua_type(L, index) != LUA_TUSERDATA) {
			return false;
		}
		if (lua_getmetatable(L, index) == 0) {
			return false;
		}
		luaL_getmetatable(L, class_name.data());
		bool const equal = lua_equal(L, -2, -1) == 1;
		lua_pop(L, 2);
		return equal;
	}

	Request* Request::as(lua_State* L, int const index) {
		return static_cast<Request*>(luaL_checkudata(L, index, class_name.data()));
	}

	Request* Request::create(lua_State* L) {
		lua::Stack const S(L);
		auto const self = S.createUserDataWithNew<Request>();
		S.setMetaTable(S.indexOfTop(), class_name);
		return self;
	}

	void Request::registerClass(lua_State* L) {
		[[maybe_unused]] lua::StackBalancer const SB(L);
		lua::Stack const S(L);

		// method

		auto const methods = S.pushModule(class_name);
		S.setMapValue(methods, "get", &RequestBinding::get);
		S.setMapValue(methods, "post", &RequestBinding::post);
		S.setMapValue(methods, "setResolveTimeout", &RequestBinding::setResolveTimeout);
		S.setMapValue(methods, "setConnectTimeout", &RequestBinding::setConnectTimeout);
		S.setMapValue(methods, "setSendTimeout", &RequestBinding::setSendTimeout);
		S.setMapValue(methods, "setReceiveTimeout", &RequestBinding::setReceiveTimeout);
		S.setMapValue(methods, "addHeader", &RequestBinding::addHeader);
		S.setMapValue(methods, "body", &RequestBinding::body);
		S.setMapValue(methods, "execute", &RequestBinding::execute);

		// meta method

		auto const meta = S.createMetaTable(class_name);
		S.setMapValue(meta, "__gc", &RequestBinding::__gc);
		S.setMapValue(meta, "__tostring", &RequestBinding::__tostring);
		S.setMapValue(meta, "__index", methods);
	}
}

namespace http {
	std::string_view ResponseEntity::class_name{ "http.ResponseEntity" };

	struct ResponseEntityBinding : ResponseEntity {
		// meta methods

		static int /* NOLINT(*-reserved-identifier) */ __gc(lua_State* L) {
			auto const self = as(L, 1);
			self->~ResponseEntity();
			return 0;
		}

		static int /* NOLINT(*-reserved-identifier) */ __tostring(lua_State* L) {
			lua::Stack const S(L);
			[[maybe_unused]] auto const self = as(L, 1);
			S.pushValue(class_name);
			return 1;
		}

		// methods

		static int hasHeader(lua_State* L) {
			lua::Stack const S(L);
			auto const self = as(L, 1);
			auto const name = S.getValue<std::string>(2);
			S.pushValue(self->headers.contains(name));
			return 1;
		}

		static int getHeader(lua_State* L) {
			lua::Stack const S(L);
			auto const self = as(L, 1);
			// ReSharper disable once CppTooWideScopeInitStatement
			auto const name = S.getValue<std::string>(2);
			if (self->headers.contains(name)) {
				S.pushValue(self->headers.at(name));
			}
			else {
				S.pushValue(std::nullopt);
			}
			return 1;
		}

		static int body(lua_State* L) {
			lua::Stack const S(L);
			auto const self = as(L, 1);
			S.pushValue(self->body);
			return 1;
		}
	};

	bool ResponseEntity::is(lua_State* L, int const index) {
		if (lua_type(L, index) != LUA_TUSERDATA) {
			return false;
		}
		if (lua_getmetatable(L, index) == 0) {
			return false;
		}
		luaL_getmetatable(L, class_name.data());
		bool const equal = lua_equal(L, -2, -1) == 1;
		lua_pop(L, 2);
		return equal;
	}

	ResponseEntity* ResponseEntity::as(lua_State* L, int const index) {
		return static_cast<ResponseEntity*>(luaL_checkudata(L, index, class_name.data()));
	}

	ResponseEntity* ResponseEntity::create(lua_State* L) {
		lua::Stack const S(L);
		auto const self = S.createUserDataWithNew<ResponseEntity>();
		S.setMetaTable(S.indexOfTop(), class_name);
		return self;
	}

	void ResponseEntity::registerClass(lua_State* L) {
		[[maybe_unused]] lua::StackBalancer const SB(L);
		lua::Stack const S(L);

		// method

		auto const methods = S.pushModule(class_name);
		S.setMapValue(methods, "hasHeader", &ResponseEntityBinding::hasHeader);
		S.setMapValue(methods, "getHeader", &ResponseEntityBinding::getHeader);
		S.setMapValue(methods, "body", &ResponseEntityBinding::body);

		// meta method

		auto const meta = S.createMetaTable(class_name);
		S.setMapValue(meta, "__gc", &ResponseEntityBinding::__gc);
		S.setMapValue(meta, "__tostring", &ResponseEntityBinding::__tostring);
		S.setMapValue(meta, "__index", methods);
	}
}
