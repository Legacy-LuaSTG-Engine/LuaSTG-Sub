#include "core/ScriptEngine.hpp"
#include <iostream>
#include <vector>

using namespace std::string_view_literals;

int main() {
	auto& vm = core::ScriptEngine::getInstance();
	if (!vm.open()) {
		return 1;
	}
	if (auto const load_result = vm.loadFromFile("main.lua"sv); !load_result) {
		std::cout << load_result.getMessage() << std::endl;
	}
	std::vector<std::string_view> tests{
		"test_call_error"sv,
		"test_call_error_with_nil"sv,
		"test_call_error_with_false"sv,
		"test_call_error_with_true"sv,
		"test_call_error_with_pi"sv,
		"test_call_error_with_hello"sv,
		"test_call_error_with_table"sv,
		"test_call_error_with_table_x"sv,
		"test_call_error_with_userdata"sv,
		"call_self"sv,
		"call_f0"sv,
	};
	for (auto const& test : tests) {
		std::cout << "========== test: "sv << test << std::endl;
		auto const call_result = vm.call(test);
		if (!call_result) {
			std::cout << call_result.getMessage() << std::endl;
		}
	}
	vm.close();
	return 0;
}
