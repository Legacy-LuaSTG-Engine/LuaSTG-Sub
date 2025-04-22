#include <string_view>
#include <string>
#include <filesystem>
#include <fstream>
#include "core/FileSystemWindows.hpp"
#include "gtest/gtest.h"

using std::string_view_literals::operator ""sv;

TEST(FileSystemWindows, isFilePathCaseCorrect) {
	std::filesystem::path const path1(LR"(Hello world 你好世界.txt)"sv);
	std::ofstream file1(path1);
	file1.close();

	std::string c1;
	ASSERT_TRUE(win32::isFilePathCaseCorrect(R"(Hello world 你好世界.txt)"sv, c1));
	ASSERT_FALSE(win32::isFilePathCaseCorrect(R"(hello world 你好世界.txt)"sv, c1));
}
