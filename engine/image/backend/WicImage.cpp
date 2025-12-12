#include "backend/WicImage.hpp"
#include "core/SmartReference.hpp"
#include <wincodec.h>

namespace {
    class WICBitmapLockImpl : public IWICBitmapLock {
	public:
		// IUnknown

		HRESULT STDMETHODCALLTYPE QueryInterface(
			/* [in] */ REFIID riid,
			/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject
		) override {
			if (ppvObject == nullptr) {
				return E_INVALIDARG;
			}
			*ppvObject = nullptr;
			if (riid == __uuidof(IUnknown)) {
                AddRef();
				*ppvObject = (void*)(IUnknown*)(this);
				return S_OK;
			}
			if (riid == __uuidof(IWICBitmapLock)) {
                AddRef();
				*ppvObject = (void*)(IWICBitmapLock*)(this);
				return S_OK;
			}
			return E_NOINTERFACE;
		}

		ULONG STDMETHODCALLTYPE AddRef() override {
            m_ref += 1;
            return m_ref;
        }

		ULONG STDMETHODCALLTYPE Release() override {
			m_ref -= 1;
			const auto ret = m_ref;
			if (ret == 0) {
				delete this;
			}
			return ret;
		}

		// IWICBitmapLock

		HRESULT STDMETHODCALLTYPE GetSize(
			/* [out] */ __RPC__out UINT* puiWidth,
			/* [out] */ __RPC__out UINT* puiHeight
		) override {
			if (puiWidth == nullptr || puiHeight == nullptr) {
				return E_INVALIDARG;
			}
			*puiWidth = static_cast<UINT>(m_rect.Width);
			*puiHeight = static_cast<UINT>(m_rect.Height);
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE GetStride(
			/* [out] */ __RPC__out UINT* pcbStride
		) override {
			if (pcbStride == nullptr) {
				return E_INVALIDARG;
			}
			*pcbStride = m_buffer.stride;
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE GetDataPointer(
			/* [out] */ __RPC__out UINT* pcbBufferSize,
			/* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*pcbBufferSize) WICInProcPointer* ppbData
		) override {
			if (pcbBufferSize == nullptr || ppbData == nullptr) {
				return E_INVALIDARG;
			}
			const auto pixels = static_cast<uint8_t*>(m_buffer.data);
			const auto sub = pixels + static_cast<size_t>(m_rect.Y) * m_buffer.stride + static_cast<size_t>(m_rect.X) * sizeof(uint32_t); // B8G8R8A8
			*pcbBufferSize = m_buffer.size; // oops
			*ppbData = sub;
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE GetPixelFormat(
			/* [out] */ __RPC__out WICPixelFormatGUID* pPixelFormat
		) override {
			if (pPixelFormat == nullptr) {
				return E_INVALIDARG;
			}
			*pPixelFormat = GUID_WICPixelFormat32bppPBGRA; // B8G8R8A8
			return S_OK;
		}

        // WICBitmapLockImpl

        WICBitmapLockImpl(core::IImage* const image, core::ImageMappedBuffer const& buffer, WICRect const& rect) noexcept
            : m_image(image), m_buffer(buffer), m_rect(rect) {
        }

    private:
		core::SmartReference<core::IImage> m_image;
		core::ImageMappedBuffer m_buffer{};
		WICRect m_rect{};
		ULONG m_ref{ 1 };
	};

	class WICBitmapImpl : public IWICBitmap {
	public:

		// IUnknown

		HRESULT STDMETHODCALLTYPE QueryInterface(
			/* [in] */ REFIID riid,
			/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject
		) override {
			if (ppvObject == nullptr) {
				return E_INVALIDARG;
			}
			*ppvObject = nullptr;
			if (riid == __uuidof(IUnknown)) {
                AddRef();
				*ppvObject = (void*)(IUnknown*)(this);
				return S_OK;
			}
			if (riid == __uuidof(IWICBitmapSource)) {
                AddRef();
				*ppvObject = (void*)(IWICBitmapSource*)(this);
				return S_OK;
			}
			if (riid == __uuidof(IWICBitmap)) {
                AddRef();
				*ppvObject = (void*)(IWICBitmap*)(this);
				return S_OK;
			}
			return E_NOINTERFACE;
		}

		ULONG STDMETHODCALLTYPE AddRef() override {
            m_ref += 1;
            return m_ref;
        }

		ULONG STDMETHODCALLTYPE Release() override {
			m_ref -= 1;
			const auto ret = m_ref;
			if (ret == 0) {
				delete this;
			}
			return ret;
		}

		// IWICBitmapSource

		HRESULT STDMETHODCALLTYPE GetSize(
			/* [out] */ __RPC__out UINT* puiWidth,
			/* [out] */ __RPC__out UINT* puiHeight
		) override {
			if (puiWidth == nullptr || puiHeight == nullptr) {
				return E_INVALIDARG;
			}
			const auto size = m_image->getSize();
			*puiWidth = size.x;
			*puiHeight = size.y;
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE GetPixelFormat(
			/* [out] */ __RPC__out WICPixelFormatGUID* pPixelFormat
		) override {
			if (pPixelFormat == nullptr) {
				return E_INVALIDARG;
			}
			*pPixelFormat = GUID_WICPixelFormat32bppPBGRA;
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE GetResolution(
			/* [out] */ __RPC__out double* pDpiX,
			/* [out] */ __RPC__out double* pDpiY
		) override {
			if (pDpiX == nullptr || pDpiY == nullptr) {
				return E_INVALIDARG;
			}
			*pDpiX = m_dpi_x;
			*pDpiY = m_dpi_y;
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE CopyPalette(
			/* [in] */ __RPC__in_opt IWICPalette* pIPalette
		) override {
			UNREFERENCED_PARAMETER(pIPalette);
			return E_NOTIMPL;
		}

		HRESULT STDMETHODCALLTYPE CopyPixels(
			/* [unique][in] */ __RPC__in_opt const WICRect* prc,
			/* [in] */ UINT cbStride,
			/* [in] */ UINT cbBufferSize,
			/* [size_is][out] */ __RPC__out_ecount_full(cbBufferSize) BYTE* pbBuffer
		) override {
			UNREFERENCED_PARAMETER(cbBufferSize);
			const auto size = m_image->getSize();
			WICRect rc{ 0, 0, static_cast<INT>(size.x), static_cast<INT>(size.y) };
			if (prc != nullptr) {
				if (
					prc->X < 0
					|| prc->X > static_cast<INT>(size.x)
					|| prc->Y < 0
					|| prc->Y > static_cast<INT>(size.y)
					|| prc->Width < 0
					|| prc->Width > (static_cast<INT>(size.x) - prc->X)
					|| prc->Height < 0
					|| prc->Height > (static_cast<INT>(size.y) - prc->Y)
					) {
					return E_INVALIDARG;
				}
				rc = *prc;
			}
			auto src = static_cast<uint8_t*>(m_buffer.data) + static_cast<size_t>(rc.Y) * m_buffer.stride;
			const auto x_offset = static_cast<size_t>(rc.X) * sizeof(uint32_t);
			const auto line_size = static_cast<size_t>(rc.Width) * sizeof(uint32_t);
			uint8_t* dst = pbBuffer;
			for (INT y = 0; y < rc.Height; y += 1) {
				std::memcpy(dst, src + x_offset, line_size);
				src += m_buffer.stride;
				dst += cbStride;
			}
			return S_OK;
		}

		// IWICBitmap

		HRESULT STDMETHODCALLTYPE Lock(
			/* [unique][in] */ __RPC__in_opt const WICRect* prcLock,
			/* [in] */ DWORD flags,
			/* [out] */ __RPC__deref_out_opt IWICBitmapLock** ppILock
		) override {
			if (ppILock == nullptr) {
				return E_INVALIDARG;
			}
			const auto size = m_image->getSize();
			WICRect rc{ 0, 0, static_cast<INT>(size.x), static_cast<INT>(size.y) };
			if (prcLock != nullptr) {
				if (
					prcLock->X < 0
					|| prcLock->X > static_cast<INT>(size.x)
					|| prcLock->Y < 0
					|| prcLock->Y > static_cast<INT>(size.y)
					|| prcLock->Width < 0
					|| prcLock->Width >(static_cast<INT>(size.x) - prcLock->X)
					|| prcLock->Height < 0
					|| prcLock->Height >(static_cast<INT>(size.y) - prcLock->Y)
					) {
					return E_INVALIDARG;
				}
				rc = *prcLock;
			}
			if ((flags & (~(WICBitmapLockRead | WICBitmapLockWrite))) != 0) {
				return E_INVALIDARG;
			}
			*ppILock = new WICBitmapLockImpl(m_image.get(), m_buffer, rc);
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE SetPalette(
			/* [in] */ __RPC__in_opt IWICPalette* pIPalette
		) override {
			UNREFERENCED_PARAMETER(pIPalette);
			return E_NOTIMPL;
		}

		HRESULT STDMETHODCALLTYPE SetResolution(
			/* [in] */ double dpiX,
			/* [in] */ double dpiY
		) override {
			m_dpi_x = dpiX;
			m_dpi_y = dpiY;
			return S_OK;
		}

        // WICBitmapImpl

        explicit WICBitmapImpl(core::IImage* const image) noexcept : m_image(image) {
            std::ignore = m_image->map(m_buffer);
        }

        WICBitmapImpl(core::IImage* const image, core::ImageMappedBuffer const& buffer) noexcept
            : m_image(image), m_buffer(buffer) {
        }

    private:
		core::SmartReference<core::IImage> m_image;
		core::ImageMappedBuffer m_buffer{};
		double m_dpi_x{ USER_DEFAULT_SCREEN_DPI };
		double m_dpi_y{ USER_DEFAULT_SCREEN_DPI };
		ULONG m_ref{ 1 };
	};
}

namespace core {
    bool WicImage::createFromImage(IImage* const image, void** const output) {
        if (image == nullptr || output == nullptr) {
            return false;
        }
        ImageMappedBuffer buffer{};
        if (!image->map(buffer)) {
            return false;
        }
        *output = new WICBitmapImpl(image, buffer);
        return true;
    }
	bool WicImage::createFromImage(IImage* const image, ImageMappedBuffer* const buffer, void** const output) {
        if (image == nullptr || buffer == nullptr || output == nullptr) {
            return false;
        }
        *output = new WICBitmapImpl(image, *buffer);
        return true;
    }
}
