#include "core/Logger.hpp"
#include "core/SmartReference.hpp"
#include "core/FontCollection.hpp"
#include "utf8.hpp"
#include "win32/base.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "gtest/gtest.h"
#include <filesystem>

namespace {
    using std::string_literals::operator ""s;
    using std::string_view_literals::operator ""sv;

    void win32LoggerWriter(const std::string_view message) {
        core::Logger::error(message);
    }

    void setupLogger() {
        if (!spdlog::get("test"s)) {
            spdlog::set_default_logger(spdlog::stdout_color_mt("test"s));
        }
        win32::set_logger_writer(&win32LoggerWriter);
    }

    std::string_view to_string_view(const core::FontWeight weight) {
        static std::string s;
        switch (weight) {
            case core::FontWeight::thin:        return "thin"sv;
            case core::FontWeight::extra_light: return "extra_light"sv;
            case core::FontWeight::light:       return "light"sv;
            case core::FontWeight::normal:      return "normal"sv;
            case core::FontWeight::medium:      return "medium"sv;
            case core::FontWeight::semi_bold:   return "semi_bold"sv;
            case core::FontWeight::bold:        return "bold"sv;
            case core::FontWeight::extra_bold:  return "extra_bold"sv;
            case core::FontWeight::black:       return "black"sv;
            default: return s = std::to_string(std::to_underlying(weight));
        }
    }
    std::string_view to_string_view(const core::FontStyle style) {
        static std::string s;
        switch (style) {
            case core::FontStyle::normal:  return "normal"sv;
            case core::FontStyle::oblique: return "oblique"sv;
            case core::FontStyle::italic:  return "italic"sv;
            default: return s = std::to_string(std::to_underlying(style));
        }
    }
    std::string_view to_string_view(const core::FontWidth width) {
        static std::string s;
        switch (width) {
            case core::FontWidth::ultra_condensed: return "ultra_condensed"sv;
            case core::FontWidth::extra_condensed: return "extra_condensed"sv;
            case core::FontWidth::condensed:       return "condensed"sv;
            case core::FontWidth::semi_condensed:  return "semi_condensed"sv;
            case core::FontWidth::normal:          return "normal"sv;
            case core::FontWidth::semi_expanded:   return "semi_expanded"sv;
            case core::FontWidth::expanded:        return "expanded"sv;
            case core::FontWidth::extra_expanded:  return "extra_expanded"sv;
            case core::FontWidth::ultra_expanded:  return "ultra_expanded"sv;
            default: return s = std::to_string(std::to_underlying(width));
        }
    }

    void printFontCollectionInfo(core::IFontCollection* const font_collection) {
        using namespace core;

        for (uint32_t i = 0; i < font_collection->getFontFamilyCount(); i += 1) {
            SmartReference<IImmutableString> font_family_name;
            ASSERT_TRUE(font_collection->getFontFamilyName(i, font_family_name.put()));
            Logger::info("[{}] {}"sv, i, font_family_name->view());

            for (uint32_t j = 0; j < font_collection->getFontCount(i); j += 1) {
                SmartReference<IImmutableString> font_name;
                ASSERT_TRUE(font_collection->getFontName(i, j, font_name.put()));
                const auto weight = font_collection->getFontWeight(i, j);
                const auto style = font_collection->getFontStyle(i, j);
                const auto width = font_collection->getFontWidth(i, j);
                Logger::info(
                    "    [{}] '{}' (weight: {}, style: {}, width: {})"sv,
                    j, font_name->view(),
                    to_string_view(weight), to_string_view(style), to_string_view(width)
                );
            }
        }
    }
}

TEST(FontCollection, system) {
    setupLogger();
    using namespace core;

    SmartReference<IFontCollection> font_collection;
    ASSERT_TRUE(IFontCollection::createFromSystem(font_collection.put()));

    printFontCollectionInfo(font_collection.get());
}

TEST(FontCollection, custom) {
    setupLogger();
    using namespace core;

    SmartReference<IFontCollection> font_collection;
    ASSERT_TRUE(IFontCollection::create(font_collection.put()));

    EXPECT_FALSE(font_collection->addFile("not_a_file"sv));

    for (auto it : std::filesystem::directory_iterator("assets/font/"sv)) {
        const auto path = it.path().lexically_normal().generic_u8string();
        ASSERT_TRUE(font_collection->addFile(utf8::to_string(path)));
    }

    ASSERT_TRUE(font_collection->build());

    printFontCollectionInfo(font_collection.get());
}
