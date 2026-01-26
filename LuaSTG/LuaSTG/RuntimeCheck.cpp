#include "RuntimeCheck.hpp"
#include "core/Configuration.hpp"
#include "windows/MessageBox.hpp"
#include "windows/WindowsVersion.hpp"
#include "windows/CleanWindows.hpp"
#include "windows/ModuleLoader.hpp"
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include "windows/Direct3D11.hpp"
#include <shellapi.h>

namespace luastg
{
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

	inline void open_website_windows_7_end_of_support()
	{
		ShellExecuteW(NULL, NULL, L"https://www.microsoft.com/en-us/windows/end-of-support", NULL, NULL, SW_SHOWNORMAL);
	}

	inline void open_website_download_windows_7_sp1()
	{
		ShellExecuteW(NULL, NULL, L"https://www.catalog.update.microsoft.com/Search.aspx?q=KB976932", NULL, NULL, SW_SHOWNORMAL);
	}

	inline void open_website_download_platform_update_for_windows_7()
	{
		ShellExecuteW(NULL, NULL, L"https://www.catalog.update.microsoft.com/Search.aspx?q=KB2670838", NULL, NULL, SW_SHOWNORMAL);
		ShellExecuteW(NULL, NULL, L"https://www.catalog.update.microsoft.com/Search.aspx?q=KB2834140", NULL, NULL, SW_SHOWNORMAL);
	}

	inline void open_website_download_secure_library_loading_update()
	{
		ShellExecuteW(NULL, NULL, L"https://support.microsoft.com/help/2533623", NULL, NULL, SW_SHOWNORMAL);
	}

	static bool CheckSystem()
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
					(void)(&open_website_windows_7_end_of_support);
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
						open_website_download_windows_7_sp1();
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
						open_website_download_platform_update_for_windows_7();
					}
				}

				if (debug_test || !secure_library_loading)
				{
					std::stringstream message;

					if (chinese_simplified)
					{
						message << "下载 " << secure_library_loading_name << " ？\n";
						message << "将打开以下链接：\n";
						message << "1、https://support.microsoft.com/help/2533623";
					}
					else
					{
						message << "Download " << secure_library_loading_name << " ?\n";
						message << "The following link will open:\n";
						message << "1. https://support.microsoft.com/help/2533623";
					}

					if (Platform::MessageBox::Warning(title, message.str()))
					{
						open_website_download_secure_library_loading_update();
					}
				}
			}
		}

		return is_all_satisfied;
	}

	bool checkEngineRuntimeRequirement()
	{
		if (!CheckSystem()) return false;
		bool const chinese_simplified = is_chinese_simplified();
		auto const title = make_message_box_title(chinese_simplified);
		if (!Platform::Direct3D11::HasDevice(D3D11_CREATE_DEVICE_BGRA_SUPPORT, D3D_FEATURE_LEVEL_10_0))
		{
			std::string_view text;
			if (chinese_simplified)
			{
				text = "找不到可用的显卡：\n"
					"1、显卡驱动程序可能未安装或未正确安装\n"
					"2、显卡驱动程序可能未正常工作\n"
					"3、显卡可能过于老旧，无法被识别\n"
					"4、虚拟机或者云电脑可能不支持或未安装显卡\n";
			}
			else
			{
				text = "No available Graphics Card found: \n"
					"1. Graphics Driver may not be installed or installed incorrectly\n"
					"2. Graphics Driver might not working correctly\n"
					"3. Graphics Card may be too old and can not recognized\n"
					"4. Virtual Machine or Cloud/Remote Computer may not support or not install a graphics card\n";
			}
			Platform::MessageBox::Error(title, text);
			if (!core::ConfigurationLoader::getInstance().getGraphicsSystem().isAllowSoftwareDevice()) {
				return false;
			}
		}
		return true;
	}
}
