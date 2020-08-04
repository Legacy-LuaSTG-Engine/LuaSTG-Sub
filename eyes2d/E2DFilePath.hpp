#pragma once
#include <string>

namespace Eyes2D {
	namespace Platform {
		//将文件路径格式转换为Windows样式
		inline void PathFormatWin32(char* str, int strlen) {
			for (int index = 0; index < strlen; index++) {
				char& ref = str[index];
				if (ref == '/') {
					ref = '\\';
				}
			}
		}

		inline void PathFormatWin32(wchar_t* str, int strlen) {
			for (int index = 0; index < strlen; index++) {
				wchar_t& ref = str[index];
				if (ref == L'/') {
					ref = L'\\';
				}
			}
		}

		inline void PathFormatWin32(std::string& str) {
			for (auto& c : str) {
				if (c == '/') {
					c = '\\';
				}
			}
		}

		inline void PathFormatWin32(std::wstring& str) {
			for (auto& c : str) {
				if (c == L'/') {
					c = L'\\';
				}
			}
		}

		//将文件路径格式转换为Windows样式，并保持小写，Windows系统有毒
		inline void PathFormatWin32LowCase(char* str, int strlen) {
			for (int index = 0; index < strlen; index++) {
				char& ref = str[index];
				if (ref == '/') {
					ref = '\\';
				}
				else if ((ref >= 'A') && (ref <= 'Z')) {
					ref = ref - 'A' + 'a';
				}
			}
		}

		inline void PathFormatWin32LowCase(wchar_t* str, int strlen) {
			for (int index = 0; index < strlen; index++) {
				wchar_t& ref = str[index];
				if (ref == L'/') {
					ref = L'\\';
				}
				else if ((ref >= L'A') && (ref <= L'Z')) {
					ref = ref - L'A' + L'a';
				}
			}
		}

		inline void PathFormatWin32LowCase(std::string& str) {
			for (auto& c : str) {
				if (c == '/') {
					c = '\\';
				}
				else if ((c >= 'A') && (c <= 'Z')) {
					c = c - 'A' + 'a';
				}
			}
		}

		inline void PathFormatWin32LowCase(std::wstring& str) {
			for (auto& c : str) {
				if (c == L'/') {
					c = L'\\';
				}
				else if ((c >= L'A') && (c <= L'Z')) {
					c = c - L'A' + L'a';
				}
			}
		}

		//将文件路径格式转换为Linux样式，Linux系统的文件系统对路径大小写敏感
		inline void PathFormatLinux(char* str, int strlen) {
			for (int index = 0; index < strlen; index++) {
				char& ref = str[index];
				if (ref == '\\') {
					ref = '/';
				}
			}
		}

		inline void PathFormatLinux(wchar_t* str, int strlen) {
			for (int index = 0; index < strlen; index++) {
				wchar_t& ref = str[index];
				if (ref == L'\\') {
					ref = L'/';
				}
			}
		}

		inline void PathFormatLinux(std::string& str) {
			for (auto& c : str) {
				if (c == '\\') {
					c = '/';
				}
			}
		}

		inline void PathFormatLinux(std::wstring& str) {
			for (auto& c : str) {
				if (c == L'\\') {
					c = L'/';
				}
			}
		}
	}
}
