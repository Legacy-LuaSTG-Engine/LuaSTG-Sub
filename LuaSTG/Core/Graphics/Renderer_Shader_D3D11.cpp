#include "Core/Graphics/Renderer_D3D11.hpp"
#include "Core/FileManager.hpp"

static char const g_VertexShader11[] = R"(

cbuffer view_proj_buffer : register(b0)
{
    float4x4 view_proj;
};
#if defined(WORLD_MATRIX)
cbuffer world_buffer : register(b1)
{
    float4x4 world;
};
#endif

struct VS_Input
{
    float3 pos : POSITION0;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};
struct VS_Output
{
    float4 sxy : SV_POSITION;
#if defined(FOG_ENABLE)
    float4 pos : POSITION0;
#endif
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

VS_Output main(VS_Input input)
{
    float4 pos_world = float4(input.pos, 1.0f);
#if defined(WORLD_MATRIX)
    pos_world = mul(world, pos_world);
#endif

    VS_Output output;
    output.sxy = mul(view_proj, pos_world);
#if defined(FOG_ENABLE)
    output.pos = pos_world;
#endif
    output.uv = input.uv;
    output.col = input.col;

    return output;
};

)";

static char const g_PixelShader11[] = R"(

#if defined(FOG_ENABLE)
cbuffer camera_data : register(b0)
{
    float4 camera_pos;
};
cbuffer fog_data : register(b1)
{
    float4 fog_color;
    float4 fog_range;
};
#endif

#if !defined(VERTEX_COLOR_BLEND_ONE)
Texture2D    texture0 : register(t0);
SamplerState sampler0 : register(s0);
#endif

float channel_minimum = 1.0f / 255.0f;

struct PS_Input
{
    float4 sxy : SV_POSITION;
#if defined(FOG_ENABLE)
    float4 pos : POSITION0;
#endif
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

struct PS_Output
{
    float4 col : SV_TARGET;
};

PS_Output main(PS_Input input)
{
    // 对纹理进行采样
    #if !defined(VERTEX_COLOR_BLEND_ONE)
        float4 color = texture0.Sample(sampler0, input.uv);
    #else
        float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    #endif

    // 顶点色混合，顶点色是直通颜色（没有预乘 alpha）；最终的结果是预乘 alpha 的
    #if defined(VERTEX_COLOR_BLEND_MUL)
        color = color * input.col;
        #if !defined(PREMUL_ALPHA)
            color.rgb *= color.a;
        #else // PREMUL_ALPHA
            color.rgb *= input.col.a; // 需要少乘以纹理色的 alpha
        #endif
    #elif defined(VERTEX_COLOR_BLEND_ADD)
        #if !defined(PREMUL_ALPHA)
            // 已经是直通颜色
        #else // PREMUL_ALPHA
            // 先解除预乘 alpha
            if (color.a < channel_minimum)
            {
                discard; // 这里原本是完全透明的，应该舍弃
            }
            color.rgb /= color.a;
        #endif
        color.rgb += input.col.rgb;
        color.r = min(color.r, 1.0f);
        color.g = min(color.g, 1.0f);
        color.b = min(color.b, 1.0f);
        color.a *= input.col.a;
        color.rgb *= color.a;
    #elif defined(VERTEX_COLOR_BLEND_ONE)
        color = input.col;
        #if !defined(PREMUL_ALPHA)
            color.rgb *= color.a;
        #else // PREMUL_ALPHA
            // 已经乘过 alpha
        #endif
    #else // VERTEX_COLOR_BLEND_ZERO
        // color = color;
        #if !defined(PREMUL_ALPHA)
            color.rgb *= color.a;
        #else // PREMUL_ALPHA
            // 已经乘过 alpha
        #endif
    #endif

    // 雾颜色混合，雾颜色是直通颜色（没有预乘 alpha）；最终的结果是预乘 alpha 的
    #if defined(FOG_ENABLE)
        // camera_pos.xyz 和 input.pos.xyz 都是在世界坐标系下的坐标，求得的距离也是基于世界坐标系的
        float dist = distance(camera_pos.xyz, input.pos.xyz);
        #if defined(FOG_EXP)
            // 指数雾，fog_range.x 是雾密度
            float k = clamp(1.0f - exp(-(dist * fog_range.x)), 0.0f, 1.0f);
        #elif defined(FOG_EXP2)
            // 二次指数雾，fog_range.x 是雾密度
            float k = clamp(1.0f - exp(-pow(dist * fog_range.x, 2.0f)), 0.0f, 1.0f);
        #else // FOG_LINEAR
            // 线性雾，fog_range.x 是雾起始距离，fog_range.y 是雾浓度最大处的距离，fog_range.w 是雾范围（fog_range.y - fog_range.x）
            float k = clamp((dist - fog_range.x) / fog_range.w, 0.0f, 1.0f);
        #endif
        // 基于预乘 alpha 的雾颜色混合，原理请见后面
        float k1 = 1.0f - k;
        float alpha = k1 * color.a + k * color.a * fog_color.a;
        float ka = k1 * (k1 + k * fog_color.a);
        float kb = k * alpha;
        color = float4(ka * color.rgb + kb * fog_color.rgb, alpha);
    #endif

    // 填充输出
    PS_Output output;
    output.col = color;

    return output;
}

)";

