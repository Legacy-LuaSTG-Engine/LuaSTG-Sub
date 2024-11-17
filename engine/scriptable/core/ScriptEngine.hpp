#pragma once
#include <string_view>
#include <string>

namespace core {
	class ScriptEngine {
	public:
		class CallResult {
			friend class ScriptEngine;
		public:
			inline explicit CallResult(bool status) : message(status ? "" : "unknown error") {}
			inline explicit CallResult(std::string_view const& s) : message(s) {}
			inline explicit CallResult(std::string&& s) : message(s) {}
			inline operator bool() const noexcept { return message.empty(); }
			inline std::string const& getMessage() const noexcept { return message; }
		private:
			std::string message;
		};
	public:
		CallResult loadFromFile(std::string_view const& path);
		CallResult call(std::string_view const& function_name);
		bool open();
		void close();
		void* getNativeHandle();
	public:
		ScriptEngine();
		~ScriptEngine();
	public:
		static ScriptEngine& getInstance();
	private:
		bool registerStackTraceBackHandler();
	private:
		void* handle;
	};
}
