#include "core/Logger.hpp"
#include "core/SmartReference.hpp"
#include "core/FileSystem.hpp"
#include "backend/QoiImageFactory.hpp"
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
#include "backend/WicImageFactory.hpp"
#endif

namespace {
    using std::string_view_literals::operator ""sv;

    constexpr auto log_header_file{ "[core] [ImageFactory::createFromFile]"sv };
    constexpr auto log_header_memory{ "[core] [ImageFactory::createFromMemory]"sv };
    constexpr auto log_header_data{ "[core] [ImageFactory::createFromData]"sv };
    constexpr auto invalid_parameter_header{ "invalid parameter:"sv };
}

namespace core {
    bool ImageFactory::createFromFile(const StringView path, IImage** const output_image) {
        if (output_image == nullptr) {
            Logger::error("{} {} output_image is null pointer"sv, log_header_file, invalid_parameter_header);
            return false;
        }
        SmartReference<IData> data;
        if (!FileSystemManager::readFile(path, data.put())) {
            Logger::error("{} read file '{}' failed"sv, log_header_file, path);
            return false;
        }
        return createFromData(data.get(), output_image);
    }
    bool ImageFactory::createFromMemory(const void* const data, const uint32_t size_in_bytes, IImage** const output_image) {
        if (data == nullptr) {
            Logger::error("{} {} data is null pointer"sv, log_header_memory, invalid_parameter_header);
            return false;
        }
        if (size_in_bytes == 0) {
            Logger::error("{} {} size_in_bytes is 0"sv, log_header_memory, invalid_parameter_header);
            return false;
        }
        if (output_image == nullptr) {
            Logger::error("{} {} output_image is null pointer"sv, log_header_memory, invalid_parameter_header);
            return false;
        }
        
        if (QoiImageFactory::createFromMemory(data, size_in_bytes, output_image)) {
            return true;
        }

    #ifdef LUASTG_IMAGE_PNG_ENABLE
        if (PngImageFactory::createFromMemory(data, size_in_bytes, output_image)) {
            return true;
        }
    #endif

    #ifdef LUASTG_IMAGE_WEBP_ENABLE
        if (WebpImageFactory::createFromMemory(data, size_in_bytes, output_image)) {
            return true;
        }
    #endif 

    #ifdef LUASTG_IMAGE_JPEG_ENABLE
        if (JpegImageFactory::createFromMemory(data, size_in_bytes, output_image)) {
            return true;
        }
    #endif

    #ifdef LUASTG_IMAGE_STB_ENABLE
        if (StbImageFactory::createFromMemory(data, size_in_bytes, output_image)) {
            return true;
        }
    #endif

    #ifdef LUASTG_IMAGE_WINDOWS_IMAGING_COMPONENT_ENABLE
        if (WicImageFactory::createFromMemory(data, size_in_bytes, output_image)) {
            return true;
        }
    #endif

        return false;
    }
    bool ImageFactory::createFromData(IData* const data, IImage** const output_image) {
        if (data == nullptr) {
            Logger::error("{} {} data is null pointer"sv, log_header_data, invalid_parameter_header);
            return false;
        }
        if (output_image == nullptr) {
            Logger::error("{} {} output_image is null pointer"sv, log_header_data, invalid_parameter_header);
            return false;
        }
        return createFromMemory(data->data(), static_cast<uint32_t>(data->size()), output_image);
    }
}
