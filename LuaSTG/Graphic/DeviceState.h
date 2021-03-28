#pragma once
#include "Graphic/Type.h"
#include "Graphic/Object.h"

namespace slow::Graphic
{
    // common
    
    enum class EComparisonFunction : uint8_t
    {
        Never        = 1,
        Less         = 2,
        Equal        = 3,
        LessEqual    = 4,
        Greater      = 5,
        NotEqual     = 6,
        GreaterEqual = 7,
        Always       = 8,
    };
    
    //
    
    class IInputLayout : public IObject
    {
    };
    
    // rasterizer state
    
    enum class EFillMode : uint8_t
    {
        WireFrame = 2,
        Solid     = 3,
    };
    
    enum class ECullMode : uint8_t
    {
        None  = 1,
        Front = 2,
        Back  = 3,
    };
    
    struct DRasterizerState
    {
        EFillMode fillMode               = EFillMode::Solid;
        ECullMode cullMode               = ECullMode::None;
        uint8_t   frontCounterClockwise  = false;
        uint8_t   depthClipEnable        = true;
        uint8_t   scissorEnable          = true;
        uint8_t   multiSampleEnable      = false;
        uint8_t   antialiasingLineEnable = false;
        uint8_t   _                      = 0;
    };
    
    class IRasterizerState : public IObject
    {
    public:
        virtual handle_t getHandle() = 0;
        virtual DRasterizerState getDefinition() = 0;
    };
    
    // sampler state
    
    enum class ETextureFilter : uint8_t
    {
        Point        = 0x00,
        MipLinear    = 0x01,
        MagLinear    = 0x04,
        MagMipLinear = 0x05,
        MinLinear    = 0x10,
        MinMipLinear = 0x11,
        MinMagLinear = 0x14,
        Linear       = 0x15,
        Anisotropic  = 0x55,
    };
    
    enum class ETextureAddress : uint8_t
    {
        Wrap       = 1,
        Mirror     = 2,
        Clamp      = 3,
        Border     = 4,
        MirrorOnce = 5
    };
    
    struct DSamplerState
    {
        ETextureFilter  filter         = ETextureFilter::Linear;
        ETextureAddress addressU       = ETextureAddress::Clamp;
        ETextureAddress addressV       = ETextureAddress::Clamp;
        ETextureAddress addressW       = ETextureAddress::Clamp;
        uint32_t        maxAnisotropy  = 1;
        float           borderColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    };
    
    class ISamplerState : public IObject
    {
    public:
        virtual handle_t getHandle() = 0;
        virtual DSamplerState getDefinition() = 0;
    };
    
    // depth stencil state
    
    enum class EDepthWriteEnable : uint8_t
    {
        None  = 0,
        All   = 1,
    };
    
    struct DDepthStencilState
    {
        uint8_t             depthEnable      = false;
        EDepthWriteEnable   depthWriteEnable = EDepthWriteEnable::All;
        EComparisonFunction depthFunction    = EComparisonFunction::Less;
        uint8_t             _                = 0;
    };
    
    class IDepthStencilState : public IObject
    {
    public:
        virtual handle_t getHandle() = 0;
        virtual DDepthStencilState getDefinition() = 0;
    };
    
    // blend state
    
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
