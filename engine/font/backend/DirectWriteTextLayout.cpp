#include "backend/DirectWriteTextLayout.hpp"
#include "backend/WicImage.hpp"
#include <cassert>
#include "utf8.hpp"
#include "backend/DirectWriteHelpers.hpp"
#include <d2d1_3.h>
#include <wincodec.h>

#pragma comment(lib, "d2d1.lib")

namespace {
    using std::string_view_literals::operator ""sv;

    DWRITE_TEXT_ALIGNMENT toTextAlignment(const core::TextAlignment alignment) noexcept {
        switch (alignment) {
        case core::TextAlignment::start:  return DWRITE_TEXT_ALIGNMENT_LEADING;
        case core::TextAlignment::center: return DWRITE_TEXT_ALIGNMENT_CENTER;
        case core::TextAlignment::end:    return DWRITE_TEXT_ALIGNMENT_TRAILING;
        default: return {}; // unlikely
        }
    }
    DWRITE_PARAGRAPH_ALIGNMENT toParagraphAlignment(const core::ParagraphAlignment alignment) noexcept {
        switch (alignment) {
        case core::ParagraphAlignment::start:  return DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
        case core::ParagraphAlignment::center: return DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
        case core::ParagraphAlignment::end:    return DWRITE_PARAGRAPH_ALIGNMENT_FAR;
        default: return {}; // unlikely
        }
    }
    D2D1_COLOR_F toColor(const core::Vector4F& color) {
        D2D1_COLOR_F result{};
        result.r = color.x;
        result.g = color.y;
        result.b = color.z;
        result.a = color.w;
        return result;
    };

    win32::com_ptr<ID2D1Factory> s_d2d1_factory;

    ID2D1Factory* getRendererFactory() {
        if (!s_d2d1_factory) {
            constexpr D2D1_FACTORY_OPTIONS options{
        #ifdef NDEBUG
                .debugLevel = D2D1_DEBUG_LEVEL_NONE,
        #else
                .debugLevel = D2D1_DEBUG_LEVEL_INFORMATION,
        #endif
            };
            win32::check_hresult_throw_if_failed(
                D2D1CreateFactory(
                    D2D1_FACTORY_TYPE_SINGLE_THREADED,
                    __uuidof(ID2D1Factory),
                    &options,
                    s_d2d1_factory.put<void>()
                ),
                "D2D1CreateFactory"sv
            );
        }
        return s_d2d1_factory.get();
    }

    class DirectWriteTextRenderer final : public IDWriteTextRenderer1 {
    public:
        // IUnknown
        
