#pragma once
#include <string>
#include <string_view>
#include <format>

enum class i18n_lang_t
{
	zh_cn,
	en_us,
};

void i18n_set_lang(i18n_lang_t lang);
std::string_view i18n(std::string_view const& key);

#define i18n_log_info(STR)           spdlog::info(i18n(STR))
#define i18n_log_info_fmt(FMT, ...)  spdlog::info(fmt::runtime(i18n(FMT)), __VA_ARGS__)

#define i18n_log_warn(STR)           spdlog::warn(i18n(STR))
#define i18n_log_warn_fmt(FMT, ...)  spdlog::warn(fmt::runtime(i18n(FMT)), __VA_ARGS__)

#define i18n_log_error(STR)          spdlog::error(i18n(STR))
#define i18n_log_error_fmt(FMT, ...) spdlog::error(fmt::runtime(i18n(FMT)), __VA_ARGS__)

#define i18n_log_critical(STR)          spdlog::critical(i18n(STR))
#define i18n_log_critical_fmt(FMT, ...) spdlog::critical(fmt::runtime(i18n(FMT)), __VA_ARGS__)

inline void i18n_core_system_call_report_error(std::string_view message)
{
	spdlog::error(fmt::runtime(i18n("[core].system_call_failed_f")), message);
}
