fxc /nologo /T vs_4_0 /E main /O3 /Ges                           /Fh vertex_shader_xy_normal.h  /Vn vertex_shader_xy_normal  vertex_shader.hlsl
fxc /nologo /T vs_4_0 /E main /O3 /Ges             /D FOG_ENABLE /Fh vertex_shader_xy_fog.h     /Vn vertex_shader_xy_fog     vertex_shader.hlsl

fxc /nologo /T vs_4_0 /E main /O3 /Ges /D Z_ENABLE               /Fh vertex_shader_xyz_normal.h /Vn vertex_shader_xyz_normal vertex_shader.hlsl
fxc /nologo /T vs_4_0 /E main /O3 /Ges /D Z_ENABLE /D FOG_ENABLE /Fh vertex_shader_xyz_fog.h    /Vn vertex_shader_xyz_fog    vertex_shader.hlsl
