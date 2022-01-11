#include "Core/Renderer.hpp"
#include "Backend/Direct3D9/Shader.hpp"
#include "Backend/framework.hpp"

#define IDX(x) (size_t)static_cast<uint8_t>(x)

namespace LuaSTG::Core
{
	bool compileShaderMacro(char const* name, void const* data, size_t size, char const* target, const D3D_SHADER_MACRO* defs, ID3DBlob** ppBlob)
	{
		UINT flag_ = D3DCOMPILE_ENABLE_STRICTNESS;
	#ifdef _DEBUG
		flag_ |= D3DCOMPILE_DEBUG;
		flag_ |= D3DCOMPILE_SKIP_OPTIMIZATION;
	#endif
		Microsoft::WRL::ComPtr<ID3DBlob> errmsg_;
		if (FAILED(D3DCompile(data, size, name, defs, NULL, "main", target, flag_, 0, ppBlob, &errmsg_)))
		{
			OutputDebugStringA(name);
			OutputDebugStringA(":\n");
			OutputDebugStringA((char*)errmsg_->GetBufferPointer());
			assert(false);
			return false;
		}
		return true;
	}
	bool compileVertexShaderMacro(char const* name, void const* data, size_t size, const D3D_SHADER_MACRO* defs, ID3DBlob** ppBlob)
	{
		return compileShaderMacro(name, data, size, "vs_3_0", defs, ppBlob);
	}
	bool compilePixelShaderMacro(char const* name, void const* data, size_t size, const D3D_SHADER_MACRO* defs, ID3DBlob** ppBlob)
	{
		return compileShaderMacro(name, data, size, "ps_3_0", defs, ppBlob);
	}
	bool compileShader(char const* name, void const* data, size_t size, char const* target, ID3DBlob** ppBlob)
	{
		return compileShaderMacro(name, data, size, target, NULL, ppBlob);
	}
	bool compileVertexShader(char const* name, void const* data, size_t size, ID3DBlob** ppBlob)
	{
		return compileShader(name, data, size, "vs_3_0", ppBlob);
	}
	bool compilePixelShader(char const* name, void const* data, size_t size, ID3DBlob** ppBlob)
	{
		return compileShader(name, data, size, "ps_3_0", ppBlob);
	}

