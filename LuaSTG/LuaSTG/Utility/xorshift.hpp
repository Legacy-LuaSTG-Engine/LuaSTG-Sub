// Source Code From: https://prng.di.unimi.it/
// Reorganize code into C++ style by 璀境石
//
// Written in 2019 by David Blackman and Sebastiano Vigna (vigna@acm.org)
//
// To the extent possible under law, the author has dedicated all copyright
// and related and neighboring rights to this software to the public domain
// worldwide. This software is distributed without any warranty.
//
// See: http://creativecommons.org/publicdomain/zero/1.0/

#pragma once
#include <cstdint>

namespace random
{
    class splitmix64
    {
    public:
        using result_type = uint64_t;

    private:
        uint64_t x = 0;

    public:
        static uint64_t min()
        {
            return UINT64_C(0);
        }
        static uint64_t max()
        {
            return UINT64_MAX;
        }
        void seed(uint64_t seedv)
        {
            x = seedv;
        }
        uint64_t next()
        {
            uint64_t z = (x += 0x9e3779b97f4a7c15);
            z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
            z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
            return z ^ (z >> 31);
        }
        inline uint64_t operator()()
        {
            return next();
        }

    public:
        splitmix64()
        {
            x = uint64_t(this);
        }
        explicit splitmix64(uint64_t s) : x(s) {}
    };

    inline uint64_t rotl(const uint64_t x, const int k)
    {
        return (x << k) | (x >> (64 - k));
    }

    // https://github.com/imneme/pcg-cpp/blob/428802d1a5634f96bcd0705fab379ff0113bcf13/include/pcg_extras.hpp#L540
    template <typename RNG>
    inline uint64_t bounded_rand(RNG &rng, uint64_t upper_bound)
    {
        uint64_t const threshold = (RNG::max() - RNG::min() + uint64_t(1) - upper_bound) % upper_bound;
        for (;;)
        {
            uint64_t const r = rng() - RNG::min();
            if (r >= threshold)
                return r % upper_bound;
        }
    }

    inline double to_double(const uint64_t x)
    {
        return (x >> 11) * 0x1.0p-53;
    }

    class xoroshiro128_family
    {
    public:
        using result_type = uint64_t;

