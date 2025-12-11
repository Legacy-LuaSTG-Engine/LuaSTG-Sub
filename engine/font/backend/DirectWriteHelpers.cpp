#include "backend/DirectWriteHelpers.hpp"
#include "win32/base.hpp"

#pragma comment(lib, "dwrite.lib")

namespace {
    using std::string_view_literals::operator ""sv;

    class Instance final {
    public:
        Instance() = default;
        Instance(const Instance&) = delete;
        Instance(Instance&&) = delete;
        ~Instance() {
            if (m_factory != nullptr) {
                m_factory->Release();
            }
        }
    
        Instance& operator=(const Instance&) = delete;
        Instance& operator=(Instance&&) = delete;

        IDWriteFactory* get() {
            if (m_factory == nullptr) {
                IDWriteFactory* factory{};
                if (!win32::check_hresult_as_boolean(
                    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&factory)),
                    "DWriteCreateFactory"sv
                )) {
                    return nullptr;
                }
                if (m_factory != nullptr) {
                    factory->Release();
                    return m_factory;
                }
                const auto original = InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(&m_factory), factory, nullptr);
                if (original != nullptr) {
                    factory->Release();
                }
            }
            return m_factory;
        }

    private:
        IDWriteFactory* volatile m_factory{};
    };

    Instance s_instance;
}

namespace core {
    IDWriteFactory* DirectWriteHelpers::getFactory() {
        return s_instance.get();
    }

    DWRITE_FONT_WEIGHT DirectWriteHelpers::toFontWeight(const FontWeight weight) noexcept {
        switch (weight) {
        case FontWeight::thin:        return DWRITE_FONT_WEIGHT_THIN;
        case FontWeight::extra_light: return DWRITE_FONT_WEIGHT_EXTRA_BLACK;
        case FontWeight::light:       return DWRITE_FONT_WEIGHT_LIGHT;
        case FontWeight::normal:      return DWRITE_FONT_WEIGHT_NORMAL;
        case FontWeight::medium:      return DWRITE_FONT_WEIGHT_MEDIUM;
        case FontWeight::semi_bold:   return DWRITE_FONT_WEIGHT_SEMI_BOLD;
        case FontWeight::bold:        return DWRITE_FONT_WEIGHT_BOLD;
        case FontWeight::extra_bold:  return DWRITE_FONT_WEIGHT_EXTRA_BOLD;
        case FontWeight::black:       return DWRITE_FONT_WEIGHT_BLACK;
        default: return {}; // unlikely
        }
    }
    DWRITE_FONT_STYLE DirectWriteHelpers::toFontStyle(const FontStyle style) noexcept {
        switch (style) {
        case FontStyle::normal: return DWRITE_FONT_STYLE_NORMAL;
        case FontStyle::italic: return DWRITE_FONT_STYLE_ITALIC;
        default: return {}; // unlikely
        }
    }
    DWRITE_FONT_STRETCH DirectWriteHelpers::toFontWidth(const FontWidth width) noexcept {
        switch (width) {
        case FontWidth::ultra_condensed: return DWRITE_FONT_STRETCH_ULTRA_CONDENSED;
        case FontWidth::extra_condensed: return DWRITE_FONT_STRETCH_EXTRA_CONDENSED;
        case FontWidth::condensed:       return DWRITE_FONT_STRETCH_CONDENSED;
        case FontWidth::semi_condensed:  return DWRITE_FONT_STRETCH_SEMI_CONDENSED;
        case FontWidth::normal:          return DWRITE_FONT_STRETCH_NORMAL;
        case FontWidth::semi_expanded:   return DWRITE_FONT_STRETCH_SEMI_EXPANDED;
        case FontWidth::expanded:        return DWRITE_FONT_STRETCH_EXPANDED;
        case FontWidth::extra_expanded:  return DWRITE_FONT_STRETCH_EXTRA_EXPANDED;
        case FontWidth::ultra_expanded:  return DWRITE_FONT_STRETCH_ULTRA_EXPANDED;
        default: return {}; // unlikely
        }
    }
}
