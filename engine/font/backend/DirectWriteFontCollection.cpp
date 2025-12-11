#include "backend/DirectWriteFontCollection.hpp"
#include "core/FileSystem.hpp"
#include <cassert>
#include <filesystem>
#include <ranges>
#include "utf8.hpp"
#include "backend/DirectWriteHelpers.hpp"

namespace {
    using std::string_view_literals::operator ""sv;

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

    constexpr const wchar_t* locales[]{
        L"en-us",
        L"zh-cn",
    };

    /* throw exception */ std::wstring getLocaleName(IDWriteLocalizedStrings* const str, const UINT32 index) {
        std::wstring locale_name;
        UINT32 locale_name_length{};
        win32::check_hresult_throw_if_failed(
            str->GetLocaleNameLength(index, &locale_name_length),
            "IDWriteLocalizedStrings::GetLocaleNameLength"sv
        );
        locale_name.resize(locale_name_length);
        win32::check_hresult_throw_if_failed(
            str->GetLocaleName(index, locale_name.data(), locale_name_length),
            "IDWriteLocalizedStrings::GetLocaleName"sv
        );
        return locale_name;
    }
    /* throw exception */ std::wstring getString(IDWriteLocalizedStrings* const str, const UINT32 index) {
        std::wstring s;
        UINT32 length{};
        win32::check_hresult_throw_if_failed(
            str->GetStringLength(index, &length),
            "IDWriteLocalizedStrings::GetStringLength"sv
        );
        s.resize(length);
        win32::check_hresult_throw_if_failed(
            str->GetString(index, s.data(), length),
            "IDWriteLocalizedStrings::GetString"sv
        );
        return s;
    }
    std::wstring getString(IDWriteLocalizedStrings* const str) {
        assert(str != nullptr);
        try {
            UINT32 index{};
            BOOL exists{};
            for (const auto locale : locales) {
                win32::check_hresult_throw_if_failed(
                    str->FindLocaleName(locale, &index, &exists),
                    "IDWriteLocalizedStrings::FindLocaleName"sv
                );
                if (exists) {
                    break;
                }
            }
            if (!exists) {
                for (UINT32 i = 0; i < str->GetCount(); i += 1) {
                    const auto locale_name{ getLocaleName(str, i) };
                    if (locale_name.starts_with(L"en-")) {
                        index = i;
                        exists = TRUE;
                        break;
                    }
                }
            }
            if (!exists) {
                for (UINT32 i = 0; i < str->GetCount(); i += 1) {
                    const auto locale_name{ getLocaleName(str, i) };
                    if (locale_name.starts_with(L"zh-")) {
                        index = i;
                        exists = TRUE;
                        break;
                    }
                }
            }
            if (!exists) {
                return {};
            }
            return getString(str, index);
        }
        catch (const std::exception&) {
            return {};
        }
    }

    win32::com_ptr<IDWriteFont> getFont(IDWriteFontCollection* const font_collection, const uint32_t font_family_index, const uint32_t font_index) {
        win32::com_ptr<IDWriteFontFamily> font_family;
        win32::com_ptr<IDWriteFont> font;
        if (!win32::check_hresult_as_boolean(
            font_collection->GetFontFamily(font_family_index, font_family.put()),
            "IDWriteFontCollection::GetFontFamily"sv
        )) {
            return font;
        }
        if (!win32::check_hresult_as_boolean(
            font_family->GetFont(font_index, font.put()),
            "IDWriteFontList::GetFont"sv
        )) {
            return font;
        }
        return font;
    }
}

namespace core {
    // IFontCollection

    bool DirectWriteFontCollection::addFile(const StringView path) {
        const auto normalized = normalizePath(path);
        const auto view = getStringView(normalized);
        auto& files = m_files->getData();
        if (std::ranges::any_of(files, [view](const std::string& path) -> bool { return path == view; })) {
            return true;
        }
        if (!FileSystemManager::hasFile(view)) {
            return false;
        }
        files.emplace_back(view);
        return true;
    }

    bool DirectWriteFontCollection::findFontFamily(const StringView font_family_name, uint32_t& font_family_index) {
        if (m_dirty) {
            if (!build()) {
                return false;
            }
        }
        const auto font_family_name_w{ utf8::to_wstring(font_family_name) };
        UINT32 index{};
        BOOL exists{};
        if (!win32::check_hresult_as_boolean(
            m_font_collection->FindFamilyName(font_family_name_w.c_str(), &index, &exists),
            "IDWriteFontCollection::FindFamilyName"sv
        )) {
            return false;
        }
        if (!exists) {
            return false;
        }
        font_family_index = index;
        return true;
    }
    uint32_t DirectWriteFontCollection::getFontFamilyCount() {
        if (m_dirty) {
            if (!build()) {
                return 0;
            }
        }
        return m_font_collection->GetFontFamilyCount();
    }
    bool DirectWriteFontCollection::getFontFamilyName(const uint32_t font_family_index, IImmutableString** const output) {
        if (m_dirty) {
            if (!build()) {
                return false;
            }
        }
        win32::com_ptr<IDWriteFontFamily> font_family;
        if (!win32::check_hresult_as_boolean(
            m_font_collection->GetFontFamily(font_family_index, font_family.put()),
            "IDWriteFontCollection::GetFontFamily"sv
        )) {
            return false;
        }
        win32::com_ptr<IDWriteLocalizedStrings> str;
        if (!win32::check_hresult_as_boolean(
            font_family->GetFamilyNames(str.put()),
            "IDWriteFontFamily::GetFamilyNames"sv
        )) {
            return false;
        }
        const auto name{ getString(str.get()) };
        const auto name_u8{ utf8::to_string(name) };
        IImmutableString::create(name_u8, output);
        return true;
    }

    uint32_t DirectWriteFontCollection::getFontCount(const uint32_t font_family_index) {
        if (m_dirty) {
            if (!build()) {
                return 0;
            }
        }
    }
    bool DirectWriteFontCollection::getFontName(const uint32_t font_family_index, const uint32_t font_index, IImmutableString** const output) {
        if (m_dirty) {
            if (!build()) {
                return false;
            }
        }
    }
    FontWeight DirectWriteFontCollection::getFontWeight(const uint32_t font_family_index, const uint32_t font_index) {
        if (m_dirty) {
            if (!build()) {
                return FontWeight::normal;
            }
        }
        win32::com_ptr<IDWriteFontFamily> font_family;
        if (!win32::check_hresult_as_boolean(
            m_font_collection->GetFontFamily(font_family_index, font_family.put()),
            "IDWriteFontCollection::GetFontFamily"sv
        )) {
            return FontWeight::normal;
        }
        win32::com_ptr<IDWriteFont> font;
        return static_cast<FontWeight>(font->GetWeight());
    }
    FontStyle DirectWriteFontCollection::getFontStyle(const uint32_t font_family_index, const uint32_t font_index) {
        if (m_dirty) {
            if (!build()) {
                return FontStyle::normal;
            }
        }
    }
    FontWidth DirectWriteFontCollection::getFontWidth(const uint32_t font_family_index, const uint32_t font_index) {
        if (m_dirty) {
            if (!build()) {
                return FontWidth::normal;
            }
        }
    }

    // DirectWriteFontCollection

    DirectWriteFontCollection::DirectWriteFontCollection() : m_files(new implement::StringList()) {
    }
    DirectWriteFontCollection::~DirectWriteFontCollection() = default;

    bool DirectWriteFontCollection::build() {
        m_dirty = false;
        return true;
    }
}
