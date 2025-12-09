#include "Core/Graphics/Common/TextRenderer2.hpp"
#include "core/Image.hpp"
#include "backend/WicImage.hpp"
#include <cassert>
#include "utf8.hpp"
#include <d2d1_3.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

namespace {
    using std::string_view_literals::operator ""sv;

    DWRITE_FONT_WEIGHT toFontWeight(const core::FontWeight weight) noexcept {
        switch (weight) {
        case core::FontWeight::thin:        return DWRITE_FONT_WEIGHT_THIN;
        case core::FontWeight::extra_light: return DWRITE_FONT_WEIGHT_EXTRA_BLACK;
        case core::FontWeight::light:       return DWRITE_FONT_WEIGHT_LIGHT;
        case core::FontWeight::normal:      return DWRITE_FONT_WEIGHT_NORMAL;
        case core::FontWeight::medium:      return DWRITE_FONT_WEIGHT_MEDIUM;
        case core::FontWeight::semi_bold:   return DWRITE_FONT_WEIGHT_SEMI_BOLD;
        case core::FontWeight::bold:        return DWRITE_FONT_WEIGHT_BOLD;
        case core::FontWeight::extra_bold:  return DWRITE_FONT_WEIGHT_EXTRA_BOLD;
        case core::FontWeight::black:       return DWRITE_FONT_WEIGHT_BLACK;
        default: return {}; // unlikely
        }
    }
    DWRITE_FONT_STYLE toFontStyle(const core::FontStyle style) noexcept {
        switch (style) {
        case core::FontStyle::normal: return DWRITE_FONT_STYLE_NORMAL;
        case core::FontStyle::italic: return DWRITE_FONT_STYLE_ITALIC;
        default: return {}; // unlikely
        }
    }
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

    win32::com_ptr<IDWriteFactory> s_dwrite_factory;
    win32::com_ptr<ID2D1Factory> s_d2d1_factory;

    IDWriteFactory* getFactory() {
        if (!s_dwrite_factory) {
            win32::check_hresult_throw_if_failed(
                DWriteCreateFactory(
                    DWRITE_FACTORY_TYPE_SHARED,
                    __uuidof(IDWriteFactory),
                    s_dwrite_factory.put<IUnknown>()
                ),
                "DWriteCreateFactory"sv
            );
        }
        return s_dwrite_factory.get();
    }
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
}

namespace core {
    // ITextLayout

    void TextLayout_DirectWrite::setText(const StringView text) {
        if (m_text == text) {
            return;
        }
        m_text = text;
        m_dirty = true;
    }
    void TextLayout_DirectWrite::setFontFamilyName(const StringView font_family_name) {
        if (m_font_family_name == font_family_name) {
            return;
        }
        m_font_family_name = font_family_name;
        m_dirty = true;
    }
    void TextLayout_DirectWrite::setFontSize(const float size) {
        if (m_font_size == size) {
            return;
        }
        m_font_size = size;
        m_dirty = true;
    }
    void TextLayout_DirectWrite::setFontWeight(const FontWeight weight) {
        if (m_font_weight == weight) {
            return;
        }
        m_font_weight = weight;
        m_dirty = true;
    }
    void TextLayout_DirectWrite::setFontStyle(const FontStyle style) {
        if (m_font_style == style) {
            return;
        }
        m_font_style = style;
        m_dirty = true;
    }
    void TextLayout_DirectWrite::setLayoutSize(const Vector2F size) {
        if (m_layout_size == size) {
            return;
        }
        m_layout_size = size;
        m_dirty = true;
    }
    void TextLayout_DirectWrite::setTextAlignment(const TextAlignment alignment) {
        if (m_text_alignment == alignment) {
            return;
        }
        m_text_alignment = alignment;
        m_dirty = true;
    }
    void TextLayout_DirectWrite::setParagraphAlignment(const ParagraphAlignment alignment) {
        if (m_paragraph_alignment == alignment) {
            return;
        }
        m_paragraph_alignment = alignment;
        m_dirty = true;
    }

    bool TextLayout_DirectWrite::build() {
        if (!m_dirty) {
            return true;
        }

        // Stage 1: text

        const auto dwrite = getFactory();

        const auto font_family_name = utf8::to_wstring(m_font_family_name);
        const auto locale_name = utf8::to_wstring(m_locale_name);
        if (!win32::check_hresult_as_boolean(
            dwrite->CreateTextFormat(
                font_family_name.c_str(),
                nullptr,
                toFontWeight(m_font_weight),
                toFontStyle(m_font_style),
                DWRITE_FONT_STRETCH_NORMAL,
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
        image_description.size.x = static_cast<uint32_t>(std::ceil(m_layout_size.x));
        image_description.size.y = static_cast<uint32_t>(std::ceil(m_layout_size.y));
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

        constexpr D2D1_COLOR_F text_color{ 1.0f, 1.0f, 1.0f, 1.0f };
        const auto brush_properties = D2D1::BrushProperties();
        win32::com_ptr<ID2D1SolidColorBrush> brush;
        if (!win32::check_hresult_as_boolean(
            render_target->CreateSolidColorBrush(&text_color, &brush_properties, brush.put()),
            "ID2D1RenderTarget::CreateSolidColorBrush"sv
        )) {
            return false;
        }

        render_target->BeginDraw();
        render_target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        render_target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
        constexpr D2D1_COLOR_F clear_color{};
        render_target->Clear(&clear_color);
        constexpr D2D1_POINT_2F position{};
        constexpr D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_NO_SNAP
            | D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
            | D2D1_DRAW_TEXT_OPTIONS_DISABLE_COLOR_BITMAP_SNAPPING;
        render_target->DrawTextLayout(position, m_text_layout.get(), brush.get(), options);
        if (!win32::check_hresult_as_boolean(
            render_target->EndDraw(),
            "ID2D1RenderTarget::EndDraw"sv
        )) {
            return false;
        }

        // Stage 4: texture

        if (!m_texture) {
            if (!m_device->createTexture(image_description.size, m_texture.put())) {
                return false;
            }
            m_texture->setPremultipliedAlpha(true);
        }
        if (m_texture->getSize() != image_description.size) {
            if (!m_texture->setSize(image_description.size)) {
                return false;
            }
        }

        const RectU rect(0u, 0u, image_description.size.x, image_description.size.y);
        if (!m_texture->uploadPixelData(rect, buffer.data, buffer.stride)) {
            return false;
        }
        m_texture->setImage(m_image.get());

        m_dirty = false;
        return true;
    }
    Graphics::ITexture2D* TextLayout_DirectWrite::getTexture() {
        return m_texture.get();
    }
}
