# Design

## D3D10 style

IGraphicsDevice

IVertexBuffer
IIndexBuffer
IConstantBuffer
IRenderTarget
IDepthStencilBuffer
ITexture2D

IInputLayout
IVertexShader
IRasterizerState
ISamplerState
IPixelShader
IDepthStencilState
IBlendState

## Minimal

IGraphicsDevice

IBuffer
IRenderTarget
IDepthStencilBuffer
ITexture2D

IPipelineState
ISamplerState

## Mixed style

IGraphicsDevice

IVertexBuffer
IIndexBuffer
IConstantBuffer
IRenderTarget
IDepthStencilBuffer
ITexture2D

IPipelineState
ISamplerState

## Tree

GraphicsResource
- GraphicsBuffer
    - VertexBuffer
    - IndexBuffer
    - ConstantBuffer
- GraphicsTexture
    - LineTexture
    - SurfaceTexture
    - CubeTexture
    - VolumeTexture

## HLSL Semantics

- LUASTG_CAMERA_POSITION
- LUASTG_CAMERA_UP
- LUASTG_CAMERA_DIRECTION
- LUASTG_TEXTURE_SIZE
- LUASTG_TEXTURE_PREMUL_ALPHA
- LUASTG_FOG_NEAR
- LUASTG_FOG_FAR
- LUASTG_FOG_COLOR
- LUASTG_FOG_DENSITY
