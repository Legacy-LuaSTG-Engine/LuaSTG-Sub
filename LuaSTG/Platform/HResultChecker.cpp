#include "Shared.hpp"
#include "HResultChecker.hpp"

namespace Platform
{
	struct Lock
	{
		struct ScopeLock
		{
			CRITICAL_SECTION& sec_;
			ScopeLock(CRITICAL_SECTION& sec) : sec_(sec)
			{
				EnterCriticalSection(&sec_);
			}
			~ScopeLock()
			{
				LeaveCriticalSection(&sec_);
			}
		};

		CRITICAL_SECTION sec_;

		ScopeLock lock()
		{
			return ScopeLock(sec_);
		}

		Lock()
		{
			InitializeCriticalSection(&sec_);
		}
		~Lock()
		{
			DeleteCriticalSection(&sec_);
		}
	};

	static void DefaultPrintCallback(std::string_view const message)
	{
		OutputDebugStringA(message.data());
	}

	static HResultChecker::PrintCallback g_pfn = &DefaultPrintCallback;

	HResultChecker& HResultChecker::operator=(HResult const hr)
	{
		if (FAILED(hr))
		{
			static Lock lock_;
			static WCHAR buffer_[2048]{}; // 去掉空终止字符和换行符，实际上只有 2046 可用

			// 先锁上，重置缓冲区
			auto scope_lock_ = lock_.lock();
			ZeroMemory(buffer_, sizeof(buffer_));
			// 源文件和行号，错误号
		#ifdef _DEBUG
			int const head_result_ = std::swprintf(
				buffer_,
				2046u,
				L"系统调用出错，文件：'%s' 第 %d 行：(HRESULT = 0x%08X) ",
				source_, line_, static_cast<DWORD>(hr));
			assert(head_result_ > 0);
			DWORD const head_length_ = (DWORD)head_result_;
		#else
			int const head_result_ = std::swprintf(
				buffer_,
				2046u,
				L"系统调用出错：(HRESULT = 0x%08X) ",
				static_cast<DWORD>(hr));
			assert(head_result_ > 0);
			DWORD const head_length_ = (DWORD)head_result_;
		#endif
			// 获得人类可读的错误描述
			DWORD body_result_ = FormatMessageW(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				static_cast<DWORD>(hr),
				LANG_USER_DEFAULT,
				buffer_ + head_length_,
				2046u - head_length_,
				NULL);
			// 移除 /r/n
			for (int i_ = 0; i_ < 2; i_ += 1)
			{
				if (body_result_ > 0)
				{
					WCHAR& c_ = buffer_[head_length_ + body_result_ - 1];
					if (c_ == L'\r' || c_ == L'\n')
					{
						c_ = L'\0';
						body_result_ -= 1;
					}
				}
			}
			DWORD const body_length_ = (DWORD)body_result_;
			// 转换编码并打印
			DWORD const length_ = head_length_ + body_length_;
		#ifdef _DEBUG
			buffer_[length_] = L'\n';
			OutputDebugStringW(buffer_);
			buffer_[length_] = L'\0';
		#endif
			std::string const message_(to_utf8(std::wstring_view(buffer_, length_)));
			g_pfn(message_);
			// 禁用编译器警告
			std::ignore = scope_lock_;
		}
		hr_ = hr;
		return *this;
	}
	HResultChecker::operator HResult() const noexcept
	{
		return hr_;
	}

	HResultChecker::HResultChecker(WChar const* const source, int const line, HResult const hr) noexcept
		: hr_(hr)
	#ifdef _DEBUG
		, source_(source)
		, line_(line)
	#endif
	{
	#ifndef _DEBUG
		UNREFERENCED_PARAMETER(source);
		UNREFERENCED_PARAMETER(line);
	#endif
	}
	HResultChecker::HResultChecker(WChar const* const source, int const line) noexcept
		: hr_(S_OK)
	#ifdef _DEBUG
		, source_(source)
		, line_(line)
	#endif
	{
	#ifndef _DEBUG
		UNREFERENCED_PARAMETER(source);
		UNREFERENCED_PARAMETER(line);
	#endif
	}
	HResultChecker::HResultChecker(HResult const hr) noexcept
		: hr_(hr)
	#ifdef _DEBUG
		, source_(L"未知")
		, line_(0)
	#endif
	{
	}
	HResultChecker::HResultChecker() noexcept
		: hr_(S_OK)
	#ifdef _DEBUG
		, source_(L"未知")
		, line_(0)
	#endif
	{
	}

	void HResultChecker::SetPrintCallback(PrintCallback pfn) noexcept
	{
		g_pfn = (pfn != nullptr) ? pfn : &DefaultPrintCallback;
	}

	HResultToBool& HResultToBool::operator=(HResult v) noexcept { hr = v; return *this; }
	HResultToBool::operator bool() const noexcept { return SUCCEEDED(hr); }
	HResultToBool::HResultToBool() noexcept : hr(S_OK) {}
	HResultToBool::HResultToBool(HResult const hr_) noexcept : hr(hr_) {}
}
