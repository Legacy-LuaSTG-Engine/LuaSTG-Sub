////////////////////////////////////////////////////////////////////////////////
/// @file  f2dParticle.h
/// @brief fancy2D 2D粒子
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "f2dRenderer.h"

#include <vector>

#include "fcyMisc\fcyRandom.h"

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 2D粒子池实现
////////////////////////////////////////////////////////////////////////////////
class f2dParticlePoolImpl : 
	public fcyRefObjImpl<f2dParticlePool>
{
public:
	struct Particle
	{
		f2dSprite* pSprite;
		fBool bInUse;

		float CurTime;    // 当前生命值
		float Angle;      // 当前角度

		fcyVec2 Pos;
		fcyVec2 CreatePos;
		fcyVec2 V;        // 速度
		float RA;         // 法向加速度
		float TA;         // 切向加速度
		float Spin;       // 自旋速度
		float LifeTime;   // 生命值

		float RandomSeed; // 随机数

		fcyColor CurColor;   // 当前颜色
		fcyColor StartColor; // 开始颜色
		fcyColor EndColor;   // 终止颜色
		
		fcyVec2 CurScale;    // 当前缩放
		fcyVec2 StartScale;  // 开始缩放
		fcyVec2 EndScale;    // 终止缩放
	};
protected:
	fcyRandomWELL512 m_Randomizer;

	std::vector<Particle> m_ParticlePool;

	std::vector<f2dParticleForce*> m_ForcePool;

	fuInt m_ParticleCount;
public: // 接口实现
	fuInt GetRandomSeed()
	{
		return m_Randomizer.GetRandSeed();
	}
	void SetRandomSeed(fuInt Seed)
	{
		m_Randomizer.SetSeed(Seed);
	}
	fFloat RandFloat(fFloat Min, fFloat Max)
	{
		return m_Randomizer.GetRandFloat(Min, Max);
	}
	
	fuInt GetCount()
	{
		return m_ParticleCount;
	}

	fuInt GetCapacity()
	{
		return m_ParticlePool.size();
	}

	void Clear()
	{
		// 处理所有粒子
		std::vector<Particle>::iterator i = m_ParticlePool.begin();

		while(i != m_ParticlePool.end())
		{
			FCYSAFEKILL(i->pSprite);

			++i;
		}

		m_ParticlePool.clear();
	}

	fResult AppendForce(f2dParticleForce* pForce);
	bool RemoveForce(f2dParticleForce* pForce);
	void ClearForce();
	fResult Emitted(f2dSprite* pSprite, const fcyVec2& Center, const fcyVec2& EmittedCountRange, const f2dParticleCreationDesc& Desc);
	void Update(fFloat ElapsedTime);
	void Render(f2dGraphics2D* pGraph);
public:
	f2dParticlePoolImpl()
		: m_ParticleCount(0) {}
	~f2dParticlePoolImpl();
};
