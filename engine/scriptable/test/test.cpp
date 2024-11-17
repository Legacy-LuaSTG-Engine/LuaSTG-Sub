#include "core/ScriptEngine.hpp"
#include <iostream>
#include <vector>
#include <filesystem>

using namespace std::string_view_literals;

int main(int argc, char** argv) {
	auto& vm = core::ScriptEngine::getInstance();
	if (!vm.open()) {
		return 1;
	}
	std::filesystem::path const path(argv[0]);
	auto const test_path = path.parent_path() / "test.lua"sv;
	if (auto const load_result = vm.loadFromFile(test_path.lexically_normal().generic_string()); !load_result) {
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
