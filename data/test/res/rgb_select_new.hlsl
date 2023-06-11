// 引擎设置的参数，不可修改

SamplerState screen_texture_sampler : register(s4); // RenderTarget 纹理的采样器
Texture2D screen_texture            : register(t4); // RenderTarget 纹理
cbuffer engine_data : register(b1)
{
    float4 screen_texture_size; // 纹理大小
    float4 viewport;            // 视口
};

// 用户传递的浮点参数

cbuffer user_data : register(b0)
{
    float4 channel_factor;
};

// 用户传递的纹理参数

//SamplerState screen_texture_sampler2 : register(s0);
//Texture2D screen_texture2            : register(t0);

// 主函数

struct PS_Input
{
    float4 sxy : SV_Position;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};
struct PS_Output
{
    float4 col : SV_Target;
};

PS_Output main(PS_Input input)
{
    float4 tex_col = screen_texture.Sample(screen_texture_sampler, input.uv);
    float value = dot(tex_col.rgb, channel_factor.rgb);
    tex_col.r = value;
    tex_col.g = value;
    tex_col.b = value;
    
    PS_Output output;
    output.col = tex_col;
    return output;
}
