// shader toy

const int screen_door_mask[16 * 17] = int[16 * 17](
    // 0%
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    // 1/16
    1,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    // 2/16
    1,0,0,0,
    0,0,0,0,
    0,0,1,0,
    0,0,0,0,
    // 3/16
    1,0,0,0,
    0,0,0,0,
    1,0,1,0,
    0,0,0,0,
    // 25%
    1,0,1,0,
    0,0,0,0,
    1,0,1,0,
    0,0,0,0,
    // 5/16
    1,0,1,0,
    0,1,0,0,
    1,0,1,0,
    0,0,0,0,
    // 6/16
    1,0,1,0,
    0,1,0,0,
    1,0,1,0,
    0,0,0,1,
    // 7/16
    1,0,1,0,
    0,1,0,0,
    1,0,1,0,
    0,1,0,1,
    // 50%
    1,0,1,0,
    0,1,0,1,
    1,0,1,0,
    0,1,0,1,
    // 9/16
    1,0,1,0,
    0,1,1,1,
    1,0,1,0,
    0,1,0,1,
    // 10/16
    1,0,1,0,
    0,1,1,1,
    1,0,1,0,
    1,1,0,1,
    // 11/16
    1,0,1,0,
    0,1,1,1,
    1,0,1,0,
    1,1,1,1,
    // 75%
    1,0,1,0,
    1,1,1,1,
    1,0,1,0,
    1,1,1,1,
    // 13/16
    1,1,1,0,
    1,1,1,1,
    1,0,1,0,
    1,1,1,1,
    // 14/16
    1,1,1,0,
    1,1,1,1,
    1,0,1,1,
    1,1,1,1,
    // 15/16
    1,1,1,0,
    1,1,1,1,
    1,1,1,1,
    1,1,1,1,
    // 100%
    1,1,1,1,
    1,1,1,1,
    1,1,1,1,
    1,1,1,1
);

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;

    // Time varying pixel color
    //vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4));

    float pml = length(fragCoord - iMouse.xy);
    float a = 1.0 - (min(200.0, pml) / 200.0);
    
    if (fragCoord.x > iResolution.x / 2.0) {
        float ar = pow(a, 1.0 / 2.2);
        fragColor = vec4(ar, ar, ar, 1.0);
        return;
    }

    vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
    int offset = int(floor(a * 16.0 + 0.5));
    int ix = int(floor(fragCoord.x));
    int iy = int(floor(fragCoord.y));
    int mask_value = screen_door_mask[16 * offset + 4 * (iy % 4) + (ix % 4)];
    if (mask_value == 0) {
        color.x = 0.0;
        color.y = 0.0;
        color.z = 0.0;
    }

    // Output to screen
    fragColor = color;
}

// 4x4 mask, 0.0625
// 0 - 0.0625 - 0.125 
