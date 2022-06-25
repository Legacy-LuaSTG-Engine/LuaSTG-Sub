#ifndef SPLITMIX_HPP_INCLUDED
#define SPLITMIX_HPP_INCLUDED 1

/*
 * A C++ implementation of SplitMix
 *   Original design by Guy L. Steele, Jr., Doug Lea and Christine H. Flood
 *   Described in _Fast splittable pseudorandom number generators_
 *       http://dx.doi.org/10.1145/2714064.2660195 and implemented in
 *       Java 8 as SplittableRandom
 *   Based on code from the original paper, with revisions based on changes
 *   made to the the Java 8 source, at
 *      http://hg.openjdk.java.net/jdk8/jdk8/jdk/file/tip/
 *             src/share/classes/java/util/SplittableRandom.java
 *   and other publicly available implementations.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Melissa E. O'Neill
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <cstdint>

namespace splitmix_detail {

template <typename IntRep>
IntRep fast_exp(IntRep x, IntRep power)
{
    IntRep result = IntRep(1);
    IntRep multiplier = x;
    while (power != IntRep(0)) {
        IntRep thismult = power & IntRep(1) ? multiplier : IntRep(1);
        result *= thismult;
        power >>= 1;
        multiplier *= multiplier;
    }
    return result;
}

template <typename IntRep>
inline IntRep modular_inverse(IntRep x)
{
    return fast_exp(x, IntRep(-1));
}

#ifndef __GNUC__

// Fallback code based on code at https://en.wikipedia.org/wiki/Hamming_weight
// use a popcount intrinsic if you can since most modern CPUs have one.

template <typename IntRep>
inline unsigned int pop_count(IntRep x) {
    unsigned int count = 0;
    while (x) {
        ++count;
        x &= x - IntRep(1);
    }
    return count;
}

#else

template <typename IntRep>
inline IntRep pop_count(IntRep x) {
    if (sizeof(int) <= sizeof(IntRep))
        return __builtin_popcount(x);
    else if (sizeof(long) <= sizeof(IntRep))
        return __builtin_popcountl(x);
    else
        return __builtin_popcountll(x);
}
#endif


template <uint64_t m1, uint64_t m2, 
          unsigned int p, unsigned int q, unsigned int r,
          uint64_t m3, uint64_t m4,
          unsigned int s, unsigned int t, unsigned int u>
class splitmix64_base {
public:
    using result_type = uint64_t;
    static constexpr result_type min() { return 0; };
    static constexpr result_type max() { return ~result_type(0); };

protected:

    uint64_t seed_;
    uint64_t gamma_;

    static uint64_t mix_gamma(uint64_t x) {
        x ^= x >> p;
        x *= m1; 
        x ^= x >> q;
        x *= m2;
        x ^= x >> r;
        x |= 1ul;
        int n = pop_count(x ^ (x >> 1));
        return (n < 24) ? x ^ 0xaaaaaaaaaaaaaaaa : x;
    }

    static uint64_t mix64(uint64_t x) {
        x ^= x >> s;
        x *= m3; 
        x ^= x >> t;
        x *= m4;
        x ^= x >> u;
        return x;
    }

    void advance() {
        seed_ += gamma_;
    }

    uint64_t next_seed() {
        uint64_t result = seed_;
        advance();
        return result;
    }

public:
    splitmix64_base(uint64_t seed  = 0xbad0ff1ced15ea5e,
                    uint64_t gamma = 0x9e3779b97f4a7c15)
        : seed_(seed), gamma_(gamma | 1)
    {
        // Nothing (else) to do.
    }

    uint64_t operator()() {
        return mix64(next_seed());
    }

    void advance(uint64_t delta) {
        seed_ += delta * gamma_;
    }

    void backstep(uint64_t delta) {
        advance(-delta);
    }

    bool wrapped() {
        return seed_ == 0;
    }

    uint64_t operator-(const splitmix64_base& other) {
        return (seed_ - other.seed_) * modular_inverse(other.gamma_);
    }

    splitmix64_base split() {
        uint64_t new_seed  = operator()();
        uint64_t new_gamma = mix_gamma(next_seed());
        return { new_seed, new_gamma };
    }

    bool operator==(const splitmix64_base& rhs) {
        return (seed_ == rhs.seed_) && (gamma_ == rhs.gamma_);
    }
};

template <uint64_t m5, uint64_t m6, unsigned int v, unsigned int w,
          typename splitmix>
class splitmix32_base : public splitmix {
public:
    using result_type = uint32_t;
    static constexpr result_type min() { return 0; };
    static constexpr result_type max() { return ~result_type(0); };

    using splitmix::splitmix;

    result_type operator()() {
        uint64_t seed = splitmix::next_seed();
        seed ^= seed >> v;
        seed *= m5;
        seed ^= seed >> w;
        seed *= m6;
        return result_type(seed >> 32);
    }

    splitmix32_base split() {
        return splitmix::split();
    }
};

}

using splitmix64 = splitmix_detail::splitmix64_base<
                       0xff51afd7ed558ccdul, 0xc4ceb9fe1a85ec53ul,
                       33, 33, 33,
                       0xbf58476d1ce4e5b9ul, 0x94d049bb133111ebul,
                       30, 27, 31>;

using splitmix32 = splitmix_detail::splitmix32_base<
                       0x62a9d9ed799705f5ul, 0xcb24d0a5c88c35b3ul,
                       33, 28, splitmix64>;

#endif // SPLITMIX_HPP_INCLUDED
