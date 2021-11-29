#pragma once
#include "slow/Type.hpp"
#include "slow/Object.hpp"

namespace slow::Graphics
{
	enum class EResourceType : u8
	{
		Unknown,
		Buffer,
		VertexBuffer,
		IndexBuffer,
		ConstantBuffer,
		Texture,
		Texture2D,
		RenderTarget,
		DepthStencil,
	};

	struct IResource : public IObject
	{
		virtual EResourceType getType() const noexcept = 0;
		virtual void* getNativeResource() const noexcept = 0;
	};

	struct ITexture : public IResource
	{
		virtual u32 getWidth() const noexcept = 0;
		virtual u32 getHeight() const noexcept = 0;
	};

	struct ITexture2D : public ITexture
	{
		virtual void* getNativeShaderResourceView() const noexcept = 0;

		virtual b8 isDynamic() const noexcept = 0;
		// autoOffset: if your data's size (width x height) is same as texture's size (width x height) and the rect position (left, top) is not at (0, 0)
		virtual b8 updateRegion(u32x4 rect, u8view data, u32 rowPitch, b8 autoOffset = false) = 0;
		virtual b8 saveToFile(c8view path, b8 isJPEG = false) = 0;
	};

	struct IRenderTarget : public ITexture
	{
		virtual void* getNativeShaderResourceView() const noexcept = 0;
		virtual void* getNativeRenderTargetView() const noexcept = 0;

		virtual b8 fillColor(f32x4 color) = 0;
		virtual b8 saveToFile(c8view path, b8 isJPEG = false) = 0;
	};

	struct IDepthStencil : public ITexture
	{
		virtual void* getNativeDepthStencilView() const noexcept = 0;

		virtual b8 fillDepth(f32 depth) = 0;
		virtual b8 fillStencil(u8 stencil) = 0;
		virtual b8 fillDepthStencil(f32 depth, u8 stencil) = 0;
	};
}
