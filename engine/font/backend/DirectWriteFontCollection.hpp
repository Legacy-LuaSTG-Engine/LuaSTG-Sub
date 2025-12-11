#pragma once
#include "core/FontCollection.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "core/implement/StringList.hpp"
#include "win32/base.hpp"

struct IDWriteFontCollection;

namespace core {
    class DirectWriteFontCollection final : public implement::ReferenceCounted<IFontCollection> {
    public:
        // IFontCollection

        bool addFile(StringView path) override;
        bool build() override;

        bool findFontFamily(StringView font_family_name, uint32_t& font_family_index) override;
        uint32_t getFontFamilyCount() override;
        bool getFontFamilyName(uint32_t font_family_index, IImmutableString** output) override;

        bool findFont(
            StringView font_family_name,
            FontWeight weight, FontStyle style, FontWidth width,
            uint32_t& font_family_index, uint32_t& font_index
        ) override;
        uint32_t getFontCount(uint32_t font_family_index) override;
        bool getFontName(uint32_t font_family_index, uint32_t font_index, IImmutableString** output) override;
        FontWeight getFontWeight(uint32_t font_family_index, uint32_t font_index) override;
        FontStyle getFontStyle(uint32_t font_family_index, uint32_t font_index) override;
        FontWidth getFontWidth(uint32_t font_family_index, uint32_t font_index) override;

        // DirectWriteFontCollection

        DirectWriteFontCollection();
        DirectWriteFontCollection(const DirectWriteFontCollection&) = delete;
        DirectWriteFontCollection(DirectWriteFontCollection&&) = delete;
        ~DirectWriteFontCollection();

        DirectWriteFontCollection& operator=(const DirectWriteFontCollection&) = delete;
        DirectWriteFontCollection& operator=(DirectWriteFontCollection&&) = delete;

        bool initializeFromSystem();

    private:
        core::SmartReference<core::implement::StringList> m_files;
        win32::com_ptr<IDWriteFontCollection> m_font_collection;
        bool m_read_only{false};
        bool m_dirty{true};
    };
}
