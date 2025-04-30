#pragma once
#include <string_view>
#include <string>
#include <ranges>
#include <algorithm>
#include <filesystem>

namespace {
	bool isSeparator(char const c) {
		return c == '/' || c == '\\';
	}
	bool isPathEndsWithSeparator(std::string_view const& path) {
		return !path.empty() && isSeparator(path.back());
	}
	bool isPathEquals(std::string_view const& path, std::string_view const& right) {
		if (path.size() != right.size()) {
			return false;
		}
		for (size_t i = 0; i < path.size(); ++i) {
			if (isSeparator(path[i]) && isSeparator(right[i])) {
				continue;
			}
			if (path[i] != right[i]) {
				return false;
			}
		}
		return true;
	}
	bool isPathStartsWith(std::string_view const& path, std::string_view const& start) {
		if (path.size() < start.size()) {
			return false;
		}
		for (size_t i = 0; i < start.size(); ++i) {
			if (isSeparator(path[i]) && isSeparator(start[i])) {
				continue;
			}
			if (path[i] != start[i]) {
				return false;
			}
		}
		return true;
	}
	bool isPatternMatched(std::string_view const& path, std::string const& pattern) {
		using std::string_view_literals::operator ""sv;
        if (pattern.empty()) {
			return true;
		}
		if (pattern.ends_with("**"sv)) {
			auto const head = pattern.substr(0, pattern.size() - 2);
			if (isPathEndsWithSeparator(head) && isPathEquals(path, head)) {
				return false; // exclude parent level directory
			}
			return isPathStartsWith(path, head);
		}
		if (pattern.ends_with("*"sv)) {
			auto const head = pattern.substr(0, pattern.size() - 1);
			if (isPathEndsWithSeparator(head) && isPathEquals(path, head)) {
				return false; // exclude parent level directory
			}
			if (!isPathStartsWith(path, head)) {
				return false;
			}
			auto const tail = path.substr(head.size());
			auto const separator_count = std::ranges::count_if(tail, &isSeparator);
			if (separator_count == 0) {
				return true; // current level file
			}
			if (separator_count == 1 && isSeparator(tail.back())) {
				return true; // current level directory
			}
			return false; // in child directory
		}
		return isPathEquals(path, pattern);
	}
	bool isPathMatched(std::string_view const& path, std::string const& directory, bool const recursive) {
		if (!isPathStartsWith(path, directory)) {
			return false;
		}
		if (path.size() == directory.size() /* isPathEquals(path, directory) */) {
			return false; // exclude 'directory'
		}
		if (recursive) {
			return true;
		}
		auto const leaf = path.substr(directory.size());
		auto const separator_count = std::ranges::count_if(leaf, &isSeparator);
		if (separator_count == 0) {
			return true; // current level file
		}
		if (separator_count == 1 && isPathEndsWithSeparator(leaf)) {
			return true; // current level directory
		}
		return false; // in child directory
	}
	std::u8string_view getUtf8StringView(std::string_view const& s) {
		return { reinterpret_cast<char8_t const*>(s.data()), s.size() };
	}
	std::string_view getStringView(std::u8string_view const& s) {
		return { reinterpret_cast<char const*>(s.data()), s.size() };
	}
    std::u8string normalizePath(std::string_view const& path, bool const no_parent = false) {
        using std::string_view_literals::operator ""sv;
		std::u8string_view const directory_u8(reinterpret_cast<char8_t const*>(path.data()), path.size()); // as utf-8
		std::filesystem::path const directory_path(directory_u8);
		std::u8string normalized = directory_path.lexically_normal().generic_u8string();
		if (normalized == u8"."sv || normalized == u8"/"sv) {
			normalized.clear();
		}
        if (no_parent && normalized.find(u8".."sv) != std::u8string::npos) {
            normalized.clear();
        }
		return normalized;
	}
}