#define IDX(x) (size_t)static_cast<uint8_t>(x)

namespace LuaSTG::Core::Graphics
{
	static bool compileShaderMacro(char const* name, void const* data, size_t size, char const* target, const D3D_SHADER_MACRO* defs, ID3DBlob** ppBlob)
	{
		UINT flag_ = D3DCOMPILE_ENABLE_STRICTNESS;
	#ifdef _DEBUG
		flag_ |= D3DCOMPILE_DEBUG;
		flag_ |= D3DCOMPILE_SKIP_OPTIMIZATION;
	#endif
		Microsoft::WRL::ComPtr<ID3DBlob> errmsg_;
		HRESULT hr = gHR = D3DCompile(data, size, name, defs, NULL, "main", target, flag_, 0, ppBlob, &errmsg_);
		if (FAILED(hr))
		{
			spdlog::error("[core] D3DCompile 调用失败");
			spdlog::error("[core] 编译着色器 '{}' 失败：{}", name, (char*)errmsg_->GetBufferPointer());
			return false;
		}
		return true;
	}
	static bool compileVertexShaderMacro11(char const* name, void const* data, size_t size, const D3D_SHADER_MACRO* defs, ID3DBlob** ppBlob)
	{
		return compileShaderMacro(name, data, size, "vs_4_0", defs, ppBlob);
	}
	static bool compilePixelShaderMacro11(char const* name, void const* data, size_t size, const D3D_SHADER_MACRO* defs, ID3DBlob** ppBlob)
	{
		return compileShaderMacro(name, data, size, "ps_4_0", defs, ppBlob);
	}

	bool PostEffectShader_D3D11::createResources()
	{
		if (!d3d_ps_blob)
		{
			if (is_path)
			{
				std::vector<uint8_t> src;
				if (!GFileManager().loadEx(source, src))
					return false;
				if (!compilePixelShaderMacro11(source.c_str(), src.data(), src.size(), NULL, &d3d_ps_blob))
					return false;
			}
			else
			{
				if (!compilePixelShaderMacro11(source.c_str(), source.data(), source.size(), NULL, &d3d_ps_blob))
					return false;
			}
		}
		assert(m_device->GetD3D11Device());
		HRESULT hr = gHR = m_device->GetD3D11Device()->CreatePixelShader(d3d_ps_blob->GetBufferPointer(), d3d_ps_blob->GetBufferSize(), NULL, &d3d11_ps);
		if (FAILED(hr))
			return false;
		M_D3D_SET_DEBUG_NAME_SIMPLE(d3d11_ps.Get());
		return true;
	}

