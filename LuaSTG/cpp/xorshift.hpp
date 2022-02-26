#pragma once

#include <cstdint>

namespace random {
    // https://prng.di.unimi.it/
    
    class splitmix64 {
    public:
        using result_type = uint64_t;
    private:
        uint64_t x = 0;
    public:
        static uint64_t min() {
            return UINT64_C(0);
        }
        static uint64_t max() {
            return UINT64_MAX;
        }
        void seed(uint64_t seedv) {
            x = seedv;
        }
        uint64_t next() {
            uint64_t z = (x += 0x9e3779b97f4a7c15);
            z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
            z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
            return z ^ (z >> 31);
        }
        uint64_t operator()() {
            return next();
        }
    public:
        splitmix64() {
            x = uint64_t(this);
        }
        explicit splitmix64(uint64_t s) : x(s) {}
    };
    
    inline uint64_t rotl(const uint64_t x, const int k) {
        return (x << k) | (x >> (64 - k));
    }
    
    class xoroshiro128_family {
    public:
        using result_type = uint64_t;
    protected:
        uint64_t s[2] = {};
    public:
        static uint64_t min() {
            return UINT64_C(0);
        }
        static uint64_t max() {
            return UINT64_MAX;
        }
        void seed(uint64_t seedv) {
            splitmix64 gn(seedv);
            s[0] = gn.next();
            s[1] = gn.next();
        }
        virtual uint64_t next() {
            return 0;
        }
        uint64_t operator()() {
            return next();
        }
    };
    
    class xoroshiro128p : public xoroshiro128_family {
    public:
        uint64_t next() override {
            const uint64_t s0 = s[0];
            uint64_t s1 = s[1];
            const uint64_t result = s0 + s1;
            
            s1 ^= s0;
            s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
            s[1] = rotl(s1, 37); // c
            
            return result;
        }
        void jump() {
            static const uint64_t JUMP[2] = {
                0xdf900294d8f554a5,
                0x170865df4b3201fc,
            };
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            for(int i = 0; i < 2; i++) {
                for(int b = 0; b < 64; b++) {
                    if (JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                    }
                    next();
                }
            }
            s[0] = s0;
            s[1] = s1;
        }
        void long_jump() {
            static const uint64_t LONG_JUMP[2] = {
                0xd2a98b26625eee7b,
                0xdddf9b1090aa7ac1,
            };
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            for(int i = 0; i < 2; i++) {
                for(int b = 0; b < 64; b++) {
                    if (LONG_JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                    }
                    next();
                }
            }
            s[0] = s0;
            s[1] = s1;
        }
    public:
        xoroshiro128p() {
            seed(uint64_t(this));
        }
        explicit xoroshiro128p(uint64_t seedv) {
            seed(seedv);
        }
    };
    
    class xoroshiro128pp : public xoroshiro128_family {
    public:
        uint64_t next() override {
            const uint64_t s0 = s[0];
            uint64_t s1 = s[1];
            const uint64_t result = rotl(s0 + s1, 17) + s0;
            
            s1 ^= s0;
            s[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21); // a, b
            s[1] = rotl(s1, 28); // c
            
            return result;
        }
        void jump() {
            static const uint64_t JUMP[2] = {
                0x2bd7a6a6e99c2ddc,
                0x0992ccaf6a6fca05,
            };
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            for(int i = 0; i < 2; i++) {
                for(int b = 0; b < 64; b++) {
                    if (JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                    }
                    next();
                }
            }
            s[0] = s0;
            s[1] = s1;
        }
        void long_jump() {
            static const uint64_t LONG_JUMP[2] = {
                0x360fd5f2cf8d5d99,
                0x9c6e6877736c46e3,
            };
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            for(int i = 0; i < 2; i++) {
                for(int b = 0; b < 64; b++) {
                    if (LONG_JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                    }
                    next();
                }
            }
            s[0] = s0;
            s[1] = s1;
        }
    public:
        xoroshiro128pp() {
            seed(uint64_t(this));
        }
        explicit xoroshiro128pp(uint64_t seedv) {
            seed(seedv);
        }
    };
    
    class xoroshiro128ss : public xoroshiro128_family {
    public:
        uint64_t next() override {
            const uint64_t s0 = s[0];
            uint64_t s1 = s[1];
            const uint64_t result = rotl(s0 * 5, 7) * 9;
            
            s1 ^= s0;
            s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
            s[1] = rotl(s1, 37); // c
            
            return result;
        }
        void jump() {
            static const uint64_t JUMP[2] = {
                0xdf900294d8f554a5,
                0x170865df4b3201fc,
            };
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            for(int i = 0; i < 2; i++) {
                for(int b = 0; b < 64; b++) {
                    if (JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                    }
                    next();
                }
            }
            s[0] = s0;
            s[1] = s1;
        }
        void long_jump() {
            static const uint64_t LONG_JUMP[2] = {
                0xd2a98b26625eee7b,
                0xdddf9b1090aa7ac1,
            };
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            for(int i = 0; i < 2; i++) {
                for(int b = 0; b < 64; b++) {
                    if (LONG_JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                    }
                    next();
                }
            }
            s[0] = s0;
            s[1] = s1;
        }
    public:
        xoroshiro128ss() {
            seed(uint64_t(this));
        }
        explicit xoroshiro128ss(uint64_t seedv) {
            seed(seedv);
        }
    };
    
