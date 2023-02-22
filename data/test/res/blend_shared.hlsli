// color burn
// color dodge

static const float _1_255 = 1.0f / 255.0f;

float color_burn_single_channel(float bottom_color, float top_color)
{
    if (top_color < _1_255)
    {
        return 0.0f;
    }
    else
    {
        return 1.0f - clamp((1.0f - bottom_color) / top_color, 0.0f, 1.0f);
    }
}

float3 color_burn(float3 bottom_color, float3 top_color)
{
    float3 color;
    color.r = color_burn_single_channel(bottom_color.r, top_color.r);
    color.g = color_burn_single_channel(bottom_color.g, top_color.g);
    color.b = color_burn_single_channel(bottom_color.b, top_color.b);
    return color;
}

float color_dodge_single_channel(float bottom_color, float top_color)
{
    if ((1.0f - top_color) < _1_255)
    {
        return 0.0f;
    }
    else
    {
        return clamp(bottom_color / (1.0f - top_color), 0.0f, 1.0f);
    }
}

float3 color_dodge(float3 bottom_color, float3 top_color)
{
    float3 color;
    color.r = color_dodge_single_channel(bottom_color.r, top_color.r);
    color.g = color_dodge_single_channel(bottom_color.g, top_color.g);
    color.b = color_dodge_single_channel(bottom_color.b, top_color.b);
    return color;
}
