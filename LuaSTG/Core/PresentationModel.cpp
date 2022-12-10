#include "Core/PresentationModel.hpp"
#include "framework.hpp"
#include "i18n.hpp"
#include "Core/Object.hpp"
#include "Platform/RuntimeLoader/DirectComposition.hpp"
#include "Platform/RuntimeLoader/DXGI.hpp"
#include "Platform/RuntimeLoader/Direct3D11.hpp"

#ifndef HRNew
#define HRNew HRESULT hr = S_OK;
#define HRGet hr = gHR
#define BHR bHR
#define HRCheckCallReturnBool(x) if (FAILED(hr)) { i18n_core_system_call_report_error(x); return false; }
#define HRCheckCallReport(x) if (FAILED(hr)) { i18n_core_system_call_report_error(x); }
#define LOG_INFO(x, ...)
#define LOG_ERROR(x, ...)
#endif

namespace Core
{
	static DXGI_MATRIX_3X2_F MakeLetterBoxing(UINT box_width, UINT box_height, UINT letter_width, UINT letter_height)
	{
		float const o_w = (float)(letter_width);
		float const o_h = (float)(letter_height);
		float const c_w = (float)(box_width);
		float const c_h = (float)(box_height);

		float const s_w = o_w / c_w;
		float const s_h = o_h / c_h;
		float const s_m = std::min(s_w, s_h);

		float const b_w = c_w * s_m;
		float const b_h = c_h * s_m;
		float const b_x = (o_w - b_w) * 0.5f;
		float const b_y = (o_h - b_h) * 0.5f;

		DXGI_MATRIX_3X2_F const mat = {
			s_m, 0.0f,
			0.0f, s_m,
			b_x, b_y,
		};

		return mat;
	}

	class PresentationModelBase : public IPresentationModel
	{
	protected:
		intptr_t m_reference{ 1 };
		Vector2U window_size{ 1, 1 };
		Vector2U canvas_size{ 1, 1 };
	protected:
		HWND win32_window{};
		Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory;
		Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11_device_context;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3d11_device_context1;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2d1_device_context;
	protected:
		Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgi_swapchain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3d11_swapchain_rendertarget_view;
		BOOL dxgi_swapchain_present_allow_tearing{ FALSE };
	protected:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3d11_shaderresource_view;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3d11_rendertarget_view;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> d3d11_depthstencil_view;
		Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2d1_bitmap;
	protected:
		virtual void DeleteThis() = 0;
		virtual bool CreateSwapChain() = 0;
		virtual void DestroySwapChain() = 0;
		bool CreateDepthStencilBuffer()
		{
			assert(d3d11_device);

			HRNew;

			Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;

			D3D11_TEXTURE2D_DESC texture_2d_info = {};
			texture_2d_info.Width = canvas_size.x;
			texture_2d_info.Height = canvas_size.y;
			texture_2d_info.MipLevels = 1;
			texture_2d_info.ArraySize = 1;
			texture_2d_info.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			texture_2d_info.SampleDesc.Count = 1;
			texture_2d_info.Usage = D3D11_USAGE_DEFAULT;
			texture_2d_info.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			HRGet = d3d11_device->CreateTexture2D(&texture_2d_info, NULL, &texture);
			HRCheckCallReturnBool("ID3D11Device::CreateTexture2D");

			D3D11_DEPTH_STENCIL_VIEW_DESC view_info = {};
			view_info.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			view_info.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			HRGet = d3d11_device->CreateDepthStencilView(texture.Get(), &view_info, &d3d11_depthstencil_view);
			HRCheckCallReturnBool("ID3D11Device::CreateDepthStencilView");

			return true;
		}
		void DestroyDepthStencilBuffer()
		{
			d3d11_depthstencil_view.Reset();
		}
		bool CreateCanvasRenderAttachment()
		{
			assert(d3d11_device);

			HRNew;

			LOG_INFO("Create Canvas RenderAttachment (%ux%u)", canvas_size.x, canvas_size.y);

			// Direct3D 11 RenderTarget

			Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;

			D3D11_TEXTURE2D_DESC texture_2d_info = {};
			texture_2d_info.Width = canvas_size.x;
			texture_2d_info.Height = canvas_size.y;
			texture_2d_info.MipLevels = 1;
			texture_2d_info.ArraySize = 1;
			texture_2d_info.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			texture_2d_info.SampleDesc.Count = 1;
			texture_2d_info.Usage = D3D11_USAGE_DEFAULT;
			texture_2d_info.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			HRGet = d3d11_device->CreateTexture2D(&texture_2d_info, NULL, &texture);
			HRCheckCallReturnBool("ID3D11Device::CreateTexture2D");

			D3D11_RENDER_TARGET_VIEW_DESC render_target_view_info = {};
			render_target_view_info.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			render_target_view_info.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			HRGet = d3d11_device->CreateRenderTargetView(texture.Get(), &render_target_view_info, &d3d11_rendertarget_view);
			HRCheckCallReturnBool("ID3D11Device::CreateRenderTargetView");

			LOG_INFO("Create Canvas Direct3D 11 RenderTarget View");

			D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_info = {};
			shader_resource_view_info.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			shader_resource_view_info.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shader_resource_view_info.Texture2D.MipLevels = 1;
			HRGet = d3d11_device->CreateShaderResourceView(texture.Get(), &shader_resource_view_info, &d3d11_shaderresource_view);
			HRCheckCallReturnBool("ID3D11Device::CreateShaderResourceView");

			LOG_INFO("Create Canvas Direct3D 11 ShaderResource View");

			if (!CreateDepthStencilBuffer()) return false;

			LOG_INFO("Create Canvas Direct3D 11 DepthStencil View");

			// Direct2D Bitmap Target

			if (d2d1_device_context)
			{
				Microsoft::WRL::ComPtr<IDXGISurface> dxgi_surface;
				HRGet = texture.As(&dxgi_surface);
				HRCheckCallReturnBool("ID3D11Texture2D::QueryInterface -> IDXGISurface");

				D2D1_BITMAP_PROPERTIES1 bitmap_info = {};
				bitmap_info.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
				bitmap_info.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
				bitmap_info.dpiX = USER_DEFAULT_SCREEN_DPI;
				bitmap_info.dpiY = USER_DEFAULT_SCREEN_DPI;
				bitmap_info.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;
				HRGet = d2d1_device_context->CreateBitmapFromDxgiSurface(dxgi_surface.Get(), &bitmap_info, &d2d1_bitmap);
				HRCheckCallReturnBool("ID2D1DeviceContext::CreateBitmapFromDxgiSurface");

				LOG_INFO("Create Canvas Direct2D Bitmap Target");
			}

			LOG_INFO("Create Canvas RenderAttachment Successfully");

			return true;
		}
		void DestroyCanvasRenderAttachment()
		{
			DestroyDepthStencilBuffer();
			d3d11_rendertarget_view.Reset();
			d3d11_shaderresource_view.Reset();
			d2d1_bitmap.Reset();
		}
		bool CreateSwapChainRenderAttachment()
		{
			assert(d3d11_device);
			assert(dxgi_swapchain);

			HRNew;

			LOG_INFO("Create RenderAttachment");

			// DXGI Surface

			Microsoft::WRL::ComPtr<IDXGISurface> dxgi_surface;
			HRGet = dxgi_swapchain->GetBuffer(0, IID_PPV_ARGS(&dxgi_surface));
			HRCheckCallReturnBool("IDXGISwapChain1::GetBuffer -> 0 -> IDXGISurface");

			// Direct3D 11 RenderTarget View

			Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture2d;
			HRGet = dxgi_surface.As(&d3d11_texture2d);
			HRCheckCallReturnBool("IDXGISurface::QueryInterface -> ID3D11Texture2D");

			HRGet = d3d11_device->CreateRenderTargetView(d3d11_texture2d.Get(), NULL, &d3d11_swapchain_rendertarget_view);
			HRCheckCallReturnBool("ID3D11Device::CreateRenderTargetView");

			LOG_INFO("Create Direct3D 11 RenderTarget View");

			LOG_INFO("Create RenderAttachment Successfully");

			return true;
		}
		void DestroySwapChainRenderAttachment()
		{
			if (d3d11_device_context)
			{
				d3d11_device_context->ClearState();
				d3d11_device_context->Flush();
			}
			d3d11_swapchain_rendertarget_view.Reset();
		}
		bool CreateBothRenderAttachment()
		{
			assert(d3d11_device);
			assert(dxgi_swapchain);

			HRNew;

			LOG_INFO("Create RenderAttachment");

			// DXGI Surface

			Microsoft::WRL::ComPtr<IDXGISurface> dxgi_surface;
			HRGet = dxgi_swapchain->GetBuffer(0, IID_PPV_ARGS(&dxgi_surface));
			HRCheckCallReturnBool("IDXGISwapChain1::GetBuffer -> 0 -> IDXGISurface");

			// Direct3D 11 RenderTarget View

			Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture2d;
			HRGet = dxgi_surface.As(&d3d11_texture2d);
			HRCheckCallReturnBool("IDXGISurface::QueryInterface -> ID3D11Texture2D");

			HRGet = d3d11_device->CreateRenderTargetView(d3d11_texture2d.Get(), NULL, &d3d11_swapchain_rendertarget_view);
			HRCheckCallReturnBool("ID3D11Device::CreateRenderTargetView");

			d3d11_rendertarget_view = d3d11_swapchain_rendertarget_view;

			LOG_INFO("Create Direct3D 11 RenderTarget View");

			if (!CreateDepthStencilBuffer()) return false;

			LOG_INFO("Create Direct3D 11 DepthStencil View");

			// Direct2D Bitmap Target

			if (d2d1_device_context)
			{
				D2D1_BITMAP_PROPERTIES1 bitmap_info = {};
				bitmap_info.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
				bitmap_info.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
				bitmap_info.dpiX = USER_DEFAULT_SCREEN_DPI;
				bitmap_info.dpiY = USER_DEFAULT_SCREEN_DPI;
				bitmap_info.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

				HRGet = d2d1_device_context->CreateBitmapFromDxgiSurface(dxgi_surface.Get(), &bitmap_info, &d2d1_bitmap);
				HRCheckCallReturnBool("ID2D1DeviceContext::CreateBitmapFromDxgiSurface");

				LOG_INFO("Create Direct2D Bitmap Target");
			}

			LOG_INFO("Create RenderAttachment Successfully");

			return true;
		}
		void DestroyBothRenderAttachment()
		{
			if (d3d11_device_context)
			{
				d3d11_device_context->ClearState();
				d3d11_device_context->Flush();
			}
			DestroyDepthStencilBuffer();
			d3d11_swapchain_rendertarget_view.Reset();
			d3d11_rendertarget_view.Reset();
			d2d1_bitmap.Reset();
		}
		bool PresentBase(bool vsync)
		{
			assert(dxgi_swapchain);
			assert(d3d11_device_context);

			HRNew;

			UINT interval = vsync ? 1 : 0;
			UINT flags = vsync ? 0 : (dxgi_swapchain_present_allow_tearing ? DXGI_PRESENT_ALLOW_TEARING : 0);
			HRGet = dxgi_swapchain->Present(interval, flags);

			d3d11_device_context->ClearState();
			if (d3d11_device_context1)
			{
				d3d11_device_context1->DiscardView(d3d11_swapchain_rendertarget_view.Get());
			}

			// delay error check
			HRCheckCallReturnBool("IDXGISwapChain1::Present");

			return true;
		}
	public:
		void IncreaseReference() { m_reference += 1; }
		void DecreaseReference() { m_reference -= 1; if (m_reference <= 0) DeleteThis(); };
		intptr_t retain() { m_reference += 1; return m_reference; }
		intptr_t release() { m_reference -= 1; intptr_t const v = m_reference; if (v <= 0) { DeleteThis(); } return v; };
	public:
		bool AttachDevice(HWND window, ID3D11Device* device, ID2D1DeviceContext* renderer)
		{
			assert(window);
			assert(device);

			HRNew;

			win32_window = window;
			d3d11_device = device;
			d3d11_device->GetImmediateContext(&d3d11_device_context);

			HRGet = d3d11_device_context.As(&d3d11_device_context1);
			HRCheckCallReport("ID3D11DeviceContext::QueryInterface -> ID3D11DeviceContext1");

			HRGet = Platform::RuntimeLoader::Direct3D11::GetFactory(d3d11_device.Get(), &dxgi_factory);
			HRCheckCallReturnBool("ID3D11Device::GetParent -> IDXGIFactory2");

			d2d1_device_context = renderer; // might be NULL

			return CreateSwapChain();
		}
		void DetachDevice()
		{
			DestroySwapChain();
			win32_window = NULL;
			dxgi_factory.Reset();
			d3d11_device.Reset();
			d3d11_device_context.Reset();
			d3d11_device_context1.Reset();
			d2d1_device_context.Reset();
		}
		Vector2U GetWindowSize() { return window_size; }
		Vector2U GetCanvasSize() { return canvas_size; }
		ID3D11RenderTargetView* GetCanvasRenderTargetView() { return d3d11_rendertarget_view.Get(); }
		ID3D11DepthStencilView* GetCanvasDepthStencilView() { return d3d11_depthstencil_view.Get(); }
		ID2D1Bitmap1* GetCanvasBitmapTarget() { return d2d1_bitmap.Get(); }
	};

