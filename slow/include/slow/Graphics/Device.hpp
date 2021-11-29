#pragma once
#include "slow/Graphics/PipelineState.hpp"
#include "slow/Graphics/Resource.hpp"

namespace slow::Graphics
{
	struct ICommandList : public IObject
	{
		virtual void* getNativeCommandList() const noexcept = 0;
		
		virtual void setPrimitiveTopology(EPrimitiveTopology type) = 0;
		virtual void setViewport(f32x4 viewport) = 0;
		virtual void setScissorRect(f32x4 scissor) = 0;
		virtual void setStencilRef(u32 value) = 0;
		virtual void setBlendFactor(f32x4 factor) = 0;
		virtual void setPipelineState(IPipelineState* state) = 0;
	};

	struct IDevice : public IObject
	{
		virtual void* getNativeDevice() const noexcept = 0;

		virtual b8 createTexture2D(u32 width, u32 height, b8 dynamic, ITexture2D** outObject) = 0;
		virtual b8 createTexture2DFromFile(c8view path, b8 mipmap, ITexture2D** outObject) = 0;
		virtual b8 createTexture2DFromMemory(u8view data, b8 mipmap, ITexture2D** outObject) = 0;
		virtual b8 createRenderTarget(u32 width, u32 height, IRenderTarget** outObject) = 0;
		virtual b8 createDepthStencil(u32 width, u32 height, IDepthStencil** outObject) = 0;

		virtual b8 createPipelineState(TPipelineState def, IPipelineState** outObject) = 0;
	};

	b8 createDevice(IDevice** outObject);
}