	class Renderer::RendererImpl
	{
	private:
		Microsoft::WRL::ComPtr<IDirect3DDevice9Ex> _device;
		Microsoft::WRL::ComPtr<IDirect3DVertexDeclaration9> _input_layout;
		Microsoft::WRL::ComPtr<IDirect3DVertexBuffer9> _vertex_buffer;
		Microsoft::WRL::ComPtr<IDirect3DIndexBuffer9> _index_buffer;
		Microsoft::WRL::ComPtr<IDirect3DVertexShader9> _vertex_shader[4]; // FogState
		Microsoft::WRL::ComPtr<IDirect3DPixelShader9> _pixel_shader[4][4][2]; // VertexColorBlendState, FogState, TextureAlphaType
		Microsoft::WRL::ComPtr<IDirect3DStateBlock9> _state_backup;
		DrawList _draw_list;
		Box _viewport = {};
		Rect _scissor_rect = {};
		float _fog_range[2] = {};
		Color _fog_color = {};
		bool _state_dirty = false;
		VertexColorBlendState _vertex_color_blend_state = VertexColorBlendState::Mul;
		SamplerState _sampler_state = SamplerState::LinearClamp;
		FogState _fog_state = FogState::Disable;
		TextureAlphaType _texture_alpha_type = TextureAlphaType::Normal;
		DepthState _depth_state = DepthState::Disable;
		BlendState _blend_state = BlendState::Alpha;
	private:
		void releaseTexture()
		{
			for (size_t j_ = 0; j_ < _draw_list.command.size; j_ += 1)
			{
				DrawCommand& cmd_ = _draw_list.command.data[j_];
				if (cmd_.texture.handle)
				{
					((IDirect3DTexture9*)(cmd_.texture.handle))->Release();
				}
			}
		}
		bool batchFlush(bool discard = false)
		{
			if (!discard)
			{
				// copy vertex data
				if (_draw_list.vertex.size > 0)
				{
					void* p_vertex_data_ = NULL;
					const size_t p_vertex_data_size_ = _draw_list.vertex.size * sizeof(DrawVertex2D);
					if (FAILED(_vertex_buffer->Lock(0, (UINT)p_vertex_data_size_, &p_vertex_data_, D3DLOCK_DISCARD)))
						return false;
					CopyMemory(p_vertex_data_, _draw_list.vertex.data, p_vertex_data_size_);
					if (FAILED(_vertex_buffer->Unlock()))
						return false;
				}
				// copy index data
				if (_draw_list.index.size > 0)
				{
					void* p_index_data_ = NULL;
					const size_t p_index_data_size_ = _draw_list.index.size * sizeof(DrawIndex2D);
					if (FAILED(_index_buffer->Lock(0, (UINT)p_index_data_size_, &p_index_data_, D3DLOCK_DISCARD)))
						return false;
					CopyMemory(p_index_data_, _draw_list.index.data, p_index_data_size_);
					if (FAILED(_index_buffer->Unlock()))
						return false;
				}
				// draw
				if (_draw_list.command.size > 0)
				{
					INT vertex_offset_ = 0;
					UINT index_offset_ = 0;
					for (size_t j_ = 0; j_ < _draw_list.command.size; j_ += 1)
					{
						const DrawCommand& cmd_ = _draw_list.command.data[j_];
						if (cmd_.vertex_count > 0 && cmd_.index_count > 0)
						{
							_device->SetTexture(0, (IDirect3DTexture9*)cmd_.texture.handle);
							if (FAILED(_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vertex_offset_, 0, cmd_.vertex_count, index_offset_, cmd_.index_count / 3)))
								return false;
							// QUESTION: will vertex_count != index_count happen?
							vertex_offset_ += cmd_.vertex_count;
							index_offset_ += cmd_.index_count;
						}
					}
				}
			}
			// clear
			releaseTexture();
			_draw_list.vertex.size = 0;
			_draw_list.index.size = 0;
			_draw_list.command.size = 0;
			return true;
		}
	public:
		bool attachDevice(void* dev)
		{
			if (dev == nullptr)
				return false;
			_device = static_cast<IDirect3DDevice9Ex*>(dev);

			D3DVERTEXELEMENT9 const input_layout_desc_[4] = {
				{ 0,  0, D3DDECLTYPE_FLOAT3  , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
				{ 0, 12, D3DDECLTYPE_FLOAT2  , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
				{ 0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR   , 0 },
				D3DDECL_END(),
			};
			if (FAILED(_device->CreateVertexDeclaration(input_layout_desc_, &_input_layout)))
				return false;

			const DWORD usage_ = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
			if (FAILED(_device->CreateVertexBuffer(65536 * sizeof(DrawVertex2D), usage_, 0, D3DPOOL_DEFAULT, &_vertex_buffer, NULL)))
				return false;
			if (FAILED(_device->CreateIndexBuffer(65536 * sizeof(DrawIndex2D), usage_, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &_index_buffer, NULL)))
				return false;

			{
				const D3D_SHADER_MACRO vs_def_fog0_[] = {
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO vs_def_fog1_[] = {
					{ "FOG_ENABLE", "1"},
					{ NULL, NULL },
				};

				Microsoft::WRL::ComPtr<ID3DBlob> vs_blob_;

				if (!compileVertexShaderMacro("LVS0", g_VertexShader, sizeof(g_VertexShader), vs_def_fog0_, &vs_blob_))
					return false;
				if (FAILED(_device->CreateVertexShader(static_cast<DWORD*>(vs_blob_->GetBufferPointer()), &_vertex_shader[0])))
					return false;

				if (!compileVertexShaderMacro("LVS1", g_VertexShader, sizeof(g_VertexShader), vs_def_fog1_, &vs_blob_))
					return false;
				if (FAILED(_device->CreateVertexShader(static_cast<DWORD*>(vs_blob_->GetBufferPointer()), &_vertex_shader[1])))
					return false;

				_vertex_shader[2] = _vertex_shader[1];
				_vertex_shader[3] = _vertex_shader[1];
			}
			{
				Microsoft::WRL::ComPtr<ID3DBlob> ps_blob_;

				const D3D_SHADER_MACRO ps_def_zero_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};

				if (!compilePixelShaderMacro("LPS000", g_PixelShader, sizeof(g_PixelShader), ps_def_zero_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[0][0][0])))
					return false;

				if (!compilePixelShaderMacro("LPS100", g_PixelShader, sizeof(g_PixelShader), ps_def_one0_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[1][0][0])))
					return false;

				if (!compilePixelShaderMacro("LPS200", g_PixelShader, sizeof(g_PixelShader), ps_def_add0_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[2][0][0])))
					return false;

				if (!compilePixelShaderMacro("LPS300", g_PixelShader, sizeof(g_PixelShader), ps_def_mul0_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[3][0][0])))
					return false;

