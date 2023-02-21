#include "RuntimeCheck.hpp"
#include "Platform/MessageBox.hpp"
#include "Platform/WindowsVersion.hpp"
#include "Platform/CleanWindows.hpp"
#include "Platform/ModuleLoader.hpp"
#include <shellapi.h>

namespace LuaSTG
{
	static void OpenWebsiteWindows7EndOfSupport()
	{
		ShellExecuteW(NULL, NULL, L"https://www.microsoft.com/en-us/windows/end-of-support", NULL, NULL, SW_SHOWNORMAL);
	}

	static void OpenWebsiteDownloadWindows7SP1()
	{
		ShellExecuteW(NULL, NULL, L"https://www.catalog.update.microsoft.com/Search.aspx?q=KB976932", NULL, NULL, SW_SHOWNORMAL);
	}

	static void OpenWebsiteDownloadPlatformUpdateForWindows7()
	{
		ShellExecuteW(NULL, NULL, L"https://www.catalog.update.microsoft.com/Search.aspx?q=KB2670838", NULL, NULL, SW_SHOWNORMAL);
		ShellExecuteW(NULL, NULL, L"https://www.catalog.update.microsoft.com/Search.aspx?q=KB2834140", NULL, NULL, SW_SHOWNORMAL);
	}

	static void OpenWebsiteDownloadSecureLibraryLoadingUpdate(bool zh_cn)
	{
		auto link_en_us = L"https://support.microsoft.com/en-us/topic/microsoft-security-advisory-insecure-library-loading-could-allow-remote-code-execution-486ea436-2d47-27e5-6cb9-26ab7230c704";
		auto link_zh_cn = L"https://support.microsoft.com/zh-cn/topic/microsoft-%E5%AE%89%E5%85%A8%E5%85%AC%E5%91%8A-%E4%B8%8D%E5%AE%89%E5%85%A8%E7%9A%84%E5%BA%93%E5%8A%A0%E8%BD%BD%E5%8F%AF%E8%83%BD%E5%85%81%E8%AE%B8%E8%BF%9C%E7%A8%8B%E6%89%A7%E8%A1%8C%E4%BB%A3%E7%A0%81-486ea436-2d47-27e5-6cb9-26ab7230c704";
		ShellExecuteW(NULL, NULL, zh_cn ? link_zh_cn : link_en_us, NULL, NULL, SW_SHOWNORMAL);
	}

	constexpr std::string_view const indent_string("    ");
	constexpr std::string_view const windows_7_name("Windows 7");
	constexpr std::string_view const service_pack_1_name("Windows 7 Service Pack 1 (KB976932)");
	constexpr std::string_view const platform_update_name("Platform Update for Windows 7 (KB2670838, KB2834140)");
	constexpr std::string_view const secure_library_loading_name("Secure Library Loading Update for Windows 7 (KB2533623)");

	inline bool is_chinese_simplified()
	{
		LANGID const language_chinese_simplified = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
	#pragma warning(push)
	#pragma warning(disable: 6387)
		LANGID const language_user_default = LANGIDFROMLCID(LocaleNameToLCID(LOCALE_NAME_USER_DEFAULT, 0));
	#pragma warning(pop)
		bool const chinese_simplified = (language_user_default == language_chinese_simplified);
		return chinese_simplified;
	}

	inline std::string_view make_message_box_title(bool zh_cn)
	{
		if (zh_cn)
		{
			return LUASTG_INFO " 诊断工具";
		}
		else
		{
			return LUASTG_INFO " Diagnostic Tools";
		}
	}

	inline std::string_view make_download_guide_text(bool zh_cn)
	{
		if (zh_cn)
		{
			return
				"系统有多个更新未安装。是否从微软官网下载？\n"
				"\n"
				"提示：\n"
				"    1、如果无法打开链接，请尝试安装更新版本的谷歌/Edge/火狐浏览器。\n"
				"    2、下载时请选择对应的系统变体，有普通版、嵌入式版（Embedded）、服务器（Server）等。\n"
				"    3、下载时请选择对应的系统位数，有32位（x86）、64位（x64）等。"
				;
		}
		else
		{
			return
				"Several updates were not installed on the system. Download from Microsoft official website?\n"
				"\n"
				"Notice:\n"
				"    1. If the link can't open, try installing newer version Google/Edge/Firefox browsers.\n"
				"    2. Please select the corresponding system variant when downloading, there are normal, Embedded, Server, etc.\n"
				"    3. Please select the corresponding system architecture when downloading, there are 32bit (x86), 64bit (x64), etc."
				;
		}
	}

