cbuffer view_proj_buffer : register(b0) {
    float4x4 view_proj;
};

cbuffer world_buffer : register(b1) {
    float4x4 world;
};

struct VS_Input {
#ifdef Z_ENABLE
    float3 position: POSITION0;
#else
    float2 position: POSITION0;
#endif
    float2 uv: TEXCOORD0;
    float4 color: COLOR0;
};

struct VS_Output {
    float4 xy: SV_POSITION;
#ifdef FOG_ENABLE
    float4 position: POSITION0;
#endif
    float2 uv: TEXCOORD0;
    float4 color: COLOR0;
};

VS_Output main(VS_Input input) {
#ifdef Z_ENABLE
    float4 position_world = float4(input.position, 1.0f);
#else
    float4 position_world = float4(input.position, 0.0f, 1.0f);
#endif
    position_world = mul(world, position_world);

    VS_Output output;
    output.xy = mul(view_proj, position_world);
#ifdef FOG_ENABLE
    output.position = position_world;
#endif
    output.uv = input.uv;
    output.color = input.color;

    return output;
}