				const D3D_SHADER_MACRO ps_def_zero_line_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_line_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_line_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_line_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};

				if (!compilePixelShaderMacro("LPS010", g_PixelShader, sizeof(g_PixelShader), ps_def_zero_line_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[0][1][0])))
					return false;

				if (!compilePixelShaderMacro("LPS110", g_PixelShader, sizeof(g_PixelShader), ps_def_one0_line_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[1][1][0])))
					return false;

				if (!compilePixelShaderMacro("LPS210", g_PixelShader, sizeof(g_PixelShader), ps_def_add0_line_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[2][1][0])))
					return false;

				if (!compilePixelShaderMacro("LPS310", g_PixelShader, sizeof(g_PixelShader), ps_def_mul0_line_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[3][1][0])))
					return false;

				const D3D_SHADER_MACRO ps_def_zero_exp0_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_exp0_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_exp0_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_exp0_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};

				if (!compilePixelShaderMacro("LPS020", g_PixelShader, sizeof(g_PixelShader), ps_def_zero_exp0_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[0][2][0])))
					return false;

				if (!compilePixelShaderMacro("LPS120", g_PixelShader, sizeof(g_PixelShader), ps_def_one0_exp0_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[1][2][0])))
					return false;

				if (!compilePixelShaderMacro("LPS220", g_PixelShader, sizeof(g_PixelShader), ps_def_add0_exp0_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[2][2][0])))
					return false;

				if (!compilePixelShaderMacro("LPS320", g_PixelShader, sizeof(g_PixelShader), ps_def_mul0_exp0_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[3][2][0])))
					return false;

				const D3D_SHADER_MACRO ps_def_zero_exp2_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_exp2_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_exp2_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_exp2_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};

				if (!compilePixelShaderMacro("LPS030", g_PixelShader, sizeof(g_PixelShader), ps_def_zero_exp2_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[0][3][0])))
					return false;

				if (!compilePixelShaderMacro("LPS130", g_PixelShader, sizeof(g_PixelShader), ps_def_one0_exp2_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[1][3][0])))
					return false;

				if (!compilePixelShaderMacro("LPS230", g_PixelShader, sizeof(g_PixelShader), ps_def_add0_exp2_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[2][3][0])))
					return false;

				if (!compilePixelShaderMacro("LPS330", g_PixelShader, sizeof(g_PixelShader), ps_def_mul0_exp2_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[3][3][0])))
					return false;

				const D3D_SHADER_MACRO ps_def_zero_nfog_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_nfog_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_nfog_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_nfog_mula_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ NULL, NULL },
				};

				if (!compilePixelShaderMacro("LPS001", g_PixelShader, sizeof(g_PixelShader), ps_def_zero_nfog_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[0][0][1])))
					return false;

				if (!compilePixelShaderMacro("LPS101", g_PixelShader, sizeof(g_PixelShader), ps_def_one0_nfog_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[1][0][1])))
					return false;

				if (!compilePixelShaderMacro("LPS201", g_PixelShader, sizeof(g_PixelShader), ps_def_add0_nfog_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[2][0][1])))
					return false;

				if (!compilePixelShaderMacro("LPS301", g_PixelShader, sizeof(g_PixelShader), ps_def_mul0_nfog_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[3][0][1])))
					return false;

				const D3D_SHADER_MACRO ps_def_zero_line_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_line_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_line_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_line_mula_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ NULL, NULL },
				};

				if (!compilePixelShaderMacro("LPS011", g_PixelShader, sizeof(g_PixelShader), ps_def_zero_line_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[0][1][1])))
					return false;

				if (!compilePixelShaderMacro("LPS111", g_PixelShader, sizeof(g_PixelShader), ps_def_one0_line_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[1][1][1])))
					return false;

				if (!compilePixelShaderMacro("LPS211", g_PixelShader, sizeof(g_PixelShader), ps_def_add0_line_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[2][1][1])))
					return false;

				if (!compilePixelShaderMacro("LPS311", g_PixelShader, sizeof(g_PixelShader), ps_def_mul0_line_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[3][1][1])))
					return false;

				const D3D_SHADER_MACRO ps_def_zero_exp0_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_exp0_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_exp0_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_exp0_mula_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ NULL, NULL },
				};

				if (!compilePixelShaderMacro("LPS021", g_PixelShader, sizeof(g_PixelShader), ps_def_zero_exp0_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[0][2][1])))
					return false;

				if (!compilePixelShaderMacro("LPS121", g_PixelShader, sizeof(g_PixelShader), ps_def_one0_exp0_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[1][2][1])))
					return false;

				if (!compilePixelShaderMacro("LPS221", g_PixelShader, sizeof(g_PixelShader), ps_def_add0_exp0_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[2][2][1])))
					return false;

				if (!compilePixelShaderMacro("LPS321", g_PixelShader, sizeof(g_PixelShader), ps_def_mul0_exp0_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[3][2][1])))
					return false;

				const D3D_SHADER_MACRO ps_def_zero_exp2_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_exp2_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_exp2_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_exp2_mula_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ NULL, NULL },
				};

				if (!compilePixelShaderMacro("LPS031", g_PixelShader, sizeof(g_PixelShader), ps_def_zero_exp2_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[0][3][1])))
					return false;

				if (!compilePixelShaderMacro("LPS131", g_PixelShader, sizeof(g_PixelShader), ps_def_one0_exp2_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[1][3][1])))
					return false;

				if (!compilePixelShaderMacro("LPS231", g_PixelShader, sizeof(g_PixelShader), ps_def_add0_exp2_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[2][3][1])))
					return false;

				if (!compilePixelShaderMacro("LPS331", g_PixelShader, sizeof(g_PixelShader), ps_def_mul0_exp2_mula_, &ps_blob_))
					return false;
				if (FAILED(_device->CreatePixelShader(static_cast<DWORD*>(ps_blob_->GetBufferPointer()), &_pixel_shader[3][3][1])))
					return false;
			}

			return true;
		}
		void detachDevice()
		{
			batchFlush(true);
			_device.Reset();
			_input_layout.Reset();
			_vertex_buffer.Reset();
			_index_buffer.Reset();
			for (auto& v : _vertex_shader)
			{
				v.Reset();
			}
			for (auto& i : _pixel_shader)
			{
				for (auto& j : i)
				{
					for (auto& v : j)
					{
						v.Reset();
					}
				}
			}
			_state_backup.Reset();
		}

		bool beginScene()
		{
			if (FAILED(_device->CreateStateBlock(D3DSBT_ALL, &_state_backup)))
				return false;
			if (FAILED(_state_backup->Capture()))
			{
				_state_backup.Reset();
				return false;
			}
			if (FAILED(_device->BeginScene()))
				return false;
			
			IDirect3DDevice9Ex* ctx = _device.Get();
			
			// [IA Stage]

			ctx->SetVertexDeclaration(_input_layout.Get());
			ctx->SetStreamSource(0, _vertex_buffer.Get(), 0, sizeof(DrawVertex2D));
			ctx->SetStreamSourceFreq(0, 1);
			ctx->SetIndices(_index_buffer.Get());

			// [RS Stage]

			// rastrizer state
			ctx->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
			ctx->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			ctx->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
			ctx->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
			ctx->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);

			// [PS State]

			// sampler state
			ctx->SetSamplerState(0, D3DSAMP_BORDERCOLOR, 0x00000000);

			// [OM Stage]

			// depth stencil state
			ctx->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			ctx->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			ctx->SetRenderState(D3DRS_STENCILENABLE, FALSE);
			// blend state
			ctx->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			ctx->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
			ctx->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

			// [Fixed Pipeline] disable these features, especially lighting

			ctx->SetRenderState(D3DRS_CLIPPING, FALSE);
			ctx->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
			ctx->SetRenderState(D3DRS_LIGHTING, FALSE);
			ctx->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
			ctx->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
			ctx->SetRenderState(D3DRS_FOGENABLE, FALSE);
			ctx->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
			ctx->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			
			_state_dirty = true;

			setViewport(_viewport);
			setScissorRect(_scissor_rect);

			setVertexColorBlendState(_vertex_color_blend_state);
			setSamplerState(_sampler_state);
			setFogState(_fog_state, _fog_color, _fog_range[0], _fog_range[1]);
			setDepthState(_depth_state);
			setBlendState(_blend_state);
			setTexture(TextureID());

			_state_dirty = false;

			return true;
		}
		bool endScene()
		{
			if (!batchFlush())
				return false;
			if (FAILED(_device->EndScene()))
				return false;
			if (_state_backup)
			{
				_state_backup->Apply();
				_state_backup.Reset();
			}
			return true;
		}

		void clearRenderTarget(Color const& color)
		{
			batchFlush();
			_device->Clear(0, NULL, D3DCLEAR_TARGET, (D3DCOLOR)color.color, 0.0f, 0);
		}
		void clearDepthBuffer(float zvalue)
		{
			batchFlush();
			_device->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, zvalue, 0);
		}

		void setOrtho(Box const& box)
		{
			batchFlush();
			DirectX::XMFLOAT4X4 f4x4;
			DirectX::XMStoreFloat4x4(&f4x4, DirectX::XMMatrixOrthographicOffCenterLH(box.left, box.right, box.bottom, box.top, box.front, box.back));
			_device->SetVertexShaderConstantF(0, (float*)&f4x4, 4);
		}
		void setPerspective(Vector3 const& eye, Vector3 const& lookat, Vector3 const& headup, float fov, float aspect, float znear, float zfar)
		{
			batchFlush();
			DirectX::XMFLOAT3 const eyef3(eye.x, eye.y, eye.z);
			DirectX::XMFLOAT3 const lookatf3(lookat.x, lookat.y, lookat.z);
			DirectX::XMFLOAT3 const headupf3(headup.x, headup.y, headup.z);
			DirectX::XMFLOAT4X4 f4x4;
			DirectX::XMStoreFloat4x4(&f4x4,
				DirectX::XMMatrixMultiply(
					DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eyef3), DirectX::XMLoadFloat3(&lookatf3), DirectX::XMLoadFloat3(&headupf3)),
					DirectX::XMMatrixPerspectiveFovLH(fov, aspect, znear, zfar)));
			_device->SetVertexShaderConstantF(0, (float*)&f4x4, 4);
			float const camera_pos[4] = { eye.x, eye.y, eye.z, 0.0f };
			_device->SetPixelShaderConstantF(0, camera_pos, 1);
		}

		void setViewport(Box const& box)
		{
			if (_state_dirty || _viewport != box)
			{
				_viewport = box;
				batchFlush();
				D3DVIEWPORT9 const vp = {
					.X = (DWORD)(LONG)(box.left),
					.Y = (DWORD)(LONG)(box.top),
					.Width = (DWORD)(LONG)(box.right - box.left),
					.Height = (DWORD)(LONG)(box.bottom - box.top),
					.MinZ = box.front,
					.MaxZ = box.back,
				};
				_device->SetViewport(&vp);
			}
		}
		void setScissorRect(Rect const& rect)
		{
			if (_state_dirty || _scissor_rect != rect)
			{
				_scissor_rect = rect;
				batchFlush();
				RECT const rc = {
					.left = (LONG)rect.left,
					.top = (LONG)rect.top,
					.right = (LONG)rect.right,
					.bottom = (LONG)rect.bottom,
				};
				_device->SetScissorRect(&rc);
			}
		}
		
		void setVertexColorBlendState(VertexColorBlendState state)
		{
			if (_state_dirty || _vertex_color_blend_state != state)
			{
				_vertex_color_blend_state = state;
				batchFlush();
				_device->SetPixelShader(_pixel_shader[IDX(_vertex_color_blend_state)][IDX(_fog_state)][IDX(_texture_alpha_type)].Get());
			}
		}
		void setSamplerState(SamplerState state)
		{
			if (_state_dirty || _sampler_state != state)
			{
				_sampler_state = state;
				batchFlush();
				IDirect3DDevice9Ex* ctx = _device.Get();
				switch (state)
				{
				case SamplerState::PointWrap:
					ctx->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
					ctx->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
					ctx->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
					ctx->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
					ctx->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
					ctx->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
					break;
				case SamplerState::PointClamp:
					ctx->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
					ctx->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
					ctx->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
					ctx->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
					ctx->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
					ctx->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
					break;
				case SamplerState::LinearWrap:
					ctx->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
					ctx->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
					ctx->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
					ctx->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
					ctx->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
					ctx->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
					break;
				case SamplerState::LinearClamp:
				default:
					ctx->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
					ctx->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
					ctx->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
					ctx->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
					ctx->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
					ctx->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
					break;
				}
			}
		}
		void setFogState(FogState state, Color const& color, float density_or_znear, float zfar)
		{
			if (_state_dirty || _fog_state != state || _fog_color != color || _fog_range[0] != density_or_znear || _fog_range[1] != zfar)
			{
				_fog_state = state;
				_fog_color = color;
				_fog_range[0] = density_or_znear;
				_fog_range[1] = zfar;
				batchFlush();
				_device->SetVertexShader(_vertex_shader[IDX(_fog_state)].Get());
				float const fog_color_and_range[8] = {
					(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f, (float)color.a / 255.0f,
					density_or_znear, zfar, 0.0f, zfar - density_or_znear,
				};
				_device->SetPixelShaderConstantF(1, fog_color_and_range, 2);
				_device->SetPixelShader(_pixel_shader[IDX(_vertex_color_blend_state)][IDX(_fog_state)][IDX(_texture_alpha_type)].Get());
			}
		}
		void setDepthState(DepthState state)
		{
			if (_state_dirty || _depth_state != state)
			{
				_depth_state = state;
				batchFlush();
				IDirect3DDevice9Ex* ctx = _device.Get();
				switch (state)
				{
				default:
				case DepthState::Disable:
					ctx->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
					break;
				case DepthState::Enable:
					ctx->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
					break;
				}
			}
		}
		void setBlendState(BlendState state)
		{
			if (_state_dirty || _blend_state != state)
			{
				_blend_state = state;
				batchFlush();
				IDirect3DDevice9Ex* ctx = _device.Get();
				switch (state)
				{
				default:
				case BlendState::Disable:
					ctx->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
					break;
				case BlendState::Alpha:
					ctx->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					ctx->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
					ctx->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
					ctx->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
					ctx->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
					break;
				case BlendState::One:
					ctx->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					ctx->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
					ctx->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
					ctx->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
					ctx->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
					break;
				case BlendState::Min:
					ctx->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					ctx->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MIN);
					ctx->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_MIN);
					ctx->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
					break;
				case BlendState::Max:
					ctx->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					ctx->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MAX);
					ctx->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_MAX);
					ctx->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
					break;
				case BlendState::Mul:
					ctx->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					ctx->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
					ctx->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
					ctx->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
					ctx->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
					ctx->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ZERO);
					ctx->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
					break;
				case BlendState::Screen:
					ctx->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					ctx->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
					ctx->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
					ctx->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
					ctx->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
					break;
				case BlendState::Add:
					ctx->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					ctx->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
					ctx->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
					ctx->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
					break;
				case BlendState::Sub:
					ctx->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					ctx->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_SUBTRACT);
					ctx->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_SUBTRACT);
					ctx->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
					break;
				case BlendState::RevSub:
					ctx->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					ctx->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
					ctx->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_REVSUBTRACT);
					ctx->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
					ctx->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
					break;
				case BlendState::Inv:
					ctx->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					ctx->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
					ctx->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
					ctx->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
					ctx->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
					ctx->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ZERO);
					ctx->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
					break;
				}
			}
		}
		void setTexture(TextureID texture)
		{
			if (_draw_list.command.size > 0 && _draw_list.command.data[_draw_list.command.size - 1].texture == texture)
			{
				// no op
			}
			else
			{
				// new command
				if ((_draw_list.command.capacity - _draw_list.command.size) < 1)
				{
					batchFlush(); // no space
				}
				_draw_list.command.size += 1;
				DrawCommand& cmd_ = _draw_list.command.data[_draw_list.command.size - 1];
				cmd_.texture = texture;
				cmd_.vertex_count = 0;
				cmd_.index_count = 0;
				if (texture.handle)
				{
					((IDirect3DTexture9*)(texture.handle))->AddRef();
				}
			}
		}
		
		bool flush()
		{
			return batchFlush();
		}
		void drawTriangle(DrawVertex2D const& v1, DrawVertex2D const& v2, DrawVertex2D const& v3)
		{
			if ((_draw_list.vertex.capacity - _draw_list.vertex.size) < 3 || (_draw_list.index.capacity - _draw_list.index.size) < 3)
			{
				batchFlush();
			}
			DrawCommand& cmd_ = _draw_list.command.data[_draw_list.command.size - 1];
			DrawVertex2D* vbuf_ = _draw_list.vertex.data + _draw_list.vertex.size;
			vbuf_[0] = v1;
			vbuf_[1] = v2;
			vbuf_[2] = v3;
			_draw_list.vertex.size += 3;
			DrawIndex2D* ibuf_ = _draw_list.index.data + _draw_list.index.size;
			ibuf_[0] = cmd_.vertex_count;
			ibuf_[1] = cmd_.vertex_count + 1;
			ibuf_[2] = cmd_.vertex_count + 2;
			_draw_list.index.size += 3;
			cmd_.vertex_count += 3;
			cmd_.index_count += 3;
			
		}
		void drawQuad(DrawVertex2D const& v1, DrawVertex2D const& v2, DrawVertex2D const& v3, DrawVertex2D const& v4)
		{
			if ((_draw_list.vertex.capacity - _draw_list.vertex.size) < 4 || (_draw_list.index.capacity - _draw_list.index.size) < 6)
			{
				batchFlush();
			}
			DrawCommand& cmd_ = _draw_list.command.data[_draw_list.command.size - 1];
			DrawVertex2D* vbuf_ = _draw_list.vertex.data + _draw_list.vertex.size;
			vbuf_[0] = v1;
			vbuf_[1] = v2;
			vbuf_[2] = v3;
			vbuf_[3] = v4;
			_draw_list.vertex.size += 4;
			DrawIndex2D* ibuf_ = _draw_list.index.data + _draw_list.index.size;
			ibuf_[0] = cmd_.vertex_count;
			ibuf_[1] = cmd_.vertex_count + 1;
			ibuf_[2] = cmd_.vertex_count + 2;
			ibuf_[3] = cmd_.vertex_count;
			ibuf_[4] = cmd_.vertex_count + 2;
			ibuf_[5] = cmd_.vertex_count + 3;
			_draw_list.index.size += 6;
			cmd_.vertex_count += 4;
			cmd_.index_count += 6;
		}
	};
}

