#include "Core/Graphics/Direct3D11/Texture2D.hpp"
#include "Core/Graphics/Direct3D11/Device.hpp"
#include "core/FileSystem.hpp"
#include "Core/i18n.hpp"
#include "utf8.hpp"
#include "WICTextureLoader11.h"
#include "DDSTextureLoader11.h"
#include "QOITextureLoader11.h"
#include "ScreenGrab11.h"

// Texture2D
namespace core::Graphics::Direct3D11 {
	void Texture2D::onDeviceCreate() {
		if (m_initialized) {
			createResource();
		}
	}
	void Texture2D::onDeviceDestroy() {
		m_texture.Reset();
		m_view.Reset();
	}

	bool Texture2D::setSize(Vector2U const size) {
		if (!m_dynamic) {
			spdlog::error("[core] 不能修改静态纹理的大小");
			assert(false); return false;
		}
		if (!m_is_render_target) {
			spdlog::error("[core] 此纹理由 RenderTarget 托管，禁止直接 setSize");
			assert(false); return false;
		}
		onDeviceDestroy();
		m_size = size;
		return createResource();
	}
	bool Texture2D::uploadPixelData(RectU const rc, void const* const data, uint32_t const pitch) {
		if (!m_dynamic) {
			spdlog::error("[core] 不能修改静态纹理的内容");
			assert(false); return false;
		}
		auto const ctx = m_device->GetD3D11DeviceContext();
		assert(ctx);
		assert(m_texture);
		if (!ctx || !m_texture) {
			return false;
		}
		D3D11_BOX const box = {
			.left = rc.a.x,
			.top = rc.a.y,
			.front = 0,
			.right = rc.b.x,
			.bottom = rc.b.y,
			.back = 1,
		};
		ctx->UpdateSubresource(m_texture.Get(), 0, &box, data, pitch, 0);
		return true;
	}
	bool Texture2D::saveToFile(StringView const path) {
		std::wstring const wide_path(utf8::to_wstring(path));
		HRESULT hr = S_OK;
		hr = gHR = DirectX::SaveWICTextureToFile(
			m_device->GetD3D11DeviceContext(),
			m_texture.Get(),
			GUID_ContainerFormatJpeg,
			wide_path.c_str(),
			&GUID_WICPixelFormat24bppBGR);
		return SUCCEEDED(hr);
	}

	Texture2D::Texture2D() = default;
	Texture2D::~Texture2D() {
		if (m_initialized && m_device) {
			m_device->removeEventListener(this);
		}
	}