    protected:
        uint64_t s[2] = {};
        inline void jump_by_table(uint64_t const JUMP_TABLE[2])
        {
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            for (int i = 0; i < 2; i++)
            {
                for (int b = 0; b < 64; b++)
                {
                    if (JUMP_TABLE[i] & UINT64_C(1) << b)
                    {
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
        static uint64_t min()
        {
            return UINT64_C(0);
        }
        static uint64_t max()
        {
            return UINT64_MAX;
        }
        void seed(uint64_t seedv)
        {
            splitmix64 gn(seedv);
            s[0] = gn.next();
            s[1] = gn.next();
        }
        virtual uint64_t next()
        {
            return 0;
        }
        inline uint64_t operator()()
        {
            return next();
        }
    };

    class xoroshiro128p : public xoroshiro128_family
    {
    public:
        uint64_t next() override
        {
            const uint64_t s0 = s[0];
            uint64_t s1 = s[1];
            const uint64_t result = s0 + s1;

            s1 ^= s0;
            s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
            s[1] = rotl(s1, 37);                   // c

            return result;
        }
        void jump()
        {
            constexpr uint64_t const JUMP[2] = {
                0xdf900294d8f554a5,
                0x170865df4b3201fc,
            };
            jump_by_table(JUMP);
        }
        void long_jump()
        {
            constexpr uint64_t const LONG_JUMP[2] = {
                0xd2a98b26625eee7b,
                0xdddf9b1090aa7ac1,
            };
            jump_by_table(LONG_JUMP);
        }

    public:
        xoroshiro128p()
        {
            seed(uint64_t(this));
        }
        explicit xoroshiro128p(uint64_t seedv)
        {
            seed(seedv);
        }
        ~xoroshiro128p() {}
    };

    class xoroshiro128pp : public xoroshiro128_family
    {
    public:
        uint64_t next() override
        {
            const uint64_t s0 = s[0];
            uint64_t s1 = s[1];
            const uint64_t result = rotl(s0 + s1, 17) + s0;

            s1 ^= s0;
            s[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21); // a, b
            s[1] = rotl(s1, 28);                   // c

            return result;
        }
        void jump()
        {
            constexpr uint64_t const JUMP[2] = {
                0x2bd7a6a6e99c2ddc,
                0x0992ccaf6a6fca05,
            };
            jump_by_table(JUMP);
        }
        void long_jump()
        {
            constexpr uint64_t const LONG_JUMP[2] = {
                0x360fd5f2cf8d5d99,
                0x9c6e6877736c46e3,
            };
            jump_by_table(LONG_JUMP);
        }

    public:
        xoroshiro128pp()
        {
            seed(uint64_t(this));
        }
        explicit xoroshiro128pp(uint64_t seedv)
        {
            seed(seedv);
        }
        ~xoroshiro128pp() {}
    };

    class xoroshiro128ss : public xoroshiro128_family
    {
    public:
        uint64_t next() override
        {
            const uint64_t s0 = s[0];
            uint64_t s1 = s[1];
            const uint64_t result = rotl(s0 * 5, 7) * 9;

            s1 ^= s0;
            s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
            s[1] = rotl(s1, 37);                   // c

            return result;
        }
        void jump()
        {
            constexpr uint64_t const JUMP[2] = {
                0xdf900294d8f554a5,
                0x170865df4b3201fc,
            };
            jump_by_table(JUMP);
        }
        void long_jump()
        {
            constexpr uint64_t const LONG_JUMP[2] = {
                0xd2a98b26625eee7b,
                0xdddf9b1090aa7ac1,
            };
            jump_by_table(LONG_JUMP);
        }

    public:
        xoroshiro128ss()
        {
            seed(uint64_t(this));
        }
        explicit xoroshiro128ss(uint64_t seedv)
        {
            seed(seedv);
        }
        ~xoroshiro128ss() {}
    };

    class xoshiro256_family
    {
    public:
        using result_type = uint64_t;

    protected:
        uint64_t s[4] = {};
        inline void jump_by_table(uint64_t const JUMP_TABLE[4])
        {
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for (int i = 0; i < 4; i++)
            {
                for (int b = 0; b < 64; b++)
                {
                    if (JUMP_TABLE[i] & UINT64_C(1) << b)
                    {
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
        static uint64_t min()
        {
            return UINT64_C(0);
        }
        static uint64_t max()
        {
            return UINT64_MAX;
        }
        void seed(uint64_t seedv)
        {
            splitmix64 gn(seedv);
            s[0] = gn.next();
            s[1] = gn.next();
            s[2] = gn.next();
            s[3] = gn.next();
        }
        virtual uint64_t next()
        {
            return 0;
        }
        inline uint64_t operator()()
        {
            return next();
        }
        void jump()
        {
            constexpr uint64_t const JUMP[4] = {
                0x180ec6d33cfd0aba,
                0xd5a61266f0c9392c,
                0xa9582618e03fc9aa,
                0x39abdc4529b1661c,
            };
            jump_by_table(JUMP);
        }
        void long_jump()
        {
            constexpr uint64_t const LONG_JUMP[4] = {
                0x76e15d3efefdcbbf,
                0xc5004e441c522fb3,
                0x77710069854ee241,
                0x39109bb02acbe635,
            };
            jump_by_table(LONG_JUMP);
        }
    };

    class xoshiro256p : public xoshiro256_family
    {
    public:
        uint64_t next() override
        {
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

    public:
        xoshiro256p()
        {
            seed(uint64_t(this));
        }
        explicit xoshiro256p(uint64_t seedv)
        {
            seed(seedv);
        }
        ~xoshiro256p() {}
    };

    class xoshiro256pp : public xoshiro256_family
    {
    public:
        uint64_t next() override
        {
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

    public:
        xoshiro256pp()
        {
            seed(uint64_t(this));
        }
        explicit xoshiro256pp(uint64_t seedv)
        {
            seed(seedv);
        }
        ~xoshiro256pp() {}
    };

    class xoshiro256ss : public xoshiro256_family
    {
    public:
        uint64_t next() override
        {
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

    public:
        xoshiro256ss()
        {
            seed(uint64_t(this));
        }
        explicit xoshiro256ss(uint64_t seedv)
        {
            seed(seedv);
        }
        ~xoshiro256ss() {}
    };

    class xoshiro512_family
    {
    public:
        using result_type = uint64_t;

    protected:
        uint64_t s[8] = {};
        inline void jump_by_table(uint64_t const JUMP_TABLE[8])
        {
            uint64_t t[8] = {};
            for (int i = 0; i < 8; i++)
            {
                for (int b = 0; b < 64; b++)
                {
                    if (JUMP_TABLE[i] & UINT64_C(1) << b)
                    {
                        for (int w = 0; w < 8; w++)
                            t[w] ^= s[w];
                    }
                    next();
                }
            }

            for (int i = 0; i < 8; i++)
                s[i] = t[i];
        }

    public:
        static uint64_t min()
        {
            return UINT64_C(0);
        }
        static uint64_t max()
        {
            return UINT64_MAX;
        }
        void seed(uint64_t seedv)
        {
            splitmix64 gn(seedv);
            for (int i = 0; i < 8; i++)
                s[i] = gn.next();
        }
        virtual uint64_t next()
        {
            return 0;
        }
        inline uint64_t operator()()
        {
            return next();
        }
        void jump()
        {
            constexpr uint64_t const JUMP[] = {
                0x33ed89b6e7a353f9,
                0x760083d7955323be,
                0x2837f2fbb5f22fae,
                0x4b8c5674d309511c,
                0xb11ac47a7ba28c25,
                0xf1be7667092bcc1c,
                0x53851efdb6df0aaf,
                0x1ebbc8b23eaf25db,
            };
            jump_by_table(JUMP);
        }
        void long_jump()
        {
            constexpr uint64_t const LONG_JUMP[] = {
                0x11467fef8f921d28,
                0xa2a819f2e79c8ea8,
                0xa8299fc284b3959a,
                0xb4d347340ca63ee1,
                0x1cb0940bedbff6ce,
                0xd956c5c4fa1f8e17,
                0x915e38fd4eda93bc,
                0x5b3ccdfa5d7daca5,
            };
            jump_by_table(LONG_JUMP);
        }
    };

    class xoshiro512p : public xoshiro512_family
    {
    public:
        uint64_t next() override
        {
            const uint64_t result = s[0] + s[2];

            const uint64_t t = s[1] << 11;

            s[2] ^= s[0];
            s[5] ^= s[1];
            s[1] ^= s[2];
            s[7] ^= s[3];
            s[3] ^= s[4];
            s[4] ^= s[5];
            s[0] ^= s[6];
            s[6] ^= s[7];

            s[6] ^= t;

            s[7] = rotl(s[7], 21);

            return result;
        }

    public:
        xoshiro512p()
        {
            seed(uint64_t(this));
        }
        xoshiro512p(uint64_t seedv)
        {
            seed(seedv);
        }
        ~xoshiro512p() {}
    };

    class xoshiro512pp : public xoshiro512_family
    {
    public:
        uint64_t next() override
        {
            const uint64_t result = rotl(s[0] + s[2], 17) + s[2];

            const uint64_t t = s[1] << 11;

            s[2] ^= s[0];
            s[5] ^= s[1];
            s[1] ^= s[2];
            s[7] ^= s[3];
            s[3] ^= s[4];
            s[4] ^= s[5];
            s[0] ^= s[6];
            s[6] ^= s[7];

            s[6] ^= t;

            s[7] = rotl(s[7], 21);

            return result;
        }

    public:
        xoshiro512pp()
        {
            seed(uint64_t(this));
        }
        xoshiro512pp(uint64_t seedv)
        {
            seed(seedv);
        }
        ~xoshiro512pp() {}
    };

    class xoshiro512ss : public xoshiro512_family
    {
    public:
        uint64_t next() override
        {
            const uint64_t result = rotl(s[1] * 5, 7) * 9;

            const uint64_t t = s[1] << 11;

            s[2] ^= s[0];
            s[5] ^= s[1];
            s[1] ^= s[2];
            s[7] ^= s[3];
            s[3] ^= s[4];
            s[4] ^= s[5];
            s[0] ^= s[6];
            s[6] ^= s[7];

            s[6] ^= t;

            s[7] = rotl(s[7], 21);

            return result;
        }

    public:
        xoshiro512ss()
        {
            seed(uint64_t(this));
        }
        xoshiro512ss(uint64_t seedv)
        {
            seed(seedv);
        }
        ~xoshiro512ss() {}
    };

    class xoroshiro1024_family
    {
    public:
        using result_type = uint64_t;

    protected:
        uint64_t s[16] = {};
        int p = 0; // TODO: WTF? How should I init it?
        inline void jump_by_table(uint64_t const JUMP_TABLE[16])
        {
            uint64_t t[16] = {};
            for (int i = 0; i < 16; i++)
            {
                for (int b = 0; b < 64; b++)
                {
                    if (JUMP_TABLE[i] & UINT64_C(1) << b)
                    {
                        for (int j = 0; j < 16; j++)
                            t[j] ^= s[(j + p) & 16 - 1];
                    }
                    next();
                }
            }

            for (int i = 0; i < 16; i++)
            {
                s[(i + p) & 16 - 1] = t[i];
            }
        }

    public:
        static uint64_t min()
        {
            return UINT64_C(0);
        }
        static uint64_t max()
        {
            return UINT64_MAX;
        }
        void seed(uint64_t seedv)
        {
            splitmix64 gn(seedv);
            for (int i = 0; i < 16; i++)
                s[i] = gn.next();
            p = 0; // TODO: WTF? How should I init it?
        }
        virtual uint64_t next()
        {
            return 0;
        }
        inline uint64_t operator()()
        {
            return next();
        }
        void jump()
        {
            constexpr uint64_t const JUMP[] = {
                0x931197d8e3177f17,
                0xb59422e0b9138c5f,
                0xf06a6afb49d668bb,
                0xacb8a6412c8a1401,
                0x12304ec85f0b3468,
                0xb7dfe7079209891e,
                0x405b7eec77d9eb14,
                0x34ead68280c44e4a,
                0xe0e4ba3e0ac9e366,
                0x8f46eda8348905b7,
                0x328bf4dbad90d6ff,
                0xc8fd6fb31c9effc3,
                0xe899d452d4b67652,
                0x45f387286ade3205,
                0x03864f454a8920bd,
                0xa68fa28725b1b384,
            };
            jump_by_table(JUMP);
        }
        void long_jump()
        {
            constexpr uint64_t const LONG_JUMP[] = {
                0x7374156360bbf00f,
                0x4630c2efa3b3c1f6,
                0x6654183a892786b1,
                0x94f7bfcbfb0f1661,
                0x27d8243d3d13eb2d,
                0x9701730f3dfb300f,
                0x2f293baae6f604ad,
                0xa661831cb60cd8b6,
                0x68280c77d9fe008c,
                0x50554160f5ba9459,
                0x2fc20b17ec7b2a9a,
                0x49189bbdc8ec9f8f,
                0x92a65bca41852cc1,
                0xf46820dd0509c12a,
                0x52b00c35fbf92185,
                0x1e5b3b7f589e03c1,
            };
            jump_by_table(LONG_JUMP);
        }
    };

    class xoroshiro1024s : public xoroshiro1024_family
    {
    public:
        uint64_t next() override
        {
            const int q = p;
            const uint64_t s0 = s[p = (p + 1) & 15];
            uint64_t s15 = s[q];
            const uint64_t result = s0 * 0x9e3779b97f4a7c13;

            s15 ^= s0;
            s[q] = rotl(s0, 25) ^ s15 ^ (s15 << 27);
            s[p] = rotl(s15, 36);

            return result;
        }

    public:
        xoroshiro1024s()
        {
            seed(uint64_t(this));
        }
        xoroshiro1024s(uint64_t seedv)
        {
            seed(seedv);
        }
        ~xoroshiro1024s() {}
    };

    class xoroshiro1024pp : public xoroshiro1024_family
    {
    public:
        uint64_t next() override
        {
            const int q = p;
            const uint64_t s0 = s[p = (p + 1) & 15];
            uint64_t s15 = s[q];
            const uint64_t result = rotl(s0 + s15, 23) + s15;

            s15 ^= s0;
            s[q] = rotl(s0, 25) ^ s15 ^ (s15 << 27);
            s[p] = rotl(s15, 36);

            return result;
        }

    public:
        xoroshiro1024pp()
        {
            seed(uint64_t(this));
        }
        xoroshiro1024pp(uint64_t seedv)
        {
            seed(seedv);
        }
        ~xoroshiro1024pp() {}
    };

    class xoroshiro1024ss : public xoroshiro1024_family
    {
    public:
        uint64_t next() override
        {
            const int q = p;
            const uint64_t s0 = s[p = (p + 1) & 15];
            uint64_t s15 = s[q];
            const uint64_t result = rotl(s0 * 5, 7) * 9;

            s15 ^= s0;
            s[q] = rotl(s0, 25) ^ s15 ^ (s15 << 27);
            s[p] = rotl(s15, 36);

            return result;
        }

    public:
        xoroshiro1024ss()
        {
            seed(uint64_t(this));
        }
        xoroshiro1024ss(uint64_t seedv)
        {
            seed(seedv);
        }
        ~xoroshiro1024ss() {}
    };
}
