#include <string_view>
#include <string>
#include <filesystem>
#include <fstream>
#include <print>
#include <iostream>
#include "core/FileSystemWindows.hpp"
#include "core/FileSystem.hpp"
#include "core/SmartReference.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
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

/*
TEST(FileSystemArchive, all) {
	core::SmartReference<core::IFileSystemArchive> archive;
	ASSERT_TRUE(core::IFileSystemArchive::createFromFile(R"(（窗口与显示分支）LuaSTG-Sub-v0.21.7.zip)"sv, archive.put()));

	ASSERT_FALSE(archive->hasFile("x"sv));
	ASSERT_FALSE(archive->hasDirectory("y"sv));

	ASSERT_TRUE(archive->hasDirectory("LuaSTG-Sub-v0.21.7/"sv));
	ASSERT_FALSE(archive->hasFile("LuaSTG-Sub-v0.21.7/"sv));

	ASSERT_TRUE(archive->hasDirectory("LuaSTG-Sub-v0.21.7/doc/"sv));
	ASSERT_TRUE(archive->hasDirectory("LuaSTG-Sub-v0.21.7/license/"sv));
	ASSERT_TRUE(archive->hasDirectory("LuaSTG-Sub-v0.21.7/res/"sv));
	ASSERT_TRUE(archive->hasDirectory("LuaSTG-Sub-v0.21.7/src/"sv));
	ASSERT_TRUE(archive->hasDirectory("LuaSTG-Sub-v0.21.7/windows-32bit/"sv));
	ASSERT_FALSE(archive->hasFile("LuaSTG-Sub-v0.21.7/doc/"sv));
	ASSERT_FALSE(archive->hasFile("LuaSTG-Sub-v0.21.7/license/"sv));
	ASSERT_FALSE(archive->hasFile("LuaSTG-Sub-v0.21.7/res/"sv));
	ASSERT_FALSE(archive->hasFile("LuaSTG-Sub-v0.21.7/src/"sv));
	ASSERT_FALSE(archive->hasFile("LuaSTG-Sub-v0.21.7/windows-32bit/"sv));

	ASSERT_TRUE(archive->hasFile("LuaSTG-Sub-v0.21.7/d3dcompiler_47.dll"sv));
	ASSERT_TRUE(archive->hasFile("LuaSTG-Sub-v0.21.7/LuaSTGSub.exe"sv));
	ASSERT_TRUE(archive->hasFile("LuaSTG-Sub-v0.21.7/xaudio2_9redist.dll"sv));
	ASSERT_FALSE(archive->hasDirectory("LuaSTG-Sub-v0.21.7/d3dcompiler_47.dll"sv));
	ASSERT_FALSE(archive->hasDirectory("LuaSTG-Sub-v0.21.7/LuaSTGSub.exe"sv));
	ASSERT_FALSE(archive->hasDirectory("LuaSTG-Sub-v0.21.7/xaudio2_9redist.dll"sv));
}
//*/

TEST(FileSystemOs, readFile) {
	if (!spdlog::get("test")) {
		spdlog::set_default_logger(spdlog::stdout_color_mt("test"));
	}

	auto const file_system = core::IFileSystemOS::getInstance();

	std::filesystem::path const p(u8"Windows.txt"sv);
	std::ofstream file(p, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
	file.close();

	core::SmartReference<core::IData> data;
	ASSERT_FALSE(file_system->readFile("windows.txt"sv, data.put()));
	ASSERT_TRUE(file_system->readFile("Windows.txt"sv, data.put()));
}

TEST(FileSystemOsEnumerator, all) {
	auto const file_system = core::IFileSystemOS::getInstance();

	core::SmartReference<core::IFileSystemEnumerator> enumerator;
	ASSERT_TRUE(file_system->createEnumerator(enumerator.put(), "Core.FileSystem.dir\\/"sv));

	while (enumerator->next()) {
		std::println("{}", enumerator->getName());
	}
}

/*
TEST(FileSystemOsEnumerator, recursive) {
	auto const file_system = core::IFileSystemOS::getInstance();

	core::SmartReference<core::IFileSystemEnumerator> enumerator;
	ASSERT_TRUE(file_system->createEnumerator(enumerator.put(), "Core.FileSystem.dir\\/"sv, true));

	while (enumerator->next()) {
		std::println("{}", enumerator->getName());
	}
}

TEST(FileSystemArchiveEnumerator, all) {
	core::SmartReference<core::IFileSystemArchive> archive;
	ASSERT_TRUE(core::IFileSystemArchive::createFromFile(R"(（窗口与显示分支）LuaSTG-Sub-v0.21.7.zip)"sv, archive.put()));

	core::SmartReference<core::IFileSystemEnumerator> enumerator;
	ASSERT_TRUE(archive->createEnumerator(enumerator.put(), "LuaSTG-Sub-v0.21.7/src"sv));

	while (enumerator->next()) {
		std::println("{}", enumerator->getName());
	}
}

TEST(FileSystemArchiveEnumerator, pattern) {
	core::SmartReference<core::IFileSystemArchive> archive;
	ASSERT_TRUE(core::IFileSystemArchive::createFromFile(R"(（窗口与显示分支）LuaSTG-Sub-v0.21.7.zip)"sv, archive.put()));

	core::SmartReference<core::IFileSystemEnumerator> enumerator;
	ASSERT_TRUE(archive->createEnumerator(enumerator.put(), "LuaSTG-Sub-v0.21.7////////////src"sv, true));

	while (enumerator->next()) {
		std::println("{}", enumerator->getName());
	}
}
//*/