#include <ctime>
#include <string>
#include "LogSystem.h"
#include "Config.h"
#include "Utility.h"
#include "SystemDirectory.hpp"

#define LOGSYS_COMPANY APP_COMPANY
#define LOGSYS_PRODUCT APP_PRODUCT
#define LOGSYS_LOGFILE L"log.txt"

namespace LuaSTGPlus {
	bool checkDirectory(std::wstring& out) {
		bool ok = false;
		std::wstring path; // APPDATA
		if (app::getRoamingAppDataDirectory(path)) {
			BOOL ret;
			path.push_back(L'\\'); // APPDATA/
			path += LOGSYS_COMPANY; // APPDATA/COMPANY
			ret = CreateDirectoryW(path.c_str(), NULL);
			if (ret == TRUE || (ret == FALSE && GetLastError() == ERROR_ALREADY_EXISTS)) {
				path.push_back(L'\\'); // APPDATA/COMPANY/
				path += LOGSYS_PRODUCT; // APPDATA/COMPANY/PRODUCT
				ret = CreateDirectoryW(path.c_str(), NULL);
				if (ret == TRUE || (ret == FALSE && GetLastError() == ERROR_ALREADY_EXISTS)) {
					path.push_back(L'\\'); // APPDATA/COMPANY/PRODUCT/
					out = path;
					ok = true;
				}
			}
		}
		return ok;
	}

	bool checkFile(std::wstring& out) {
		bool ok = false;
		std::wstring filename;
		std::time_t rawtime;
		if (std::time(&rawtime) != (std::time_t)(-1)) {
			std::tm tminfo;
			if (::localtime_s(&tminfo, &rawtime) == 0) {
				const size_t ymdhms_size = 14u + 1u; // 14, plus 1 to make c style string
				filename.resize(ymdhms_size, 0);
				if (std::wcsftime((wchar_t*)filename.data(), ymdhms_size, L"%Y%m%d%H%M%S", &tminfo) == 14u) {
					filename.pop_back(); // remove tail 0
					filename = L"log" + filename + L".txt";
					out = filename;
					ok = true;
				}
			}
		}
		return ok;
	}

	__declspec(noinline) LogSystem& LogSystem::GetInstance() {
		static LogSystem s_Instance;
		return s_Instance;
	}

	LogSystem::LogSystem() {
		std::wstring path = L"";
		std::wstring file = LOGSYS_LOGFILE;
#ifdef USING_SYSTEM_DIRECTORY
		checkDirectory(path);
		//checkFile(file);
#endif // USING_SYSTEM_DIRECTORY
		m_LogFileName = path + file;
		m_LogFile.open(m_LogFileName, std::ios::out);
		if (!m_LogFile.is_open()) {
			Log(LogType::Error, L"无法创建日志文件");
		}
	}

	LogSystem::~LogSystem() {
		if (m_LogFile.is_open()) {
			m_LogFile.flush();
			m_LogFile.close();
		}
	}

	__declspec(noinline) void LogSystem::Log(LogType type, const wchar_t* info, ...) noexcept {
		std::wstring tRet;

		try {
			switch (type)
			{
			case LogType::Error:
				tRet = L"[ERROR] ";
				break;
			case LogType::Warning:
				tRet = L"[WARN]  ";
				break;
			case LogType::Information:
				tRet = L"[INFO]  ";
				break;
			case LogType::Debug:
				tRet = L"[DEBUG] ";
				break;
			case LogType::Fatal:
				tRet = L"[FATAL] ";
				break;
			default:
				tRet = L"[INFO]  ";
				break;
			}

			va_list vargs;
			va_start(vargs, info);
			tRet += std::move(StringFormatV(info, vargs));
			va_end(vargs);
			tRet.push_back(L'\n');
		}
		catch (const std::bad_alloc&) {
			OutputDebugStringW(L"[ERROR] 格式化日志内容时发生内存不足错误");
			return;
		}

		OutputDebugStringW(tRet.c_str());

		try {
			if (m_LogFile.is_open()) {
				m_LogFile << std::move(fcyStringHelper::WideCharToMultiByte(tRet, CP_UTF8));
				m_LogFile.flush();
			}
		}
		catch (const std::bad_alloc&) {
			OutputDebugStringW(L"[ERROR] 写入日志到文件时发生内存不足错误");
			return;
		}
	}
};