	class LetterBoxingRenderer
	{
	private:
		struct Vertex
		{
			DirectX::XMFLOAT2 pos;
			DirectX::XMFLOAT2 uv;
			inline Vertex() = default;
			inline Vertex(float x, float y, float u, float v) : pos(x, y), uv(u, v) {}
		};
		using Index = uint16_t;
		struct TransformBuffer
		{
			DirectX::XMFLOAT4X4 mvp;
		};
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11_device_context;
	private:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> d3d11_input_layout;
		Vertex vertex_buffer[4];
		Microsoft::WRL::ComPtr<ID3D11Buffer> d3d11_vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> d3d11_index_buffer;
		TransformBuffer transform_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> d3d11_constant_buffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> d3d11_vertex_shader;
		D3D11_VIEWPORT d3d11_viewport{};
		D3D11_RECT d3d11_scissor_rect{};
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> d3d11_rasterizer_state;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> d3d11_sampler_state_linear;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> d3d11_sampler_state_point;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> d3d11_pixel_shader;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> d3d11_depth_stencil_state;
		Microsoft::WRL::ComPtr<ID3D11BlendState> d3d11_blend_state;
	private:
		bool CreateResource()
		{
			assert(d3d11_device);
			assert(d3d11_device_context);

			HRNew;

			/*
				cbuffer TransformBuffer : register(b0)
				{
					float4x4 mvp;
				};

				struct ShaderInput
				{
					float2 pos : POSITION;
					float2 uv  : TEXCOORD;
				};

				struct ShaderOutput
				{
					float4 pos : SV_POSITION;
					float2 uv  : TEXCOORD;
				};

				ShaderOutput main(ShaderInput input)
				{
					ShaderOutput output;
					output.pos = mul(float4(input.pos, 0.0f, 1.0f), mvp);
					output.uv  = input.uv;
					return output;
				}
			*/
			static BYTE vertex_shader[] = {
			#define INS(A, B, C, D) 0x##A, 0x##B, 0x##C, 0x##D
				INS(44, 58, 42, 43),
				INS(3B, 40, D6, E3),
				INS(6A, 1D, 64, A9),
				INS(E5, B4, 45, E8),
				INS(45, 3A, EF, 13),
				INS(01, 00, 00, 00),
				INS(38, 03, 00, 00),
				INS(05, 00, 00, 00),
				INS(34, 00, 00, 00),
				INS(F4, 00, 00, 00),
				INS(48, 01, 00, 00),
				INS(A0, 01, 00, 00),
				INS(BC, 02, 00, 00),
				INS(52, 44, 45, 46),
				INS(B8, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(4C, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(1C, 00, 00, 00),
				INS(00, 04, FE, FF),
				INS(00, 01, 00, 00),
				INS(90, 00, 00, 00),
				INS(3C, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(54, 72, 61, 6E),
				INS(73, 66, 6F, 72),
				INS(6D, 42, 75, 66),
				INS(66, 65, 72, 00),
				INS(3C, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(64, 00, 00, 00),
				INS(40, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(7C, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(40, 00, 00, 00),
				INS(02, 00, 00, 00),
				INS(80, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(6D, 76, 70, 00),
				INS(03, 00, 03, 00),
				INS(04, 00, 04, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(4D, 69, 63, 72),
				INS(6F, 73, 6F, 66),
				INS(74, 20, 28, 52),
				INS(29, 20, 48, 4C),
				INS(53, 4C, 20, 53),
				INS(68, 61, 64, 65),
				INS(72, 20, 43, 6F),
				INS(6D, 70, 69, 6C),
				INS(65, 72, 20, 31),
				INS(30, 2E, 31, 00),
				INS(49, 53, 47, 4E),
				INS(4C, 00, 00, 00),
				INS(02, 00, 00, 00),
				INS(08, 00, 00, 00),
				INS(38, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(03, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(03, 03, 00, 00),
				INS(41, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(03, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(03, 03, 00, 00),
				INS(50, 4F, 53, 49),
				INS(54, 49, 4F, 4E),
				INS(00, 54, 45, 58),
				INS(43, 4F, 4F, 52),
				INS(44, 00, AB, AB),
				INS(4F, 53, 47, 4E),
				INS(50, 00, 00, 00),
				INS(02, 00, 00, 00),
				INS(08, 00, 00, 00),
				INS(38, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(03, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(0F, 00, 00, 00),
				INS(44, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(03, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(03, 0C, 00, 00),
				INS(53, 56, 5F, 50),
				INS(4F, 53, 49, 54),
				INS(49, 4F, 4E, 00),
				INS(54, 45, 58, 43),
				INS(4F, 4F, 52, 44),
				INS(00, AB, AB, AB),
				INS(53, 48, 44, 52),
				INS(14, 01, 00, 00),
				INS(40, 00, 01, 00),
				INS(45, 00, 00, 00),
				INS(59, 00, 00, 04),
				INS(46, 8E, 20, 00),
				INS(00, 00, 00, 00),
				INS(04, 00, 00, 00),
				INS(5F, 00, 00, 03),
				INS(32, 10, 10, 00),
				INS(00, 00, 00, 00),
				INS(5F, 00, 00, 03),
				INS(32, 10, 10, 00),
				INS(01, 00, 00, 00),
				INS(67, 00, 00, 04),
				INS(F2, 20, 10, 00),
				INS(00, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(65, 00, 00, 03),
				INS(32, 20, 10, 00),
				INS(01, 00, 00, 00),
				INS(68, 00, 00, 02),
				INS(01, 00, 00, 00),
				INS(36, 00, 00, 05),
				INS(32, 00, 10, 00),
				INS(00, 00, 00, 00),
				INS(46, 10, 10, 00),
				INS(00, 00, 00, 00),
				INS(36, 00, 00, 05),
				INS(42, 00, 10, 00),
				INS(00, 00, 00, 00),
				INS(01, 40, 00, 00),
				INS(00, 00, 80, 3F),
				INS(10, 00, 00, 08),
				INS(12, 20, 10, 00),
				INS(00, 00, 00, 00),
				INS(46, 02, 10, 00),
				INS(00, 00, 00, 00),
				INS(46, 83, 20, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(10, 00, 00, 08),
				INS(22, 20, 10, 00),
				INS(00, 00, 00, 00),
				INS(46, 02, 10, 00),
				INS(00, 00, 00, 00),
				INS(46, 83, 20, 00),
				INS(00, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(10, 00, 00, 08),
				INS(42, 20, 10, 00),
				INS(00, 00, 00, 00),
				INS(46, 02, 10, 00),
				INS(00, 00, 00, 00),
				INS(46, 83, 20, 00),
				INS(00, 00, 00, 00),
				INS(02, 00, 00, 00),
				INS(10, 00, 00, 08),
				INS(82, 20, 10, 00),
				INS(00, 00, 00, 00),
				INS(46, 02, 10, 00),
				INS(00, 00, 00, 00),
				INS(46, 83, 20, 00),
				INS(00, 00, 00, 00),
				INS(03, 00, 00, 00),
				INS(36, 00, 00, 05),
				INS(32, 20, 10, 00),
				INS(01, 00, 00, 00),
				INS(46, 10, 10, 00),
				INS(01, 00, 00, 00),
				INS(3E, 00, 00, 01),
				INS(53, 54, 41, 54),
				INS(74, 00, 00, 00),
				INS(08, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(04, 00, 00, 00),
				INS(04, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(03, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
			#undef INS
			};

			/*
				SamplerState sampler0 : register(s0);
				Texture2D    texture0 : register(t0);

				struct ShaderInput
				{
					float4 pos : SV_POSITION;
					float2 uv  : TEXCOORD;
				};

				struct ShaderOutput
				{
					float4 col : SV_TARGET;
				};

				ShaderOutput main(ShaderInput input)
				{
					ShaderOutput output;
					output.col = texture0.Sample(sampler0, input.uv);
					return output;
				}
			*/
			static BYTE pixel_shader[] = {
			#define INS(A, B, C, D) 0x##A, 0x##B, 0x##C, 0x##D
				INS(44, 58, 42, 43),
				INS(49, 32, 4F, C1),
				INS(DD, 1D, 06, 4C),
				INS(A1, D5, 06, 65),
				INS(FE, EA, F9, 3B),
				INS(01, 00, 00, 00),
				INS(48, 02, 00, 00),
				INS(05, 00, 00, 00),
				INS(34, 00, 00, 00),
				INS(D4, 00, 00, 00),
				INS(2C, 01, 00, 00),
				INS(60, 01, 00, 00),
				INS(CC, 01, 00, 00),
				INS(52, 44, 45, 46),
				INS(98, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(02, 00, 00, 00),
				INS(1C, 00, 00, 00),
				INS(00, 04, FF, FF),
				INS(00, 01, 00, 00),
				INS(6E, 00, 00, 00),
				INS(5C, 00, 00, 00),
				INS(03, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(65, 00, 00, 00),
				INS(02, 00, 00, 00),
				INS(05, 00, 00, 00),
				INS(04, 00, 00, 00),
				INS(FF, FF, FF, FF),
				INS(00, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(0D, 00, 00, 00),
				INS(73, 61, 6D, 70),
				INS(6C, 65, 72, 30),
				INS(00, 74, 65, 78),
				INS(74, 75, 72, 65),
				INS(30, 00, 4D, 69),
				INS(63, 72, 6F, 73),
				INS(6F, 66, 74, 20),
				INS(28, 52, 29, 20),
				INS(48, 4C, 53, 4C),
				INS(20, 53, 68, 61),
				INS(64, 65, 72, 20),
				INS(43, 6F, 6D, 70),
				INS(69, 6C, 65, 72),
				INS(20, 31, 30, 2E),
				INS(31, 00, AB, AB),
				INS(49, 53, 47, 4E),
				INS(50, 00, 00, 00),
				INS(02, 00, 00, 00),
				INS(08, 00, 00, 00),
				INS(38, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(03, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(0F, 00, 00, 00),
				INS(44, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(03, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(03, 03, 00, 00),
				INS(53, 56, 5F, 50),
				INS(4F, 53, 49, 54),
				INS(49, 4F, 4E, 00),
				INS(54, 45, 58, 43),
				INS(4F, 4F, 52, 44),
				INS(00, AB, AB, AB),
				INS(4F, 53, 47, 4E),
				INS(2C, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(08, 00, 00, 00),
				INS(20, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(03, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(0F, 00, 00, 00),
				INS(53, 56, 5F, 54),
				INS(41, 52, 47, 45),
				INS(54, 00, AB, AB),
				INS(53, 48, 44, 52),
				INS(64, 00, 00, 00),
				INS(40, 00, 00, 00),
				INS(19, 00, 00, 00),
				INS(5A, 00, 00, 03),
				INS(00, 60, 10, 00),
				INS(00, 00, 00, 00),
				INS(58, 18, 00, 04),
				INS(00, 70, 10, 00),
				INS(00, 00, 00, 00),
				INS(55, 55, 00, 00),
				INS(62, 10, 00, 03),
				INS(32, 10, 10, 00),
				INS(01, 00, 00, 00),
				INS(65, 00, 00, 03),
				INS(F2, 20, 10, 00),
				INS(00, 00, 00, 00),
				INS(45, 00, 00, 09),
				INS(F2, 20, 10, 00),
				INS(00, 00, 00, 00),
				INS(46, 10, 10, 00),
				INS(01, 00, 00, 00),
				INS(46, 7E, 10, 00),
				INS(00, 00, 00, 00),
				INS(00, 60, 10, 00),
				INS(00, 00, 00, 00),
				INS(3E, 00, 00, 01),
				INS(53, 54, 41, 54),
				INS(74, 00, 00, 00),
				INS(02, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(02, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(01, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
				INS(00, 00, 00, 00),
			#undef INS
			};

			/* input layout */ {
				D3D11_INPUT_ELEMENT_DESC input_layout_p2f_t2f[] = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				};
				HRGet = d3d11_device->CreateInputLayout(input_layout_p2f_t2f, 2, vertex_shader, sizeof(vertex_shader), &d3d11_input_layout);
				HRCheckCallReturnBool("ID3D11Device::CreateInputLayout");
			}

			/* vertex buffer */ {
				D3D11_BUFFER_DESC vertex_buffer_info = {};
				vertex_buffer_info.ByteWidth = 4 * sizeof(Vertex);
				vertex_buffer_info.Usage = D3D11_USAGE_DYNAMIC;
				vertex_buffer_info.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				vertex_buffer_info.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				HRGet = d3d11_device->CreateBuffer(&vertex_buffer_info, NULL, &d3d11_vertex_buffer);
				HRCheckCallReturnBool("ID3D11Device::CreateBuffer");
			}

			/* index buffer */ {
				D3D11_BUFFER_DESC index_buffer_info = {};
				index_buffer_info.ByteWidth = 6 * sizeof(Index);
				index_buffer_info.Usage = D3D11_USAGE_DEFAULT;
				index_buffer_info.BindFlags = D3D11_BIND_INDEX_BUFFER;
				Index index_buffer_data[6] = { 0, 1, 2, 0, 2, 3 };
				D3D11_SUBRESOURCE_DATA index_buffer_data_info = {};
				index_buffer_data_info.pSysMem = index_buffer_data;
				HRGet = d3d11_device->CreateBuffer(&index_buffer_info, &index_buffer_data_info, &d3d11_index_buffer);
				HRCheckCallReturnBool("ID3D11Device::CreateBuffer");
			}

			/* constant buffer */ {
				D3D11_BUFFER_DESC constant_buffer_info = {};
				constant_buffer_info.ByteWidth = sizeof(TransformBuffer);
				constant_buffer_info.Usage = D3D11_USAGE_DYNAMIC;
				constant_buffer_info.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				constant_buffer_info.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				HRGet = d3d11_device->CreateBuffer(&constant_buffer_info, NULL, &d3d11_constant_buffer);
				HRCheckCallReturnBool("ID3D11Device::CreateBuffer");
			}

			/* vertex shader */ {
				HRGet = d3d11_device->CreateVertexShader(vertex_shader, sizeof(vertex_shader), NULL, &d3d11_vertex_shader);
				HRCheckCallReturnBool("ID3D11Device::CreateVertexShader");
			}

			/* rasterizer state */ {
				D3D11_RASTERIZER_DESC state_info = {};
				state_info.FillMode = D3D11_FILL_SOLID;
				state_info.CullMode = D3D11_CULL_NONE;
				HRGet = d3d11_device->CreateRasterizerState(&state_info, &d3d11_rasterizer_state);
				HRCheckCallReturnBool("ID3D11Device::CreateRasterizerState");
			}

			/* sampler state */ {
				D3D11_SAMPLER_DESC state_info = {};
				state_info.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				state_info.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
				state_info.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				state_info.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
				state_info.MaxAnisotropy = 1;
				state_info.ComparisonFunc = D3D11_COMPARISON_NEVER;
				state_info.MinLOD = -FLT_MAX;
				state_info.MaxLOD = FLT_MAX;
				HRGet = d3d11_device->CreateSamplerState(&state_info, &d3d11_sampler_state_linear);
				HRCheckCallReturnBool("ID3D11Device::CreateSamplerState");

				state_info.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				HRGet = d3d11_device->CreateSamplerState(&state_info, &d3d11_sampler_state_point);
				HRCheckCallReturnBool("ID3D11Device::CreateSamplerState");
			}

			/* pixel shader */ {
				HRGet = d3d11_device->CreatePixelShader(pixel_shader, sizeof(pixel_shader), NULL, &d3d11_pixel_shader);
				HRCheckCallReturnBool("ID3D11Device::CreatePixelShader");
			}

			/* depth stencil state */ {
				D3D11_DEPTH_STENCILOP_DESC state_op_info = {
					.StencilFailOp = D3D11_STENCIL_OP_KEEP,
					.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
					.StencilPassOp = D3D11_STENCIL_OP_KEEP,
					.StencilFunc = D3D11_COMPARISON_ALWAYS,
				};
				D3D11_DEPTH_STENCIL_DESC state_info = {
					.DepthEnable = FALSE,
					.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
					.DepthFunc = D3D11_COMPARISON_LESS,
					.StencilEnable = FALSE,
					.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
					.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
					.FrontFace = state_op_info,
					.BackFace = state_op_info,
				};
				HRGet = d3d11_device->CreateDepthStencilState(&state_info, &d3d11_depth_stencil_state);
				HRCheckCallReturnBool("ID3D11Device::CreateDepthStencilState");
			}

			/* blend state */ {
				D3D11_RENDER_TARGET_BLEND_DESC blend_state_info = {
					.BlendEnable = FALSE,
					.SrcBlend = D3D11_BLEND_ONE,
					.DestBlend = D3D11_BLEND_ZERO,
					.BlendOp = D3D11_BLEND_OP_ADD,
					.SrcBlendAlpha = D3D11_BLEND_ONE,
					.DestBlendAlpha = D3D11_BLEND_ZERO,
					.BlendOpAlpha = D3D11_BLEND_OP_ADD,
					.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL,
				};
				D3D11_BLEND_DESC state_info = {
					.AlphaToCoverageEnable = FALSE,
					.IndependentBlendEnable = FALSE,
					.RenderTarget = {
						blend_state_info,
						blend_state_info,
						blend_state_info,
						blend_state_info,
						blend_state_info,
						blend_state_info,
						blend_state_info,
						blend_state_info,
					},
				};
				HRGet = d3d11_device->CreateBlendState(&state_info, &d3d11_blend_state);
				HRCheckCallReturnBool("ID3D11Device::CreateBlendState");
			}

			return true;
		}
		void DestroyResource()
		{
			d3d11_input_layout.Reset();
			d3d11_vertex_buffer.Reset();
			d3d11_index_buffer.Reset();
			d3d11_constant_buffer.Reset();
			d3d11_vertex_shader.Reset();
			d3d11_rasterizer_state.Reset();
			d3d11_sampler_state_linear.Reset();
			d3d11_sampler_state_point.Reset();
			d3d11_pixel_shader.Reset();
			d3d11_depth_stencil_state.Reset();
			d3d11_blend_state.Reset();
		}
	public:
		bool AttachDevice(ID3D11Device* device)
		{
			assert(device);
			d3d11_device = device;
			d3d11_device->GetImmediateContext(&d3d11_device_context);
			return CreateResource();
		}
		void DetachDevice()
		{
			DestroyResource();
			d3d11_device.Reset();
			d3d11_device_context.Reset();
		}
		bool UpdateTransform(ID3D11ShaderResourceView* srv, ID3D11RenderTargetView* rtv)
		{
			assert(srv);
			assert(rtv);
			assert(d3d11_device_context);
			assert(d3d11_vertex_buffer);
			assert(d3d11_constant_buffer);

			HRNew;

			// info

			Microsoft::WRL::ComPtr<ID3D11Resource> srv_res;
			srv->GetResource(&srv_res);
			Microsoft::WRL::ComPtr<ID3D11Texture2D> srv_res_tex;
			HRGet = srv_res->QueryInterface(IID_PPV_ARGS(&srv_res_tex));
			HRCheckCallReturnBool("ID3D11Resource::QueryInterface -> ID3D11Texture2D");
			D3D11_TEXTURE2D_DESC srv_res_tex_info = {};
			srv_res_tex->GetDesc(&srv_res_tex_info);

			Microsoft::WRL::ComPtr<ID3D11Resource> rtv_res;
			rtv->GetResource(&rtv_res);
			Microsoft::WRL::ComPtr<ID3D11Texture2D> rtv_res_tex;
			HRGet = rtv_res->QueryInterface(IID_PPV_ARGS(&rtv_res_tex));
			HRCheckCallReturnBool("ID3D11Resource::QueryInterface -> ID3D11Texture2D");
			D3D11_TEXTURE2D_DESC rtv_res_tex_info = {};
			rtv_res_tex->GetDesc(&rtv_res_tex_info);

			// letter boxing

			float const window_w = float(rtv_res_tex_info.Width);
			float const window_h = float(rtv_res_tex_info.Height);
			float const canvas_w = float(srv_res_tex_info.Width);
			float const canvas_h = float(srv_res_tex_info.Height);

			float const scale_w = window_w / canvas_w;
			float const scale_h = window_h / canvas_h;
			float const scale = std::min(scale_w, scale_h);

			float const draw_w = canvas_w * scale;
			float const draw_h = canvas_h * scale;
			float const draw_x = (window_w - draw_w) * 0.5f;
			float const draw_y = (window_h - draw_h) * 0.5f;

			// vertex buffer

			float const ______ = 0.0f;

			vertex_buffer[0] = { draw_x + ______, draw_y + draw_h, 0.0f, 0.0f };
			vertex_buffer[1] = { draw_x + draw_w, draw_y + draw_h, 1.0f, 0.0f };
			vertex_buffer[2] = { draw_x + draw_w, draw_y + ______, 1.0f, 1.0f };
			vertex_buffer[3] = { draw_x + ______, draw_y + ______, 0.0f, 1.0f };

			D3D11_MAPPED_SUBRESOURCE vertex_data_range = {};
			HRGet = d3d11_device_context->Map(d3d11_vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vertex_data_range);
			HRCheckCallReturnBool("ID3D11DeviceContext::Map");
			memcpy(vertex_data_range.pData, vertex_buffer, sizeof(vertex_buffer));
			d3d11_device_context->Unmap(d3d11_vertex_buffer.Get(), 0);

			// constant buffer

			DirectX::XMStoreFloat4x4(
				&transform_buffer.mvp,
				DirectX::XMMatrixTranspose(
					DirectX::XMMatrixOrthographicOffCenterLH(
						0.0f, window_w, 0.0f, window_h, 0.0f, 1.0f
					)
				)
			);

			D3D11_MAPPED_SUBRESOURCE constant_data_range = {};
			HRGet = d3d11_device_context->Map(d3d11_constant_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &constant_data_range);
			HRCheckCallReturnBool("ID3D11DeviceContext::Map");
			memcpy(constant_data_range.pData, &transform_buffer, sizeof(transform_buffer));
			d3d11_device_context->Unmap(d3d11_constant_buffer.Get(), 0);

			// state

			d3d11_viewport = { 0.0f, 0.0f, window_w , window_h ,0.0f, 1.0f };
			d3d11_scissor_rect = { 0, 0, static_cast<LONG>(rtv_res_tex_info.Width), static_cast<LONG>(rtv_res_tex_info.Height) };

			return true;
		}
		bool Draw(ID3D11ShaderResourceView* srv, ID3D11RenderTargetView* rtv, bool clear_rtv)
		{
			assert(d3d11_device_context);

			assert(d3d11_input_layout);
			assert(d3d11_vertex_buffer);
			assert(d3d11_index_buffer);

			d3d11_device_context->IASetInputLayout(d3d11_input_layout.Get());
			d3d11_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			ID3D11Buffer* vertex_buffer_list[1] = { d3d11_vertex_buffer.Get() };
			UINT stride_list[1] = { (UINT)sizeof(Vertex) };
			UINT offset_list[1] = { 0 };
			d3d11_device_context->IASetVertexBuffers(0, 1, vertex_buffer_list, stride_list, offset_list);
			d3d11_device_context->IASetIndexBuffer(d3d11_index_buffer.Get(), DXGI_FORMAT_R16_UINT, 0);

			assert(d3d11_constant_buffer);
			assert(d3d11_vertex_shader);

			ID3D11Buffer* constant_buffer_list[1] = { d3d11_constant_buffer.Get() };
			d3d11_device_context->VSSetConstantBuffers(0, 1, constant_buffer_list);
			d3d11_device_context->VSSetShader(d3d11_vertex_shader.Get(), NULL, 0);

			assert(d3d11_rasterizer_state);

			d3d11_device_context->RSSetState(d3d11_rasterizer_state.Get());
			d3d11_device_context->RSSetViewports(1, &d3d11_viewport);
			d3d11_device_context->RSSetScissorRects(1, &d3d11_scissor_rect);

			assert(d3d11_sampler_state_linear);
			assert(d3d11_sampler_state_point);
			assert(srv);
			assert(d3d11_pixel_shader);

			ID3D11SamplerState* sampler_state_list[1] = { d3d11_sampler_state_linear.Get() };
			d3d11_device_context->PSSetSamplers(0, 1, sampler_state_list);
			ID3D11ShaderResourceView* shader_resource_view_list[1] = { srv };
			d3d11_device_context->PSSetShaderResources(0, 1, shader_resource_view_list);
			d3d11_device_context->PSSetShader(d3d11_pixel_shader.Get(), NULL, 0);

			assert(d3d11_depth_stencil_state);
			assert(d3d11_blend_state);
			assert(rtv);

			d3d11_device_context->OMSetDepthStencilState(d3d11_depth_stencil_state.Get(), D3D11_DEFAULT_STENCIL_REFERENCE);
			FLOAT const blend_factor[4]{};
			d3d11_device_context->OMSetBlendState(d3d11_blend_state.Get(), blend_factor, D3D11_DEFAULT_SAMPLE_MASK);
			ID3D11RenderTargetView* render_target_view_list[1] = { rtv };
			d3d11_device_context->OMSetRenderTargets(1, render_target_view_list, NULL);

			if (clear_rtv)
			{
				FLOAT const clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
				d3d11_device_context->ClearRenderTargetView(rtv, clear_color);
			}
			d3d11_device_context->DrawIndexed(6, 0, 0);

			return true;
		}
	public:
		LetterBoxingRenderer() = default;
		~LetterBoxingRenderer() { DetachDevice(); }
	};

	class LegacyExclusiveFullscreenPresentationModel : public PresentationModelBase
	{
	protected:
		LetterBoxingRenderer renderer;
	protected:
		BOOL m_fullscreen{ FALSE };
		DXGI_MODE_DESC1 display_mode{};
		BOOL allow_mode_switch{ FALSE };
	protected:
		void DeleteThis() { delete this; }
		bool CreateSwapChain()
		{
			assert(win32_window);
			assert(dxgi_factory);
			assert(d3d11_device);

			HRNew;

			LOG_INFO("Create PresentationModel (LegacyWindow)");

			// Window

			RECT window_client_rect = {};
			HRGet = GetClientRect(win32_window, &window_client_rect);
			HRCheckCallReturnBool("GetClientRect");

			window_size.x = static_cast<uint32_t>(window_client_rect.right - window_client_rect.left);
			window_size.y = static_cast<uint32_t>(window_client_rect.bottom - window_client_rect.top);

			if (window_size.x == 0 || window_size.y == 0)
			{
				LOG_ERROR("Window size can not be (%ux%u)", window_size.x, window_size.y);
				assert(false);
				return false;
			}

			LOG_INFO("Window Size (%ux%u)", window_size.x, window_size.y);

			// DXGI SwapChain

			DXGI_SWAP_CHAIN_DESC1 swapchain_info = {};
			swapchain_info.Width = window_size.x;
			swapchain_info.Height = window_size.y;
			swapchain_info.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			swapchain_info.SampleDesc.Count = 1;
			swapchain_info.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchain_info.BufferCount = 1;
			swapchain_info.Scaling = DXGI_SCALING_STRETCH;
			swapchain_info.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapchain_info.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

			HRGet = dxgi_factory->CreateSwapChainForHwnd(
				d3d11_device.Get(), win32_window, &swapchain_info, NULL, NULL, &dxgi_swapchain);
			HRCheckCallReturnBool("IDXGIFactory2::CreateSwapChainForHwnd");

			dxgi_swapchain_present_allow_tearing = FALSE;
			allow_mode_switch = FALSE; // 暂时不可用

			LOG_INFO("Create DXGI SwapChain");

			HRGet = Platform::RuntimeLoader::DXGI::MakeSwapChainWindowAssociation(dxgi_swapchain.Get(), DXGI_MWA_NO_ALT_ENTER);
			HRCheckCallReturnBool("IDXGIFactory::MakeWindowAssociation -> DXGI_MWA_NO_ALT_ENTER");

			HRGet = Platform::RuntimeLoader::DXGI::SetDeviceMaximumFrameLatency(dxgi_swapchain.Get(), 1);
			HRCheckCallReport("IDXGIDevice1::SetMaximumFrameLatency -> 1");

			if (!CreateSwapChainRenderAttachment()) return false;

			if (!CreateCanvasRenderAttachment()) return false;

			if (!renderer.AttachDevice(d3d11_device.Get())) return false;

			if (!CanvasUpdateTransform()) return false;

			LOG_INFO("Create PresentationModel (LegacyWindow) Successfully");

			return true;
		}
		void DestroySwapChain()
		{
			LeaveFullscreen();
			renderer.DetachDevice();
			DestroyCanvasRenderAttachment();
			DestroySwapChainRenderAttachment();
			dxgi_swapchain.Reset();
			dxgi_swapchain_present_allow_tearing = FALSE;
			allow_mode_switch = FALSE;
		}
		bool CanvasUpdateTransform()
		{
			return renderer.UpdateTransform(
				d3d11_shaderresource_view.Get(),
				d3d11_swapchain_rendertarget_view.Get()
			);
		}
		bool CreateFullscreenSwapChain()
		{
			assert(win32_window);
			assert(dxgi_factory);
			assert(dxgi_swapchain); // 仍然需要旧的交换链存在，才能枚举显示模式
			assert(d3d11_device);

			HRNew;

			// 筛选出合适的独占全屏模式

			DXGI_MODE_DESC1 select_mode = {};

			auto filter = [&]() -> bool
			{
				Microsoft::WRL::ComPtr<IDXGIOutput> output;
				HRGet = dxgi_swapchain->GetContainingOutput(&output);
				HRCheckCallReturnBool("IDXGISwapChain1::GetContainingOutput");

				Microsoft::WRL::ComPtr<IDXGIOutput1> output1;
				HRGet = output.As(&output1);
				HRCheckCallReturnBool("IDXGIOutput::QueryInterface -> IDXGIOutput1");

				DXGI_OUTPUT_DESC output_info = {};
				HRGet = output1->GetDesc(&output_info);
				HRCheckCallReturnBool("IDXGIOutput1::GetDesc");
				assert(output_info.Monitor);

				MONITORINFOEXW monitor_info = {};
				monitor_info.cbSize = sizeof(MONITORINFOEXW);
				HRGet = GetMonitorInfoW(output_info.Monitor, &monitor_info);
				HRCheckCallReturnBool("GetMonitorInfoW");

				UINT const monitor_w = static_cast<UINT>(monitor_info.rcMonitor.right - monitor_info.rcMonitor.left);
				UINT const monitor_h = static_cast<UINT>(monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top);
				assert(monitor_w > 0 && monitor_h > 0);

				UINT mode_count = 0;
				HRGet = output1->GetDisplayModeList1(DXGI_FORMAT_B8G8R8A8_UNORM, 0, &mode_count, NULL);
				HRCheckCallReturnBool("IDXGIOutput1::GetDisplayModeList1 -> DXGI_FORMAT_B8G8R8A8_UNORM");

				std::vector<DXGI_MODE_DESC1> mode_list(mode_count);
				HRGet = output1->GetDisplayModeList1(DXGI_FORMAT_B8G8R8A8_UNORM, 0, &mode_count, mode_list.data());
				HRCheckCallReturnBool("IDXGIOutput1::GetDisplayModeList1 -> DXGI_FORMAT_B8G8R8A8_UNORM");

				// 公共函数

				auto compare_refresh_rate = [](DXGI_MODE_DESC1 const& a, DXGI_MODE_DESC1 const& b) -> bool
				{
					double const hz_a = (double)a.RefreshRate.Numerator / (double)a.RefreshRate.Denominator;
					double const hz_b = (double)b.RefreshRate.Numerator / (double)b.RefreshRate.Denominator;
					return hz_a > hz_b;
				};

				auto check_required_refresh_rate = [](DXGI_MODE_DESC1 const& v) -> bool
				{
					return (double)v.RefreshRate.Numerator / (double)v.RefreshRate.Denominator >= 59.5;
				};

				auto check_required_canvas_size = [&](DXGI_MODE_DESC1 const& v)
				{
					return v.Width >= canvas_size.x && v.Height >= canvas_size.y;
				};

				auto check_aspect_ratio = [&](DXGI_MODE_DESC1 const& v) -> bool
				{
					if (monitor_w > monitor_h)
					{
						UINT const width = v.Height * monitor_w / monitor_h;
						assert(width > 0);
						if (width == 0) return false;
						else if (v.Width == width) return true;
						else if (width > v.Width && (width - v.Width) <= 2) return true;
						else if (v.Width > width && (v.Width - width) <= 2) return true;
						else return false;
					}
					else
					{
						UINT const height = v.Width * monitor_h / monitor_w;
						assert(height > 0);
						if (height == 0) return false;
						else if (v.Height == height) return true;
						else if (height > v.Height && (height - v.Height) <= 2) return true;
						else if (v.Height > height && (v.Height - height) <= 2) return true;
						else return false;
					}
				};

				// 先准备好桌面分辨率

				std::vector<DXGI_MODE_DESC1> default_mode_list;
				for (auto const& v : mode_list)
				{
					if (v.Width == monitor_w && v.Height == monitor_h)
					{
						default_mode_list.emplace_back(v);
					}
				}

				assert(!default_mode_list.empty());
				if (default_mode_list.empty()) return false;

				std::sort(default_mode_list.begin(), default_mode_list.end(), compare_refresh_rate);
				DXGI_MODE_DESC1 default_mode = default_mode_list.at(0);
				default_mode_list.clear();

				// 剔除刷新率过低的

				for (auto it = mode_list.begin(); it != mode_list.end();)
				{
					if (!check_required_refresh_rate(*it))
					{
						it = mode_list.erase(it);
					}
					else
					{
						it++;
					}
				}

				// 剔除分辨率比画布分辨率低的

				for (auto it = mode_list.begin(); it != mode_list.end();)
				{
					if (!check_required_canvas_size(*it))
					{
						it = mode_list.erase(it);
					}
					else
					{
						it++;
					}
				}

				// 剔除横纵比不和显示器匹配的

				for (auto it = mode_list.begin(); it != mode_list.end();)
				{
					if (!check_aspect_ratio(*it))
					{
						it = mode_list.erase(it);
					}
					else
					{
						it++;
					}
				}

				// 根据面积比和刷新率进行排序，优先刷新率，然后是面积比

				double const canvas_sz = (double)(canvas_size.x * canvas_size.y);
				auto compare_size = [=](DXGI_MODE_DESC1 const& a, DXGI_MODE_DESC1 const& b) -> bool
				{
					double const sz_a = canvas_sz / (double)(a.Width * a.Height);
					double const sz_b = canvas_sz / (double)(b.Width * b.Height);
					if (sz_a == sz_b)
					{
						return compare_refresh_rate(a, b);
					}
					else
					{
						return sz_b > sz_b;
					}
				};

				std::sort(mode_list.begin(), mode_list.end(), compare_size);

				// 打印排序结果

				for (auto const& v : mode_list)
				{
					LOG_INFO("Display Mode (%ux%u@%.2fHz)",
						v.Width,
						v.Height,
						(double)v.RefreshRate.Numerator / (double)v.RefreshRate.Denominator)
				}

				// 最终，挑选出面积比最大且刷新率最高的

				if (!mode_list.empty())
				{
					select_mode = mode_list.at(0);
				}
				else
				{
					select_mode = default_mode;
				}

				return true;
			};

			if (!filter()) return false;

			display_mode = select_mode;

			// 重新创建交换链

			DestroySwapChainRenderAttachment();
			dxgi_swapchain.Reset();
			dxgi_swapchain_present_allow_tearing = FALSE;
			allow_mode_switch = FALSE;

			// DXGI SwapChain

			DXGI_SWAP_CHAIN_DESC1 swapchain_info = {};
			swapchain_info.Width = select_mode.Width;
			swapchain_info.Height = select_mode.Height;
			swapchain_info.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			swapchain_info.SampleDesc.Count = 1;
			swapchain_info.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchain_info.BufferCount = 1;
			swapchain_info.Scaling = DXGI_SCALING_STRETCH;
			swapchain_info.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapchain_info.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
			swapchain_info.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreen_info = {};
			fullscreen_info.RefreshRate = select_mode.RefreshRate;
			fullscreen_info.ScanlineOrdering = select_mode.ScanlineOrdering;
			fullscreen_info.Scaling = select_mode.Scaling;
			fullscreen_info.Windowed = TRUE;

			HRGet = dxgi_factory->CreateSwapChainForHwnd(
				d3d11_device.Get(), win32_window, &swapchain_info, &fullscreen_info, NULL, &dxgi_swapchain);
			HRCheckCallReturnBool("IDXGIFactory2::CreateSwapChainForHwnd");

			dxgi_swapchain_present_allow_tearing = FALSE;
			allow_mode_switch = TRUE;

			HRGet = Platform::RuntimeLoader::DXGI::MakeSwapChainWindowAssociation(dxgi_swapchain.Get(), DXGI_MWA_NO_ALT_ENTER);
			HRCheckCallReturnBool("IDXGIFactory::MakeWindowAssociation -> DXGI_MWA_NO_ALT_ENTER");

			HRGet = Platform::RuntimeLoader::DXGI::SetDeviceMaximumFrameLatency(dxgi_swapchain.Get(), 1);
			HRCheckCallReport("IDXGIDevice1::SetMaximumFrameLatency -> 1");

			if (!CreateSwapChainRenderAttachment()) return false;
			if (!CanvasUpdateTransform()) return false;

			return true;
		}
		bool EnterFullscreen()
		{
			if (!dxgi_swapchain) return true;

			HRNew;

			BOOL get_state = FALSE;
			HRGet = dxgi_swapchain->GetFullscreenState(&get_state, NULL);
			HRCheckCallReturnBool("IDXGISwapChain1::GetFullscreenState");

			if (!get_state)
			{
				HRGet = dxgi_swapchain->SetFullscreenState(TRUE, NULL);
				HRCheckCallReturnBool("IDXGISwapChain1::SetFullscreenState -> TRUE");

				//auto* mode = (DXGI_MODE_DESC*)(&display_mode);
				//HRGet = dxgi_swapchain->ResizeTarget(mode);
				//HRCheckCallReturnBool("IDXGISwapChain1::ResizeTarget");
			}

			return true;
		}
		bool LeaveFullscreen()
		{
			if (!dxgi_swapchain) return true;

			HRNew;

			BOOL get_state = FALSE;
			HRGet = dxgi_swapchain->GetFullscreenState(&get_state, NULL);
			HRCheckCallReturnBool("IDXGISwapChain1::GetFullscreenState");

			if (get_state)
			{
				HRGet = dxgi_swapchain->SetFullscreenState(FALSE, NULL);
				HRCheckCallReturnBool("IDXGISwapChain1::SetFullscreenState -> FALSE");
			}

			return true;
		}
	public:
		bool CheckSupport(HWND, ID3D11Device*, ID2D1DeviceContext*) { return true; }
		bool OnWindowSize(Vector2U size)
		{
			assert(dxgi_swapchain);
			assert(size.x > 0 && size.y > 0);

			HRNew;

			if (size.x == 0 || size.y == 0)
			{
				LOG_ERROR("Window size can not be (%ux%u)", size.x, size.y);
				return false;
			}

			DestroySwapChainRenderAttachment();

			window_size = size;

			UINT flags = 0;
			if (dxgi_swapchain_present_allow_tearing) flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			if (allow_mode_switch) flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			HRGet = dxgi_swapchain->ResizeBuffers(0, window_size.x, window_size.y, DXGI_FORMAT_UNKNOWN, flags);
			HRCheckCallReturnBool("IDXGISwapChain1::ResizeBuffers");

			if (!CreateSwapChainRenderAttachment()) return false;

			if (!CanvasUpdateTransform()) return false;

			return true;
		}
		void OnWindowActive(bool active)
		{
			if (m_fullscreen && dxgi_swapchain)
			{
				if (active)
				{
					EnterFullscreen();
				}
				else
				{
					LeaveFullscreen();
				}
			}
		}
		void OnWindowFullscreenStateChange(bool fullscreen)
		{
			HRNew;
			if (fullscreen)
			{
				if (!m_fullscreen)
				{
					CreateFullscreenSwapChain();

					HRGet = SetWindowPos(
						win32_window, NULL, 0, 0,
						static_cast<int>(display_mode.Width),
						static_cast<int>(display_mode.Height),
						SWP_NOZORDER | SWP_NOMOVE);
					HRCheckCallReport("SetWindowPos");

					EnterFullscreen();

					m_fullscreen = true;
				}
			}
			else
			{
				if (m_fullscreen)
				{
					LeaveFullscreen();

					m_fullscreen = false;
				}
			}
		}
		bool SetCanvasSize(Vector2U size)
		{
			assert(size.x > 0 && size.y > 0);

			if (size.x == 0 || size.y == 0)
			{
				LOG_ERROR("Canvas size can not be (%ux%u)", size.x, size.y);
				return false;
			}

			DestroyCanvasRenderAttachment();

			canvas_size = size;

			if (!CreateCanvasRenderAttachment()) return false;

			if (!CanvasUpdateTransform()) return false;

			return true;
		}
		bool Present(bool vsync)
		{
			if (!renderer.Draw(
				d3d11_shaderresource_view.Get(),
				d3d11_swapchain_rendertarget_view.Get(),
				true
			)) return false;

			if (!PresentBase(vsync)) return false;

			return true;
		}
	public:
		LegacyExclusiveFullscreenPresentationModel() = default;
		LegacyExclusiveFullscreenPresentationModel(Vector2U size) { canvas_size = size; }
		~LegacyExclusiveFullscreenPresentationModel() { DetachDevice(); }
	};

	class WindowPresentationModelBase : public PresentationModelBase
	{
	protected:
		LetterBoxingRenderer renderer;
	protected:
		void DestroySwapChain()
		{
			renderer.DetachDevice();
			DestroyCanvasRenderAttachment();
			DestroySwapChainRenderAttachment();
			dxgi_swapchain.Reset();
			dxgi_swapchain_present_allow_tearing = FALSE;
		}
		bool CanvasUpdateTransform()
		{
			return renderer.UpdateTransform(
				d3d11_shaderresource_view.Get(),
				d3d11_swapchain_rendertarget_view.Get()
			);
		}
	public:
		bool OnWindowSize(Vector2U size)
		{
			assert(dxgi_swapchain);
			assert(size.x > 0 && size.y > 0);

			HRNew;

			if (size.x == 0 || size.y == 0)
			{
				LOG_ERROR("Window size can not be (%ux%u)", size.x, size.y);
				return false;
			}

			DestroySwapChainRenderAttachment();

			window_size = size;

			UINT flags = dxgi_swapchain_present_allow_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
			HRGet = dxgi_swapchain->ResizeBuffers(0, window_size.x, window_size.y, DXGI_FORMAT_UNKNOWN, flags);
			HRCheckCallReturnBool("IDXGISwapChain1::ResizeBuffers");

			if (!CreateSwapChainRenderAttachment()) return false;

			if (!CanvasUpdateTransform()) return false;

			return true;
		}
		void OnWindowActive(bool) {}
		void OnWindowFullscreenStateChange(bool) {}
		bool SetCanvasSize(Vector2U size)
		{
			assert(size.x > 0 && size.y > 0);

			if (size.x == 0 || size.y == 0)
			{
				LOG_ERROR("Canvas size can not be (%ux%u)", size.x, size.y);
				return false;
			}

			DestroyCanvasRenderAttachment();

			canvas_size = size;

			if (!CreateCanvasRenderAttachment()) return false;

			if (!CanvasUpdateTransform()) return false;

			return true;
		}
		bool Present(bool vsync)
		{
			if (!renderer.Draw(
				d3d11_shaderresource_view.Get(),
				d3d11_swapchain_rendertarget_view.Get(),
				true
			)) return false;

			if (!PresentBase(vsync)) return false;

			return true;
		}
	};

	class LegacyWindowPresentationModel : public WindowPresentationModelBase
	{
	protected:
		void DeleteThis() { delete this; }
		bool CreateSwapChain()
		{
			assert(win32_window);
			assert(dxgi_factory);
			assert(d3d11_device);

			HRNew;

			LOG_INFO("Create PresentationModel (LegacyWindow)");

			// Window

			RECT window_client_rect = {};
			HRGet = GetClientRect(win32_window, &window_client_rect);
			HRCheckCallReturnBool("GetClientRect");

			window_size.x = static_cast<uint32_t>(window_client_rect.right - window_client_rect.left);
			window_size.y = static_cast<uint32_t>(window_client_rect.bottom - window_client_rect.top);

			if (window_size.x == 0 || window_size.y == 0)
			{
				LOG_ERROR("Window size can not be (%ux%u)", window_size.x, window_size.y);
				assert(false);
				return false;
			}

			LOG_INFO("Window Size (%ux%u)", window_size.x, window_size.y);

			// DXGI SwapChain

			DXGI_SWAP_CHAIN_DESC1 swapchain_info = {};
			swapchain_info.Width = window_size.x;
			swapchain_info.Height = window_size.y;
			swapchain_info.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			swapchain_info.SampleDesc.Count = 1;
			swapchain_info.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchain_info.BufferCount = 1;
			swapchain_info.Scaling = DXGI_SCALING_STRETCH;
			swapchain_info.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapchain_info.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

			HRGet = dxgi_factory->CreateSwapChainForHwnd(
				d3d11_device.Get(), win32_window, &swapchain_info, NULL, NULL, &dxgi_swapchain);
			HRCheckCallReturnBool("IDXGIFactory2::CreateSwapChainForHwnd");

			dxgi_swapchain_present_allow_tearing = FALSE;

			LOG_INFO("Create DXGI SwapChain");

			HRGet = Platform::RuntimeLoader::DXGI::MakeSwapChainWindowAssociation(dxgi_swapchain.Get(), DXGI_MWA_NO_ALT_ENTER);
			HRCheckCallReturnBool("IDXGIFactory::MakeWindowAssociation -> DXGI_MWA_NO_ALT_ENTER");

			HRGet = Platform::RuntimeLoader::DXGI::SetDeviceMaximumFrameLatency(dxgi_swapchain.Get(), 1);
			HRCheckCallReport("IDXGIDevice1::SetMaximumFrameLatency -> 1");

			if (!CreateSwapChainRenderAttachment()) return false;

			if (!CreateCanvasRenderAttachment()) return false;

			if (!renderer.AttachDevice(d3d11_device.Get())) return false;

			if (!CanvasUpdateTransform()) return false;

			LOG_INFO("Create PresentationModel (LegacyWindow) Successfully");

			return true;
		}
	public:
		bool CheckSupport(HWND, ID3D11Device*, ID2D1DeviceContext*) { return true; }
	public:
		LegacyWindowPresentationModel() = default;
		LegacyWindowPresentationModel(Vector2U size) { canvas_size = size; }
		~LegacyWindowPresentationModel() { DetachDevice(); }
	};

	class WindowPresentationModel : public WindowPresentationModelBase
	{
	protected:
		void DeleteThis() { delete this; }
		bool CreateSwapChain()
		{
			assert(win32_window);
			assert(dxgi_factory);
			assert(d3d11_device);

			HRNew;

			LOG_INFO("Create PresentationModel (Window)");

			// Window

			RECT window_client_rect = {};
			HRGet = GetClientRect(win32_window, &window_client_rect);
			HRCheckCallReturnBool("GetClientRect");

			window_size.x = static_cast<uint32_t>(window_client_rect.right - window_client_rect.left);
			window_size.y = static_cast<uint32_t>(window_client_rect.bottom - window_client_rect.top);

			if (window_size.x == 0 || window_size.y == 0)
			{
				LOG_ERROR("Window size can not be (%ux%u)", window_size.x, window_size.y);
				assert(false);
				return false;
			}

			LOG_INFO("Window Size (%ux%u)", window_size.x, window_size.y);

			// DXGI SwapChain

			DXGI_SWAP_CHAIN_DESC1 swapchain_info = {};
			swapchain_info.Width = window_size.x;
			swapchain_info.Height = window_size.y;
			swapchain_info.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			swapchain_info.SampleDesc.Count = 1;
			swapchain_info.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchain_info.BufferCount = 2;
			swapchain_info.Scaling = DXGI_SCALING_NONE;
			swapchain_info.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapchain_info.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
			swapchain_info.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

			HRGet = dxgi_factory->CreateSwapChainForHwnd(
				d3d11_device.Get(), win32_window, &swapchain_info, NULL, NULL, &dxgi_swapchain);
			HRCheckCallReturnBool("IDXGIFactory2::CreateSwapChainForHwnd");

			dxgi_swapchain_present_allow_tearing = TRUE;

			LOG_INFO("Create DXGI SwapChain");

			HRGet = Platform::RuntimeLoader::DXGI::MakeSwapChainWindowAssociation(dxgi_swapchain.Get(), DXGI_MWA_NO_ALT_ENTER);
			HRCheckCallReturnBool("IDXGIFactory::MakeWindowAssociation -> DXGI_MWA_NO_ALT_ENTER");

			HRGet = Platform::RuntimeLoader::DXGI::SetDeviceMaximumFrameLatency(dxgi_swapchain.Get(), 1);
			HRCheckCallReport("IDXGIDevice1::SetMaximumFrameLatency -> 1");

			if (!CreateSwapChainRenderAttachment()) return false;

			if (!CreateCanvasRenderAttachment()) return false;

			if (!renderer.AttachDevice(d3d11_device.Get())) return false;

			if (!CanvasUpdateTransform()) return false;

			LOG_INFO("Create PresentationModel (Window) Successfully");

			return true;
		}
	public:
		bool CheckSupport(HWND, ID3D11Device* device, ID2D1DeviceContext*)
		{
			assert(device);

			HRNew;

			// System

			if (!IsWindows10OrGreater())
			{
				LOG_ERROR("PresentationModel (Window) requires Windows 10+");
				return false;
			}

			Microsoft::WRL::ComPtr<IDXGIFactory2> v_dxgi_factory;
			Microsoft::WRL::ComPtr<ID3D11Device> v_d3d11_device = device;

			HRGet = Platform::RuntimeLoader::Direct3D11::GetFactory(v_d3d11_device.Get(), &v_dxgi_factory);
			HRCheckCallReturnBool("ID3D11Device::GetParent -> IDXGIFactory2");

			if (!Platform::RuntimeLoader::DXGI::CheckFeatureSupportPresentAllowTearing(v_dxgi_factory.Get()))
			{
				LOG_ERROR("PresentationModel (Window) requires DXGI_FEATURE_PRESENT_ALLOW_TEARING");
				return false;
			}

			return true;
		}
	public:
		WindowPresentationModel() = default;
		WindowPresentationModel(Vector2U size) { canvas_size = size; }
		~WindowPresentationModel() { DetachDevice(); }
	};

	class DirectCompositionPresentationModel : public PresentationModelBase
	{
	protected:
		Platform::RuntimeLoader::DirectComposition DC;
		Microsoft::WRL::ComPtr<IDCompositionDesktopDevice> dc_desktop_device;
		Microsoft::WRL::ComPtr<IDCompositionTarget> dc_target;
		Microsoft::WRL::ComPtr<IDCompositionVisual2> dc_visual_root;
		Microsoft::WRL::ComPtr<IDCompositionVisual2> dc_visual_swapchain;
		Microsoft::WRL::ComPtr<IDCompositionVisual2> dc_visual_background;
		Microsoft::WRL::ComPtr<IDCompositionSurface> dc_surface_background;
	protected:
		void DeleteThis() { delete this; }
		bool DirectCompositionUpdateTransform()
		{
			assert(dxgi_swapchain);
			assert(dc_visual_swapchain);
			assert(dc_visual_background);
			assert(window_size.x > 0 && window_size.y > 0);
			assert(canvas_size.x > 0 && canvas_size.y > 0);

			HRNew;

			Microsoft::WRL::ComPtr<IDXGISwapChain2> dxgi_swapchain2;
			HRGet = dxgi_swapchain.As(&dxgi_swapchain2);
			HRCheckCallReport("IDXGISwapChain1::QueryInterface -> IDXGISwapChain2");

			DXGI_MATRIX_3X2_F const swapchain_transform = MakeLetterBoxing(canvas_size.x, canvas_size.y, window_size.x, window_size.y);
			if (dxgi_swapchain2)
			{
				HRGet = dxgi_swapchain2->SetMatrixTransform(&swapchain_transform);
				HRCheckCallReturnBool("IDXGISwapChain2::SetMatrixTransform");
			}
			else
			{
				D2D_MATRIX_3X2_F const d2d_matrix3x2 = *((D2D_MATRIX_3X2_F*)(&swapchain_transform));
				HRGet = dc_visual_swapchain->SetTransform(d2d_matrix3x2);
				HRCheckCallReturnBool("IDCompositionVisual2::SetTransform");
			}

			D2D_MATRIX_3X2_F const background_transform = {
				(float)(window_size.x), 0.0f,
				0.0f, (float)(window_size.y),
				0.0f, 0.0f,
			};
			HRGet = dc_visual_background->SetTransform(background_transform);
			HRCheckCallReturnBool("IDCompositionVisual2::SetTransform");

			return true;
		}
		bool CommitAndWait()
		{
			assert(dc_desktop_device);

			HRNew;

			HRGet = dc_desktop_device->Commit();
			HRCheckCallReturnBool("IDCompositionDesktopDevice::Commit");

			HRGet = dc_desktop_device->WaitForCommitCompletion();
			HRCheckCallReturnBool("IDCompositionDesktopDevice::WaitForCommitCompletion");

			return true;
		}
		bool CreateSwapChain()
		{
			// NOTE: Windows 10+

			assert(win32_window);
			assert(dxgi_factory);
			assert(d3d11_device);
			assert(d3d11_device_context);

			HRNew;

			LOG_INFO("Create PresentationModel (DirectComposition)");

			// Window

			RECT window_client_rect = {};
			HRGet = GetClientRect(win32_window, &window_client_rect);
			HRCheckCallReturnBool("GetClientRect");

			window_size.x = static_cast<uint32_t>(window_client_rect.right - window_client_rect.left);
			window_size.y = static_cast<uint32_t>(window_client_rect.bottom - window_client_rect.top);

			if (window_size.x == 0 || window_size.y == 0)
			{
				LOG_ERROR("Window size can not be (%ux%u)", window_size.x, window_size.y);
				assert(false);
				return false;
			}

			LOG_INFO("Window Size (%ux%u)", window_size.x, window_size.y);

			// DXGI SwapChain

			if (canvas_size.x == 0 || canvas_size.y == 0)
			{
				LOG_ERROR("Canvas size can not be (%ux%u)", canvas_size.x, canvas_size.y);
				assert(false);
				return false;
			}

			LOG_INFO("Canvas Size (%ux%u)", canvas_size.x, canvas_size.y);

			DXGI_SWAP_CHAIN_DESC1 swapchain_info = {};
			swapchain_info.Width = canvas_size.x;
			swapchain_info.Height = canvas_size.y;
			swapchain_info.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			swapchain_info.SampleDesc.Count = 1;
			swapchain_info.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchain_info.BufferCount = 2;
			swapchain_info.Scaling = DXGI_SCALING_STRETCH;
			swapchain_info.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapchain_info.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
			swapchain_info.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

			HRGet = dxgi_factory->CreateSwapChainForComposition(
				d3d11_device.Get(), &swapchain_info, NULL, &dxgi_swapchain);
			HRCheckCallReturnBool("IDXGIFactory2::CreateSwapChainForComposition");

			dxgi_swapchain_present_allow_tearing = TRUE;

			LOG_INFO("Create DXGI SwapChain");

			HRGet = Platform::RuntimeLoader::DXGI::SetDeviceMaximumFrameLatency(dxgi_swapchain.Get(), 1);
			HRCheckCallReport("IDXGIDevice1::SetMaximumFrameLatency -> 1");

			if (!CreateBothRenderAttachment()) return false;

			// DirectComposition

			LOG_INFO("Create DirectComposition");

			HRGet = DC.CreateDevice(d3d11_device.Get(), IID_PPV_ARGS(&dc_desktop_device));
			HRCheckCallReturnBool("DCompositionCreateDevice2");

			HRGet = dc_desktop_device->CreateTargetForHwnd(win32_window, TRUE, &dc_target);
			HRCheckCallReturnBool("IDCompositionDesktopDevice::CreateTargetForHwnd");

			{
				HRGet = dc_desktop_device->CreateSurface(
					1, 1, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ALPHA_MODE_IGNORE,
					&dc_surface_background);
				HRCheckCallReturnBool("IDCompositionDesktopDevice::CreateVirtualSurface");

				{
					Microsoft::WRL::ComPtr<IDXGISurface> dxgi_surface;
					POINT offset = { 0, 0 };
					HRGet = dc_surface_background->BeginDraw(NULL, IID_PPV_ARGS(&dxgi_surface), &offset);
					HRCheckCallReturnBool("IDCompositionSurface::BeginDraw -> IDXGISurface");

					Microsoft::WRL::ComPtr<ID3D11Resource> d3d11_res;
					HRGet = dxgi_surface.As(&d3d11_res);
					HRCheckCallReturnBool("IDXGISurface::QueryInterface -> ID3D11Resource");

					Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3d11_rtv;
					HRGet = d3d11_device->CreateRenderTargetView(d3d11_res.Get(), NULL, &d3d11_rtv);
					HRCheckCallReturnBool("ID3D11Device::CreateRenderTargetView");

					FLOAT const clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
					d3d11_device_context->ClearRenderTargetView(d3d11_rtv.Get(), clear_color);
					d3d11_device_context->Flush();

					HRGet = dc_surface_background->EndDraw();
					HRCheckCallReturnBool("IDCompositionSurface::EndDraw");
				}

				HRGet = dc_desktop_device->CreateVisual(&dc_visual_background);
				HRCheckCallReturnBool("IDCompositionDesktopDevice::CreateVisual");

				HRGet = dc_visual_background->SetContent(dc_surface_background.Get());
				HRCheckCallReturnBool("IDCompositionVisual2::SetContent");
			}

			{
				HRGet = dc_desktop_device->CreateVisual(&dc_visual_swapchain);
				HRCheckCallReturnBool("IDCompositionDesktopDevice::CreateVisual");

				HRGet = dc_visual_swapchain->SetContent(dxgi_swapchain.Get());
				HRCheckCallReturnBool("IDCompositionVisual2::SetContent");

				HRGet = dc_visual_swapchain->SetBitmapInterpolationMode(DCOMPOSITION_BITMAP_INTERPOLATION_MODE_LINEAR);
				HRCheckCallReturnBool("IDCompositionVisual2::SetBitmapInterpolationMode");
			}

			{
				HRGet = dc_desktop_device->CreateVisual(&dc_visual_root);
				HRCheckCallReturnBool("IDCompositionDesktopDevice::CreateVisual");

				HRGet = dc_visual_root->AddVisual(dc_visual_background.Get(), TRUE, NULL);
				HRCheckCallReturnBool("IDCompositionVisual2::AddVisual");

				HRGet = dc_visual_root->AddVisual(dc_visual_swapchain.Get(), FALSE, NULL);
				HRCheckCallReturnBool("IDCompositionVisual2::AddVisual");
			}

			HRGet = dc_target->SetRoot(dc_visual_root.Get());
			HRCheckCallReturnBool("IDCompositionTarget::SetRoot");

			if (!DirectCompositionUpdateTransform()) return false;

			if (!CommitAndWait()) return false;

			LOG_INFO("Create DirectComposition Successfully");

			LOG_INFO("Create PresentationModel (DirectComposition) Successfully");

			return true;
		}
		void DestroySwapChain()
		{
			DestroyBothRenderAttachment();
			if (dc_visual_swapchain)
			{
				HRNew;
				HRGet = dc_visual_swapchain->SetContent(NULL);
			}
			if (dc_desktop_device)
			{
				HRNew;
				HRGet = dc_desktop_device->Commit();
				HRGet = dc_desktop_device->WaitForCommitCompletion();
			}
			dxgi_swapchain.Reset();
			dxgi_swapchain_present_allow_tearing = FALSE;
			dc_desktop_device.Reset();
			dc_target.Reset();
			dc_visual_root.Reset();
			dc_visual_swapchain.Reset();
			dc_visual_background.Reset();
			dc_surface_background.Reset();
		}
	public:
		bool CheckSupport(HWND, ID3D11Device* device, ID2D1DeviceContext*)
		{
			assert(device);

			HRNew;

			// System

			if (!IsWindows10OrGreater())
			{
				LOG_ERROR("PresentationModel (DirectComposition) requires Windows 10+");
				return false;
			}

			Microsoft::WRL::ComPtr<IDXGIFactory2> v_dxgi_factory;
			Microsoft::WRL::ComPtr<ID3D11Device> v_d3d11_device = device;

			HRGet = Platform::RuntimeLoader::Direct3D11::GetFactory(v_d3d11_device.Get(), &v_dxgi_factory);
			HRCheckCallReturnBool("ID3D11Device::GetParent -> IDXGIFactory2");

			if (!Platform::RuntimeLoader::DXGI::CheckFeatureSupportPresentAllowTearing(v_dxgi_factory.Get()))
			{
				LOG_ERROR("PresentationModel (DirectComposition) requires DXGI_FEATURE_PRESENT_ALLOW_TEARING");
				return false;
			}

			bool v_support = true;

			Microsoft::WRL::ComPtr<IDXGIAdapter1> v_adapter;
			for (UINT i_adapter = 0; BHR = v_dxgi_factory->EnumAdapters1(i_adapter, &v_adapter); i_adapter += 1)
			{
				LOG_INFO("Display Adapter %u:", i_adapter);
				Microsoft::WRL::ComPtr<IDXGIOutput> v_output;
				for (UINT i_output = 0; BHR = v_adapter->EnumOutputs(i_output, &v_output); i_output += 1)
				{
					BOOL overlays = FALSE;
					UINT overlay_support = 0;
					UINT hardware_composition_support = 0;

					Microsoft::WRL::ComPtr<IDXGIOutput2> v_output2;
					HRGet = v_output.As(&v_output2);
					HRCheckCallReport("IDXGIOutput::QueryInterface -> IDXGIOutput2");
					if (v_output2)
					{
						overlays = v_output2->SupportsOverlays();
					}

					Microsoft::WRL::ComPtr<IDXGIOutput3> v_output3;
					HRGet = v_output.As(&v_output3);
					HRCheckCallReport("IDXGIOutput::QueryInterface -> IDXGIOutput3");
					if (v_output3)
					{
						HRGet = v_output3->CheckOverlaySupport(DXGI_FORMAT_B8G8R8A8_UNORM, v_d3d11_device.Get(), &overlay_support);
						HRCheckCallReport("IDXGIOutput3::CheckOverlaySupport -> DXGI_FORMAT_B8G8R8A8_UNORM");
						if (FAILED(hr)) overlay_support = 0;
					}

					Microsoft::WRL::ComPtr<IDXGIOutput6> v_output6;
					HRGet = v_output.As(&v_output6);
					HRCheckCallReport("IDXGIOutput::QueryInterface -> IDXGIOutput6");
					if (v_output6)
					{
						HRGet = v_output6->CheckHardwareCompositionSupport(&hardware_composition_support);
						HRCheckCallReport("IDXGIOutput6::CheckHardwareCompositionSupport");
						if (FAILED(hr)) hardware_composition_support = 0;
					}

				#define MAKE_SUPPORT(x) ((x) ? "Support" : "Not Support")
					LOG_INFO("    Display Output %u:", i_output);
					LOG_INFO("        Overlay: %s", MAKE_SUPPORT(overlays));
					LOG_INFO("        Overlay Feature:");
					LOG_INFO("            Direct: %s", MAKE_SUPPORT(overlay_support & DXGI_OVERLAY_SUPPORT_FLAG_DIRECT));
					LOG_INFO("            Scaling: %s", MAKE_SUPPORT(overlay_support & DXGI_OVERLAY_SUPPORT_FLAG_SCALING));
					LOG_INFO("        Hardware Composition:");
					LOG_INFO("            Fullscreen: %s", MAKE_SUPPORT(hardware_composition_support & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_FULLSCREEN));
					LOG_INFO("            Windowed: %s", MAKE_SUPPORT(hardware_composition_support & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_WINDOWED));
					LOG_INFO("            Cursor Stretched: %s", MAKE_SUPPORT(hardware_composition_support & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_CURSOR_STRETCHED));
				#undef MAKE_SUPPORT

					bool const condition1 = (overlays);
					bool const condition2 = (overlay_support & DXGI_OVERLAY_SUPPORT_FLAG_DIRECT)
						&& (overlay_support & DXGI_OVERLAY_SUPPORT_FLAG_SCALING);
					// Reduce system requirements
					//bool const condition3 = (hardware_composition_support & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_FULLSCREEN)
					//	&& (hardware_composition_support & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_WINDOWED);
					bool const condition3 = (hardware_composition_support & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_FULLSCREEN);
					bool const condition = condition1 || condition2 || condition3;
					if (!condition)
					{
						LOG_ERROR("    This Display Output does not meet the requirements.");
						v_support = false;
					}
				}
			}

			return v_support;
		}
		bool OnWindowSize(Vector2U size)
		{
			assert(size.x > 0 && size.y > 0);

			if (size.x == 0 || size.y == 0)
			{
				LOG_ERROR("Window size can not be (%ux%u)", size.x, size.y);
				return false;
			}

			window_size = size;

			if (!DirectCompositionUpdateTransform()) return false;

			if (!CommitAndWait()) return false;

			return true;
		}
		void OnWindowActive(bool) {}
		void OnWindowFullscreenStateChange(bool) {}
		bool SetCanvasSize(Vector2U size)
		{
			assert(dxgi_swapchain);
			assert(size.x > 0 && size.y > 0);

			HRNew;

			if (size.x == 0 || size.y == 0)
			{
				LOG_ERROR("Canvas size can not be (%ux%u)", size.x, size.y);
				return false;
			}

			DestroyBothRenderAttachment();

			canvas_size = size;

			UINT flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			HRGet = dxgi_swapchain->ResizeBuffers(0, canvas_size.x, canvas_size.y, DXGI_FORMAT_UNKNOWN, flags);
			HRCheckCallReturnBool("IDXGISwapChain1::ResizeBuffers");

			if (!CreateBothRenderAttachment()) return false;

			if (!DirectCompositionUpdateTransform()) return false;

			if (!CommitAndWait()) return false;

			return true;
		}
		bool Present(bool vsync) { return PresentBase(vsync); }
	public:
		DirectCompositionPresentationModel() = default;
		DirectCompositionPresentationModel(Vector2U size) { canvas_size = size; }
		~DirectCompositionPresentationModel() { DetachDevice(); }
	};

	class PresentationModelManager : public Object<IPresentationModelManager>
	{
	private:
		HWND win32_window{};
		Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2d1_device_context;
	private:
		PresentationModelType m_model_type{ PresentationModelType::LegacyWindowAndExclusiveFullscreen };
		LegacyExclusiveFullscreenPresentationModel m_model_efs;
		LegacyWindowPresentationModel m_model_blt;
		WindowPresentationModel m_model_flip;
		DirectCompositionPresentationModel m_model_dwm;
	public:
		bool CheckSupport(HWND window, ID3D11Device* device, ID2D1DeviceContext* renderer)
		{
			switch (m_model_type)
			{
			case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
				return m_model_efs.CheckSupport(window, device, renderer);
			case PresentationModelType::LegacyWindow:
				return m_model_blt.CheckSupport(window, device, renderer);
			case PresentationModelType::Window:
				return m_model_flip.CheckSupport(window, device, renderer);
			case PresentationModelType::DirectComposition:
				return m_model_dwm.CheckSupport(window, device, renderer);
			default:
				assert(false); return false;
			}
		}
		bool AttachDevice(HWND window, ID3D11Device* device, ID2D1DeviceContext* renderer)
		{
			win32_window = window;
			d3d11_device = device;
			d2d1_device_context = renderer;
			switch (m_model_type)
			{
			case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
				return m_model_efs.AttachDevice(window, device, renderer);
			case PresentationModelType::LegacyWindow:
				return m_model_blt.AttachDevice(window, device, renderer);
			case PresentationModelType::Window:
				return m_model_flip.AttachDevice(window, device, renderer);
			case PresentationModelType::DirectComposition:
				return m_model_dwm.AttachDevice(window, device, renderer);
			default:
				assert(false); return false;
			}
		}
		void DetachDevice()
		{
			win32_window = NULL;
			d3d11_device.Reset();
			d2d1_device_context.Reset();
			switch (m_model_type)
			{
			case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
				m_model_efs.DetachDevice();
				break;
			case PresentationModelType::LegacyWindow:
				m_model_blt.DetachDevice();
				break;
			case PresentationModelType::Window:
				m_model_flip.DetachDevice();
				break;
			case PresentationModelType::DirectComposition:
				m_model_dwm.DetachDevice();
				break;
			default:
				assert(false);
				break;
			}
		}
		bool OnWindowSize(Vector2U size)
		{
			switch (m_model_type)
			{
			case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
				return m_model_efs.OnWindowSize(size);
			case PresentationModelType::LegacyWindow:
				return m_model_blt.OnWindowSize(size);
			case PresentationModelType::Window:
				return m_model_flip.OnWindowSize(size);
			case PresentationModelType::DirectComposition:
				return m_model_dwm.OnWindowSize(size);
			default:
				assert(false); return false;
			}
		}
		Vector2U GetWindowSize()
		{
			switch (m_model_type)
			{
			case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
				return m_model_efs.GetWindowSize();
			case PresentationModelType::LegacyWindow:
				return m_model_blt.GetWindowSize();
			case PresentationModelType::Window:
				return m_model_flip.GetWindowSize();
			case PresentationModelType::DirectComposition:
				return m_model_dwm.GetWindowSize();
			default:
				assert(false); return {};
			}
		}
		void OnWindowActive(bool active)
		{
			switch (m_model_type)
			{
			case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
				m_model_efs.OnWindowActive(active);
				break;
			case PresentationModelType::LegacyWindow:
				m_model_blt.OnWindowActive(active);
				break;
			case PresentationModelType::Window:
				m_model_flip.OnWindowActive(active);
				break;
			case PresentationModelType::DirectComposition:
				m_model_dwm.OnWindowActive(active);
				break;
			default:
				assert(false);
				break;
			}
		}
		void OnWindowFullscreenStateChange(bool fullscreen)
		{
			switch (m_model_type)
			{
			case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
				m_model_efs.OnWindowFullscreenStateChange(fullscreen);
				break;
			case PresentationModelType::LegacyWindow:
				m_model_blt.OnWindowFullscreenStateChange(fullscreen);
				break;
			case PresentationModelType::Window:
				m_model_flip.OnWindowFullscreenStateChange(fullscreen);
				break;
			case PresentationModelType::DirectComposition:
				m_model_dwm.OnWindowFullscreenStateChange(fullscreen);
				break;
			default:
				assert(false);
				break;
			}
		}
		bool SetCanvasSize(Vector2U size)
		{
			switch (m_model_type)
			{
			case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
				return m_model_efs.SetCanvasSize(size);
			case PresentationModelType::LegacyWindow:
				return m_model_blt.SetCanvasSize(size);
			case PresentationModelType::Window:
				return m_model_flip.SetCanvasSize(size);
			case PresentationModelType::DirectComposition:
				return m_model_dwm.SetCanvasSize(size);
			default:
				assert(false); return false;
			}
		}
		Vector2U GetCanvasSize()
		{
			switch (m_model_type)
			{
			case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
				return m_model_efs.GetCanvasSize();
			case PresentationModelType::LegacyWindow:
				return m_model_blt.GetCanvasSize();
			case PresentationModelType::Window:
				return m_model_flip.GetCanvasSize();
			case PresentationModelType::DirectComposition:
				return m_model_dwm.GetCanvasSize();
			default:
				assert(false); return {};
			}
		}
		ID3D11RenderTargetView* GetCanvasRenderTargetView()
		{
			switch (m_model_type)
			{
			case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
				return m_model_efs.GetCanvasRenderTargetView();
			case PresentationModelType::LegacyWindow:
				return m_model_blt.GetCanvasRenderTargetView();
			case PresentationModelType::Window:
				return m_model_flip.GetCanvasRenderTargetView();
			case PresentationModelType::DirectComposition:
				return m_model_dwm.GetCanvasRenderTargetView();
			default:
				assert(false); return {};
			}
		}
		ID3D11DepthStencilView* GetCanvasDepthStencilView()
		{
			switch (m_model_type)
			{
			case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
				return m_model_efs.GetCanvasDepthStencilView();
			case PresentationModelType::LegacyWindow:
				return m_model_blt.GetCanvasDepthStencilView();
			case PresentationModelType::Window:
				return m_model_flip.GetCanvasDepthStencilView();
			case PresentationModelType::DirectComposition:
				return m_model_dwm.GetCanvasDepthStencilView();
			default:
				assert(false); return {};
			}
		}
		ID2D1Bitmap1* GetCanvasBitmapTarget()
		{
			switch (m_model_type)
			{
			case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
				return m_model_efs.GetCanvasBitmapTarget();
			case PresentationModelType::LegacyWindow:
				return m_model_blt.GetCanvasBitmapTarget();
			case PresentationModelType::Window:
				return m_model_flip.GetCanvasBitmapTarget();
			case PresentationModelType::DirectComposition:
				return m_model_dwm.GetCanvasBitmapTarget();
			default:
				assert(false); return {};
			}
		}
		bool Present(bool vsync)
		{
			switch (m_model_type)
			{
			case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
				return m_model_efs.Present(vsync);
			case PresentationModelType::LegacyWindow:
				return m_model_blt.Present(vsync);
			case PresentationModelType::Window:
				return m_model_flip.Present(vsync);
			case PresentationModelType::DirectComposition:
				return m_model_dwm.Present(vsync);
			default:
				assert(false); return false;
			}
		}
	public:
		bool SetModelType(PresentationModelType type)
		{
			switch (m_model_type)
			{
			case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
				m_model_efs.DetachDevice();
				break;
			case PresentationModelType::LegacyWindow:
				m_model_blt.DetachDevice();
				break;
			case PresentationModelType::Window:
				m_model_flip.DetachDevice();
				break;
			case PresentationModelType::DirectComposition:
				m_model_dwm.DetachDevice();
				break;
			default:
				assert(false);
				return false;
			}
			m_model_type = type;
			if (win32_window && d3d11_device)
			{
				switch (m_model_type)
				{
				case PresentationModelType::LegacyWindowAndExclusiveFullscreen:
					return m_model_efs.AttachDevice(win32_window, d3d11_device.Get(), d2d1_device_context.Get());
				case PresentationModelType::LegacyWindow:
					return m_model_blt.AttachDevice(win32_window, d3d11_device.Get(), d2d1_device_context.Get());
				case PresentationModelType::Window:
					return m_model_flip.AttachDevice(win32_window, d3d11_device.Get(), d2d1_device_context.Get());
				case PresentationModelType::DirectComposition:
					return m_model_dwm.AttachDevice(win32_window, d3d11_device.Get(), d2d1_device_context.Get());
				default:
					assert(false); return false;
				}
			}
			return true;
		}
		PresentationModelType GetModelType() { return m_model_type; }
	public:
		PresentationModelManager() = default;
		PresentationModelManager(Vector2U size)
			: m_model_efs(size)
			, m_model_blt(size)
			, m_model_flip(size)
			, m_model_dwm(size)
		{
		}
		~PresentationModelManager() = default;
	};

	bool CreatePresentationModel(IPresentationModel** pp_compositor, int type)
	{
		try
		{
			assert(pp_compositor);
			assert(type >= 0 && type < 4);
			switch (type)
			{
			case 0:
				*pp_compositor = new LegacyExclusiveFullscreenPresentationModel();
				break;
			case 1:
				*pp_compositor = new LegacyWindowPresentationModel();
				break;
			case 2:
				*pp_compositor = new WindowPresentationModel();
				break;
			case 3:
				*pp_compositor = new DirectCompositionPresentationModel();
				break;
			default:
				assert(false);
				return false;
			}
			return true;
		}
		catch (std::exception const&)
		{
			assert(false);
			return false;
		}
	}
	bool CreatePresentationModelWithCanvasSize(IPresentationModel** pp_compositor, int type, Vector2U size)
	{
		try
		{
			assert(pp_compositor);
			assert(type >= 0 && type < 4);
			switch (type)
			{
			case 0:
				*pp_compositor = new LegacyExclusiveFullscreenPresentationModel(size);
				break;
			case 1:
				*pp_compositor = new LegacyWindowPresentationModel(size);
				break;
			case 2:
				*pp_compositor = new WindowPresentationModel(size);
				break;
			case 3:
				*pp_compositor = new DirectCompositionPresentationModel(size);
				break;
			default:
				assert(false);
				return false;
			}
			return true;
		}
		catch (std::exception const&)
		{
			assert(false);
			return false;
		}
	}

	bool CreatePresentationModelManager(IPresentationModelManager** pp_manager)
	{
		try
		{
			assert(pp_manager);
			*pp_manager = new PresentationModelManager();
			return true;
		}
		catch (std::exception const&)
		{
			assert(false);
			return false;
		}
	}
	bool CreatePresentationModelManagerWithCanvasSize(Vector2U size, IPresentationModelManager** pp_manager)
	{
		try
		{
			assert(pp_manager);
			assert(size.x > 0 && size.y > 0);
			*pp_manager = new PresentationModelManager(size);
			return true;
		}
		catch (std::exception const&)
		{
			assert(false);
			return false;
		}
	}
}
