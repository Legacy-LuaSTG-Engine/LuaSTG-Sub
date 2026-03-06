#include "core/Logger.hpp"
#include "core/SmartReference.hpp"
#include "core/FileSystem.hpp"
#include "core/Image.hpp"
#ifdef LUASTG_IMAGE_JPEG_ENABLE
#include "backend/JpegImageFactory.hpp"
#endif
#ifdef LUASTG_IMAGE_PNG_ENABLE
#include "backend/PngImageFactory.hpp"
#endif
#ifdef LUASTG_IMAGE_WEBP_ENABLE
#include "backend/WebpImageFactory.hpp"
#endif
#ifdef LUASTG_IMAGE_STB_ENABLE
#include "backend/StbImageFactory.hpp"
#endif
#ifdef LUASTG_IMAGE_WINDOWS_IMAGING_COMPONENT_ENABLE
#include "win32/base.hpp"
#include "backend/WicImageFactory.hpp"
#endif
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "gtest/gtest.h"
#include <DirectXPackedVector.h>

namespace {
    using std::string_literals::operator ""s;
    using std::string_view_literals::operator ""sv;

#ifdef LUASTG_IMAGE_WINDOWS_IMAGING_COMPONENT_ENABLE
    void win32LoggerWriter(const std::string_view message) {
        core::Logger::error(message);
    }
#endif

    void setupLogger() {
        if (!spdlog::get("test"s)) {
            spdlog::set_default_logger(spdlog::stdout_color_mt("test"s));
        }
    #ifdef LUASTG_IMAGE_WINDOWS_IMAGING_COMPONENT_ENABLE
        win32::set_logger_writer(&win32LoggerWriter);
    #endif
    }
}

TEST(Image, create) {
    setupLogger();
    using namespace core;

    ImageDescription description;
    description.size.x = 1;
    description.size.y = 1;
    description.format = ImageFormat::r8g8b8a8_normalized;
    description.color_space = ImageColorSpace::linear;
    description.alpha_mode = ImageAlphaMode::straight;

    SmartReference<IImage> image;
    ASSERT_TRUE(ImageFactory::create(description, image.put()));
}

TEST(Image, create_with_nullptr) {
    setupLogger();
    using namespace core;

    ImageDescription description;
    description.size.x = 1;
    description.size.y = 1;
    description.format = ImageFormat::r8g8b8a8_normalized;
    description.color_space = ImageColorSpace::linear;
    description.alpha_mode = ImageAlphaMode::straight;

    ASSERT_FALSE(ImageFactory::create(description, nullptr));
}

TEST(Image, create_with_wrong_size) {
    setupLogger();
    using namespace core;

    ImageDescription description;
    description.format = ImageFormat::r8g8b8a8_normalized;
    description.color_space = ImageColorSpace::linear;
    description.alpha_mode = ImageAlphaMode::straight;

    SmartReference<IImage> image;

    description.size.x = 0;
    description.size.y = 1;
    ASSERT_FALSE(ImageFactory::create(description, image.put()));

    description.size.x = 1;
    description.size.y = 0;
    ASSERT_FALSE(ImageFactory::create(description, image.put()));

    description.size.x = 16385;
    description.size.y = 1;
    ASSERT_FALSE(ImageFactory::create(description, image.put()));

    description.size.x = 1;
    description.size.y = 16385;
    ASSERT_FALSE(ImageFactory::create(description, image.put()));
}

TEST(Image, create_with_wrong_format) {
    setupLogger();
    using namespace core;

    ImageDescription description;
    description.size.x = 1;
    description.size.y = 1;
    description.format = ImageFormat::unknown;
    description.color_space = ImageColorSpace::linear;
    description.alpha_mode = ImageAlphaMode::straight;

    SmartReference<IImage> image;
    ASSERT_FALSE(ImageFactory::create(description, image.put()));
}

TEST(Image, create_with_wrong_color_space) {
    setupLogger();
    using namespace core;

    ImageDescription description;
    description.size.x = 1;
    description.size.y = 1;
    description.color_space = ImageColorSpace::srgb_gamma_2_2;
    description.alpha_mode = ImageAlphaMode::straight;

    SmartReference<IImage> image;

#define I(FMT) static_cast<int32_t>(FMT)
    for (int32_t i = I(ImageFormat::unknown) + 1; i < I(ImageFormat::count); i++) {
        description.format = static_cast<ImageFormat>(i);
        switch (description.format) {
            case ImageFormat::r8g8b8a8_normalized:
            case ImageFormat::b8g8r8a8_normalized: {
                ASSERT_TRUE(ImageFactory::create(description, image.put()));
                break;
            }
            default: {
                ASSERT_FALSE(ImageFactory::create(description, image.put()));
                break;
            }
        }
    }
#undef I
}