	bool Renderer_D3D11::createShaders()
	{
		assert(m_device->GetD3D11Device());

		HRESULT hr = 0;

		Microsoft::WRL::ComPtr<ID3DBlob> blob_;

		/* vertex shader */ {
			Microsoft::WRL::ComPtr<ID3DBlob> vs_blob_;

			auto loadVSf = [&](FogState f, D3D_SHADER_MACRO const* m, std::string_view f_str) -> bool
			{
				std::string name("LuaSTG VertexShader: ");
				name.append(f_str);
				if (!compileVertexShaderMacro11(name.c_str(), g_VertexShader11, sizeof(g_VertexShader11), m, &vs_blob_))
					return false;
				hr = gHR = m_device->GetD3D11Device()->CreateVertexShader(vs_blob_->GetBufferPointer(), vs_blob_->GetBufferSize(), NULL,
					&_vertex_shader[IDX(f)]);
				if (FAILED(hr))
					return false;
				M_D3D_SET_DEBUG_NAME_SIMPLE(_vertex_shader[IDX(f)].Get());
				return true;
			};

		#define loadVS(A, DEF) if (!loadVSf(A, DEF, #A)) return false;

			const D3D_SHADER_MACRO vs_def_fog0_[] = {
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO vs_def_fog1_[] = {
				{ "FOG_ENABLE", "1"},
				{ NULL, NULL },
			};

			loadVS(FogState::Disable, vs_def_fog0_);
			blob_ = vs_blob_; // 用这个保存签名

			loadVS(FogState::Linear, vs_def_fog1_);
			loadVS(FogState::Exp, vs_def_fog1_);
			loadVS(FogState::Exp2, vs_def_fog1_);
		}

		/* input layout */ {
			D3D11_INPUT_ELEMENT_DESC layout_[] =
			{
				// DrawVertex2D
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR",    0, DXGI_FORMAT_B8G8R8A8_UNORM , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			hr = gHR = m_device->GetD3D11Device()->CreateInputLayout(layout_, 3, blob_->GetBufferPointer(), blob_->GetBufferSize(), &_input_layout);
			if (FAILED(hr))
				return false;
		}

		/* pixel shader normal set */ {
			Microsoft::WRL::ComPtr<ID3DBlob> ps_blob_;

			auto loadPSf = [&](VertexColorBlendState v, FogState f, TextureAlphaType t, D3D_SHADER_MACRO const* m,
				std::string_view v_str, std::string_view f_str, std::string_view t_str) -> bool
			{
				std::string name("LuaSTG PixelShader: ");
				name.append(v_str); name.append(", ");
				name.append(f_str); name.append(", ");
				name.append(t_str);
				if (!compilePixelShaderMacro11(name.c_str(), g_PixelShader11, sizeof(g_PixelShader11), m, &ps_blob_))
					return false;
				hr = gHR = m_device->GetD3D11Device()->CreatePixelShader(ps_blob_->GetBufferPointer(), ps_blob_->GetBufferSize(), NULL,
					&_pixel_shader[IDX(v)][IDX(f)][IDX(t)]);
				if (FAILED(hr))
					return false;
				M_D3D_SET_DEBUG_NAME_SIMPLE(_pixel_shader[IDX(v)][IDX(f)][IDX(t)].Get());
				return true;
			};

		#define loadPS(A, B, C, DEF) if (!loadPSf(A, B, C, DEF, #A, #B, #C)) return false;

			const D3D_SHADER_MACRO ps_def_zero_[] = {
				{ "VERTEX_COLOR_BLEND_ZERO", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_one0_[] = {
				{ "VERTEX_COLOR_BLEND_ONE", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_add0_[] = {
				{ "VERTEX_COLOR_BLEND_ADD", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_mul0_[] = {
				{ "VERTEX_COLOR_BLEND_MUL", "1"},
				{ NULL, NULL },
			};

			loadPS(VertexColorBlendState::Zero, FogState::Disable, TextureAlphaType::Normal, ps_def_zero_);
			loadPS(VertexColorBlendState::One, FogState::Disable, TextureAlphaType::Normal, ps_def_one0_);
			loadPS(VertexColorBlendState::Add, FogState::Disable, TextureAlphaType::Normal, ps_def_add0_);
			loadPS(VertexColorBlendState::Mul, FogState::Disable, TextureAlphaType::Normal, ps_def_mul0_);

			const D3D_SHADER_MACRO ps_def_zero_line_[] = {
				{ "VERTEX_COLOR_BLEND_ZERO", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_LINEAR", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_one0_line_[] = {
				{ "VERTEX_COLOR_BLEND_ONE", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_LINEAR", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_add0_line_[] = {
				{ "VERTEX_COLOR_BLEND_ADD", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_LINEAR", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_mul0_line_[] = {
				{ "VERTEX_COLOR_BLEND_MUL", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_LINEAR", "1"},
				{ NULL, NULL },
			};

			loadPS(VertexColorBlendState::Zero, FogState::Linear, TextureAlphaType::Normal, ps_def_zero_line_);
			loadPS(VertexColorBlendState::One, FogState::Linear, TextureAlphaType::Normal, ps_def_one0_line_);
			loadPS(VertexColorBlendState::Add, FogState::Linear, TextureAlphaType::Normal, ps_def_add0_line_);
			loadPS(VertexColorBlendState::Mul, FogState::Linear, TextureAlphaType::Normal, ps_def_mul0_line_);

			const D3D_SHADER_MACRO ps_def_zero_exp0_[] = {
				{ "VERTEX_COLOR_BLEND_ZERO", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_one0_exp0_[] = {
				{ "VERTEX_COLOR_BLEND_ONE", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_add0_exp0_[] = {
				{ "VERTEX_COLOR_BLEND_ADD", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_mul0_exp0_[] = {
				{ "VERTEX_COLOR_BLEND_MUL", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP", "1"},
				{ NULL, NULL },
			};

			loadPS(VertexColorBlendState::Zero, FogState::Exp, TextureAlphaType::Normal, ps_def_zero_exp0_);
			loadPS(VertexColorBlendState::One, FogState::Exp, TextureAlphaType::Normal, ps_def_one0_exp0_);
			loadPS(VertexColorBlendState::Add, FogState::Exp, TextureAlphaType::Normal, ps_def_add0_exp0_);
			loadPS(VertexColorBlendState::Mul, FogState::Exp, TextureAlphaType::Normal, ps_def_mul0_exp0_);

			const D3D_SHADER_MACRO ps_def_zero_exp2_[] = {
				{ "VERTEX_COLOR_BLEND_ZERO", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP2", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_one0_exp2_[] = {
				{ "VERTEX_COLOR_BLEND_ONE", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP2", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_add0_exp2_[] = {
				{ "VERTEX_COLOR_BLEND_ADD", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP2", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_mul0_exp2_[] = {
				{ "VERTEX_COLOR_BLEND_MUL", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP2", "1"},
				{ NULL, NULL },
			};

			loadPS(VertexColorBlendState::Zero, FogState::Exp2, TextureAlphaType::Normal, ps_def_zero_exp2_);
			loadPS(VertexColorBlendState::One, FogState::Exp2, TextureAlphaType::Normal, ps_def_one0_exp2_);
			loadPS(VertexColorBlendState::Add, FogState::Exp2, TextureAlphaType::Normal, ps_def_add0_exp2_);
			loadPS(VertexColorBlendState::Mul, FogState::Exp2, TextureAlphaType::Normal, ps_def_mul0_exp2_);
		}

		/* pixel shader premul alpha set */ {
			Microsoft::WRL::ComPtr<ID3DBlob> ps_blob_;

			auto loadPSf = [&](VertexColorBlendState v, FogState f, TextureAlphaType t, D3D_SHADER_MACRO const* m,
				std::string_view v_str, std::string_view f_str, std::string_view t_str) -> bool
			{
				std::string name("LuaSTG PixelShader: ");
				name.append(v_str); name.append(", ");
				name.append(f_str); name.append(", ");
				name.append(t_str);
				if (!compilePixelShaderMacro11(name.c_str(), g_PixelShader11, sizeof(g_PixelShader11), m, &ps_blob_))
					return false;
				hr = gHR = m_device->GetD3D11Device()->CreatePixelShader(ps_blob_->GetBufferPointer(), ps_blob_->GetBufferSize(), NULL,
					&_pixel_shader[IDX(v)][IDX(f)][IDX(t)]);
				if (FAILED(hr))
					return false;
				M_D3D_SET_DEBUG_NAME_SIMPLE(_pixel_shader[IDX(v)][IDX(f)][IDX(t)].Get());
				return true;
			};

			const D3D_SHADER_MACRO ps_def_zero_nfog_mula_[] = {
				{ "VERTEX_COLOR_BLEND_ZERO", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_one0_nfog_mula_[] = {
				{ "VERTEX_COLOR_BLEND_ONE", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_add0_nfog_mula_[] = {
				{ "VERTEX_COLOR_BLEND_ADD", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_mul0_nfog_mula_[] = {
				{ "VERTEX_COLOR_BLEND_MUL", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};

			loadPS(VertexColorBlendState::Zero, FogState::Disable, TextureAlphaType::PremulAlpha, ps_def_zero_nfog_mula_);
			loadPS(VertexColorBlendState::One, FogState::Disable, TextureAlphaType::PremulAlpha, ps_def_one0_nfog_mula_);
			loadPS(VertexColorBlendState::Add, FogState::Disable, TextureAlphaType::PremulAlpha, ps_def_add0_nfog_mula_);
			loadPS(VertexColorBlendState::Mul, FogState::Disable, TextureAlphaType::PremulAlpha, ps_def_mul0_nfog_mula_);

			const D3D_SHADER_MACRO ps_def_zero_line_mula_[] = {
				{ "VERTEX_COLOR_BLEND_ZERO", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_LINEAR", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_one0_line_mula_[] = {
				{ "VERTEX_COLOR_BLEND_ONE", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_LINEAR", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_add0_line_mula_[] = {
				{ "VERTEX_COLOR_BLEND_ADD", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_LINEAR", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_mul0_line_mula_[] = {
				{ "VERTEX_COLOR_BLEND_MUL", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_LINEAR", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};

			loadPS(VertexColorBlendState::Zero, FogState::Linear, TextureAlphaType::PremulAlpha, ps_def_zero_line_mula_);
			loadPS(VertexColorBlendState::One, FogState::Linear, TextureAlphaType::PremulAlpha, ps_def_one0_line_mula_);
			loadPS(VertexColorBlendState::Add, FogState::Linear, TextureAlphaType::PremulAlpha, ps_def_add0_line_mula_);
			loadPS(VertexColorBlendState::Mul, FogState::Linear, TextureAlphaType::PremulAlpha, ps_def_mul0_line_mula_);

			const D3D_SHADER_MACRO ps_def_zero_exp0_mula_[] = {
				{ "VERTEX_COLOR_BLEND_ZERO", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_one0_exp0_mula_[] = {
				{ "VERTEX_COLOR_BLEND_ONE", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_add0_exp0_mula_[] = {
				{ "VERTEX_COLOR_BLEND_ADD", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_mul0_exp0_mula_[] = {
				{ "VERTEX_COLOR_BLEND_MUL", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};

			loadPS(VertexColorBlendState::Zero, FogState::Exp, TextureAlphaType::PremulAlpha, ps_def_zero_exp0_mula_);
			loadPS(VertexColorBlendState::One, FogState::Exp, TextureAlphaType::PremulAlpha, ps_def_one0_exp0_mula_);
			loadPS(VertexColorBlendState::Add, FogState::Exp, TextureAlphaType::PremulAlpha, ps_def_add0_exp0_mula_);
			loadPS(VertexColorBlendState::Mul, FogState::Exp, TextureAlphaType::PremulAlpha, ps_def_mul0_exp0_mula_);

			const D3D_SHADER_MACRO ps_def_zero_exp2_mula_[] = {
				{ "VERTEX_COLOR_BLEND_ZERO", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP2", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_one0_exp2_mula_[] = {
				{ "VERTEX_COLOR_BLEND_ONE", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP2", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_add0_exp2_mula_[] = {
				{ "VERTEX_COLOR_BLEND_ADD", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP2", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};
			const D3D_SHADER_MACRO ps_def_mul0_exp2_mula_[] = {
				{ "VERTEX_COLOR_BLEND_MUL", "1"},
				{ "FOG_ENABLE", "1"},
				{ "FOG_EXP2", "1"},
				{ "PREMUL_ALPHA", "1"},
				{ NULL, NULL },
			};

			loadPS(VertexColorBlendState::Zero, FogState::Exp2, TextureAlphaType::PremulAlpha, ps_def_zero_exp2_mula_);
			loadPS(VertexColorBlendState::One, FogState::Exp2, TextureAlphaType::PremulAlpha, ps_def_one0_exp2_mula_);
			loadPS(VertexColorBlendState::Add, FogState::Exp2, TextureAlphaType::PremulAlpha, ps_def_add0_exp2_mula_);
			loadPS(VertexColorBlendState::Mul, FogState::Exp2, TextureAlphaType::PremulAlpha, ps_def_mul0_exp2_mula_);
		}

		return true;
	}
}
