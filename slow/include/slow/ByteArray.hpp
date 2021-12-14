#pragma once
#include "slow/Object.hpp"

namespace slow
{
    struct IByteArray : public IObject
    {
        virtual u8view view() const = 0;
        virtual u8* data() const = 0;
        virtual u32 size() const = 0;
        virtual b8 resizable() const = 0;
        virtual u32 align() const = 0;
        virtual b8 resize(u32 size, u32 align) = 0;
        
        virtual b8 writeu8(u8 v, u32 offset) = 0;
        virtual b8 writeu16(u16 v, u32 offset) = 0;
        virtual b8 writeu32(u32 v, u32 offset) = 0;
        virtual b8 writeu64(u64 v, u32 offset) = 0;
        
        virtual b8 writei8(i8 v, u32 offset) = 0;
        virtual b8 writei16(i16 v, u32 offset) = 0;
        virtual b8 writei32(i32 v, u32 offset) = 0;
        virtual b8 writei64(i64 v, u32 offset) = 0;
        
        virtual b8 writef32(f32 v, u32 offset) = 0;
        virtual b8 writef64(f64 v, u32 offset) = 0;
        
        virtual b8 readu8(u8* v, u32 offset) const = 0;
        virtual b8 readu16(u16* v, u32 offset) const = 0;
        virtual b8 readu32(u32* v, u32 offset) const = 0;
        virtual b8 readu64(u64* v, u32 offset) const = 0;
        
        virtual b8 readi8(i8* v, u32 offset) const = 0;
        virtual b8 readi16(i16* v, u32 offset) const = 0;
        virtual b8 readi32(i32* v, u32 offset) const = 0;
        virtual b8 readi64(i64* v, u32 offset) const = 0;
        
        virtual b8 readf32(f32* v, u32 offset) const = 0;
        virtual b8 readf64(f64* v, u32 offset) const = 0;
    };
    
    b8 createByteArray(IByteArray** obj, u32 size, u32 align, b8 resizable);
    b8 createByteArrayFromFile(IByteArray** obj, c8view path, u32 align);
}
