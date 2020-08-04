#include "Renderer/f2dParticle.h"

#include "Renderer/f2dSpriteImpl.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

f2dParticlePoolImpl::~f2dParticlePoolImpl()
{
	Clear();
}

fResult f2dParticlePoolImpl::AppendForce(f2dParticleForce* pForce)
{
	if(!pForce)
		return FCYERR_INVAILDPARAM;

	m_ForcePool.push_back(pForce);

	return FCYERR_OK;
}

bool f2dParticlePoolImpl::RemoveForce(f2dParticleForce* pForce)
{
	for(vector<f2dParticleForce*>::iterator i = m_ForcePool.begin();
		i != m_ForcePool.end() ; ++i)
	{
		if(*i == pForce)
		{
			m_ForcePool.erase(i);
			return true;
		}
	}

	return false;
}

void f2dParticlePoolImpl::ClearForce()
{
	m_ForcePool.clear();
}

fResult f2dParticlePoolImpl::Emitted(f2dSprite* pSprite, const fcyVec2& Center, const fcyVec2& EmittedCountRange, const f2dParticleCreationDesc& Desc)
{
	if(!pSprite)
		return FCYERR_INVAILDPARAM;

	// 计算粒子创建数量
	fInt tCreateCount = (fInt)m_Randomizer.GetRandFloat(EmittedCountRange.x, EmittedCountRange.y);

	// 创建粒子
	fuInt tPos = 0;
	for(int i = 0; i<tCreateCount; ++i)
	{
		// 计算创建位置
		Particle tParticle;

		tParticle.CurTime = 0;
		tParticle.Angle = m_Randomizer.GetRandFloat(Desc.InitialAngle.x, Desc.InitialAngle.y);

		tParticle.Pos = Center;
		tParticle.Pos.x += m_Randomizer.GetRandFloat(Desc.PosRange.a.x, Desc.PosRange.b.x);
		tParticle.Pos.y += m_Randomizer.GetRandFloat(Desc.PosRange.a.y, Desc.PosRange.b.y);
		
		float tVSize = m_Randomizer.GetRandFloat(Desc.VRange.x, Desc.VRange.y);
		float tVAngle = m_Randomizer.GetRandFloat(Desc.VAngleRange.x, Desc.VAngleRange.y);
		tParticle.V = fcyVec2(cos(tVAngle), sin(tVAngle)) * tVSize;
		tParticle.RA = m_Randomizer.GetRandFloat(Desc.ARRange.x, Desc.ARRange.y);
		tParticle.TA = m_Randomizer.GetRandFloat(Desc.ATRange.x, Desc.ATRange.y);
		tParticle.Spin = m_Randomizer.GetRandFloat(Desc.SpinRange.x, Desc.SpinRange.y);
		tParticle.LifeTime = m_Randomizer.GetRandFloat(Desc.LifeTimeRange.x, Desc.LifeTimeRange.y);

		tParticle.RandomSeed = m_Randomizer.GetRandFloat();

		fInt tA = Desc.StartColor.a + (fInt)m_Randomizer.GetRandFloat(Desc.StartColorRange.x, Desc.StartColorRange.y);
		fInt tR = Desc.StartColor.r + (fInt)m_Randomizer.GetRandFloat(Desc.StartColorRange.x, Desc.StartColorRange.y);
		fInt tG = Desc.StartColor.g + (fInt)m_Randomizer.GetRandFloat(Desc.StartColorRange.x, Desc.StartColorRange.y);
		fInt tB = Desc.StartColor.b + (fInt)m_Randomizer.GetRandFloat(Desc.StartColorRange.x, Desc.StartColorRange.y);
		if(tA>255)tA = 255; if(tA<0) tA=0;
		if(tR>255)tR = 255; if(tR<0) tR=0;
		if(tG>255)tG = 255; if(tG<0) tG=0;
		if(tB>255)tB = 255; if(tB<0) tB=0;
		tParticle.StartColor = fcyColor(tA,tR,tG,tB);

		tParticle.CurColor = tParticle.StartColor;
		tParticle.EndColor = Desc.EndColor;
		
		float tV = m_Randomizer.GetRandFloat(Desc.StartScaleRange.x, Desc.StartScaleRange.y);
		tParticle.StartScale = Desc.StartScale + fcyVec2(tV,tV);
		if(tParticle.StartScale.x < 0) tParticle.StartScale.x = 0;
		if(tParticle.StartScale.y < 0) tParticle.StartScale.y = 0;
		tParticle.CurScale = tParticle.StartScale;
		tParticle.EndScale = Desc.EndScale;

		tParticle.pSprite = pSprite;
		tParticle.pSprite->AddRef();

		tParticle.bInUse = true;
		
		// 加入粒子
		while(tPos < m_ParticlePool.size())
		{
			if(m_ParticlePool[tPos].bInUse)
				tPos++;
			else
			{
				m_ParticlePool[tPos] = tParticle;
				break;
			}
		}
		
		if(tPos >= m_ParticlePool.size())
			m_ParticlePool.push_back(tParticle);
	}

	return FCYERR_OK;
}

