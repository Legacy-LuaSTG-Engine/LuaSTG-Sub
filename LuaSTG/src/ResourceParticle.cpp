#include "AppFrame.h"
#include "Utility.h"
#include "ResourceParticle.hpp"

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

using namespace std;

namespace LuaSTGPlus {
	fcyMemPool<sizeof(LuaSTGPlus::ResParticle::ParticlePool)> LuaSTGPlus::ResParticle::s_MemoryPool(512);  // 预分配512个对象

	ResParticle::ResParticle(const char* name, const ParticleInfo& pinfo, fcyRefPointer<f2dSprite> sprite, BlendMode bld, double a, double b, bool rect)
		: Resource(ResourceType::Particle, name), m_ParticleInfo(pinfo), m_BindedSprite(sprite), m_BlendMode(bld), m_HalfSizeX(a), m_HalfSizeY(b), m_bRectangle(rect)
	{
	}

	ResParticle::ParticlePool* ResParticle::AllocInstance()LNOEXCEPT
	{
		// ！ 警告：潜在bad_alloc导致错误，暂时不予处理
		ParticlePool* pRet = new(s_MemoryPool.Alloc()) ParticlePool(this);
		pRet->SetBlendMode(m_BlendMode);
		return pRet;
	}

	void ResParticle::FreeInstance(ResParticle::ParticlePool* p)LNOEXCEPT
	{
		p->~ParticlePool();
		s_MemoryPool.Free(p);
	}

	ResParticle::ParticlePool::ParticlePool(fcyRefPointer<ResParticle> ref) {
		m_pInstance = ref;
		m_fEmission = (float)ref->GetParticleInfo().nEmission;
		m_MixColor.Set(255, 255, 255, 255);
	}

	void ResParticle::ParticlePool::Update(float delta)
	{
		static fcyRandomWELL512 s_ParticleRandomizer;

		const ParticleInfo& pInfo = m_pInstance->GetParticleInfo();

		if (m_iStatus == Status::Alive)
		{
			m_fAge += delta;
			if (m_fAge >= pInfo.fLifetime && pInfo.fLifetime >= 0.f)
				m_iStatus = Status::Sleep;
		}

		// 更新所有粒子
		size_t i = 0;
		while (i < m_iAlive)
		{
			ParticleInstance& tInst = m_ParticlePool[i];
			tInst.fAge += delta;
			if (tInst.fAge >= tInst.fTerminalAge)
			{
				--m_iAlive;
				if (m_iAlive > i + 1)
					memcpy(&tInst, &m_ParticlePool[m_iAlive], sizeof(ParticleInstance));
				continue;
			}

			// 计算线加速度和切向加速度
			fcyVec2 vecAccel = (tInst.vecLocation - m_vCenter).GetNormalize();
			fcyVec2 vecAccel2 = vecAccel;
			vecAccel *= tInst.fRadialAccel;
			// vecAccel2.Rotate(M_PI_2);
			std::swap(vecAccel2.x, vecAccel2.y);
			vecAccel2.x = -vecAccel2.x;
			vecAccel2 *= tInst.fTangentialAccel;

			// 计算速度
			tInst.vecVelocity += (vecAccel + vecAccel2) * delta;
			tInst.vecVelocity.y += tInst.fGravity * delta;

			// 计算位置
			tInst.vecLocation += tInst.vecVelocity * delta;

			// 计算自旋和大小
			tInst.fSpin += tInst.fSpinDelta * delta;
			tInst.fSize += tInst.fSizeDelta * delta;
			tInst.colColor[0] += tInst.colColorDelta[0] * delta;
			tInst.colColor[1] += tInst.colColorDelta[1] * delta;
			tInst.colColor[2] += tInst.colColorDelta[2] * delta;
			tInst.colColor[3] += tInst.colColorDelta[3] * delta;

			++i;
		}

		// 产生新的粒子
		if (m_iStatus == Status::Alive)
		{
			float fParticlesNeeded = m_fEmission * delta + m_fEmissionResidue;
			fuInt nParticlesCreated = (fuInt)fParticlesNeeded;
			m_fEmissionResidue = fParticlesNeeded - (float)nParticlesCreated;

			for (fuInt i = 0; i < nParticlesCreated; ++i)
			{
				if (m_iAlive >= m_ParticlePool.size())
					break;

				ParticleInstance& tInst = m_ParticlePool[m_iAlive++];
				tInst.fAge = 0.0f;
				tInst.fTerminalAge = s_ParticleRandomizer.GetRandFloat(pInfo.fParticleLifeMin, pInfo.fParticleLifeMax);

				tInst.vecLocation = m_vPrevCenter + (m_vCenter - m_vPrevCenter) * s_ParticleRandomizer.GetRandFloat(0.0f, 1.0f);
				tInst.vecLocation.x += s_ParticleRandomizer.GetRandFloat(-2.0f, 2.0f);
				tInst.vecLocation.y += s_ParticleRandomizer.GetRandFloat(-2.0f, 2.0f);

				float ang = /* pInfo.fDirection */ (m_fRotation - (float)LPI_HALF) - (float)LPI_HALF + s_ParticleRandomizer.GetRandFloat(0, pInfo.fSpread) - pInfo.fSpread / 2.0f;
				tInst.vecVelocity.x = cos(ang);
				tInst.vecVelocity.y = sin(ang);
				tInst.vecVelocity *= s_ParticleRandomizer.GetRandFloat(pInfo.fSpeedMin, pInfo.fSpeedMax);

				tInst.fGravity = s_ParticleRandomizer.GetRandFloat(pInfo.fGravityMin, pInfo.fGravityMax);
				tInst.fRadialAccel = s_ParticleRandomizer.GetRandFloat(pInfo.fRadialAccelMin, pInfo.fRadialAccelMax);
				tInst.fTangentialAccel = s_ParticleRandomizer.GetRandFloat(pInfo.fTangentialAccelMin, pInfo.fTangentialAccelMax);

				tInst.fSize = s_ParticleRandomizer.GetRandFloat(pInfo.fSizeStart, pInfo.fSizeStart + (pInfo.fSizeEnd - pInfo.fSizeStart) * pInfo.fSizeVar);
				tInst.fSizeDelta = (pInfo.fSizeEnd - tInst.fSize) / tInst.fTerminalAge;

				//tInst.fSpin = /* pInfo.fSpinStart */ m_fRotation + s_ParticleRandomizer.GetRandFloat(0, pInfo.fSpinEnd) - pInfo.fSpinEnd / 2.0f;
				//tInst.fSpinDelta = pInfo.fSpinVar;

				tInst.fSpin = s_ParticleRandomizer.GetRandFloat(pInfo.fSpinStart, pInfo.fSpinStart + (pInfo.fSpinEnd - pInfo.fSpinStart) * pInfo.fSpinVar);
				tInst.fSpinDelta = (pInfo.fSpinEnd - tInst.fSpin) / tInst.fTerminalAge;

				tInst.colColor[0] = s_ParticleRandomizer.GetRandFloat(pInfo.colColorStart[0], pInfo.colColorStart[0] + (pInfo.colColorEnd[0] - pInfo.colColorStart[0]) * pInfo.fColorVar);
				tInst.colColor[1] = s_ParticleRandomizer.GetRandFloat(pInfo.colColorStart[1], pInfo.colColorStart[1] + (pInfo.colColorEnd[1] - pInfo.colColorStart[1]) * pInfo.fColorVar);
				tInst.colColor[2] = s_ParticleRandomizer.GetRandFloat(pInfo.colColorStart[2], pInfo.colColorStart[2] + (pInfo.colColorEnd[2] - pInfo.colColorStart[2]) * pInfo.fColorVar);
				tInst.colColor[3] = s_ParticleRandomizer.GetRandFloat(pInfo.colColorStart[3], pInfo.colColorStart[3] + (pInfo.colColorEnd[3] - pInfo.colColorStart[3]) * pInfo.fAlphaVar);

				tInst.colColorDelta[0] = (pInfo.colColorEnd[0] - tInst.colColor[0]) / tInst.fTerminalAge;
				tInst.colColorDelta[1] = (pInfo.colColorEnd[1] - tInst.colColor[1]) / tInst.fTerminalAge;
				tInst.colColorDelta[2] = (pInfo.colColorEnd[2] - tInst.colColor[2]) / tInst.fTerminalAge;
				tInst.colColorDelta[3] = (pInfo.colColorEnd[3] - tInst.colColor[3]) / tInst.fTerminalAge;
			}
		}

		m_vPrevCenter = m_vCenter;
	}

