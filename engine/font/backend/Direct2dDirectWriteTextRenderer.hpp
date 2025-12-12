#pragma once
#include "win32/base.hpp"
#include "backend/DirectWriteHelpers.hpp"
#include <d2d1_3.h>

namespace core {
    class Direct2dDirectWriteTextRenderer final : public IDWriteTextRenderer1 {
    public:
        // IUnknown
        
        HRESULT WINAPI QueryInterface(IID const& riid, void** ppvObject) override;
        ULONG WINAPI AddRef() override;
        ULONG WINAPI Release() override;

        // IDWritePixelSnapping

        HRESULT WINAPI IsPixelSnappingDisabled(
            void* clientDrawingContext,
            BOOL* isDisabled
        ) noexcept;
        
        HRESULT WINAPI GetCurrentTransform(
            void* clientDrawingContext,
            DWRITE_MATRIX* transform
        ) noexcept;

        HRESULT WINAPI GetPixelsPerDip(
            void* clientDrawingContext,
            FLOAT* pixelsPerDip
        ) noexcept;

        // IDWriteTextRenderer

        HRESULT WINAPI DrawGlyphRun(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_MEASURING_MODE measuringMode,
            DWRITE_GLYPH_RUN const* glyphRun,
            DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
            IUnknown* clientDrawingEffect
        ) noexcept;

        HRESULT WINAPI DrawUnderline(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_UNDERLINE const* underline,
            IUnknown* clientDrawingEffect
        ) noexcept;

        HRESULT WINAPI DrawStrikethrough(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_STRIKETHROUGH const* strikethrough,
            IUnknown* clientDrawingEffect
        ) noexcept;

        HRESULT WINAPI DrawInlineObject(
            void* clientDrawingContext,
            FLOAT originX,
            FLOAT originY,
            IDWriteInlineObject* inlineObject,
            BOOL isSideways,
            BOOL isRightToLeft,
            IUnknown* clientDrawingEffect
        ) noexcept;

        // IDWriteTextRenderer1

        HRESULT WINAPI DrawGlyphRun(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
            DWRITE_MEASURING_MODE measuringMode,
            DWRITE_GLYPH_RUN const* glyphRun,
            DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
            IUnknown* clientDrawingEffect
        ) noexcept;

        HRESULT WINAPI DrawUnderline(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
            DWRITE_UNDERLINE const* underline,
            IUnknown* clientDrawingEffect
        ) noexcept;

        HRESULT WINAPI DrawStrikethrough(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
            DWRITE_STRIKETHROUGH const* strikethrough,
            IUnknown* clientDrawingEffect
        ) noexcept;

        HRESULT WINAPI DrawInlineObject(
            void* clientDrawingContext,
            FLOAT originX,
            FLOAT originY,
            DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
            IDWriteInlineObject* inlineObject,
            BOOL isSideways,
            BOOL isRightToLeft,
            IUnknown* clientDrawingEffect
        ) noexcept;

        // DirectWriteTextRenderer

        Direct2dDirectWriteTextRenderer(
            ID2D1Factory* const factory,
            ID2D1RenderTarget* const target,
            IDWriteTextLayout* const layout,
            ID2D1Brush* const outline,
            ID2D1Brush* const fill,
            ID2D1StrokeStyle* const stroke_style,
            const FLOAT width
        );

        void setLayerEnable(const BOOL text, const BOOL stroke) noexcept;

    private:
        win32::com_ptr<ID2D1Factory> d2d1_factory;
        win32::com_ptr<ID2D1RenderTarget> d2d1_rt;
        win32::com_ptr<IDWriteTextLayout> dwrite_text_layout;
        win32::com_ptr<ID2D1Brush> d2d1_brush_outline;
        win32::com_ptr<ID2D1Brush> d2d1_brush_fill;
        win32::com_ptr<ID2D1StrokeStyle> d2d1_stroke_style;
        FLOAT outline_width{};
        BOOL layer_text{ TRUE };
        BOOL layer_stroke{ TRUE };
    };
}
