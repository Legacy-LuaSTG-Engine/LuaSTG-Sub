

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
    // TODO: 顶点颜色转到线性颜色空间
    //output.col = pow(input.col, 1.0f / 2.2f);

    return output;
};