TEST(Image, map) {
    setupLogger();
    using namespace core;

    ImageDescription description;
    description.size.x = 1;
    description.size.y = 1;
    description.format = ImageFormat::r8g8b8a8_normalized;
    description.color_space = ImageColorSpace::linear;
    description.alpha_mode = ImageAlphaMode::straight;

    SmartReference<IImage> image;
    ASSERT_TRUE(ImageFactory::create(description, image.put()));

    ImageMappedBuffer buffer{};
    ASSERT_TRUE(image->map(buffer));
    image->unmap();
}

TEST(Image, r8g8b8a8_normalized_map_write_unmap_getPixel) {
    setupLogger();
    using namespace core;

    ImageDescription description;
    description.size.x = 1;
    description.size.y = 1;
    description.format = ImageFormat::r8g8b8a8_normalized;
    description.color_space = ImageColorSpace::linear;
    description.alpha_mode = ImageAlphaMode::straight;

    SmartReference<IImage> image;
    ASSERT_TRUE(ImageFactory::create(description, image.put()));

    DirectX::PackedVector::XMCOLOR bgra(16.0f / 255.0f, 32.0f / 255.0f, 128.0f / 255.0f, 1.0f);
    std::swap(bgra.r, bgra.b); // BGRA to RGBA
    const Vector4F color(16.0f / 255.0f, 32.0f / 255.0f, 128.0f / 255.0f, 1.0f);

    ImageMappedBuffer buffer{};
    ASSERT_TRUE(image->map(buffer));
    std::memcpy(buffer.data, &bgra, sizeof(bgra));
    image->unmap();

    const auto output_color = image->getPixel(0, 0);
    EXPECT_EQ(color, output_color);
}
TEST(Image, b8g8r8a8_normalized_map_write_unmap_getPixel) {
    setupLogger();
    using namespace core;

    ImageDescription description;
    description.size.x = 1;
    description.size.y = 1;
    description.format = ImageFormat::b8g8r8a8_normalized;
    description.color_space = ImageColorSpace::linear;
    description.alpha_mode = ImageAlphaMode::straight;

    SmartReference<IImage> image;
    ASSERT_TRUE(ImageFactory::create(description, image.put()));

    const DirectX::PackedVector::XMCOLOR bgra(16.0f / 255.0f, 32.0f / 255.0f, 128.0f / 255.0f, 1.0f);
    const Vector4F color(16.0f / 255.0f, 32.0f / 255.0f, 128.0f / 255.0f, 1.0f);

    ImageMappedBuffer buffer{};
    ASSERT_TRUE(image->map(buffer));
    std::memcpy(buffer.data, &bgra, sizeof(bgra));
    image->unmap();

    const auto output_color = image->getPixel(0, 0);
    EXPECT_EQ(color, output_color);
}
TEST(Image, r16g16b16a16_float_map_write_unmap_getPixel) {
    setupLogger();
    using namespace core;

    ImageDescription description;
    description.size.x = 1;
    description.size.y = 1;
    description.format = ImageFormat::r16g16b16a16_float;
    description.color_space = ImageColorSpace::linear;
    description.alpha_mode = ImageAlphaMode::straight;

    SmartReference<IImage> image;
    ASSERT_TRUE(ImageFactory::create(description, image.put()));

    constexpr DirectX::XMFLOAT4 float4(1.0f, 128.0f / 255.0f, 0.0f, 1.0f);
    DirectX::PackedVector::XMHALF4 harf4;
    DirectX::PackedVector::XMStoreHalf4(&harf4, DirectX::XMLoadFloat4(&float4));
    DirectX::XMFLOAT4 float4_read_back;
    DirectX::XMStoreFloat4(&float4_read_back, DirectX::PackedVector::XMLoadHalf4(&harf4));
    const Vector4F color(float4_read_back.x, float4_read_back.y, float4_read_back.z, float4_read_back.w);

    ImageMappedBuffer buffer{};
    ASSERT_TRUE(image->map(buffer));
    std::memcpy(buffer.data, &harf4, sizeof(harf4));
    image->unmap();

    EXPECT_EQ(color, image->getPixel(0, 0));
}
TEST(Image, r32g32b32a32_float_map_write_unmap_getPixel) {
    setupLogger();
    using namespace core;

    ImageDescription description;
    description.size.x = 1;
    description.size.y = 1;
    description.format = ImageFormat::r32g32b32a32_float;
    description.color_space = ImageColorSpace::linear;
    description.alpha_mode = ImageAlphaMode::straight;

    SmartReference<IImage> image;
    ASSERT_TRUE(ImageFactory::create(description, image.put()));

    const Vector4F color(1.0f, 128.0f / 255.0f, 0.0f, 1.0f);

    ImageMappedBuffer buffer{};
    ASSERT_TRUE(image->map(buffer));
    std::memcpy(buffer.data, &color, sizeof(color));
    image->unmap();

    EXPECT_EQ(color, image->getPixel(0, 0));
}

