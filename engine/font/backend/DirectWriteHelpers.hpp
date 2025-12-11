#pragma once
#include "core/FontCommon.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOMINMAX
#include <windows.h>
#undef DrawText        // FUCK Microsoft
#undef GetGlyphIndices // FUCK Microsoft
#include <dwrite_3.h>

namespace core {
    class DirectWriteHelpers {
    public:
        static IDWriteFactory* getFactory();

        static DWRITE_FONT_WEIGHT toFontWeight(FontWeight weight) noexcept;
        static DWRITE_FONT_STYLE toFontStyle(FontStyle style) noexcept;
        static DWRITE_FONT_STRETCH toFontWidth(FontWidth width) noexcept;
    };
}
