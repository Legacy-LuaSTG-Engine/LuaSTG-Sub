#pragma once
#include "Graphic/Type.h"
#include "Graphic/Object.h"

namespace slow::Graphic
{
    class IInputLayout : public IObject
    {
    };
    
    class IRasterizerState : public IObject
    {
    };
    
    class ISamplerState : public IObject
    {
    };
    
    class IDepthStencilState : public IObject
    {
    };
    
    enum class EBlend : uint8_t
    {
        Zero           =  1,
        One            =  2,
        OutputColor    =  3,
        InvOutputColor =  4,
        OutputAlpha    =  5,
        InvOutputAlpha =  6,
        BufferAlpha    =  7,
        InvBufferAlpha =  8,
        BufferColor    =  9,
        InvBufferColor = 10,
        BlendFactor    = 14,
        InvBlendFactor = 15,
    };
    
    enum class EBlendOperate : uint8_t
    {
        Add    = 1,
        Sub    = 2,
        RevSub = 3,
        Min    = 4,
        Max    = 5,
    };
    
    enum class EColorWriteEnable : uint8_t
    {
        None  = 0b0000,
        Red   = 0b0001,
        Green = 0b0010,
        Blue  = 0b0100,
        Alpha = 0b1000,
        All   = 0b1111,
        X     = 0b0000,
        R     = 0b0001,
        G     = 0b0010,
        B     = 0b0100,
        A     = 0b1000,
        RG    = 0b0011,
        RB    = 0b0101,
        RA    = 0b1001,
        GB    = 0b0110,
        GA    = 0b1010,
        BA    = 0b1100,
        RGB   = 0b0111,
        RGA   = 0b1011,
        RBA   = 0b1101,
        GBA   = 0b1110,
        RGBA  = 0b1111,
    };
    
    struct DBlendState
    {
        uint8_t           enable       = false;
        EBlend            outputColor  = EBlend::One;
        EBlend            bufferColor  = EBlend::Zero;
        EBlendOperate     colorOperate = EBlendOperate::Add;
        EBlend            outputAlpha  = EBlend::One;
        EBlend            bufferAlpha  = EBlend::Zero;
        EBlendOperate     alphaOperate = EBlendOperate::Add;
        EColorWriteEnable writeEnable  = EColorWriteEnable::All;
    };
    
    class IBlendState : public IObject
    {
    public:
        virtual handle_t getHandle() = 0;
        virtual DBlendState getDefinition() = 0;
    };
};