namespace LuaSTG::Core
{
	bool Renderer::attachDevice(void* dev)
	{
		return _pImpl->attachDevice(dev);
	}
	void Renderer::detachDevice()
	{
		_pImpl->detachDevice();
	}
	
	bool Renderer::beginScene()
	{
		return _pImpl->beginScene();
	}
	bool Renderer::endScene()
	{
		return _pImpl->endScene();
	}

	void Renderer::clearRenderTarget(Color const& color)
	{
		_pImpl->clearRenderTarget(color);
	}
	void Renderer::clearDepthBuffer(float zvalue)
	{
		_pImpl->clearDepthBuffer(zvalue);
	}

	void Renderer::setOrtho(Box const& box)
	{
		_pImpl->setOrtho(box);
	}
	void Renderer::setPerspective(Vector3 const& eye, Vector3 const& lookat, Vector3 const& headup, float fov, float aspect, float znear, float zfar)
	{
		_pImpl->setPerspective(eye, lookat, headup, fov, aspect, znear, zfar);
	}

	void Renderer::setViewport(Box const& box)
	{
		_pImpl->setViewport(box);
	}
	void Renderer::setScissorRect(Rect const& rect)
	{
		_pImpl->setScissorRect(rect);
	}

	void Renderer::setVertexColorBlendState(VertexColorBlendState state)
	{
		_pImpl->setVertexColorBlendState(state);
	}
	void Renderer::setSamplerState(SamplerState state)
	{
		_pImpl->setSamplerState(state);
	}
	void Renderer::setFogState(FogState state, Color const& color, float density_or_znear, float zfar)
	{
		_pImpl->setFogState(state, color, density_or_znear, zfar);
	}
	void Renderer::setDepthState(DepthState state)
	{
		_pImpl->setDepthState(state);
	}
	void Renderer::setBlendState(BlendState state)
	{
		_pImpl->setBlendState(state);
	}
	void Renderer::setTexture(TextureID texture)
	{
		_pImpl->setTexture(texture);
	}

	bool Renderer::flush()
	{
		return _pImpl->flush();
	}
	void Renderer::drawTriangle(DrawVertex2D const& v1, DrawVertex2D const& v2, DrawVertex2D const& v3)
	{
		_pImpl->drawTriangle(v1, v2, v3);
	}
	void Renderer::drawQuad(DrawVertex2D const& v1, DrawVertex2D const& v2, DrawVertex2D const& v3, DrawVertex2D const& v4)
	{
		_pImpl->drawQuad(v1, v2, v3, v4);
	}

	Renderer::Renderer() : _pImpl(nullptr)
	{
		_pImpl = new RendererImpl;
	}
	Renderer::Renderer(Renderer&& right) noexcept : _pImpl(nullptr)
	{
		std::swap(_pImpl, right._pImpl);
	}
	Renderer::~Renderer()
	{
		delete _pImpl;
	}
}
