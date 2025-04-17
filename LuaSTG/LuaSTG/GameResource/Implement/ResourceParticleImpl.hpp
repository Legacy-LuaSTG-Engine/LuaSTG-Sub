#pragma once
#include "GameResource/ResourceParticle.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"
#include "Core/Graphics/Sprite.hpp"
#include "Utility/xorshift.hpp"

#define LPARTICLE_MAXCNT 500  // 单个粒子池最多有500个粒子，这是HGE粒子特效的实现，不应该修改

namespace luastg
{
	// https://github.com/kvakvs/hge/blob/hge1.9/include/hgeparticle.h
	// HGE 粒子实例
	struct hgeParticle
	{
		core::Vector2F vecLocation; // 位置
		core::Vector2F vecVelocity; // 速度

		float fGravity;         // 重力
		float fRadialAccel;     // 径向加速度
		float fTangentialAccel; // 切向加速度

		float fSpin;      // 自旋
		float fSpinDelta; // 自旋增量

		float fSize;      // 大小
		float fSizeDelta; // 大小增量

		float colColor[4];      // 颜色
		float colColorDelta[4]; // 颜色增量

		float fAge;         // 当前存活时间
		float fTerminalAge; // 终止时间
	};

	// 粒子效果资源定义
	struct ParticleSystemResourceInfo
	{
		hgeParticleSystemInfo tParticleSystemInfo = {};
		core::SmartReference<core::Graphics::ISprite> pSprite;
		BlendMode eBlendMode = BlendMode::MulAlpha;
		float colVertexColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		bool LoadFromMemory(void const* data, size_t size);
		bool LoadFromInfo();
	};

	class ParticlePoolImpl : public IParticlePool
	{
		//friend class ResParticle;
	private:
		enum class Status
		{
			Alive,
			Sleep
		};
	private:
		core::SmartReference<IResourceParticle> m_Res;
		ParticleSystemResourceInfo m_Info;
		std::array<hgeParticle, LPARTICLE_MAXCNT> m_ParticlePool;
		random::xoshiro128p m_Random;
		uint32_t m_RandomSeed = 0;
		Status m_iStatus = Status::Alive;  // 状态
		core::Vector2F m_vCenter;  // 中心
		core::Vector2F m_vPrevCenter;  // 上一个中心
		size_t m_iAlive = 0;  // 存活数
		float m_fDirection = 0.0f; // 相对发射角度
		float m_fAge = 0.f;  // 已存活时间
		float m_fEmissionResidue = 0.f;  // 不足的粒子数
		bool m_bOldBehavior = true; // 使用旧行为
	private:
		float RandomFloat(float a, float b);
	public:
		hgeParticleSystemInfo& GetParticleSystemInfo() { return m_Info.tParticleSystemInfo; };
		size_t GetAliveCount();
		BlendMode GetBlendMode();
		void SetBlendMode(BlendMode m);
		core::Color4B GetVertexColor();
		void SetVertexColor(core::Color4B c);
		int GetEmission();
		void SetEmission(int e);
		uint32_t GetSeed();
		void SetSeed(uint32_t seed);
		bool IsActived();
		void SetActive(bool v);
		void SetCenter(core::Vector2F pos);
		core::Vector2F GetCenter();
		float GetRotation();
		void SetRotation(float r);
		void Update(float delta);
		void Render(float scaleX, float scaleY);
		void SetOldBehavior(bool b) { m_bOldBehavior = b; }
	public:
		ParticlePoolImpl(core::SmartReference<IResourceParticle> ps_ref);
	};

	class ResourceParticleImpl : public ResourceBaseImpl<IResourceParticle>
	{
	private:
		ParticleSystemResourceInfo m_Info;
		double m_HalfSizeX = 0.;
		double m_HalfSizeY = 0.;
		bool m_bRectangle = false;
	public:
		ParticleSystemResourceInfo& GetResourceInfo() { return m_Info; }
	public:
		hgeParticleSystemInfo const& GetParticleInfo() { return m_Info.tParticleSystemInfo; }
		double GetHalfSizeX() { return m_HalfSizeX; }
		double GetHalfSizeY() { return m_HalfSizeY; }
		bool IsRectangle() { return m_bRectangle; }
	public:
		bool CreateInstance(IParticlePool** pp_pool);
		void DestroyInstance(IParticlePool* p_pool);
	public:
		ResourceParticleImpl(const char* name, const hgeParticleSystemInfo& pinfo, core::Graphics::ISprite* sprite, double a, double b, bool rect = false);
	};
}
