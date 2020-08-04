#pragma once
#include "Global.h"

namespace LuaSTGPlus {
	// 资源类型
	enum class ResourceType
	{
		Texture = 1,
		Sprite,
		Animation,
		Music,
		SoundEffect,
		Particle,
		SpriteFont,
		TrueTypeFont,
		FX,
		Model,
	};

	// 混合模式
	enum class BlendMode
	{
		__RESERVE__ = 0,

		MulAlpha = 1,		//顶点色和纹理色相乘 混合模式：正常（透明度混合）
		MulAdd = 2,			//顶点色和纹理色相乘 混合模式：线性减淡（加法）
		MulRev = 3,			//顶点色和纹理色相乘 混合模式：减去（减法）
		MulSub = 4,			//顶点色和纹理色相乘 混合模式：（图片被底图减）
		AddAlpha = 5,		//顶点色和纹理色相加 混合模式：正常（透明度混合）
		AddAdd = 6,			//顶点色和纹理色相加 混合模式：线性减淡（加法）
		AddRev = 7,			//顶点色和纹理色相加 混合模式：减去（减法）
		AddSub = 8,			//顶点色和纹理色相加 混合模式：（图片被底图减）

		AlphaBal = 9,		//顶点色和纹理色相乘 混合模式：（反色）

		MulMin = 10,		//顶点色和纹理色相乘 混合模式：变暗（取小）
		MulMax = 11,		//顶点色和纹理色相乘 混合模式：变亮（取大）
		MulMutiply = 12,	//顶点色和纹理色相乘 混合模式：正片叠底（相乘）
		MulScreen = 13,		//顶点色和纹理色相乘 混合模式：滤色（相加减去相乘）
		AddMin = 14,		//顶点色和纹理色相加 混合模式：变暗（取小）
		AddMax = 15,		//顶点色和纹理色相加 混合模式：变亮（取大）
		AddMutiply = 16,	//顶点色和纹理色相加 混合模式：正片叠底（相乘）
		AddScreen = 17,		//顶点色和纹理色相加 混合模式：滤色（相加减去相乘）

		One = 18,           //无混合，直接覆盖

		_KEY_NOT_FOUND = -1,
	};

	// 资源接口
	class Resource : public fcyRefObjImpl<fcyRefObj>
	{
	private:
		ResourceType m_Type;
		std::string m_ResName;
	public:
		ResourceType GetType()const noexcept { return m_Type; }
		const std::string& GetResName()const noexcept { return m_ResName; }
	private:
		Resource& operator=(const Resource&);
		Resource(const Resource&);
	public:
		Resource(ResourceType t, const char* name)
			: m_Type(t), m_ResName(name) {}
	};
}
