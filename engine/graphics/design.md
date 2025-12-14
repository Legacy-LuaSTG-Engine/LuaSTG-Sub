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
