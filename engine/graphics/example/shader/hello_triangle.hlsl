cbuffer view_proj_buffer : register(b0)
{
    float4x4 view_proj;
};

struct VS_Input
{
    float2 pos : POSITION0;
    float4 col : COLOR0;
};

struct VS_Output
{
    float4 vpos : SV_POSITION;
    float4 col  : COLOR0;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.vpos = mul(view_proj, float4(input.pos, 0.0f, 1.0f));
    output.col  = input.col;
    return output;
};

struct PS_Input
{
    float4 vpos : SV_POSITION;
    float4 col  : COLOR0;
};

struct PS_Output
{
    float4 col : SV_TARGET;
};

PS_Output ps_main(PS_Input input)
{
    PS_Output output;
    output.col = input.col;
    return output;
}
