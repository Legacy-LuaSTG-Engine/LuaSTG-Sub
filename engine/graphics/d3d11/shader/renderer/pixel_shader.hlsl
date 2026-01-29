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

static const float channel_minimum = 1.0f / 255.0f;

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


// 预乘 alpha 的源颜色与直通顶点色相乘混合
// 
// 约定：
// - 符号 *：相乘
// - 大写 C：颜色通道
// - 大写 C`：预乘了 alpha 的颜色通道
// - 大写 A：alpha 通道
// - 小写 out 后缀：结果
// - 小写 src 后缀：源
// - 小写 vert 后缀：顶点
// 
// 正常情况下，直通源颜色和顶点色是这样相乘混合的（Direct3D 9 固定管线的行为）：
// Cout = Csrc * Cvert
// Aout = Asrc * Avert
// 如果希望输出的颜色预乘了 alpha，那么只需要将颜色通道与 alpha 通道相乘
// C`out = Cout * Aout
// 
// 当源颜色不是直通颜色时，我们的计算方式有所变化：
// 我们把 C`out 展开：
// C`out = Cout * Aout
//       = Csrc * Cvert * Asrc * Avert  <1>
//       = C`src * Cvert * Avert        <2>
// 对比 <1> 和 <2>，不难发现，当源颜色不是直通颜色时，只需要少乘以 Asrc 即可


// 预乘 alpha 的源颜色与直通顶点色加法混合
// 
// 约定：
// - 符号 *：相乘
// - 符号 \：相除
// - 大写 C：颜色通道
// - 大写 C`：预乘了 alpha 的颜色通道
// - 大写 A：alpha 通道
// - 小写 out 后缀：结果
// - 小写 src 后缀：源
// - 小写 vert 后缀：顶点
// 
// 正常情况下，直通源颜色和顶点色是这样加法混合的（Direct3D 9 固定管线的行为）：
// Cout = min(Csrc + Cvert, 1.0)
// Aout = Asrc * Avert
// 如果希望输出的颜色预乘了 alpha，那么只需要将颜色通道与 alpha 通道相乘
// C`out = Cout * Aout
// 
// 当源颜色不是直通颜色时，我们的计算方式有所变化：
// 因为 min 函数不是可交换的，我们需要先对 C`src 进行解除预乘 alpha 的操作：
// Csrc = C`src / Asrc
// 我们马上就能发现一个问题，当 Asrc 为 0.0 的时候，计算的结果是没有意义的；
// 由于我们一般使用 8 位每通道的纹理，所以我们可以在计算前检查 Asrc，当其小于 (1.0 / 255.0) 时直接舍弃这个像素；
// 接下来的计算就和源颜色为直通时一样


// 基于预乘 alpha 的雾颜色混合，源颜色是预乘 alpha 的，雾颜色是直通的（没有预乘 alpha）
// 
// 约定：
// - 符号 *：相乘
// - 符号 ^：指数运算
// - 大写 C：颜色通道
// - 大写 C`：预乘了 alpha 的颜色通道
// - 大写 A：alpha 通道
// - 小写 out 后缀：结果
// - 小写 src 后缀：源
// - 小写 fog 后缀：雾
// - 小写 k：雾混合因子，范围是 0.0 到 1.0，越大越雾颜色贡献越多
// 
// 正常情况下，直通源颜色和直通雾颜色是这样线性混合的（Direct3D 9 固定管线的行为）：
// Cout = lerp(Csrc, Cfog, k)
// Aout = lerp(Asrc, Asrc * Afog, k)
// 如果希望输出的颜色预乘了 alpha，那么只需要将颜色通道与 alpha 通道相乘
// C`out = Cout * Aout
// 
// 当源颜色不是直通颜色时，我们的计算方式有所变化：
// 首先，我们将 lerp 展开：
// Cout = lerp(Csrc, Cfog, k)        = (1.0 - k) * Csrc + k * Cfog
// Aout = lerp(Asrc, Asrc * Afog, k) = (1.0 - k) * Asrc + k * Asrc * Afog
// 然后，我们将 C`out 展开：
// C`out = Cout * Aout
//       = [(1.0 - k) * Csrc + k * Cfog] * [(1.0 - k) * Asrc + k * Asrc * Afog]
//       = [(1.0 - k) ^ 2.0] * Csrc * Asrc + k * (1.0 - k) * Csrc * Asrc * Afog + k * (1.0 - k) * Cfog * Asrc + (k ^ 2.0) * Cfog * Afog * Asrc
//       = [(1.0 - k) ^ 2.0] * C`src + k * (1.0 - k) * C`src * Afog + k * (1.0 - k) * Cfog * Asrc + (k ^ 2.0) * Cfog * Afog * Asrc
// 到这一步就会发现，我们无需用到 Csrc，让我们尝试合并同类项：
// C`out = {[(1.0 - k) ^ 2.0] + k * (1.0 - k) * Afog} * C`src + [k * (1.0 - k) * Asrc + (k ^ 2.0) * Asrc * Afog] * Cfog
// 看着有点长？那就分而治之，分别令 ka、kb 等于：
// ka = [(1.0 - k) ^ 2.0] + k * (1.0 - k) * Afog
//    = (1.0 - k) * [(1.0 - k) + k * Afog]
// kb = k * (1.0 - k) * Asrc + (k ^ 2.0) * Asrc * Afog
//    = k * [(1.0 - k) * Asrc + k * Asrc * Afog]
// 仔细看 kb 是不是有点眼熟，是的它可以化简为：
// kb = k * Aout
// 最终 C`out 就等于：
// C`out = ka * C`src + kb * Cfog
// 
// 可以看到计算的时候大量出现了 1.0 - k，为了看起来更加简洁，整个计算过程可以写为：
// k1 = 1.0 - k
// Aout = k1 * Asrc + k * Asrc * Afog
// ka = k1 * (k1 + k * Afog)
// kb = k * Aout
// C`out = ka * C`src + kb * Cfog