	bool CheckUserRuntime()
	{
		bool const windows_7 = Platform::WindowsVersion::Is7();
		bool const service_pack_1 = Platform::WindowsVersion::Is7SP1();
		bool const platform_update = Platform::WindowsVersion::Is7SP1WithPlatformUpdate();
		bool const secure_library_loading = Platform::ModuleLoader::IsSearchOnlyInSystemSupported();

		bool const is_all_satisfied = (windows_7 && service_pack_1 && platform_update && secure_library_loading);
		bool const is_any_satisfied = (windows_7 || service_pack_1 || platform_update || secure_library_loading);

		bool const chinese_simplified = is_chinese_simplified();
		
		std::stringstream message_buffer;

		{
			int counter = 0;
			auto reset_counter = [&counter]() { counter = 0; };
			auto next_counter_string = [&counter, &chinese_simplified]() -> std::string
			{
				counter += 1;
				std::string buffer = std::to_string(counter);
				if (chinese_simplified)
				{
					buffer.append("、");
				}
				else
				{
					buffer.append(". ");
				}
				return buffer;
			};

			if (chinese_simplified)
			{
				message_buffer << "未达到最低系统要求。\n";
				message_buffer << "\n";
				message_buffer << "已满足的条件：\n";
			}
			else
			{
				message_buffer << "Minimum system requirements are not met.\n";
				message_buffer << "\n";
				message_buffer << "Satisfied:\n";
			}

			if (is_any_satisfied)
			{
				reset_counter();
				if (windows_7)              message_buffer << indent_string << next_counter_string() << windows_7_name              << "\n";
				if (service_pack_1)         message_buffer << indent_string << next_counter_string() << service_pack_1_name         << "\n";
				if (platform_update)        message_buffer << indent_string << next_counter_string() << platform_update_name        << "\n";
				if (secure_library_loading) message_buffer << indent_string << next_counter_string() << secure_library_loading_name << "\n";
			}
			else
			{
				message_buffer << "    N/A\n"; // 全都不满足，所以满足列表为空
			}

			message_buffer << "\n";

			if (chinese_simplified)
			{
				message_buffer << "未满足的条件：\n";
			}
			else
			{
				message_buffer << "Not Satisfied:\n";
			}

			if (is_all_satisfied)
			{
				message_buffer << "    N/A\n"; // 全都满足，所以不满足列表为空
			}
			else
			{
				reset_counter();
				if (!windows_7)              message_buffer << indent_string << next_counter_string() << windows_7_name              << "\n";
				if (!service_pack_1)         message_buffer << indent_string << next_counter_string() << service_pack_1_name         << "\n";
				if (!platform_update)        message_buffer << indent_string << next_counter_string() << platform_update_name        << "\n";
				if (!secure_library_loading) message_buffer << indent_string << next_counter_string() << secure_library_loading_name << "\n";
			}

			message_buffer << "\n";

			if (chinese_simplified)
			{
				message_buffer << "请更新 Windows 系统。";
			}
			else
			{
				message_buffer << "Please update Windows system.";
			}
		}

		bool const debug_test = false;

		if (debug_test || !is_all_satisfied)
		{
			auto const title = make_message_box_title(chinese_simplified);

			Platform::MessageBox::Error(title, message_buffer.str());

			if (Platform::MessageBox::Warning(title, make_download_guide_text(chinese_simplified)))
			{
				if (debug_test || !windows_7)
				{
					(void)(&OpenWebsiteWindows7EndOfSupport);
				}

				if (debug_test || !service_pack_1)
				{
					std::stringstream message;

					if (chinese_simplified)
					{
						message << "下载 " << service_pack_1_name << " ？\n";
						message << "将打开以下链接：\n";
						message << "1、https://www.catalog.update.microsoft.com/Search.aspx?q=KB976932";
					}
					else
					{
						message << "Download " << service_pack_1_name << " ?\n";
						message << "The following link will open:\n";
						message << "1. https://www.catalog.update.microsoft.com/Search.aspx?q=KB976932";
					}

					if (Platform::MessageBox::Warning(title, message.str()))
					{
						OpenWebsiteDownloadWindows7SP1();
					}
				}

				if (debug_test || !platform_update)
				{
					std::stringstream message;

					if (chinese_simplified)
					{
						message << "下载 " << platform_update_name << " ？\n";
						message << "将打开以下链接：\n";
						message << "1、https://www.catalog.update.microsoft.com/Search.aspx?q=KB2670838";
						message << "\n";
						message << "2、https://www.catalog.update.microsoft.com/Search.aspx?q=KB2834140";
					}
					else
					{
						message << "Download " << platform_update_name << " ?\n";
						message << "The following link will open:\n";
						message << "1. https://www.catalog.update.microsoft.com/Search.aspx?q=KB2670838";
						message << "\n";
						message << "2. https://www.catalog.update.microsoft.com/Search.aspx?q=KB2834140";
					}

					if (Platform::MessageBox::Warning(title, message.str()))
					{
						OpenWebsiteDownloadPlatformUpdateForWindows7();
					}
				}

				if (debug_test || !secure_library_loading)
				{
					std::stringstream message;

					if (chinese_simplified)
					{
						message << "下载 " << secure_library_loading_name << " ？\n";
						message << "将打开以下链接：\n";
						message << "1、https://support.microsoft.com/zh-cn/topic/microsoft-%E5%AE%89%E5%85%A8%E5%85%AC%E5%91%8A-%E4%B8%8D%E5%AE%89%E5%85%A8%E7%9A%84%E5%BA%93%E5%8A%A0%E8%BD%BD%E5%8F%AF%E8%83%BD%E5%85%81%E8%AE%B8%E8%BF%9C%E7%A8%8B%E6%89%A7%E8%A1%8C%E4%BB%A3%E7%A0%81-486ea436-2d47-27e5-6cb9-26ab7230c704";
					}
					else
					{
						message << "Download " << secure_library_loading_name << " ?\n";
						message << "The following link will open:\n";
						message << "1. https://support.microsoft.com/en-us/topic/microsoft-security-advisory-insecure-library-loading-could-allow-remote-code-execution-486ea436-2d47-27e5-6cb9-26ab7230c704";
					}

					if (Platform::MessageBox::Warning(title, message.str()))
					{
						OpenWebsiteDownloadSecureLibraryLoadingUpdate(chinese_simplified);
					}
				}
			}
		}

		return is_all_satisfied;
	}
}
