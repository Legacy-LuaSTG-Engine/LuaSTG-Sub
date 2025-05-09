#pragma once
#include "GameResource/ResourceBase.hpp"

namespace luastg
{
	// https://github.com/kvakvs/hge/blob/hge1.9/include/hgeparticle.h
	// HGE 粒子效果定义
	struct hgeParticleSystemInfo
	{
		uint32_t iBlendInfo;

		int nEmission;   // 每秒发射个数
		float fLifetime; // 生命期

		float fParticleLifeMin; // 粒子最小生命期
		float fParticleLifeMax; // 粒子最大生命期

		float fDirection; // 发射方向
		float fSpread;    // 偏移角度
		bool bRelative;   // 使用相对值还是绝对值

		float fSpeedMin; // 速度最小值
		float fSpeedMax; // 速度最大值

		float fGravityMin; // 重力最小值
		float fGravityMax; // 重力最大值

		float fRadialAccelMin; // 最低径向加速度
		float fRadialAccelMax; // 最高径向加速度

		float fTangentialAccelMin;  // 最低切向加速度
		float fTangentialAccelMax;  // 最高切向加速度

		float fSizeStart; // 起始大小
		float fSizeEnd;   // 最终大小
		float fSizeVar;   // 大小抖动值

		float fSpinStart; // 起始自旋
		float fSpinEnd;   // 最终自旋
		float fSpinVar;   // 自旋抖动值

		float colColorStart[4]; // 起始颜色 (rgba)
		float colColorEnd[4];   // 最终颜色 (rgba)
		float fColorVar;        // 颜色抖动值
		float fAlphaVar;        // alpha抖动值
	};

	struct IParticlePool
	{
		virtual hgeParticleSystemInfo& GetParticleSystemInfo() = 0;
		virtual size_t GetAliveCount() = 0;
		virtual BlendMode GetBlendMode() = 0;
		virtual void SetBlendMode(BlendMode m) = 0;
		virtual core::Color4B GetVertexColor() = 0;
		virtual void SetVertexColor(core::Color4B c) = 0;
		virtual int GetEmission() = 0;
		virtual void SetEmission(int e) = 0;
		virtual uint32_t GetSeed() = 0;
		virtual void SetSeed(uint32_t seed) = 0;
		virtual bool IsActived() = 0;
		virtual void SetActive(bool v) = 0;
		virtual void SetCenter(core::Vector2F pos) = 0;
		virtual core::Vector2F GetCenter() = 0;
		virtual float GetRotation() = 0;
		virtual void SetRotation(float r) = 0;
		virtual void Update(float delta) = 0;
		virtual void Render(float scaleX, float scaleY) = 0;
		virtual void SetOldBehavior(bool b) = 0;
	};

	struct IResourceParticle : public IResourceBase
	{
		virtual hgeParticleSystemInfo const& GetParticleInfo() = 0;
		virtual double GetHalfSizeX() = 0;
		virtual double GetHalfSizeY() = 0;
		virtual bool IsRectangle() = 0;

		virtual bool CreateInstance(IParticlePool** pp_pool) = 0;
		virtual void DestroyInstance(IParticlePool* p_pool) = 0;
	};
};

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/luastg.IResourceParticle
	template<> constexpr InterfaceId getInterfaceId<luastg::IResourceParticle>() { return UUID::parse("c396ef15-221a-5df9-ad11-45f4421aa5ec"); }
}
