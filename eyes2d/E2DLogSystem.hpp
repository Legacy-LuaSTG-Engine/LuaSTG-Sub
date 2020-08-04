#pragma once
#include <mutex>
#include "E2DGlobal.hpp"

#ifndef EYESLOGFILE
#define EYESLOGFILE "eyeslog.txt"
#endif

#ifdef ERROR
#undef ERROR //$B M$
#endif

namespace Eyes2D {
	namespace Debug {
		//log级别
		enum class LogLevel {
			DEBUG = 0,//调试信息，含有详细的运行时内容
			INFO  = 1,//信息，一般性的消息
			WARN  = 2,//警告，问题可能会影响到结果的正确性
			ERROR = 3,//出错，错误可能严重到程序无法继续执行
		};

		//log级别转换到数字索引
		inline int LogLevelToIndex(LogLevel lv) {
			switch (lv)
			{
			case LogLevel::DEBUG:
				return 0;
			case LogLevel::INFO:
				return 1;
			case LogLevel::WARN:
				return 2;
			case LogLevel::ERROR:
				return 3;
			default:
				return 0;
			}
		}

		//数字索引转换到log级别
		inline LogLevel IndexToLogLevel(int index) {
			switch (index)
			{
			case 0:
				return LogLevel::DEBUG;
			case 1:
				return LogLevel::INFO;
			case 2:
				return LogLevel::WARN;
			case 3:
				return LogLevel::ERROR;
			default:
				return LogLevel::DEBUG;
			}
		}
		
		//简单的log系统，多例模式
		class EYESDLLAPI LogSystem {
		private:
			struct Impl;
			Impl* m_Impl;
		public:
			LogSystem();
			~LogSystem();
		public:
			//输出UTF8编码的字符串
			void Log(LogLevel lv, const char* msg);
			//输出ANSI编码的字符串，false时不转换为UTF8，true时转换
			void Log(LogLevel lv, const char* msg, bool utf8);
			//输出UTF16编码的字符串
			void Log(LogLevel lv, const wchar_t* msg);
			//输出E2DException异常格式化字符串
			void Log(LogLevel lv, E2DException& e);
		};

		//简单的log系统，输出到log文件
		class EYESDLLAPI LogFileSystem {
		private:
			struct Impl;
			Impl* m_Impl;
		public:
			LogFileSystem(const wchar_t* logfile);
			~LogFileSystem();
		public:
			//输出UTF8编码的字符串
			void Log(LogLevel lv, const char* msg);
			//输出ANSI编码的字符串，false时不转换为UTF8，true时转换
			void Log(LogLevel lv, const char* msg, bool utf8);
			//输出UTF16编码的字符串
			void Log(LogLevel lv, const wchar_t* msg);
			//输出E2DException异常格式化字符串
			void Log(LogLevel lv, E2DException& e);
		};
	}

	//Log系统，多例模式
	inline Debug::LogSystem& GetLogger() {
		static Debug::LogSystem gs_LogSystemInstance;
		return gs_LogSystemInstance;
	}
	
	//输出log

	inline void EYESDEBUG(const char* msg) {
#ifndef NDEBUG
		GetLogger().Log(Debug::LogLevel::DEBUG, msg);
#endif // !NDEBUG
	}
	inline void EYESDEBUG(const wchar_t* msg) {
#ifndef NDEBUG
		GetLogger().Log(Debug::LogLevel::DEBUG, msg);
#endif // !NDEBUG
	}
	inline void EYESDEBUG(E2DException& e) {
#ifndef NDEBUG
		GetLogger().Log(Debug::LogLevel::DEBUG, e);
#endif // !NDEBUG
	}
	inline void EYESINFO(const char* msg) {
		GetLogger().Log(Debug::LogLevel::INFO, msg);
	}
	inline void EYESINFO(const wchar_t* msg) {
		GetLogger().Log(Debug::LogLevel::INFO, msg);
	}
	inline void EYESINFO(E2DException& e) {
		GetLogger().Log(Debug::LogLevel::INFO, e);
	}
	inline void EYESWARN(const char* msg) {
		GetLogger().Log(Debug::LogLevel::WARN, msg);
	}
	inline void EYESWARN(const wchar_t* msg) {
		GetLogger().Log(Debug::LogLevel::WARN, msg);
	}
	inline void EYESWARN(E2DException& e) {
		GetLogger().Log(Debug::LogLevel::WARN, e);
	}
	inline void EYESERROR(const char* msg) {
		GetLogger().Log(Debug::LogLevel::ERROR, msg);
	}
	inline void EYESERROR(const wchar_t* msg) {
		GetLogger().Log(Debug::LogLevel::ERROR, msg);
	}
	inline void EYESERROR(E2DException& e) {
		GetLogger().Log(Debug::LogLevel::ERROR, e);
	}
}
