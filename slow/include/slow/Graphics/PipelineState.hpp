#pragma once
#include "slow/Type.hpp"
#include "slow/Object.hpp"
#include "slow/Graphics/Format.hpp"

namespace slow::Graphics
{
	enum class EPrimitiveTopology : u8
	{
		Unknown = 0,
		PointList = 1,
		LineList = 2,
		LineStrip = 3,
		TriangleList = 4,
		TriangleStrip = 5,
	};

	enum class EInputSlotType : u8
	{
		PerVertexData = 0,
		PerInstanceData = 1,
	};

	struct TInputElement
	{
		c8view semantic_name;
		u32 semantic_index;
		EFormat format;
		u32 input_slot;
		u32 aligned_byte_offset;
		u32 instance_data_step_rate;
		EInputSlotType input_slot_type;
	};

	struct TInputLayout
	{
		TInputElement const* element;
		u32 size;
	};

	enum class EFillMode : u8
	{
		WireFrame = 2,
		Solid = 3,
	};

	enum class ECullMode : u8
	{
		None = 1,
		Front = 2,
		Back = 3,
	};

	struct TRasterizerState
	{
		EFillMode fill_mode;
		ECullMode cull_mode;
		b8 front_counter_clock_wise;
		b8 depth_clip_enable;
		b8 scissor_enable;
		b8 multi_sample_enable;
		b8 antialiased_line_enable;
		u8 _padding;
		i32 depth_bias;
		f32 depth_bias_clamp;
		f32 slope_scaled_depth_bias;
	};

	enum class EDepthWriteMask : u8
	{
		Zero = 0,
		All = 1,
	};

	enum class EComparisonFunction : u8
	{
		Never = 1,
		Less = 2,
		Equal = 3,
		LessEqual = 4,
		Greater = 5,
		NotEqual = 6,
		GreaterEqual = 7,
		Always = 8,
	};

	enum class EStencilOperation : u8
	{
		Keep = 1,
		Zero = 2,
		Replace = 3,
		Increase = 4,
		Decrease = 5,
		Invert = 6,
		IncreaseWrap = 7,
		DecreaseWrap = 8,
	};

	struct TDepthStencilOperation
	{
		EStencilOperation stencil_failed;
		EStencilOperation stencil_depth_failed;
		EStencilOperation stencil_pass;
		EComparisonFunction stencil_function;
	};

	struct TDepthStencilState
	{
		b8 depth_enable;
		EDepthWriteMask depth_write_mask;
		EComparisonFunction depth_function;
		u8 _padding1;

		b8 stencil_enable;
		u8 stencil_read_mask;
		u8 stencil_write_mask;
		u8 _padding2;

		TDepthStencilOperation front_face;
		TDepthStencilOperation back_face;
	};

	enum class EBlendFactor : u8
	{
		Zero = 1,
		One = 2,
		PixelColor = 3,
		PixelColorInv = 4,
		PixelAlpha = 5,
		PixelAlphaInv = 6,
		BufferAlpha = 7,
		BufferAlphaInv = 8,
		BufferColor = 9,
		BufferColorInv = 10,
		PixelAlphaSAT = 11,
		Factor = 14,
		FactorInv = 15,
	};

	enum class EBlendOperation : u8
	{
		Add = 1,
		Sub = 2,
		RevSub = 3,
		Min = 4,
		Max = 5,
	};

	enum class EColorWriteMask : u8
	{
		None = 0,
		Red = 1,
		Green = 2,
		Blue = 4,
		Color = Red | Green | Blue,
		Alpha = 8,
		All = Color | Alpha,
	};

	struct TBlendState
	{
		b8 enable;
		EBlendFactor pixel_color_factor;
		EBlendFactor buffer_color_factor;
		EBlendOperation color_operation;
		EBlendFactor pixel_alpha_factor;
		EBlendFactor buffer_alpha_factor;
		EBlendOperation alpha_operation;
		EColorWriteMask write_mask;
	};

	struct TPipelineState
	{
		u8view VS;
		u8view PS;
		TInputLayout input_layout;
		TRasterizerState rasterizer_state;
		TDepthStencilState depth_stencil_state;
		TBlendState blend_state;
		u32 sample_mask;
	};

	struct IPipelineState : public IObject
	{
		virtual void* getNativePipelineState() const noexcept = 0;
	};
}
