#include "backend/Direct2dDirectWriteTextRenderer.hpp"
#include <cassert>

namespace core {
    // IUnknown
        
    HRESULT WINAPI Direct2dDirectWriteTextRenderer::QueryInterface(IID const& riid, void** ppvObject) {
        if (riid == __uuidof(IUnknown)) {
            AddRef();
            *ppvObject = static_cast<IUnknown*>(this);
            return S_OK;
        }
        if (riid == __uuidof(IDWritePixelSnapping)) {
            AddRef();
            *ppvObject = static_cast<IDWritePixelSnapping*>(this);
            return S_OK;
        }
        if (riid == __uuidof(IDWriteTextRenderer)) {
            AddRef();
            *ppvObject = static_cast<IDWriteTextRenderer*>(this);
            return S_OK;
        }
        if (riid == __uuidof(IDWriteTextRenderer1)) {
            AddRef();
            *ppvObject = static_cast<IDWriteTextRenderer1*>(this);
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    ULONG WINAPI Direct2dDirectWriteTextRenderer::AddRef() { return 2; }
    ULONG WINAPI Direct2dDirectWriteTextRenderer::Release() { return 1; }

    // IDWritePixelSnapping

    HRESULT WINAPI Direct2dDirectWriteTextRenderer::IsPixelSnappingDisabled(
        void* clientDrawingContext,
        BOOL* isDisabled
    ) noexcept {
        UNREFERENCED_PARAMETER(clientDrawingContext);
        *isDisabled = FALSE; // recommended default value
        return S_OK;
    }
    
    HRESULT WINAPI Direct2dDirectWriteTextRenderer::GetCurrentTransform(
        void* clientDrawingContext,
        DWRITE_MATRIX* transform
    ) noexcept {
        UNREFERENCED_PARAMETER(clientDrawingContext);
        // forward the render target's transform
        d2d1_rt->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
        return S_OK;
    }

    HRESULT WINAPI Direct2dDirectWriteTextRenderer::GetPixelsPerDip(
        void* clientDrawingContext,
        FLOAT* pixelsPerDip
    ) noexcept {
        UNREFERENCED_PARAMETER(clientDrawingContext);
        float x = 0.0f, y = 0.0f;
        d2d1_rt->GetDpi(&x, &y);
        *pixelsPerDip = x / 96.0f;
        return S_OK;
    }

    // IDWriteTextRenderer

    HRESULT WINAPI Direct2dDirectWriteTextRenderer::DrawGlyphRun(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        IUnknown* clientDrawingEffect
    ) noexcept {
        UNREFERENCED_PARAMETER(clientDrawingContext);
        UNREFERENCED_PARAMETER(measuringMode);
        UNREFERENCED_PARAMETER(glyphRunDescription);
        UNREFERENCED_PARAMETER(clientDrawingEffect);

        HRESULT hr = S_OK;

        // Create the path geometry.

        win32::com_ptr<ID2D1PathGeometry> d2d1_path_geometry;
        hr = d2d1_factory->CreatePathGeometry(d2d1_path_geometry.put());
        if (FAILED(hr)) return hr;

        // Write to the path geometry using the geometry sink.

        win32::com_ptr<ID2D1GeometrySink> d2d1_geometry_sink;
        hr = d2d1_path_geometry->Open(d2d1_geometry_sink.put());
        if (FAILED(hr)) return hr;

        hr = glyphRun->fontFace->GetGlyphRunOutline(
            glyphRun->fontEmSize,
            glyphRun->glyphIndices,
            glyphRun->glyphAdvances,
            glyphRun->glyphOffsets,
            glyphRun->glyphCount,
            glyphRun->isSideways,
            glyphRun->bidiLevel % 2,
            d2d1_geometry_sink.get()
        );
        if (FAILED(hr)) return hr;

        hr = d2d1_geometry_sink->Close();
        if (FAILED(hr)) return hr;

        const D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Translation(baselineOriginX, baselineOriginY);

        win32::com_ptr<ID2D1TransformedGeometry> d2d1_transformed_geometry;
        hr = d2d1_factory->CreateTransformedGeometry(
            d2d1_path_geometry.get(),
            &matrix,
            d2d1_transformed_geometry.put()
        );
        if (FAILED(hr)) return hr;

        // Draw the outline of the glyph run

        if (layer_stroke) d2d1_rt->DrawGeometry(d2d1_transformed_geometry.get(), d2d1_brush_outline.get(), outline_width, d2d1_stroke_style.get());

        // Fill in the glyph run

        if (layer_text) d2d1_rt->FillGeometry(d2d1_transformed_geometry.get(), d2d1_brush_fill.get());

        return S_OK;
    }

    HRESULT WINAPI Direct2dDirectWriteTextRenderer::DrawUnderline(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_UNDERLINE const* underline,
        IUnknown* clientDrawingEffect
    ) noexcept {
        UNREFERENCED_PARAMETER(clientDrawingContext);
        UNREFERENCED_PARAMETER(clientDrawingEffect);

        HRESULT hr = S_OK;

        D2D1_RECT_F rect = D2D1::RectF(
            0,
            underline->offset,
            underline->width,
            underline->offset + underline->thickness
        );

        win32::com_ptr<ID2D1RectangleGeometry> d2d1_rect_geometry;
        hr = d2d1_factory->CreateRectangleGeometry(&rect, d2d1_rect_geometry.put());
        if (FAILED(hr)) return hr;

        const D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Translation(baselineOriginX, baselineOriginY);

        win32::com_ptr<ID2D1TransformedGeometry> d2d1_transformed_geometry;
        hr = d2d1_factory->CreateTransformedGeometry(
            d2d1_rect_geometry.get(),
            &matrix,
            d2d1_transformed_geometry.put()
        );
        if (FAILED(hr)) return hr;

        if (layer_stroke) d2d1_rt->DrawGeometry(d2d1_transformed_geometry.get(), d2d1_brush_outline.get(), outline_width);

        if (layer_text) d2d1_rt->FillGeometry(d2d1_transformed_geometry.get(), d2d1_brush_fill.get());

        return S_OK;
    }

    HRESULT WINAPI Direct2dDirectWriteTextRenderer::DrawStrikethrough(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown* clientDrawingEffect
    ) noexcept {
        UNREFERENCED_PARAMETER(clientDrawingContext);
        UNREFERENCED_PARAMETER(clientDrawingEffect);

        HRESULT hr = S_OK;

        D2D1_RECT_F rect = D2D1::RectF(
            0,
            strikethrough->offset,
            strikethrough->width,
            strikethrough->offset + strikethrough->thickness
        );

        win32::com_ptr<ID2D1RectangleGeometry> d2d1_rect_geometry;
        hr = d2d1_factory->CreateRectangleGeometry(&rect, d2d1_rect_geometry.put());
        if (FAILED(hr)) return hr;

        const D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Translation(baselineOriginX, baselineOriginY);

        win32::com_ptr<ID2D1TransformedGeometry> d2d1_transformed_geometry;
        hr = d2d1_factory->CreateTransformedGeometry(
            d2d1_rect_geometry.get(),
            &matrix,
            d2d1_transformed_geometry.put()
        );
        if (FAILED(hr)) return hr;

        if (layer_stroke) d2d1_rt->DrawGeometry(d2d1_transformed_geometry.get(), d2d1_brush_outline.get(), outline_width);

        if (layer_text) d2d1_rt->FillGeometry(d2d1_transformed_geometry.get(), d2d1_brush_fill.get());

        return S_OK;
    }

    HRESULT WINAPI Direct2dDirectWriteTextRenderer::DrawInlineObject(
        void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect
    ) noexcept {
        UNREFERENCED_PARAMETER(clientDrawingContext);
        UNREFERENCED_PARAMETER(originX);
        UNREFERENCED_PARAMETER(originY);
        UNREFERENCED_PARAMETER(inlineObject);
        UNREFERENCED_PARAMETER(isSideways);
        UNREFERENCED_PARAMETER(isRightToLeft);
        UNREFERENCED_PARAMETER(clientDrawingEffect);
        return S_OK;
    }

    // IDWriteTextRenderer1

    HRESULT WINAPI Direct2dDirectWriteTextRenderer::DrawGlyphRun(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        IUnknown* clientDrawingEffect
    ) noexcept {
        UNREFERENCED_PARAMETER(clientDrawingContext);
        UNREFERENCED_PARAMETER(measuringMode);
        UNREFERENCED_PARAMETER(glyphRunDescription);
        UNREFERENCED_PARAMETER(clientDrawingEffect);

        HRESULT hr = S_OK;

        // Create the path geometry.

        win32::com_ptr<ID2D1PathGeometry> d2d1_path_geometry;
        hr = d2d1_factory->CreatePathGeometry(d2d1_path_geometry.put());
        if (FAILED(hr)) return hr;

        // Write to the path geometry using the geometry sink.

        win32::com_ptr<ID2D1GeometrySink> d2d1_geometry_sink;
        hr = d2d1_path_geometry->Open(d2d1_geometry_sink.put());
        if (FAILED(hr)) return hr;

        hr = glyphRun->fontFace->GetGlyphRunOutline(
            glyphRun->fontEmSize,
            glyphRun->glyphIndices,
            glyphRun->glyphAdvances,
            glyphRun->glyphOffsets,
            glyphRun->glyphCount,
            glyphRun->isSideways,
            glyphRun->bidiLevel % 2,
            d2d1_geometry_sink.get()
        );
        if (FAILED(hr)) return hr;

        hr = d2d1_geometry_sink->Close();
        if (FAILED(hr)) return hr;

        // TODO: 为什么旋转方向是这样判断的？
        FLOAT rotate_angle = 0.0f;
        //switch (orientationAngle)
        //{
        //case DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES: rotate_angle = 0.0f; break;
        //case DWRITE_GLYPH_ORIENTATION_ANGLE_90_DEGREES: rotate_angle = 90.0f; break;
        //case DWRITE_GLYPH_ORIENTATION_ANGLE_180_DEGREES: rotate_angle = 180.0f; break;
        //case DWRITE_GLYPH_ORIENTATION_ANGLE_270_DEGREES: rotate_angle = 270.0f; break;
        //default: assert(false); break;
        //}
        UNREFERENCED_PARAMETER(orientationAngle);
        switch (dwrite_text_layout->GetReadingDirection()) {
            case DWRITE_READING_DIRECTION_LEFT_TO_RIGHT: rotate_angle = 0.0f; break;
            //case DWRITE_READING_DIRECTION_RIGHT_TO_LEFT: rotate_angle = 0.0f; break;
            case DWRITE_READING_DIRECTION_TOP_TO_BOTTOM: rotate_angle = 90.0f; break;
            //case DWRITE_READING_DIRECTION_BOTTOM_TO_TOP: rotate_angle = 90.0f; break;
            default: assert(false); break;
        }

        D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Rotation(rotate_angle);
        matrix.dx = baselineOriginX;
        matrix.dy = baselineOriginY;

        win32::com_ptr<ID2D1TransformedGeometry> d2d1_transformed_geometry;
        hr = d2d1_factory->CreateTransformedGeometry(
            d2d1_path_geometry.get(),
            &matrix,
            d2d1_transformed_geometry.put()
        );
        if (FAILED(hr)) return hr;

        // Draw the outline of the glyph run

        if (layer_stroke) d2d1_rt->DrawGeometry(d2d1_transformed_geometry.get(), d2d1_brush_outline.get(), outline_width, d2d1_stroke_style.get());

        // Fill in the glyph run

        if (layer_text) d2d1_rt->FillGeometry(d2d1_transformed_geometry.get(), d2d1_brush_fill.get());

        return S_OK;
    }

    HRESULT WINAPI Direct2dDirectWriteTextRenderer::DrawUnderline(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
        DWRITE_UNDERLINE const* underline,
        IUnknown* clientDrawingEffect
    ) noexcept {
        UNREFERENCED_PARAMETER(clientDrawingContext);
        UNREFERENCED_PARAMETER(clientDrawingEffect);

        HRESULT hr = S_OK;

        D2D1_RECT_F rect = D2D1::RectF(
            0,
            underline->offset,
            underline->width,
            underline->offset + underline->thickness
        );

        win32::com_ptr<ID2D1RectangleGeometry> d2d1_rect_geometry;
        hr = d2d1_factory->CreateRectangleGeometry(&rect, d2d1_rect_geometry.put());
        if (FAILED(hr)) return hr;

        // TODO: 为什么旋转方向是这样判断的？
        FLOAT rotate_angle = 0.0f;
        //switch (orientationAngle)
        //{
        //case DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES: rotate_angle = 0.0f; break;
        //case DWRITE_GLYPH_ORIENTATION_ANGLE_90_DEGREES: rotate_angle = 90.0f; break;
        //case DWRITE_GLYPH_ORIENTATION_ANGLE_180_DEGREES: rotate_angle = 180.0f; break;
        //case DWRITE_GLYPH_ORIENTATION_ANGLE_270_DEGREES: rotate_angle = 270.0f; break;
        //default: assert(false); break;
        //}
        UNREFERENCED_PARAMETER(orientationAngle);
        switch (dwrite_text_layout->GetReadingDirection()) {
            case DWRITE_READING_DIRECTION_LEFT_TO_RIGHT: rotate_angle = 0.0f; break;
            //case DWRITE_READING_DIRECTION_RIGHT_TO_LEFT: rotate_angle = 0.0f; break;
            case DWRITE_READING_DIRECTION_TOP_TO_BOTTOM: rotate_angle = 90.0f; break;
            //case DWRITE_READING_DIRECTION_BOTTOM_TO_TOP: rotate_angle = 90.0f; break;
            default: assert(false); break;
        }

        D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Rotation(rotate_angle);
        matrix.dx = baselineOriginX;
        matrix.dy = baselineOriginY;

        win32::com_ptr<ID2D1TransformedGeometry> d2d1_transformed_geometry;
        hr = d2d1_factory->CreateTransformedGeometry(
            d2d1_rect_geometry.get(),
            &matrix,
            d2d1_transformed_geometry.put()
        );
        if (FAILED(hr)) return hr;

        if (layer_stroke) d2d1_rt->DrawGeometry(d2d1_transformed_geometry.get(), d2d1_brush_outline.get(), outline_width);

        if (layer_text) d2d1_rt->FillGeometry(d2d1_transformed_geometry.get(), d2d1_brush_fill.get());

        return S_OK;
    }

    HRESULT WINAPI Direct2dDirectWriteTextRenderer::DrawStrikethrough(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
        DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown* clientDrawingEffect
    ) noexcept {
        UNREFERENCED_PARAMETER(clientDrawingContext);
        UNREFERENCED_PARAMETER(clientDrawingEffect);

        HRESULT hr = S_OK;

        D2D1_RECT_F rect = D2D1::RectF(
            0,
            strikethrough->offset,
            strikethrough->width,
            strikethrough->offset + strikethrough->thickness
        );

        win32::com_ptr<ID2D1RectangleGeometry> d2d1_rect_geometry;
        hr = d2d1_factory->CreateRectangleGeometry(&rect, d2d1_rect_geometry.put());
        if (FAILED(hr)) return hr;

        // TODO: 为什么旋转方向是这样判断的？
        FLOAT rotate_angle = 0.0f;
        //switch (orientationAngle)
        //{
        //case DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES: rotate_angle = 0.0f; break;
        //case DWRITE_GLYPH_ORIENTATION_ANGLE_90_DEGREES: rotate_angle = 90.0f; break;
        //case DWRITE_GLYPH_ORIENTATION_ANGLE_180_DEGREES: rotate_angle = 180.0f; break;
        //case DWRITE_GLYPH_ORIENTATION_ANGLE_270_DEGREES: rotate_angle = 270.0f; break;
        //default: assert(false); break;
        //}
        UNREFERENCED_PARAMETER(orientationAngle);
        switch (dwrite_text_layout->GetReadingDirection()) {
            case DWRITE_READING_DIRECTION_LEFT_TO_RIGHT: rotate_angle = 0.0f; break;
            //case DWRITE_READING_DIRECTION_RIGHT_TO_LEFT: rotate_angle = 0.0f; break;
            case DWRITE_READING_DIRECTION_TOP_TO_BOTTOM: rotate_angle = 90.0f; break;
            //case DWRITE_READING_DIRECTION_BOTTOM_TO_TOP: rotate_angle = 90.0f; break;
            default: assert(false); break;
        }

        D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Rotation(rotate_angle);
        matrix.dx = baselineOriginX;
        matrix.dy = baselineOriginY;

        win32::com_ptr<ID2D1TransformedGeometry> d2d1_transformed_geometry;
        hr = d2d1_factory->CreateTransformedGeometry(
            d2d1_rect_geometry.get(),
            &matrix,
            d2d1_transformed_geometry.put()
        );
        if (FAILED(hr)) return hr;

        if (layer_stroke) d2d1_rt->DrawGeometry(d2d1_transformed_geometry.get(), d2d1_brush_outline.get(), outline_width);

        if (layer_text) d2d1_rt->FillGeometry(d2d1_transformed_geometry.get(), d2d1_brush_fill.get());

        return S_OK;
    }

    HRESULT WINAPI Direct2dDirectWriteTextRenderer::DrawInlineObject(
        void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
        IDWriteInlineObject* inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect
    ) noexcept {
        UNREFERENCED_PARAMETER(clientDrawingContext);
        UNREFERENCED_PARAMETER(originX);
        UNREFERENCED_PARAMETER(originY);
        UNREFERENCED_PARAMETER(orientationAngle);
        UNREFERENCED_PARAMETER(inlineObject);
        UNREFERENCED_PARAMETER(isSideways);
        UNREFERENCED_PARAMETER(isRightToLeft);
        UNREFERENCED_PARAMETER(clientDrawingEffect);
        return S_OK;
    }

    // Direct2dDirectWriteTextRenderer

    Direct2dDirectWriteTextRenderer::Direct2dDirectWriteTextRenderer(
        ID2D1Factory* const factory,
        ID2D1RenderTarget* const target,
        IDWriteTextLayout* const layout,
        ID2D1Brush* const outline,
        ID2D1Brush* const fill,
        ID2D1StrokeStyle* const stroke_style,
        const FLOAT width
    ) : d2d1_factory(factory),
        d2d1_rt(target),
        dwrite_text_layout(layout),
        d2d1_brush_outline(outline),
        d2d1_brush_fill(fill),
        d2d1_stroke_style(stroke_style),
        outline_width(width) {
    }

    void Direct2dDirectWriteTextRenderer::setLayerEnable(const BOOL text, const BOOL stroke) noexcept {
        layer_text = text;
        layer_stroke = stroke;
    }
}
