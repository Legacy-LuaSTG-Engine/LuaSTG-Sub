#pragma once
#include "ResourceBase.hpp"
#include "ResourceTexture.hpp"
#include "f2dRenderer.h"
#include <vector>

namespace LuaSTGPlus
{
	// 动画资源
	class ResAnimation :
		public Resource
	{
	private:
		std::vector<fcyRefPointer<f2dSprite>> m_ImageSequences;
		fuInt m_Interval = 1;
		BlendMode m_BlendMode = BlendMode::MulAlpha;
		double m_HalfSizeX = 0.;
		double m_HalfSizeY = 0.;
		bool m_bRectangle = false;
	public:
		size_t GetCount() const noexcept { return m_ImageSequences.size(); }
		f2dSprite* GetSprite(fuInt index) noexcept
		{
			if (index >= GetCount())
				return nullptr;
			return m_ImageSequences[index];
		}
		fuInt GetSpriteIndexByTimer(int ani_timer)
		{
			return ((fuInt)ani_timer / m_Interval) % (fuInt)m_ImageSequences.size();
		}
		f2dSprite* GetSpriteByTimer(int ani_timer)
		{
			return m_ImageSequences[((fuInt)ani_timer / m_Interval) % (fuInt)m_ImageSequences.size()];
		}
		fuInt GetInterval() const noexcept { return m_Interval; }
		BlendMode GetBlendMode() const noexcept { return m_BlendMode; }
		void SetBlendMode(BlendMode m) noexcept { m_BlendMode = m; }
		double GetHalfSizeX() const noexcept { return m_HalfSizeX; }
		double GetHalfSizeY() const noexcept { return m_HalfSizeY; }
		bool IsRectangle() const noexcept { return m_bRectangle; }
	public:
		ResAnimation(const char* name, fcyRefPointer<ResTexture> tex,
			float x, float y, float w, float h,
			int n, int m, int intv,
			double a, double b, bool rect = false);
	};
}
