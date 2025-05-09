#pragma once
#include <string_view>
#include <string>
#include <format>

namespace core {
	class Logger {
	public:
		static void info(std::string_view const& message);
		static void info(std::string const& message);

		template <size_t Size>
		static void info(char const (&message)[Size]) {
			info(trim({ message, Size }));
		}

		template <class... Types>
		static void info(const std::format_string<Types...> fmt, Types&&... args) {
			info(std::vformat(fmt.get(), std::make_format_args(args...)));
		}

		static void warn(std::string_view const& message);
		static void warn(std::string const& message);

		template <size_t Size>
		static void warn(char const (&message)[Size]) {
			warn(trim({ message, Size }));
		}

		template <class... Types>
		static void warn(const std::format_string<Types...> fmt, Types&&... args) {
			warn(std::vformat(fmt.get(), std::make_format_args(args...)));
		}

		static void error(std::string_view const& message);
		static void error(std::string const& message);

		template <size_t Size>
		static void error(char const (&message)[Size]) {
			error(trim({ message, Size }));
		}

		template <class... Types>
		static void error(const std::format_string<Types...> fmt, Types&&... args) {
			error(std::vformat(fmt.get(), std::make_format_args(args...)));
		}

	private:
		static std::string_view trim(std::string_view const& message) {
			if (message.empty()) {
				return {};
			}
			for (size_t i = 0; i < message.size(); ++i) {
				if (message[i] == std::string_view::value_type{}) {
					return message.substr(0, i);
				}
			}
			return message;
		}
	};
}
