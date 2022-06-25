#ifndef SFC_HPP_INCLUDED
#define SFC_HPP_INCLUDED 1

/*
 * A C++ implementation of Chris Doty-Humphrey's SFC PRNG(s)
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

/* Based on code written by Chris Doty-Humphrey, adapted for C++-style
 * random-number generation.
 */

#include <cstdint>

namespace sfc_detail {

template <typename itype, typename rtype,
          unsigned int p, unsigned int q, unsigned int r>
class sfc {
protected:
    itype a_, b_, c_, d_;

    static constexpr unsigned int ITYPE_BITS = 8*sizeof(itype);
    static constexpr unsigned int RTYPE_BITS = 8*sizeof(rtype);

    static itype rotate(itype x, unsigned int k)
    {
        return (x << k) | (x >> (ITYPE_BITS - k));
    }

public:
    using result_type = rtype;
    using state_type = itype;

    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return ~ result_type(0); }

    sfc(itype seed = itype(0xcafef00dbeef5eedULL))
        : sfc(seed, seed, seed)
    {
        // Nothing (else) to do
    }
    
    sfc(itype seed1, itype seed2, itype seed3)
        : a_(seed3), b_(seed2), c_(seed1), d_(itype(1))
    {
        for (unsigned int i=0; i < 12; ++i)
            advance();
    }

    void advance()
    {
        (void)operator()();
    }

    rtype operator()()
    {
        itype tmp = a_ + b_ + d_++;
        a_ = b_ ^ (b_ >> q);
        b_ = c_ + (c_ << r);
        c_ = rotate(c_, p) + tmp;
        return rtype(tmp);
    }

    bool operator==(const sfc& rhs)
    {
        return (a_ == rhs.a_) && (b_ == rhs.b_) 
            && (c_ == rhs.c_) && (d_ == rhs.d_);
    }

    bool operator!=(const sfc& rhs)
    {
        return !operator==(rhs);
    }

    // Not (yet) implemented:
    //   - arbitrary jumpahead (doable, but annoying to write).
    //   - I/O
    //   - Seeding from a seed_seq.
};

} // end namespace

///// ---- Specific SFC Generators ---- ////
//
// Each size has variations corresponding to different parameter sets.
// Each variant will create a distinct (and hopefully statistically
// independent) sequence.
//

// - 256 state bits, uint64_t output

using sfc64a = sfc_detail::sfc<uint64_t, uint64_t, 24,11,3>;
using sfc64b = sfc_detail::sfc<uint64_t, uint64_t, 25,12,3>; // old, less good

using sfc64 = sfc64a;

// - 128 state bits, uint32_t output

using sfc32a = sfc_detail::sfc<uint32_t, uint32_t, 21,9,3>;
using sfc32b = sfc_detail::sfc<uint32_t, uint32_t, 15,8,3>;
using sfc32c = sfc_detail::sfc<uint32_t, uint32_t, 25,8,3>; // old, less good

using sfc32 = sfc32a;

// TINY VERSIONS FOR TESTING AND SPECIALIZED USES ONLY

// - 64 state bits, uint16_t output

using sfc16a = sfc_detail::sfc<uint16_t, uint16_t, 4,3,2>;
using sfc16b = sfc_detail::sfc<uint16_t, uint16_t, 6,5,2>;
using sfc16c = sfc_detail::sfc<uint16_t, uint16_t, 4,5,3>;
using sfc16d = sfc_detail::sfc<uint16_t, uint16_t, 6,5,3>;
using sfc16e = sfc_detail::sfc<uint16_t, uint16_t, 7,5,3>;
using sfc16f = sfc_detail::sfc<uint16_t, uint16_t, 7,3,2>; // old, less good

using sfc16 = sfc16d;

// - 32 state bits, uint8_t output
// Not by Chris

using sfc8 = sfc_detail::sfc<uint8_t, uint8_t, 3, 2, 1>;

#endif // SFC_HPP_INCLUDED
