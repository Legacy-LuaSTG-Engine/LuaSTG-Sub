#pragma once
#include "ResourceBase.hpp"

namespace LuaSTGPlus {
	// 粒子系统
	// HGE粒子系统的f2d实现
	class ResParticle :
		public Resource
	{
	public:
		/// @brief 粒子信息
		struct ParticleInfo
		{
			fuInt iBlendInfo;

			int nEmission;  // 每秒发射个数
			float fLifetime;  // 生命期
			float fParticleLifeMin;  // 粒子最小生命期
			float fParticleLifeMax;  // 粒子最大生命期
			float fDirection;  // 发射方向
			float fSpread;  // 偏移角度
			bool bRelative;  // 使用相对值还是绝对值

			float fSpeedMin;  // 速度最小值
			float fSpeedMax;  // 速度最大值

			float fGravityMin;  // 重力最小值
			float fGravityMax;  // 重力最大值

			float fRadialAccelMin;  // 最低线加速度
			float fRadialAccelMax;  // 最高线加速度

			float fTangentialAccelMin;  // 最低角加速度
			float fTangentialAccelMax;  // 最高角加速度

			float fSizeStart;  // 起始大小
			float fSizeEnd;  // 最终大小
			float fSizeVar;  // 大小抖动值

			float fSpinStart;  // 起始自旋
			float fSpinEnd;  // 最终自旋
			float fSpinVar;  // 自旋抖动值

			float colColorStart[4];  // 起始颜色(rgba)
			float colColorEnd[4];  // 最终颜色
			float fColorVar;  // 颜色抖动值
			float fAlphaVar;  // alpha抖动值
		};
		/// @brief 粒子实例
		struct ParticleInstance
		{
			fcyVec2 vecLocation;  // 位置
			fcyVec2 vecVelocity;  // 速度

			float fGravity;  // 重力
			float fRadialAccel;  // 线加速度
			float fTangentialAccel;  // 角加速度

			float fSpin;  // 自旋
			float fSpinDelta;  // 自旋增量

			float fSize;  // 大小
			float fSizeDelta;  // 大小增量

			float colColor[4];  // 颜色
			float colColorDelta[4];  // 颜色增量

			float fAge;  // 当前存活时间
			float fTerminalAge;  // 终止时间
		};
		/// @brief 粒子池
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
			fcyRefPointer<ResParticle> m_pInstance;  // 信息

			BlendMode m_BlendMode = BlendMode::MulAlpha;
			fcyColor m_MixColor;//渲染时整体颜色
			Status m_iStatus = Status::Alive;  // 状态
			fcyVec2 m_vCenter;  // 中心
			fcyVec2 m_vPrevCenter;  // 上一个中心
			float m_fRotation = 0.f;  // 方向
			size_t m_iAlive = 0;  // 存活数
			float m_fAge = 0.f;  // 已存活时间
			float m_fEmission = 0.f;  // 每秒发射数
			float m_fEmissionResidue = 0.f;  // 不足的粒子数
			std::array<ParticleInstance, LPARTICLE_MAXCNT> m_ParticlePool;
		public:
			size_t GetAliveCount()const LNOEXCEPT { return m_iAlive; }
			BlendMode GetBlendMode()const LNOEXCEPT { return m_BlendMode; }
			void SetBlendMode(BlendMode m)LNOEXCEPT { m_BlendMode = m; }
			fcyColor GetMixColor()LNOEXCEPT { return m_MixColor; }
			void SetMixColor(fcyColor c)LNOEXCEPT { m_MixColor = c; }
			float GetEmission()const LNOEXCEPT { return m_fEmission; }
			void SetEmission(float e)LNOEXCEPT { m_fEmission = e; }
			bool IsActived()const LNOEXCEPT { return m_iStatus == Status::Alive; }
			void SetActive()LNOEXCEPT
			{
				m_iStatus = Status::Alive;
				m_fAge = 0.f;
			}
			void SetInactive()LNOEXCEPT
			{
				m_iStatus = Status::Sleep;
			}
			void SetCenter(fcyVec2 pos)LNOEXCEPT
			{
				if (m_iStatus == Status::Alive)
					m_vPrevCenter = m_vCenter;
				else
					m_vPrevCenter = pos;
				m_vCenter = pos;
			}
			void SetRotation(float r)LNOEXCEPT { m_fRotation = r; }
			void Update(float delta);
			void Render(f2dGraphics2D* graph, float scaleX, float scaleY);
		public:
			ParticlePool(fcyRefPointer<ResParticle> ref);
		};
	private:
		static fcyMemPool<sizeof(ParticlePool)> s_MemoryPool;

		fcyRefPointer<f2dSprite> m_BindedSprite;
		BlendMode m_BlendMode = BlendMode::MulAlpha;
		ParticleInfo m_ParticleInfo;
		double m_HalfSizeX = 0.;
		double m_HalfSizeY = 0.;
		bool m_bRectangle = false;
	public:
		ParticlePool* AllocInstance()LNOEXCEPT;
		void FreeInstance(ParticlePool* p)LNOEXCEPT;

		f2dSprite* GetBindedSprite()LNOEXCEPT { return m_BindedSprite; }
		const ParticleInfo& GetParticleInfo()const LNOEXCEPT { return m_ParticleInfo; }
		double GetHalfSizeX()const LNOEXCEPT { return m_HalfSizeX; }
		double GetHalfSizeY()const LNOEXCEPT { return m_HalfSizeY; }
		bool IsRectangle()const LNOEXCEPT { return m_bRectangle; }
	public:
		ResParticle(const char* name, const ParticleInfo& pinfo, fcyRefPointer<f2dSprite> sprite, BlendMode bld, double a, double b, bool rect = false);
	};
}
