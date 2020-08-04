#include <fstream>
#include <sstream>
#include "E2DLogSystem.hpp"
#include "E2DCodePage.hpp"

using namespace std;
using namespace Eyes2D::Debug;

//======================================

struct LogSystem::Impl {
	string levelstring[4];
	wstring levelwstring[4];
	LogSystem::Impl() {
		levelstring[0] = "[DEBUG] ";
		levelstring[1] = "[INFO]  ";
		levelstring[2] = "[WARN]  ";
		levelstring[3] = "[ERROR] ";
		levelwstring[0] = L"[DEBUG] ";
		levelwstring[1] = L"[INFO]  ";
		levelwstring[2] = L"[WARN]  ";
		levelwstring[3] = L"[ERROR] ";
	}
};

LogSystem::LogSystem() {
	m_Impl = new LogSystem::Impl;
#ifndef NDEBUG
	OutputDebugStringW(L"[DEBUG] Eyes2D::Debug::LogSystem instance was created.\n");
#endif // !NDEBUG
}

LogSystem::~LogSystem() {
	delete m_Impl;
#ifndef NDEBUG
	OutputDebugStringW(L"[DEBUG] Eyes2D::Debug::LogSystem instance was deleted.\n");
#endif // !NDEBUG
}

void LogSystem::Log(LogLevel lv, const char* msg) {
	int index = LogLevelToIndex(lv);

	wostringstream woss;

	wstring toutf16 = std::move(Eyes2D::String::UTF8ToUTF16(string(msg)));
	woss << m_Impl->levelwstring[index] << toutf16 << L"\n";
	OutputDebugStringW(woss.str().c_str());
}

void LogSystem::Log(LogLevel lv, const char* msg, bool utf8) {
	if (utf8) {
		int index = LogLevelToIndex(lv);
		
		wostringstream woss;

		wstring toutf16 = std::move(Eyes2D::String::ANSIToUTF16(string(msg)));
		woss << m_Impl->levelwstring[index] << toutf16 << L"\n";
		OutputDebugStringW(woss.str().c_str());
	}
	else {
		Log(lv, msg);
	}
}

void LogSystem::Log(LogLevel lv, const wchar_t* msg) {
	int index = LogLevelToIndex(lv);
	
	wostringstream woss;

	woss << m_Impl->levelwstring[index] << msg << L"\n";
	OutputDebugStringW(woss.str().c_str());
}

void LogSystem::Log(LogLevel lv, Eyes2D::E2DException& e) {
	int index = LogLevelToIndex(lv);
	
	wostringstream woss;

	woss << m_Impl->levelwstring[index] << e.errDesc << L" (" << e.errSrc << L")\n";
	OutputDebugStringW(woss.str().c_str());
}

//======================================

struct LogFileSystem::Impl {
	fstream logfile;
	string levelstring[4];
	wstring levelwstring[4];
	LogFileSystem::Impl() {
		levelstring[0] = "[DEBUG] ";
		levelstring[1] = "[INFO]  ";
		levelstring[2] = "[WARN]  ";
		levelstring[3] = "[ERROR] ";
		levelwstring[0] = L"[DEBUG] ";
		levelwstring[1] = L"[INFO]  ";
		levelwstring[2] = L"[WARN]  ";
		levelwstring[3] = L"[ERROR] ";
	}
};

LogFileSystem::LogFileSystem(const wchar_t* logfile) {
	m_Impl = new LogFileSystem::Impl;
	int mode = ios::out | ios::ate | ios::app | ios::trunc;
	string toansi = Eyes2D::String::UTF16ToANSI(wstring(logfile));
	m_Impl->logfile.open(toansi, mode);
	if (!m_Impl->logfile.is_open())
	{
		m_Impl->logfile.clear();//clear the fstream error state
		m_Impl->logfile.open(toansi, ios::out);//confirm that file is exist
		m_Impl->logfile.close();
		m_Impl->logfile.open(toansi, mode);
		if (!m_Impl->logfile.is_open()) {
			throw E2DException(0, 0, L"Eyes2D::Debug::LogFileSystem::LogFileSystem", L"Failed to create log file.");
		}
	}
}

LogFileSystem::~LogFileSystem() {
	if (m_Impl->logfile.is_open()) {
		m_Impl->logfile.flush();
		m_Impl->logfile.close();
	}
	delete m_Impl;
}

void LogFileSystem::Log(LogLevel lv, const char* msg) {
	int index = LogLevelToIndex(lv);

	wostringstream woss;

	wstring toutf16 = std::move(Eyes2D::String::UTF8ToUTF16(string(msg)));
	woss << m_Impl->levelwstring[index] << toutf16 << L"\n";
	OutputDebugStringW(woss.str().c_str());

	ostringstream oss;

	oss << m_Impl->levelstring[index] << msg << u8"\n";

	m_Impl->logfile << oss.str();
	m_Impl->logfile.flush();
}

void LogFileSystem::Log(LogLevel lv, const char* msg, bool utf8) {
	if (utf8) {
		int index = LogLevelToIndex(lv);

		wostringstream woss;

		wstring toutf16 = std::move(Eyes2D::String::ANSIToUTF16(string(msg)));
		woss << m_Impl->levelwstring[index] << toutf16 << L"\n";
		OutputDebugStringW(woss.str().c_str());

		ostringstream oss;

		string toutf8 = std::move(Eyes2D::String::ANSIToUTF8(string(msg)));
		oss << m_Impl->levelstring[index] << toutf8 << u8"\n";

		m_Impl->logfile << oss.str();
		m_Impl->logfile.flush();
	}
	else {
		Log(lv, msg);
	}
}

void LogFileSystem::Log(LogLevel lv, const wchar_t* msg) {
	int index = LogLevelToIndex(lv);

	wostringstream woss;

	woss << m_Impl->levelwstring[index] << msg << L"\n";
	OutputDebugStringW(woss.str().c_str());

	ostringstream oss;

	string toutf8 = std::move(Eyes2D::String::UTF16ToUTF8(wstring(msg)));
	oss << m_Impl->levelstring[index] << toutf8 << u8"\n";

	m_Impl->logfile << oss.str();
	m_Impl->logfile.flush();
}

void LogFileSystem::Log(LogLevel lv, Eyes2D::E2DException& e) {
	int index = LogLevelToIndex(lv);

	wostringstream woss;

	woss << m_Impl->levelwstring[index] << e.errDesc << L" (" << e.errSrc << L")\n";
	OutputDebugStringW(woss.str().c_str());

	ostringstream oss;

	string toutf8src = std::move(Eyes2D::String::UTF16ToUTF8(e.errSrc));
	string toutf8desc = std::move(Eyes2D::String::UTF16ToUTF8(e.errDesc));
	oss << m_Impl->levelstring[index] << toutf8desc << u8" (" << toutf8src << u8")\n";

	m_Impl->logfile << oss.str();
	m_Impl->logfile.flush();
}