	bool Texture2D::initialize(Device* const device, StringView const path, bool const mipmap) {
		assert(device);
		assert(!path.empty());
		m_device = device;
		m_source_path = path;
		m_mipmap = mipmap;
		if (!createResource()) {
			return false;
		}
		m_initialized = true;
		m_device->addEventListener(this);
		return true;
	}
	bool Texture2D::initialize(Device* const device, Vector2U const size, bool const is_render_target) {
		assert(device);
		assert(size.x > 0 && size.y > 0);
		m_device = device;
		m_size = size;
		m_dynamic = true;
		m_pre_mul_alpha = is_render_target;  // 默认是预乘 alpha 的
		m_is_render_target = is_render_target;
		if (!createResource()) {
			return false;
		}
		m_initialized = true;
		if (!is_render_target) {
			// 由 RenderTarget 托管时，不注册监听器
			// 普通的动态纹理需要注册监听器
			m_device->addEventListener(this);
		}
		return true;
	}
	bool Texture2D::createResource() {
		HRESULT hr = S_OK;

		auto* d3d11_device = m_device->GetD3D11Device();
		auto* d3d11_devctx = m_device->GetD3D11DeviceContext();
		if (!d3d11_device || !d3d11_devctx)
			return false;

		if (m_data) {
			D3D11_TEXTURE2D_DESC tex2d_desc = {
				.Width = m_size.x,
				.Height = m_size.y,
				.MipLevels = 1,
				.ArraySize = 1,
				.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
				.SampleDesc = {
					.Count = 1,
					.Quality = 0,
				},
				.Usage = D3D11_USAGE_DEFAULT,
				.BindFlags = D3D11_BIND_SHADER_RESOURCE,
				.CPUAccessFlags = 0,
				.MiscFlags = 0,
			};
			D3D11_SUBRESOURCE_DATA subres_data = {
				.pSysMem = m_data->data(),
				.SysMemPitch = 4 * m_size.x, // BGRA
				.SysMemSlicePitch = 4 * m_size.x * m_size.y,
			};
			hr = gHR = d3d11_device->CreateTexture2D(&tex2d_desc, &subres_data, &m_texture);
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("ID3D11Device::CreateTexture2D");
				return false;
			}
			M_D3D_SET_DEBUG_NAME(m_texture.Get(), "Texture2D_D3D11::d3d11_texture2d");

			D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {
				.Format = tex2d_desc.Format,
				.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
				.Texture2D = {
					.MostDetailedMip = 0,
					.MipLevels = 1,
				},
			};
			hr = gHR = d3d11_device->CreateShaderResourceView(m_texture.Get(), &view_desc, &m_view);
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("ID3D11Device::CreateShaderResourceView");
				return false;
			}
			M_D3D_SET_DEBUG_NAME(m_view.Get(), "Texture2D_D3D11::d3d11_srv");
		}
		else if (!m_source_path.empty()) {
			SmartReference<IData> src;
			if (!FileSystemManager::readFile(m_source_path, src.put())) {
				spdlog::error("[core] 无法加载文件 '{}'", m_source_path);
				return false;
			}

			// 加载图片
			Microsoft::WRL::ComPtr<ID3D11Resource> res;
			// 先尝试以 DDS 格式加载
			DirectX::DDS_ALPHA_MODE dds_alpha_mode = DirectX::DDS_ALPHA_MODE_UNKNOWN;
			HRESULT const hr1 = DirectX::CreateDDSTextureFromMemoryEx(
				d3d11_device, m_mipmap ? d3d11_devctx : nullptr,
				static_cast<uint8_t const*>(src->data()), src->size(),
				0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
				DirectX::DDS_LOADER_IGNORE_SRGB, // TODO: 这里也同样忽略了 sRGB，看以后渲染管线颜色空间怎么改
				&res, &m_view,
				&dds_alpha_mode);
			if (FAILED(hr1)) {
				// 尝试以普通图片格式加载
				HRESULT const hr2 = DirectX::CreateWICTextureFromMemoryEx(
					d3d11_device, m_mipmap ? d3d11_devctx : nullptr,
					static_cast<uint8_t const*>(src->data()), src->size(),
					0,
					D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
					DirectX::WIC_LOADER_DEFAULT | DirectX::WIC_LOADER_IGNORE_SRGB,
					// TODO: 渲染管线目前是在 sRGB 下计算的，也就是心理视觉色彩，将错就错吧……
					//DirectX::WIC_LOADER_DEFAULT | DirectX::WIC_LOADER_SRGB_DEFAULT,
					&res, &m_view);
				if (FAILED(hr2)) {
					// 尝试以 QOI 图片格式加载
					HRESULT const hr3 = DirectX::CreateQOITextureFromMemoryEx(
						d3d11_device, m_mipmap ? d3d11_devctx : nullptr, m_device->GetWICImagingFactory(),
						static_cast<uint8_t const*>(src->data()), src->size(),
						0,
						D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
						DirectX::QOI_LOADER_DEFAULT | DirectX::QOI_LOADER_IGNORE_SRGB,
						// TODO: 渲染管线目前是在 sRGB 下计算的，也就是心理视觉色彩，将错就错吧……
						//DirectX::QOI_LOADER_DEFAULT | DirectX::QOI_LOADER_SRGB_DEFAULT,
						&res, &m_view);
					if (FAILED(hr3)) {
						// 在这里一起报告，不然 log 文件里遍地都是 error
						gHR = hr1;
						i18n_core_system_call_report_error("DirectX::CreateDDSTextureFromMemoryEx");
						gHR = hr2;
						i18n_core_system_call_report_error("DirectX::CreateWICTextureFromMemoryEx");
						gHR = hr3;
						i18n_core_system_call_report_error("DirectX::CreateQOITextureFromMemoryEx");
						return false;
					}
				}
			}
			if (dds_alpha_mode == DirectX::DDS_ALPHA_MODE_PREMULTIPLIED) {
				m_pre_mul_alpha = true; // 您小子预乘了 alpha 通道是吧，行
			}
			M_D3D_SET_DEBUG_NAME(m_view.Get(), "Texture2D_D3D11::d3d11_srv");

			// 转换类型
			hr = gHR = res.As(&m_texture);
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("ID3D11Resource::QueryInterface -> ID3D11Texture2D");
				return false;
			}
			M_D3D_SET_DEBUG_NAME(m_texture.Get(), "Texture2D_D3D11::d3d11_texture2d");

			// 获取图片尺寸
			D3D11_TEXTURE2D_DESC texture_info{};
			m_texture->GetDesc(&texture_info);
			m_size.x = texture_info.Width;
			m_size.y = texture_info.Height;
		}
		else {
			D3D11_TEXTURE2D_DESC texdef = {
				.Width = m_size.x,
				.Height = m_size.y,
				.MipLevels = 1,
				.ArraySize = 1,
				.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
				.SampleDesc = DXGI_SAMPLE_DESC{.Count = 1,.Quality = 0},
				.Usage = D3D11_USAGE_DEFAULT,
				.BindFlags = D3D11_BIND_SHADER_RESOURCE | (m_is_render_target ? D3D11_BIND_RENDER_TARGET : 0u),
				.CPUAccessFlags = 0,
				.MiscFlags = 0,
			};
			hr = gHR = d3d11_device->CreateTexture2D(&texdef, nullptr, &m_texture);
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("ID3D11Device::CreateTexture2D");
				return false;
			}
			M_D3D_SET_DEBUG_NAME(m_texture.Get(), "Texture2D_D3D11::d3d11_texture2d");

			D3D11_SHADER_RESOURCE_VIEW_DESC viewdef = {
				.Format = texdef.Format,
				.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
				.Texture2D = D3D11_TEX2D_SRV{.MostDetailedMip = 0,.MipLevels = 1,},
			};
			hr = gHR = d3d11_device->CreateShaderResourceView(m_texture.Get(), &viewdef, &m_view);
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("ID3D11Device::CreateShaderResourceView");
				return false;
			}
			M_D3D_SET_DEBUG_NAME(m_view.Get(), "Texture2D_D3D11::d3d11_srv");
		}

		return true;
	}
}
namespace core::Graphics::Direct3D11 {
	bool Device::createTextureFromFile(StringView const path, bool const mipmap, ITexture2D** const pp_texture) {
		*pp_texture = nullptr;
		SmartReference<Texture2D> buffer;
		buffer.attach(new Texture2D);
		if (!buffer->initialize(this, path, mipmap)) {
			return false;
		}
		*pp_texture = buffer.detach();
		return true;
	}
	//bool createTextureFromMemory(void const* data, size_t size, bool mipmap, ITexture2D** pp_texture);
	bool Device::createTexture(Vector2U const size, ITexture2D** const pp_texture) {
		*pp_texture = nullptr;
		SmartReference<Texture2D> buffer;
		buffer.attach(new Texture2D);
		if (!buffer->initialize(this, size, false)) {
			return false;
		}
		*pp_texture = buffer.detach();
		return true;
	}
}