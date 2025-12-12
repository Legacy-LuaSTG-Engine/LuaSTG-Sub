#pragma once
#include "core/TextLayout.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "win32/base.hpp"

struct IDWriteTextFormat;
struct IDWriteTextLayout;

namespace core {
    class DirectWriteTextLayout final : public implement::ReferenceCounted<ITextLayout> {
    public:
        // ITextLayout

        void setText(StringView text) override;
        void setFontCollection(IFontCollection* font_collection) override;
        void setFontFamily(StringView font_family_name) override;
        void setFontSize(float size) override;
        void setFontWeight(FontWeight weight) override;
        void setFontStyle(FontStyle style) override;
        void setFontWidth(FontWidth width) override;
        void setLayoutSize(Vector2F size) override;
        void setTextAlignment(TextAlignment alignment) override;
        void setParagraphAlignment(ParagraphAlignment alignment) override;

        void setTextColor(Vector4F color) override;
        void setStrokeWidth(float width) override;
        void setStrokeColor(Vector4F color) override;

        bool build() override;
        uint32_t getVersion() override;
        IImage* getImage() override;

        // DirectWriteTextLayout

        DirectWriteTextLayout();
        DirectWriteTextLayout(const DirectWriteTextLayout&) = delete;
        DirectWriteTextLayout(DirectWriteTextLayout&&) = delete;
        ~DirectWriteTextLayout();

        DirectWriteTextLayout& operator=(const DirectWriteTextLayout&) = delete;
        DirectWriteTextLayout& operator=(DirectWriteTextLayout&&) = delete;

    private:
        win32::com_ptr<IDWriteTextFormat> m_text_format;
        win32::com_ptr<IDWriteTextLayout> m_text_layout;

        SmartReference<IFontCollection> m_font_collection;
        SmartReference<IImage> m_image;

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
        Vector4F m_text_color{ 1.0f, 1.0f, 1.0f, 1.0f };
        Vector4F m_stroke_color{ 0.0f, 0.0f, 0.0f, 1.0f };
        float m_stroke_width{};
        uint32_t m_version{};
        bool m_dirty{ true };
    };
}