    class xoshiro256_family {
    public:
        using result_type = uint64_t;
    protected:
        uint64_t s[4] = {};
    public:
        static uint64_t min() {
            return UINT64_C(0);
        }
        static uint64_t max() {
            return UINT64_MAX;
        }
        void seed(uint64_t seedv) {
            splitmix64 gn(seedv);
            s[0] = gn.next();
            s[1] = gn.next();
            s[2] = gn.next();
            s[3] = gn.next();
        }
        virtual uint64_t next() {
            return 0;
        }
        uint64_t operator()() {
            return next();
        }
    };
    
    class xoshiro256p : public xoshiro256_family {
    public:
        uint64_t next() override {
            const uint64_t result = s[0] + s[3];
            
            const uint64_t t = s[1] << 17;
            
            s[2] ^= s[0];
            s[3] ^= s[1];
            s[1] ^= s[2];
            s[0] ^= s[3];
            
            s[2] ^= t;
            
            s[3] = rotl(s[3], 45);
            
            return result;
        }
        void jump() {
            static const uint64_t JUMP[4] = {
                0x180ec6d33cfd0aba,
                0xd5a61266f0c9392c,
                0xa9582618e03fc9aa,
                0x39abdc4529b1661c,
            };
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for (int i = 0; i < 4; i++) {
                for (int b = 0; b < 64; b++) {
                    if (JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                        s2 ^= s[2];
                        s3 ^= s[3];
                    }
                    next();
                }
            }
            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }
        void long_jump() {
            static const uint64_t LONG_JUMP[4] = {
                0x76e15d3efefdcbbf,
                0xc5004e441c522fb3,
                0x77710069854ee241,
                0x39109bb02acbe635,
            };
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for (int i = 0; i < 4; i++) {
                for (int b = 0; b < 64; b++) {
                    if (LONG_JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                        s2 ^= s[2];
                        s3 ^= s[3];
                    }
                    next();
                }
            }
            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }
    public:
        xoshiro256p() {
            seed(uint64_t(this));
        }
        explicit xoshiro256p(uint64_t seedv) {
            seed(seedv);
        }
    };
    
    class xoshiro256pp : public xoshiro256_family {
    public:
        uint64_t next() override {
            const uint64_t result = rotl(s[0] + s[3], 23) + s[0];
            
            const uint64_t t = s[1] << 17;
            
            s[2] ^= s[0];
            s[3] ^= s[1];
            s[1] ^= s[2];
            s[0] ^= s[3];
            
            s[2] ^= t;
            
            s[3] = rotl(s[3], 45);
            
            return result;
        }
        void jump() {
            static const uint64_t JUMP[4] = {
                0x180ec6d33cfd0aba,
                0xd5a61266f0c9392c,
                0xa9582618e03fc9aa,
                0x39abdc4529b1661c,
            };
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for (int i = 0; i < 4; i++) {
                for (int b = 0; b < 64; b++) {
                    if (JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                        s2 ^= s[2];
                        s3 ^= s[3];
                    }
                    next();
                }
            }
            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }
        void long_jump() {
            static const uint64_t LONG_JUMP[4] = {
                0x76e15d3efefdcbbf,
                0xc5004e441c522fb3,
                0x77710069854ee241,
                0x39109bb02acbe635,
            };
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for (int i = 0; i < 4; i++) {
                for (int b = 0; b < 64; b++) {
                    if (LONG_JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                        s2 ^= s[2];
                        s3 ^= s[3];
                    }
                    next();
                }
            }
            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }
    public:
        xoshiro256pp() {
            seed(uint64_t(this));
        }
        explicit xoshiro256pp(uint64_t seedv) {
            seed(seedv);
        }
    };
    
    class xoshiro256ss : public xoshiro256_family {
    public:
        uint64_t next() override {
            const uint64_t result = rotl(s[1] * 5, 7) * 9;
            
            const uint64_t t = s[1] << 17;
            
            s[2] ^= s[0];
            s[3] ^= s[1];
            s[1] ^= s[2];
            s[0] ^= s[3];
            
            s[2] ^= t;
            
            s[3] = rotl(s[3], 45);
            
            return result;
        }
        void jump() {
            static const uint64_t JUMP[4] = {
                0x180ec6d33cfd0aba,
                0xd5a61266f0c9392c,
                0xa9582618e03fc9aa,
                0x39abdc4529b1661c,
            };
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for (int i = 0; i < 4; i++) {
                for (int b = 0; b < 64; b++) {
                    if (JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                        s2 ^= s[2];
                        s3 ^= s[3];
                    }
                    next();
                }
            }
            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }
        void long_jump() {
            static const uint64_t LONG_JUMP[4] = {
                0x76e15d3efefdcbbf,
                0xc5004e441c522fb3,
                0x77710069854ee241,
                0x39109bb02acbe635,
            };
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for (int i = 0; i < 4; i++) {
                for (int b = 0; b < 64; b++) {
                    if (LONG_JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                        s2 ^= s[2];
                        s3 ^= s[3];
                    }
                    next();
                }
            }
            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }
    public:
        xoshiro256ss() {
            seed(uint64_t(this));
        }
        explicit xoshiro256ss(uint64_t seedv) {
            seed(seedv);
        }
    };
}
