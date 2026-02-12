fxc /nologo /T vs_4_0 /E vs_main /O3 /Ges /Fh vertex_shader.h /Vn vertex_shader hello_triangle.hlsl
fxc /nologo /T ps_4_0 /E ps_main /O3 /Ges /Fh pixel_shader.h  /Vn pixel_shader  hello_triangle.hlsl