	void ResParticle::ParticlePool::Render(f2dGraphics2D* graph, float scaleX, float scaleY)
	{
		f2dSprite* p = m_pInstance->GetBindedSprite();
		const ParticleInfo& pInfo = m_pInstance->GetParticleInfo();
		fcyColor tOrgColor = p->GetColor(0U);//获取精灵第一个顶点的颜色（总共4个顶点）

		for (size_t i = 0; i < m_iAlive; ++i)
		{
			ParticleInstance& pInst = m_ParticlePool[i];//实例化的粒子

			if (pInfo.colColorStart[0] < 0)  // r < 0
			{
				/*
				p->SetColor(fcyColor(
					(int)(pInst.colColor[3] * 255),
					tOrgColor.r,
					tOrgColor.g,
					tOrgColor.b
				));
				*/
				p->SetColor(fcyColor(
					(fInt)(pInst.colColor[3] * m_MixColor.a),
					(fInt)(tOrgColor.r * m_MixColor.r / 255),
					(fInt)(tOrgColor.g * m_MixColor.g / 255),
					(fInt)(tOrgColor.b * m_MixColor.b / 255)
				));
			}
			else
			{
				//p->SetColor(fcyColor(pInst.colColor[3], pInst.colColor[0], pInst.colColor[1], pInst.colColor[2]));
				p->SetColor(fcyColor(
					(fInt)(pInst.colColor[3] * m_MixColor.a),
					(fInt)(pInst.colColor[0] * m_MixColor.r),
					(fInt)(pInst.colColor[1] * m_MixColor.g),
					(fInt)(pInst.colColor[2] * m_MixColor.b)
				));
			}
			p->Draw2(graph, fcyVec2(pInst.vecLocation.x, pInst.vecLocation.y), fcyVec2(scaleX * pInst.fSize, scaleY * pInst.fSize), pInst.fSpin, false);
		}

		p->SetColor(tOrgColor);//恢复精灵顶点颜色
	}
}
