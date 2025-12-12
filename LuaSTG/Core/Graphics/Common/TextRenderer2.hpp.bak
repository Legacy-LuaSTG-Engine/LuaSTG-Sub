#pragma once
#include "Core/Graphics/Font.hpp"
#include "Core/Graphics/Sprite.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"

struct IDWriteTextFormat;
struct IDWriteTextLayout;

namespace core {
    class TextLayout_DirectWrite final : public implement::ReferenceCounted<ITextLayout> {
    public:
        // ITextLayout

        void setText(StringView text) override;
        void setFontFamilyName(StringView font_family_name) override;
        void setFontSize(float size) override;
        void setFontWeight(FontWeight weight) override;
        void setFontStyle(FontStyle style) override;
        void setFontWidth(FontWidth width) override;
        void setLayoutSize(Vector2F size) override;
        void setTextAlignment(TextAlignment alignment) override;
        void setParagraphAlignment(ParagraphAlignment alignment) override;

        bool build() override;
        Graphics::ITexture2D* getTexture() override;

        // TextLayout_DirectWrite

        explicit TextLayout_DirectWrite(Graphics::IDevice* device);
        TextLayout_DirectWrite(const TextLayout_DirectWrite&) = delete;
        TextLayout_DirectWrite(TextLayout_DirectWrite&&) = delete;

        TextLayout_DirectWrite& operator=(const TextLayout_DirectWrite&) = delete;
        TextLayout_DirectWrite& operator=(TextLayout_DirectWrite&&) = delete;

    private:
        win32::com_ptr<IDWriteTextFormat> m_text_format;
        win32::com_ptr<IDWriteTextLayout> m_text_layout;

        SmartReference<IImage> m_image;
        SmartReference<Graphics::IDevice> m_device;
        SmartReference<Graphics::ITexture2D> m_texture;

        std::string m_text;
        std::string m_locale_name{ "zh-cn" };
        std::string m_font_family_name{ "Arial" };
        float m_font_size{ 10.0f };
        FontWeight m_font_weight{ FontWeight::normal };
        FontStyle m_font_style{ FontStyle::normal };
        FontWidth m_font_width{ FontWidth::normal };
        Vector2F m_layout_size{ 1.0f, 1.0f };
        TextAlignment m_text_alignment{ TextAlignment::start };
        ParagraphAlignment m_paragraph_alignment{ ParagraphAlignment::start };
        bool m_dirty{ true };
    };
}
