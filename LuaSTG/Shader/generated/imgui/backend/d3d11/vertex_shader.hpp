// This is a file generated by the compiler, DO NOT directly modify this file
#pragma once
namespace imgui::backend::d3d11
{
    static unsigned char const vertex_shader[] = {
        0x44, 0x58, 0x42, 0x43,
        0x87, 0x84, 0x02, 0xfd,
        0x99, 0x4a, 0xa5, 0x05,
        0x6c, 0xa4, 0xa1, 0x2b,
        0x5c, 0x26, 0x4b, 0x35,
        0x01, 0x00, 0x00, 0x00,
        0x6c, 0x03, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00,
        0x34, 0x00, 0x00, 0x00,
        0x04, 0x01, 0x00, 0x00,
        0x74, 0x01, 0x00, 0x00,
        0xe8, 0x01, 0x00, 0x00,
        0xf0, 0x02, 0x00, 0x00,
        0x52, 0x44, 0x45, 0x46,
        0xc8, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x4c, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x1c, 0x00, 0x00, 0x00,
        0x00, 0x04, 0xfe, 0xff,
        0x00, 0x8b, 0x04, 0x00,
        0xa0, 0x00, 0x00, 0x00,
        0x3c, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x76, 0x65, 0x72, 0x74,
        0x65, 0x78, 0x42, 0x75,
        0x66, 0x66, 0x65, 0x72,
        0x00, 0xab, 0xab, 0xab,
        0x3c, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x64, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x7c, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x90, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x50, 0x72, 0x6f, 0x6a,
        0x65, 0x63, 0x74, 0x69,
        0x6f, 0x6e, 0x4d, 0x61,
        0x74, 0x72, 0x69, 0x78,
        0x00, 0xab, 0xab, 0xab,
        0x03, 0x00, 0x03, 0x00,
        0x04, 0x00, 0x04, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x4d, 0x69, 0x63, 0x72,
        0x6f, 0x73, 0x6f, 0x66,
        0x74, 0x20, 0x28, 0x52,
        0x29, 0x20, 0x48, 0x4c,
        0x53, 0x4c, 0x20, 0x53,
        0x68, 0x61, 0x64, 0x65,
        0x72, 0x20, 0x43, 0x6f,
        0x6d, 0x70, 0x69, 0x6c,
        0x65, 0x72, 0x20, 0x31,
        0x30, 0x2e, 0x31, 0x00,
        0x49, 0x53, 0x47, 0x4e,
        0x68, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00,
        0x50, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x03, 0x03, 0x00, 0x00,
        0x59, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x0f, 0x0f, 0x00, 0x00,
        0x5f, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x03, 0x03, 0x00, 0x00,
        0x50, 0x4f, 0x53, 0x49,
        0x54, 0x49, 0x4f, 0x4e,
        0x00, 0x43, 0x4f, 0x4c,
        0x4f, 0x52, 0x00, 0x54,
        0x45, 0x58, 0x43, 0x4f,
        0x4f, 0x52, 0x44, 0x00,
        0x4f, 0x53, 0x47, 0x4e,
        0x6c, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00,
        0x50, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x0f, 0x00, 0x00, 0x00,
        0x5c, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x0f, 0x00, 0x00, 0x00,
        0x62, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x03, 0x0c, 0x00, 0x00,
        0x53, 0x56, 0x5f, 0x50,
        0x4f, 0x53, 0x49, 0x54,
        0x49, 0x4f, 0x4e, 0x00,
        0x43, 0x4f, 0x4c, 0x4f,
        0x52, 0x00, 0x54, 0x45,
        0x58, 0x43, 0x4f, 0x4f,
        0x52, 0x44, 0x00, 0xab,
        0x53, 0x48, 0x44, 0x52,
        0x00, 0x01, 0x00, 0x00,
        0x40, 0x00, 0x01, 0x00,
        0x40, 0x00, 0x00, 0x00,
        0x59, 0x00, 0x00, 0x04,
        0x46, 0x8e, 0x20, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00,
        0x5f, 0x00, 0x00, 0x03,
        0x32, 0x10, 0x10, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x5f, 0x00, 0x00, 0x03,
        0xf2, 0x10, 0x10, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x5f, 0x00, 0x00, 0x03,
        0x32, 0x10, 0x10, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x67, 0x00, 0x00, 0x04,
        0xf2, 0x20, 0x10, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x65, 0x00, 0x00, 0x03,
        0xf2, 0x20, 0x10, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x65, 0x00, 0x00, 0x03,
        0x32, 0x20, 0x10, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x68, 0x00, 0x00, 0x02,
        0x01, 0x00, 0x00, 0x00,
        0x38, 0x00, 0x00, 0x08,
        0xf2, 0x00, 0x10, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x56, 0x15, 0x10, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x46, 0x8e, 0x20, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x32, 0x00, 0x00, 0x0a,
        0xf2, 0x00, 0x10, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x46, 0x8e, 0x20, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x06, 0x10, 0x10, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x46, 0x0e, 0x10, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x08,
        0xf2, 0x20, 0x10, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x46, 0x0e, 0x10, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x46, 0x8e, 0x20, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
        0x36, 0x00, 0x00, 0x05,
        0xf2, 0x20, 0x10, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x46, 0x1e, 0x10, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x36, 0x00, 0x00, 0x05,
        0x32, 0x20, 0x10, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x46, 0x10, 0x10, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x3e, 0x00, 0x00, 0x01,
        0x53, 0x54, 0x41, 0x54,
        0x74, 0x00, 0x00, 0x00,
        0x06, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x06, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };
}