TEST(ImageFactory, createFromMemory_png) {
    setupLogger();
    using namespace core;

    SmartReference<IImage> image;
    EXPECT_TRUE(ImageFactory::createFromFile("assets/test_color.png"sv, image.put()));
    const auto size = image->getSize();
    EXPECT_TRUE(size.x == 256u && size.y == 32u);
}
TEST(ImageFactory, createFromMemory_webp) {
    setupLogger();
    using namespace core;

    SmartReference<IImage> image;
    EXPECT_TRUE(ImageFactory::createFromFile("assets/test_color_lossless.webp"sv, image.put()));
    const auto size = image->getSize();
    EXPECT_TRUE(size.x == 256u && size.y == 32u);
}
TEST(ImageFactory, createFromMemory_jpeg) {
    setupLogger();
    using namespace core;

    SmartReference<IImage> image;
    EXPECT_TRUE(ImageFactory::createFromFile("assets/test_color.jpg"sv, image.put()));
    const auto size = image->getSize();
    EXPECT_TRUE(size.x == 256u && size.y == 32u);
}
TEST(ImageFactory, createFromMemory_wtf) {
    setupLogger();
    using namespace core;

    SmartReference<IImage> image;
    EXPECT_FALSE(ImageFactory::createFromFile("assets/not_a_image"sv, image.put()));
}

#ifdef LUASTG_IMAGE_JPEG_ENABLE
TEST(JpegImageFactory, createFromMemory) {
    setupLogger();
    using namespace core;

    SmartReference<IData> data;
    SmartReference<IImage> image;

    static constexpr std::string_view files[]{
        "assets/test_color.jpg"sv,
        "assets/test_text.jpg"sv,
    };

    LoggingBuffer log;
    for (const auto file : files) {
        ASSERT_TRUE(FileSystemManager::readFile(file, data.put()));
        EXPECT_TRUE(JpegImageFactory::createFromMemory(log, data->data(), static_cast<uint32_t>(data->size()), image.put()));
    }
}
#endif // LUASTG_IMAGE_JPEG_ENABLE

#ifdef LUASTG_IMAGE_PNG_ENABLE
TEST(PngImageFactory, createFromMemory) {
    setupLogger();
    using namespace core;

    SmartReference<IData> data;
    SmartReference<IImage> image;

    static constexpr std::string_view files[]{
        "assets/test_color.png"sv,
        "assets/test_text.png"sv,
    };

    LoggingBuffer log;
    for (const auto file : files) {
        ASSERT_TRUE(FileSystemManager::readFile(file, data.put()));
        EXPECT_TRUE(PngImageFactory::createFromMemory(log, data->data(), static_cast<uint32_t>(data->size()), image.put()));
    }
}
#endif // LUASTG_IMAGE_PNG_ENABLE

#ifdef LUASTG_IMAGE_WEBP_ENABLE
TEST(WebpImageFactory, createFromMemory) {
    setupLogger();
    using namespace core;

    SmartReference<IData> data;
    SmartReference<IImage> image;

    static constexpr std::string_view files[]{
        "assets/test_color_hq.webp"sv,
        "assets/test_color_lossless.webp"sv,
        "assets/test_text_hq.webp"sv,
        "assets/test_text_lossless.webp"sv,
    };

    LoggingBuffer log;
    for (const auto file : files) {
        ASSERT_TRUE(FileSystemManager::readFile(file, data.put()));
        EXPECT_TRUE(WebpImageFactory::createFromMemory(log, data->data(), static_cast<uint32_t>(data->size()), image.put()));
    }
}
#endif // LUASTG_IMAGE_WEBP_ENABLE
