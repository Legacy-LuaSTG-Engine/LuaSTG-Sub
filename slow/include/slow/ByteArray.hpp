#pragma once
#include "slow/Object.hpp"

namespace slow
{
    struct IByteArray : public IObject
    {
        virtual u8* data() const = 0;
        virtual u32 size() const = 0;
        virtual bool resizable() const = 0;
        virtual u32 align() const = 0;
        virtual bool resize(u32 size, u32 align) = 0;
        
        virtual bool writeu8(u8 v, u32 offset) = 0;
        virtual bool writeu16(u16 v, u32 offset) = 0;
        virtual bool writeu32(u32 v, u32 offset) = 0;
        virtual bool writeu64(u64 v, u32 offset) = 0;
        
        virtual bool writei8(i8 v, u32 offset) = 0;
        virtual bool writei16(i16 v, u32 offset) = 0;
        virtual bool writei32(i32 v, u32 offset) = 0;
        virtual bool writei64(i64 v, u32 offset) = 0;
        
        virtual bool writef32(f32 v, u32 offset) = 0;
        virtual bool writef64(f64 v, u32 offset) = 0;
        
        virtual bool readu8(u8* v, u32 offset) const = 0;
        virtual bool readu16(u16* v, u32 offset) const = 0;
        virtual bool readu32(u32* v, u32 offset) const = 0;
        virtual bool readu64(u64* v, u32 offset) const = 0;
        
        virtual bool readi8(i8* v, u32 offset) const = 0;
        virtual bool readi16(i16* v, u32 offset) const = 0;
        virtual bool readi32(i32* v, u32 offset) const = 0;
        virtual bool readi64(i64* v, u32 offset) const = 0;
        
        virtual bool readf32(f32* v, u32 offset) const = 0;
        virtual bool readf64(f64* v, u32 offset) const = 0;
    };
    
    bool createByteArray(IByteArray** obj, u32 size, u32 align, bool resizable);
}
