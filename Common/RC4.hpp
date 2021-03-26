#pragma once
#include <string>
#include <algorithm>

//RC4加解密实现
class RC4
{
private:
    uint8_t S[256];
public:
    void operator()(const uint8_t* input, size_t inputlen, uint8_t* output)
    {
        uint8_t Scpy[256];
        std::memcpy(Scpy, S, sizeof(S));
        for (size_t i = 0, j = 0; i < inputlen; i++)
        {
            // S盒置换
            size_t i2 = (i + 1) % 256;
            j = (j + Scpy[i2]) % 256;
            std::swap(Scpy[i2], Scpy[j]);
            uint8_t n = Scpy[(Scpy[i2] + Scpy[j]) % 256];
            // 加解密
            *(output + i) = *(input + i) ^ n;
        }
    }
public:
    RC4(const uint8_t* password, size_t len)
    {
        len = (len < 256U) ? len : 256U;
        // 初始化S盒
        for (int i = 0; i < 256; ++i)
            S[i] = i;
        // S盒初始置换
        for (size_t i = 0, j = 0; i < 256; i++)
        {
            j = (j + S[i] + password[i % len]) % 256;
            std::swap(S[i], S[j]);
        }
    }
};
