#pragma once
#include "GameResource/ResourceTexture.hpp"
#include "GameResource/ResourceSprite.hpp"
#include "GameResource/ResourceAnimation.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"

namespace luastg
{
	class ResourceAnimationImpl : public ResourceBaseImpl<IResourceAnimation>
	{
	private:
		std::vector<core::ScopeObject<IResourceSprite>> m_sprites;
		uint32_t m_Interval = 1;
		core::Color4B m_vertex_color[4]{ core::Color4B(0xFFFFFFFF), core::Color4B(0xFFFFFFFF), core::Color4B(0xFFFFFFFF), core::Color4B(0xFFFFFFFF) };
		BlendMode m_BlendMode = BlendMode::MulAlpha;
		double m_HalfSizeX = 0.;
		double m_HalfSizeY = 0.;
		bool m_bRectangle = false;
		// 图片精灵是自己创建的
		bool m_is_sprite_cloned = false;
	public:
		size_t GetCount() { return m_sprites.size(); }
		IResourceSprite* GetSprite(uint32_t index);
		uint32_t GetSpriteIndexByTimer(int ani_timer);
		IResourceSprite* GetSpriteByTimer(int ani_timer);
		uint32_t GetInterval() { return m_Interval; }
		void SetVertexColor(core::Color4B color) { m_vertex_color[0] = m_vertex_color[1] = m_vertex_color[2]= m_vertex_color[3] = color; }
		void SetVertexColor(core::Color4B color[4])
		{
			m_vertex_color[0] = color[0];
			m_vertex_color[1] = color[1];
			m_vertex_color[2] = color[2];
			m_vertex_color[3] = color[3];
		}
		void GetVertexColor(core::Color4B color[4])
		{
			color[0] = m_vertex_color[0];
			color[1] = m_vertex_color[1];
			color[2] = m_vertex_color[2];
			color[3] = m_vertex_color[3];
		}
		BlendMode GetBlendMode() { return m_BlendMode; }
		void SetBlendMode(BlendMode m) { m_BlendMode = m; }
		double GetHalfSizeX() { return m_HalfSizeX; }
		double GetHalfSizeY() { return m_HalfSizeY; }
		bool IsRectangle() { return m_bRectangle; }
		bool IsSpriteCloned() { return m_is_sprite_cloned; }
		// 不受全局缩放影响
		void Render(int timer, float x, float y, float rot, float hscale, float vscale, float z);
		void Render(int timer, float x, float y, float rot, float hscale, float vscale, BlendMode blend, core::Color4B color, float z);
	public:
		ResourceAnimationImpl(const char* name, core::ScopeObject<IResourceTexture> tex,
			float x, float y, float w, float h,
			int n, int m, int intv,
			double a, double b, bool rect = false);
		ResourceAnimationImpl(const char* name,
			std::vector<core::ScopeObject<IResourceSprite>> const& sprite_list,
			int intv,
			double a, double b, bool rect = false);
	};
}