void f2dParticlePoolImpl::Update(fFloat ElapsedTime)
{
	m_ParticleCount = 0;

	// 处理所有粒子
	vector<Particle>::iterator i = m_ParticlePool.begin();

	while(i != m_ParticlePool.end())
	{
		++m_ParticleCount;

		// 处理生存时间
		i->CurTime += ElapsedTime;
		if(i->LifeTime < i->CurTime)
		{
			// 移除粒子
			FCYSAFEKILL(i->pSprite);

			i->bInUse = false;
			++i;
		}
		else
		{
			float k = i->CurTime / i->LifeTime;

			// 处理位置
			fcyVec2 tRA, tTA;  // 法向加速度，切向加速度

			// 计算法线
			fcyVec2 tNormal = i->V;
			if(tNormal.Length2() != 0.f)
			{
				tNormal.Normalize();
				tRA = tNormal * i->RA;

				// 旋转90度
				float t = tNormal.x;
				tNormal.x = -tNormal.y;
				tNormal.y = t;

				// 切向加速度
				tTA = tNormal * i->TA;
			}

			// 速度增量
			i->V += (tRA + tTA) * ElapsedTime;

			// 自旋增量
			i->Angle += i->Spin * ElapsedTime;

			// 位置增量
			i->Pos += i->V * ElapsedTime;
			
			// 处理颜色
			i->CurColor = fcyColor
				(
					(fInt)((i->EndColor.a - i->StartColor.a) * k + i->StartColor.a),
					(fInt)((i->EndColor.r - i->StartColor.r) * k + i->StartColor.r),
					(fInt)((i->EndColor.g - i->StartColor.g) * k + i->StartColor.g),
					(fInt)((i->EndColor.b - i->StartColor.b) * k + i->StartColor.b)
				);
			// 处理缩放
			i->CurScale = fcyVec2
				(
					(i->EndScale.x - i->StartScale.x) * k + i->StartScale.x,
					(i->EndScale.y - i->StartScale.y) * k + i->StartScale.y
				);

			// 外部影响
			for(vector<f2dParticleForce*>::iterator j = m_ForcePool.begin();
				j != m_ForcePool.end() ; ++j)
			{
				(*j)->OnUpdate(ElapsedTime, i->Pos, i->V, i->Angle);
			}

			++i;
		}
	}
}
	
void f2dParticlePoolImpl::Render(f2dGraphics2D* pGraph)
{
	// 渲染所有粒子
	vector<Particle>::iterator i = m_ParticlePool.begin();

	while(i != m_ParticlePool.end())
	{
		if(i->bInUse && i->pSprite)
		{
			fcyColor t[4];
			i->pSprite->GetColor(t);

			i->pSprite->SetColor(i->CurColor);
			
			i->pSprite->Draw(pGraph, i->Pos, i->CurScale, i->Angle + i->V.CalcuAngle() - 3.1415926f/2.f);
			
			i->pSprite->SetColor(t);
		}

		++i;
	}
}
