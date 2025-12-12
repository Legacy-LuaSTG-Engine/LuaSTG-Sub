#pragma once
#include "core/FontCollection.hpp"
#include "core/Vector2.hpp"
#include "core/Image.hpp"

namespace core {
    enum class TextAlignment : int32_t {
        start = 0,
        center = 1,
        end = 2,
    };

    enum class ParagraphAlignment : int32_t {
        start = 0,
        center = 1,
        end = 2,
    };

    CORE_INTERFACE ITextLayout : public IReferenceCounted {
        virtual void setText(StringView text) = 0;
        virtual void setFontCollection(IFontCollection* font_collection) = 0;
        virtual void setFontFamily(StringView font_family_name) = 0;
        virtual void setFontSize(float size) = 0;
        virtual void setFontWeight(FontWeight weight) = 0;
        virtual void setFontStyle(FontStyle style) = 0;
        virtual void setFontWidth(FontWidth width) = 0;
        virtual void setLayoutSize(Vector2F size) = 0;
        virtual void setTextAlignment(TextAlignment alignment) = 0;
        virtual void setParagraphAlignment(ParagraphAlignment alignment) = 0;

        virtual void setTextColor(Vector4F color) = 0;
        virtual void setStrokeWidth(float width) = 0;
        virtual void setStrokeColor(Vector4F color) = 0;

        virtual bool build() = 0;
        virtual uint32_t getVersion() = 0;
        virtual IImage* getImage() = 0;

        static bool create(ITextLayout** output);
    };
    CORE_INTERFACE_ID(ITextLayout, "4c5e5524-815f-5632-915d-0e14df3dfce3");
}
