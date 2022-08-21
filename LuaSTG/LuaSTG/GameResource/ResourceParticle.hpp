#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Graphics/Sprite.hpp"
#include "Utility/xorshift.hpp"
#include "fcyOS/fcyMemPool.h"

#define LPARTICLE_MAXCNT 500  // 单个粒子池最多有500个粒子，这是HGE粒子特效的实现，不应该修改

namespace LuaSTGPlus
{
	// 粒子系统
	// HGE粒子系统的f2d实现
	class ResParticle :
		public Resource
	{
	public:
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
		// HGE 粒子实例
		struct hgeParticle
		{
			Core::Vector2F vecLocation; // 位置
			Core::Vector2F vecVelocity; // 速度

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
			Core::ScopeObject<Core::Graphics::ISprite> pSprite;
			BlendMode eBlendMode = BlendMode::MulAlpha;
			float colVertexColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

			bool LoadFromMemory(void const* data, size_t size);
			bool LoadFromInfo();
		};
		// 粒子效果实例
		class ParticlePool
		{
			friend class ResParticle;
		public:
			enum class Status
			{
				Alive,
				Sleep
			};
		private:
			fcyRefPointer<ResParticle> m_Res;
			ParticleSystemResourceInfo m_Info;
			std::array<hgeParticle, LPARTICLE_MAXCNT> m_ParticlePool;
			random::xoshiro128p m_Random;
			uint32_t m_RandomSeed = 0;
			Status m_iStatus = Status::Alive;  // 状态
			Core::Vector2F m_vCenter;  // 中心
			Core::Vector2F m_vPrevCenter;  // 上一个中心
			size_t m_iAlive = 0;  // 存活数
			float m_fDirection = 0.0f; // 相对发射角度
			float m_fAge = 0.f;  // 已存活时间
			float m_fEmissionResidue = 0.f;  // 不足的粒子数
			bool m_bOldBehavior = true; // 使用旧行为
		private:
			float RandomFloat(float a, float b);
		public:
			hgeParticleSystemInfo& GetParticleSystemInfo() { return m_Info.tParticleSystemInfo; };
			size_t GetAliveCount() const noexcept;
			BlendMode GetBlendMode() const noexcept;
			void SetBlendMode(BlendMode m) noexcept;
			Core::Color4B GetVertexColor() const noexcept;
			void SetVertexColor(Core::Color4B c) noexcept;
			int GetEmission() const noexcept;
			void SetEmission(int e) noexcept;
			uint32_t GetSeed() const noexcept;
			void SetSeed(uint32_t seed) noexcept;
			bool IsActived() const noexcept;
			void SetActive(bool v) noexcept;
			void SetCenter(Core::Vector2F pos) noexcept;
			Core::Vector2F GetCenter() const noexcept;
			float GetRotation() const noexcept;
			void SetRotation(float r) noexcept;
			void Update(float delta);
			void Render(float scaleX, float scaleY);
			void SetOldBehavior(bool b) noexcept { m_bOldBehavior = b; }
		public:
			ParticlePool(fcyRefPointer<ResParticle> ref);
		};
	private:
		static fcyMemPool<sizeof(ParticlePool)> s_MemoryPool;
		ParticleSystemResourceInfo m_Info;
		double m_HalfSizeX = 0.;
		double m_HalfSizeY = 0.;
		bool m_bRectangle = false;
	public:
		ParticlePool* AllocInstance() noexcept;
		void FreeInstance(ParticlePool* p) noexcept;
	public:
		hgeParticleSystemInfo const& GetParticleInfo() const noexcept { return m_Info.tParticleSystemInfo; }
		double GetHalfSizeX() const noexcept { return m_HalfSizeX; }
		double GetHalfSizeY() const noexcept { return m_HalfSizeY; }
		bool IsRectangle() const noexcept { return m_bRectangle; }
	public:
		ResParticle(const char* name, const hgeParticleSystemInfo& pinfo, Core::Graphics::ISprite* sprite, double a, double b, bool rect = false);
	};
};