        HRESULT WINAPI QueryInterface(IID const& riid, void** ppvObject) override {
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
        ULONG WINAPI AddRef() override { return 2; }
        ULONG WINAPI Release() override { return 1; }

        // IDWritePixelSnapping

        HRESULT WINAPI IsPixelSnappingDisabled(
            void* clientDrawingContext,
            BOOL* isDisabled
        ) noexcept override {
            UNREFERENCED_PARAMETER(clientDrawingContext);
            *isDisabled = FALSE; // recommended default value
            return S_OK;
        }
        
        HRESULT WINAPI GetCurrentTransform(
            void* clientDrawingContext,
            DWRITE_MATRIX* transform
        ) noexcept override {
            UNREFERENCED_PARAMETER(clientDrawingContext);
            // forward the render target's transform
            d2d1_rt->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
            return S_OK;
        }

        HRESULT WINAPI GetPixelsPerDip(
            void* clientDrawingContext,
            FLOAT* pixelsPerDip
        ) noexcept override {
            UNREFERENCED_PARAMETER(clientDrawingContext);
            float x = 0.0f, y = 0.0f;
            d2d1_rt->GetDpi(&x, &y);
            *pixelsPerDip = x / 96.0f;
            return S_OK;
        }

        // IDWriteTextRenderer

        HRESULT WINAPI DrawGlyphRun(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_MEASURING_MODE measuringMode,
            DWRITE_GLYPH_RUN const* glyphRun,
            DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
            IUnknown* clientDrawingEffect
        ) noexcept override {
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

        HRESULT WINAPI DrawUnderline(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_UNDERLINE const* underline,
            IUnknown* clientDrawingEffect
        ) noexcept override {
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

        HRESULT WINAPI DrawStrikethrough(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_STRIKETHROUGH const* strikethrough,
            IUnknown* clientDrawingEffect
        ) noexcept override {
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

        HRESULT WINAPI DrawInlineObject(
            void* clientDrawingContext,
            FLOAT originX,
            FLOAT originY,
            IDWriteInlineObject* inlineObject,
            BOOL isSideways,
            BOOL isRightToLeft,
            IUnknown* clientDrawingEffect
        ) noexcept override {
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

        HRESULT WINAPI DrawGlyphRun(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
            DWRITE_MEASURING_MODE measuringMode,
            DWRITE_GLYPH_RUN const* glyphRun,
            DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
            IUnknown* clientDrawingEffect
        ) noexcept override {
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

        HRESULT WINAPI DrawUnderline(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
            DWRITE_UNDERLINE const* underline,
            IUnknown* clientDrawingEffect
        ) noexcept override {
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

        HRESULT WINAPI DrawStrikethrough(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
            DWRITE_STRIKETHROUGH const* strikethrough,
            IUnknown* clientDrawingEffect
        ) noexcept override {
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

        HRESULT WINAPI DrawInlineObject(
            void* clientDrawingContext,
            FLOAT originX,
            FLOAT originY,
            DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
            IDWriteInlineObject* inlineObject,
            BOOL isSideways,
            BOOL isRightToLeft,
            IUnknown* clientDrawingEffect
        ) noexcept override {
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

        // DirectWriteTextRenderer

        DirectWriteTextRenderer(
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

        void setLayerEnable(const BOOL text, const BOOL stroke) noexcept {
            layer_text = text;
            layer_stroke = stroke;
        }

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

namespace core {
    // ITextLayout

    void DirectWriteTextLayout::setText(const StringView text) {
        if (m_text == text) {
            return;
        }
        m_text = text;
        m_dirty = true;
    }
    void DirectWriteTextLayout::setFontCollection(IFontCollection* const font_collection) {
        if (m_font_collection.get() == font_collection) {
            return;
        }
        m_font_collection = font_collection;
        m_dirty = true;
    }
    void DirectWriteTextLayout::setFontFamily(const StringView font_family_name) {
        if (m_font_family_name == font_family_name) {
            return;
        }
        m_font_family_name = font_family_name;
        m_dirty = true;
    }
    void DirectWriteTextLayout::setFontSize(const float size) {
        if (m_font_size == size) {
            return;
        }
        m_font_size = size;
        m_dirty = true;
    }
    void DirectWriteTextLayout::setFontWeight(const FontWeight weight) {
        if (m_font_weight == weight) {
            return;
        }
        m_font_weight = weight;
        m_dirty = true;
    }
    void DirectWriteTextLayout::setFontStyle(const FontStyle style) {
        if (m_font_style == style) {
            return;
        }
        m_font_style = style;
        m_dirty = true;
    }
    void DirectWriteTextLayout::setFontWidth(const FontWidth width) {
        if (m_font_width == width) {
            return;
        }
        m_font_width = width;
        m_dirty = true;
    }
    void DirectWriteTextLayout::setLayoutSize(const Vector2F size) {
        if (m_layout_size == size) {
            return;
        }
        m_layout_size = size;
        m_dirty = true;
    }
    void DirectWriteTextLayout::setTextAlignment(const TextAlignment alignment) {
        if (m_text_alignment == alignment) {
            return;
        }
        m_text_alignment = alignment;
        m_dirty = true;
    }
    void DirectWriteTextLayout::setParagraphAlignment(const ParagraphAlignment alignment) {
        if (m_paragraph_alignment == alignment) {
            return;
        }
        m_paragraph_alignment = alignment;
        m_dirty = true;
    }

    void DirectWriteTextLayout::setTextColor(const Vector4F color) {
        if (m_text_color == color) {
            return;
        }
        m_text_color = color;
        m_dirty = true;
    }
    void DirectWriteTextLayout::setStrokeWidth(const float width) {
        const auto value = std::max(0.0f, width);
        if (m_stroke_width == value) {
            return;
        }
        m_stroke_width = value;
        m_dirty = true;
    }
    void DirectWriteTextLayout::setStrokeColor(const Vector4F color) {
        if (m_stroke_color == color) {
            return;
        }
        m_stroke_color = color;
        m_dirty = true;
    }

    bool DirectWriteTextLayout::build() {
        if (!m_dirty) {
            return true;
        }

        // Stage 1: text

        const auto dwrite = DirectWriteHelpers::getFactory();
        if (dwrite == nullptr) {
            return false;
        }

        IDWriteFontCollection* font_collection{};
        if (m_font_collection) {
            font_collection = static_cast<IDWriteFontCollection*>(m_font_collection->getNativeHandle());
        }

        const auto font_family_name = utf8::to_wstring(m_font_family_name);
        const auto locale_name = utf8::to_wstring(m_locale_name);
        if (!win32::check_hresult_as_boolean(
            dwrite->CreateTextFormat(
                font_family_name.c_str(),
                font_collection,
                DirectWriteHelpers::toFontWeight(m_font_weight),
                DirectWriteHelpers::toFontStyle(m_font_style),
                DirectWriteHelpers::toFontWidth(m_font_width),
                m_font_size,
                locale_name.c_str(),
                m_text_format.put()
            ),
            "IDWriteFactory::CreateTextFormat"sv
        )) {
            return false;
        }

        if (!win32::check_hresult_as_boolean(
            m_text_format->SetTextAlignment(toTextAlignment(m_text_alignment)),
            "IDWriteTextFormat::SetTextAlignment"sv
        )) {
            return false;
        }

        if (!win32::check_hresult_as_boolean(
            m_text_format->SetParagraphAlignment(toParagraphAlignment(m_paragraph_alignment)),
            "IDWriteTextFormat::SetParagraphAlignment"sv
        )) {
            return false;
        }

        const auto text = utf8::to_wstring(m_text);
        if (!win32::check_hresult_as_boolean(
            dwrite->CreateTextLayout(
                text.c_str(), static_cast<UINT32>(text.length()),
                m_text_format.get(),
                m_layout_size.x, m_layout_size.y,
                m_text_layout.put()
            ),
            "IDWriteFactory::CreateTextLayout"sv
        )) {
            return false;
        }

        // Stage 2: image

        ImageDescription image_description{};
        if (m_stroke_width > 0.00001f) {
            image_description.size.x = static_cast<uint32_t>(std::ceil(m_layout_size.x) + 2.0f * std::ceil(m_stroke_width));
            image_description.size.y = static_cast<uint32_t>(std::ceil(m_layout_size.y) + 2.0f * std::ceil(m_stroke_width));
        }
        else {
            image_description.size.x = static_cast<uint32_t>(std::ceil(m_layout_size.x));
            image_description.size.y = static_cast<uint32_t>(std::ceil(m_layout_size.y));
        }
        image_description.format = ImageFormat::b8g8r8a8_normalized;
        image_description.color_space = ImageColorSpace::srgb_gamma_2_2;

        if (!m_image || m_image->getDescription()->size != image_description.size) {
            if (!ImageFactory::create(image_description, m_image.put())) {
                return false;
            }
        }

        ScopedImageMappedBuffer buffer{};
        if (!m_image->createScopedMap(buffer)) {
            return false;
        }

        // Stage 3: draw

        const auto d2d1 = getRendererFactory();

        win32::com_ptr<IWICBitmap> bitmap;
        if (!WicImage::createFromImage(m_image.get(), &buffer, bitmap.put<void>())) {
            return false;
        }

        D2D1_RENDER_TARGET_PROPERTIES render_target_properties{};
        render_target_properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
        render_target_properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

        win32::com_ptr<ID2D1RenderTarget> render_target;
        if (!win32::check_hresult_as_boolean(
            d2d1->CreateWicBitmapRenderTarget(
                bitmap.get(), &render_target_properties,
                render_target.put()
            ),
            "ID2D1Factory::CreateWicBitmapRenderTarget"sv
        )) {
            return false;
        }

        const D2D1_COLOR_F text_color{ toColor(m_text_color) };
        const auto brush_properties = D2D1::BrushProperties();
        win32::com_ptr<ID2D1SolidColorBrush> text_brush;
        if (!win32::check_hresult_as_boolean(
            render_target->CreateSolidColorBrush(&text_color, &brush_properties, text_brush.put()),
            "ID2D1RenderTarget::CreateSolidColorBrush"sv
        )) {
            return false;
        }

        win32::com_ptr<IDWriteRenderingParams> rendering_params;
        if (!win32::check_hresult_as_boolean(
            dwrite->CreateCustomRenderingParams(
                1.0f, // gamma
                0.0f, // no Enhanced contrast
                0.0f, // no ClearType
                DWRITE_PIXEL_GEOMETRY_FLAT,
                DWRITE_RENDERING_MODE_OUTLINE,
                rendering_params.put()
            ),
            "IDWriteFactory::CreateCustomRenderingParams"sv
        )) {
            return false;
        }

        render_target->BeginDraw();
        render_target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        render_target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
        render_target->SetTextRenderingParams(rendering_params.get());
        constexpr D2D1_COLOR_F clear_color{};
        render_target->Clear(&clear_color);

        if (m_stroke_width > 0.00001f) {
            const D2D1_COLOR_F stroke_color{ toColor(m_stroke_color) };
            win32::com_ptr<ID2D1SolidColorBrush> stroke_brush;
            if (!win32::check_hresult_as_boolean(
                render_target->CreateSolidColorBrush(&stroke_color, &brush_properties, stroke_brush.put()),
                "ID2D1RenderTarget::CreateSolidColorBrush"sv
            )) {
                return false;
            }

            win32::com_ptr<ID2D1StrokeStyle> stroke_style;
            if (!win32::check_hresult_as_boolean(
                d2d1->CreateStrokeStyle(D2D1::StrokeStyleProperties(
                    D2D1_CAP_STYLE_ROUND,
                    D2D1_CAP_STYLE_ROUND,
                    D2D1_CAP_STYLE_ROUND,
                    D2D1_LINE_JOIN_ROUND
                ), nullptr, 0, stroke_style.put()),
                "ID2D1Factory::CreateStrokeStyle"sv
            )) {
                return false;
            }

            DirectWriteTextRenderer renderer(
                d2d1,
                render_target.get(),
                m_text_layout.get(),
                stroke_brush.get(),
                text_brush.get(),
                stroke_style.get(),
                m_stroke_width
            );

            const auto offset = std::ceil(m_stroke_width);

            // draw stroke

            renderer.setLayerEnable(FALSE, TRUE);
            if (!win32::check_hresult_as_boolean(
                m_text_layout->Draw(nullptr, &renderer, offset, offset),
                "IDWriteTextLayout::Draw"sv
            )) {
                return false;
            }

            // draw text

            renderer.setLayerEnable(TRUE, FALSE);
            if (!win32::check_hresult_as_boolean(
                m_text_layout->Draw(nullptr, &renderer, offset, offset),
                "IDWriteTextLayout::Draw"sv
            )) {
                return false;
            }
        }
        else {
            constexpr D2D1_POINT_2F position{};
            constexpr D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_NO_SNAP
                | D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
                | D2D1_DRAW_TEXT_OPTIONS_DISABLE_COLOR_BITMAP_SNAPPING;
            render_target->DrawTextLayout(position, m_text_layout.get(), text_brush.get(), options);
        }

        if (!win32::check_hresult_as_boolean(
            render_target->EndDraw(),
            "ID2D1RenderTarget::EndDraw"sv
        )) {
            return false;
        }

        m_version += 1;
        m_dirty = false;
        return true;
    }
    uint32_t DirectWriteTextLayout::getVersion() {
        return m_version;
    }
    IImage* DirectWriteTextLayout::getImage() {
        return m_image.get();
    }

    // DirectWriteTextLayout

    DirectWriteTextLayout::DirectWriteTextLayout() = default;
    DirectWriteTextLayout::~DirectWriteTextLayout() = default;
}

namespace core {
    bool ITextLayout::create(ITextLayout** const output) {
        if (output == nullptr) {
            assert(false);
            return false;
        }
        *output = new DirectWriteTextLayout();
        return true;
    }
}
