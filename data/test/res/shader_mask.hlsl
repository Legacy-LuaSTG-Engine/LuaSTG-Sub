#include "res/shared.hlsli"

Texture2D    g_texture : register(t0);
SamplerState g_texture_sampler : register(s0);

Texture2D    g_render_target : register(t1);
SamplerState g_render_target_sampler : register(s1);

cbuffer g_buffer : register(b0)
{
    float2 g_render_target_size;
    float4 g_viewport;
};

PS_Output main(PS_Input input)
{
    float2 xy = input.uv * g_render_target_size;
    if (xy.x < g_viewport.x || xy.y < g_viewport.y || xy.x > g_viewport.z || xy.y > g_viewport.w)
    {
        discard;
    }

    float4 color_mask = g_texture.Sample(g_texture_sampler, input.uv);
    float mask_value = (color_mask.r + color_mask.g + color_mask.b) / 3.0f;
    float4 color_buffer = g_render_target.Sample(g_render_target_sampler, input.uv);
    float4 color = color_buffer * float4(mask_value, mask_value, mask_value, mask_value);

    PS_Output output;
    output.col = color;
    return output;
}
