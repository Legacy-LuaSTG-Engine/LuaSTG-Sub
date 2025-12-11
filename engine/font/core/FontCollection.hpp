#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/ImmutableString.hpp"
#include "core/FontCommon.hpp"

namespace core {
    CORE_INTERFACE IFontCollection : public IReferenceCounted {
        virtual bool addFile(StringView path) = 0;

        virtual bool findFontFamily(StringView font_family_name, uint32_t& font_family_index) = 0;
        virtual uint32_t getFontFamilyCount() = 0;
        virtual StringView getFontFamilyName(uint32_t font_family_index) = 0;

        virtual bool findFont(
            StringView font_family_name,
            FontWeight weight, FontStyle style, FontWidth width,
            uint32_t& font_family_index, uint32_t& font_index
        ) = 0;
        virtual uint32_t getFontCount(uint32_t font_family_index) = 0;
        virtual StringView getFontName(uint32_t font_family_index, uint32_t font_index) = 0;
        virtual FontWeight getFontWeight(uint32_t font_family_index, uint32_t font_index) = 0;
        virtual FontStyle getFontStyle(uint32_t font_family_index, uint32_t font_index) = 0;
        virtual FontWidth getFontWidth(uint32_t font_family_index, uint32_t font_index) = 0;

        static bool create(IFontCollection** output);
        static bool createFromSystem(IFontCollection** output);
    };
    CORE_INTERFACE_ID(IFontCollection, "7128a2f7-a292-5393-ac3c-abf2d1b10cde");
}
