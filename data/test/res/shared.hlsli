
struct PS_Input
{
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

struct PS_Output
{
    float4 col : SV_Target;
};
