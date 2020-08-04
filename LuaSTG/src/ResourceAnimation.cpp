#include "ResourceAnimation.hpp"
#include "AppFrame.h" // 用于创建图片精灵

namespace LuaSTGPlus {
	ResAnimation::ResAnimation(const char* name, fcyRefPointer<ResTexture> tex,
		float x, float y, float w, float h,
		int n, int m, int intv,
		double a, double b, bool rect)
		: Resource(ResourceType::Animation, name), m_Interval(intv), m_HalfSizeX(a), m_HalfSizeY(b), m_bRectangle(rect)
	{
		LASSERT(LAPP.GetRenderer());

		// 分割纹理
		for (int j = 0; j < m; ++j)  // 行
		{
			for (int i = 0; i < n; ++i)  // 列
			{
				fcyRefPointer<f2dSprite> t;
				if (FCYFAILED(LAPP.GetRenderer()->CreateSprite2D(tex->GetTexture(), fcyRect(
					x + w * i, y + h * j, x + w * (i + 1), y + h * (j + 1)
				), &t)))
				{
					throw fcyException("ResAnimation::ResAnimation", "CreateSprite2D failed.");
				}
				t->SetZ(0.5f);
				t->SetColor(0xFFFFFFFF);
				m_ImageSequences.push_back(t);
			}
		}
	}
}
