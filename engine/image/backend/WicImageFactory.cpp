#ifdef LUASTG_IMAGE_WINDOWS_IMAGING_COMPONENT_ENABLE

#include "backend/WicImageFactory.hpp"
#include "core/SmartReference.hpp"
#include "backend/Image.hpp"
#include <cassert>
#include <wincodec.h>
#include "win32/base.hpp"

// FUCK MICROSOFT
#ifdef CLSID_WICImagingFactory
#undef CLSID_WICImagingFactory
#endif

namespace {
    using std::string_view_literals::operator ""sv;

    class ScopeCoInitialize {
    public:
        ScopeCoInitialize() : m_result(CoInitializeEx(nullptr, COINIT_MULTITHREADED)) {}
        ~ScopeCoInitialize() { if (m_result == S_OK) CoUninitialize(); }

        [[nodiscard]] explicit operator bool() const noexcept { return SUCCEEDED(m_result); }

    private:
        HRESULT m_result{};
    };
}

namespace core {
    bool WicImageFactory::createFromMemory(LoggingBuffer& log, const void* const data, const uint32_t size_in_bytes, IImage** const output_image) {
        thread_local ScopeCoInitialize init;
        if (!init) {
            return false;
        }

        thread_local win32::com_ptr<IWICImagingFactory> wic_factory;
        if (!wic_factory) {
            if (!win32::check_hresult_as_boolean(
                win32::create_instance(CLSID_WICImagingFactory, CLSCTX_INPROC_SERVER, wic_factory.put()),
                "CoCreateInstance WICImagingFactory"sv
            )) {
                return false;
            }
        }

        win32::com_ptr<IWICStream> wic_stream;
        if (!win32::check_hresult_as_boolean(
            wic_factory->CreateStream(wic_stream.put()),
            "IWICImagingFactory::CreateStream"sv
        )) {
            return false;
        }
        if (!win32::check_hresult_as_boolean(
            wic_stream->InitializeFromMemory(const_cast<uint8_t*>(static_cast<const uint8_t*>(data)), size_in_bytes),
            "IWICStream::InitializeFromMemory"sv
        )) {
            return false;
        }

        win32::com_ptr<IWICBitmapDecoder> wic_bitmap_decoder;
        if (!win32::check_hresult_as_boolean(
            wic_factory->CreateDecoderFromStream(wic_stream.get(), nullptr, WICDecodeMetadataCacheOnDemand, wic_bitmap_decoder.put()),
            "IWICImagingFactory::CreateDecoderFromStream"sv
        )) {
            return false;
        }

        GUID container_format{};
        if (!win32::check_hresult_as_boolean(
            wic_bitmap_decoder->GetContainerFormat(&container_format),
            "IWICBitmapDecoder::GetContainerFormat"sv
        )) {
            return false;
        }
        if (container_format != GUID_ContainerFormatJpeg && container_format != GUID_ContainerFormatPng && container_format != GUID_ContainerFormatBmp) {
            return false; // This is intentional.
        }

        UINT frame_count{};
        if (!win32::check_hresult_as_boolean(
            wic_bitmap_decoder->GetFrameCount(&frame_count),
            "IWICBitmapDecoder::GetFrameCount"sv
        )) {
            return false;
        }
        if (frame_count == 0) {
            return false;
        }

        win32::com_ptr<IWICBitmapFrameDecode> wic_bitmap_frame;
        if (!win32::check_hresult_as_boolean(
            wic_bitmap_decoder->GetFrame(0, wic_bitmap_frame.put()),
            "IWICBitmapDecoder::GetFrame"sv
        )) {
            return false;
        }

        UINT width{}, height{};
        if (!win32::check_hresult_as_boolean(
            wic_bitmap_frame->GetSize(&width, &height),
            "IWICBitmapSource::GetSize"sv
        )) {
            return false;
        }

        WICPixelFormatGUID pixel_format{};
        if (!win32::check_hresult_as_boolean(
            wic_bitmap_frame->GetPixelFormat(&pixel_format),
            "IWICBitmapSource::GetPixelFormat"sv
        )) {
            return false;
        }

        ImageFormat image_format{};
        bool need_convert{};
        if (pixel_format == GUID_WICPixelFormat32bppRGBA) {
            image_format = ImageFormat::b8g8r8a8_normalized;
        }
        else if (pixel_format == GUID_WICPixelFormat32bppBGRA) {
            image_format = ImageFormat::b8g8r8a8_normalized;
        }
        else {
            image_format = ImageFormat::b8g8r8a8_normalized;
            need_convert = true;
        }

        win32::com_ptr<IWICFormatConverter> wic_format_converter;
        if (need_convert) {
            if (!win32::check_hresult_as_boolean(
                wic_factory->CreateFormatConverter(wic_format_converter.put()),
                "IWICImagingFactory::CreateFormatConverter"sv
            )) {
                return false;
            }

            BOOL can_convert{};
            if (!win32::check_hresult_as_boolean(
                wic_format_converter->CanConvert(pixel_format, GUID_WICPixelFormat32bppBGRA, &can_convert),
                "IWICFormatConverter::CanConvert"sv
            )) {
                return false;
            }
            if (!can_convert) {
                return false;
            }

            if (!win32::check_hresult_as_boolean(
                wic_format_converter->Initialize(
                    wic_bitmap_frame.get(),
                    GUID_WICPixelFormat32bppBGRA,
                    WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom
                ),
                "IWICFormatConverter::Initialize"sv
            )) {
                return false;
            }
        }

        ImageDescription description;
        description.size.x = width;
        description.size.y = height;
        description.format = ImageFormat::r8g8b8a8_normalized;
        description.color_space = ImageColorSpace::srgb_gamma_2_2;

        SmartReference<Image> image;
        image.attach(new Image());
        if (!image->initialize(description)) {
            assert(false);
            return false;
        }

        const auto wic_bitmap_source = wic_format_converter
            ? static_cast<IWICBitmapSource*>(wic_format_converter.get())
            : static_cast<IWICBitmapSource*>(wic_bitmap_frame.get());

        ScopedImageMappedBuffer buffer{};
        if (!image->createScopedMap(buffer)) {
            assert(false);
            return false;
        }

        const WICRect rect{ 0, 0, static_cast<INT>(width), static_cast<INT>(height) };
        if (!win32::check_hresult_as_boolean(
            wic_bitmap_source->CopyPixels(
                &rect,
                buffer.stride,
                buffer.size,
                static_cast<uint8_t*>(buffer.data)
            ),
            "IWICBitmapSource::CopyPixels"sv
        )) {
            return false;
        }

        *output_image = image.detach();
        return true;
    }
}

#endif // LUASTG_IMAGE_WINDOWS_IMAGING_COMPONENT_ENABLE
