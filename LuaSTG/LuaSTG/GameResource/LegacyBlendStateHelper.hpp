#pragma once
#include "Core/Graphics/Renderer.hpp"
#include "GameResource/ResourceBase.hpp"

namespace luastg {
	struct LegacyBlendStateData {
		Core::Graphics::IRenderer::VertexColorBlendState vertex_color_blend_state{ Core::Graphics::IRenderer::VertexColorBlendState::Mul };
		Core::Graphics::IRenderer::BlendState blend_state{ Core::Graphics::IRenderer::BlendState::Alpha };
	};

	inline LegacyBlendStateData translateLegacyBlendState(BlendMode const legacy_blend_state) {
		LegacyBlendStateData data;
	#define CASE(A, B, C) case BlendMode::##A: data.vertex_color_blend_state = Core::Graphics::IRenderer::VertexColorBlendState::##B; data.blend_state = Core::Graphics::IRenderer::BlendState::##C; break
		switch (legacy_blend_state) {  // NOLINT(clang-diagnostic-switch-enum)
		default:
			CASE(MulAlpha, Mul, Alpha);
			CASE(MulAdd, Mul, Add);
			CASE(MulRev, Mul, RevSub);
			CASE(MulSub, Mul, Sub);
			CASE(AddAlpha, Add, Alpha);
			CASE(AddAdd, Add, Add);
			CASE(AddRev, Add, RevSub);
			CASE(AddSub, Add, Sub);
			CASE(AlphaBal, Mul, Inv);
			CASE(MulMin, Mul, Min);
			CASE(MulMax, Mul, Max);
			CASE(MulMutiply, Mul, Mul);
			CASE(MulScreen, Mul, Screen);
			CASE(AddMin, Add, Min);
			CASE(AddMax, Add, Max);
			CASE(AddMutiply, Add, Mul);
			CASE(AddScreen, Add, Screen);
			CASE(One, Mul, One);
		}
	#undef CASE
		return data;
	}
}
