#include "res/shared.hlsli"
#include "res/blend_shared.hlsli"

//cbuffer g_buffer : register(b0)
//{
//    float2 g_render_target_size;
//    float4 g_viewport;
//};

Texture2D    g_render_target : register(t0);
SamplerState g_render_target_sampler : register(s0);

Texture2D    g_texture : register(t1);
SamplerState g_texture_sampler : register(s1);

static const float _1_2dot2 = 1.0f / 2.2f;

PS_Output main(PS_Input input)
{
    //float2 xy = input.uv * g_render_target_size;
    //if (xy.x < g_viewport.x || xy.y < g_viewport.y || xy.x > g_viewport.z || xy.y > g_viewport.w)
    //{
    //    discard;
    //}

    float4 bottom_color = g_render_target.Sample(g_render_target_sampler, input.uv);
    //bottom_color.rgb = pow(bottom_color.rgb, _1_2dot2);
    float4 top_color = g_texture.Sample(g_texture_sampler, input.uv);
    //top_color.rgb = pow(top_color.rgb, _1_2dot2);

    float4 final_color = float4(color_burn(bottom_color.rgb, top_color.rgb), 1.0f);
    //final_color.rgb = pow(final_color.rgb, 2.2f);

    PS_Output output;
    output.col = final_color;
    return output;
}
