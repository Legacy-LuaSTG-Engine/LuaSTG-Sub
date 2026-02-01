fxc /nologo /T vs_4_0 /E main /O3 /Ges               /Fh vertex_shader_normal.h /Vn vertex_shader_normal vertex_shader.hlsl
fxc /nologo /T vs_4_0 /E main /O3 /Ges /D FOG_ENABLE /Fh vertex_shader_fog.h    /Vn vertex_shader_fog    vertex_shader.hlsl

:: vertex color: add

fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ADD                                             /Fh pixel_shader_add_none_straight.h   /Vn pixel_shader_add_none_straight   pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ADD /D FOG_ENABLE /D FOG_LINEAR                 /Fh pixel_shader_add_linear_straight.h /Vn pixel_shader_add_linear_straight pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ADD /D FOG_ENABLE /D FOG_EXP                    /Fh pixel_shader_add_exp_straight.h    /Vn pixel_shader_add_exp_straight    pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ADD /D FOG_ENABLE /D FOG_EXP2                   /Fh pixel_shader_add_exp2_straight.h   /Vn pixel_shader_add_exp2_straight   pixel_shader.hlsl

fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ADD                             /D PREMUL_ALPHA /Fh pixel_shader_add_none_premul.h     /Vn pixel_shader_add_none_premul     pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ADD /D FOG_ENABLE /D FOG_LINEAR /D PREMUL_ALPHA /Fh pixel_shader_add_linear_premul.h   /Vn pixel_shader_add_linear_premul   pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ADD /D FOG_ENABLE /D FOG_EXP    /D PREMUL_ALPHA /Fh pixel_shader_add_exp_premul.h      /Vn pixel_shader_add_exp_premul      pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ADD /D FOG_ENABLE /D FOG_EXP2   /D PREMUL_ALPHA /Fh pixel_shader_add_exp2_premul.h     /Vn pixel_shader_add_exp2_premul     pixel_shader.hlsl

:: vertex color: mul

fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_MUL                                             /Fh pixel_shader_mul_none_straight.h   /Vn pixel_shader_mul_none_straight   pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_MUL /D FOG_ENABLE /D FOG_LINEAR                 /Fh pixel_shader_mul_linear_straight.h /Vn pixel_shader_mul_linear_straight pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_MUL /D FOG_ENABLE /D FOG_EXP                    /Fh pixel_shader_mul_exp_straight.h    /Vn pixel_shader_mul_exp_straight    pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_MUL /D FOG_ENABLE /D FOG_EXP2                   /Fh pixel_shader_mul_exp2_straight.h   /Vn pixel_shader_mul_exp2_straight   pixel_shader.hlsl

fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_MUL                             /D PREMUL_ALPHA /Fh pixel_shader_mul_none_premul.h     /Vn pixel_shader_mul_none_premul     pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_MUL /D FOG_ENABLE /D FOG_LINEAR /D PREMUL_ALPHA /Fh pixel_shader_mul_linear_premul.h   /Vn pixel_shader_mul_linear_premul   pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_MUL /D FOG_ENABLE /D FOG_EXP    /D PREMUL_ALPHA /Fh pixel_shader_mul_exp_premul.h      /Vn pixel_shader_mul_exp_premul      pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_MUL /D FOG_ENABLE /D FOG_EXP2   /D PREMUL_ALPHA /Fh pixel_shader_mul_exp2_premul.h     /Vn pixel_shader_mul_exp2_premul     pixel_shader.hlsl

:: vertex color: one

fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ONE                                             /Fh pixel_shader_one_none_straight.h   /Vn pixel_shader_one_none_straight   pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ONE /D FOG_ENABLE /D FOG_LINEAR                 /Fh pixel_shader_one_linear_straight.h /Vn pixel_shader_one_linear_straight pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ONE /D FOG_ENABLE /D FOG_EXP                    /Fh pixel_shader_one_exp_straight.h    /Vn pixel_shader_one_exp_straight    pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ONE /D FOG_ENABLE /D FOG_EXP2                   /Fh pixel_shader_one_exp2_straight.h   /Vn pixel_shader_one_exp2_straight   pixel_shader.hlsl

fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ONE                             /D PREMUL_ALPHA /Fh pixel_shader_one_none_premul.h     /Vn pixel_shader_one_none_premul     pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ONE /D FOG_ENABLE /D FOG_LINEAR /D PREMUL_ALPHA /Fh pixel_shader_one_linear_premul.h   /Vn pixel_shader_one_linear_premul   pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ONE /D FOG_ENABLE /D FOG_EXP    /D PREMUL_ALPHA /Fh pixel_shader_one_exp_premul.h      /Vn pixel_shader_one_exp_premul      pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ONE /D FOG_ENABLE /D FOG_EXP2   /D PREMUL_ALPHA /Fh pixel_shader_one_exp2_premul.h     /Vn pixel_shader_one_exp2_premul     pixel_shader.hlsl

:: vertex color: zero

fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ZERO                                             /Fh pixel_shader_zero_none_straight.h   /Vn pixel_shader_zero_none_straight   pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ZERO /D FOG_ENABLE /D FOG_LINEAR                 /Fh pixel_shader_zero_linear_straight.h /Vn pixel_shader_zero_linear_straight pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ZERO /D FOG_ENABLE /D FOG_EXP                    /Fh pixel_shader_zero_exp_straight.h    /Vn pixel_shader_zero_exp_straight    pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ZERO /D FOG_ENABLE /D FOG_EXP2                   /Fh pixel_shader_zero_exp2_straight.h   /Vn pixel_shader_zero_exp2_straight   pixel_shader.hlsl

fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ZERO                             /D PREMUL_ALPHA /Fh pixel_shader_zero_none_premul.h     /Vn pixel_shader_zero_none_premul     pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ZERO /D FOG_ENABLE /D FOG_LINEAR /D PREMUL_ALPHA /Fh pixel_shader_zero_linear_premul.h   /Vn pixel_shader_zero_linear_premul   pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ZERO /D FOG_ENABLE /D FOG_EXP    /D PREMUL_ALPHA /Fh pixel_shader_zero_exp_premul.h      /Vn pixel_shader_zero_exp_premul      pixel_shader.hlsl
fxc /nologo /T ps_4_0 /E main /O3 /Ges /D VERTEX_COLOR_BLEND_ZERO /D FOG_ENABLE /D FOG_EXP2   /D PREMUL_ALPHA /Fh pixel_shader_zero_exp2_premul.h     /Vn pixel_shader_zero_exp2_premul     pixel_shader.hlsl
