#include "GameResource/Implement/ResourceParticleImpl.hpp"

namespace luastg
{
	static std::pmr::unsynchronized_pool_resource s_particle_pool_res;
	
	bool ParticleSystemResourceInfo::LoadFromMemory(void const* data, size_t size)
	{
		if (size != sizeof(hgeParticleSystemInfo))
		{
			return false;
		}
		std::memcpy(this, data, sizeof(hgeParticleSystemInfo));
		return LoadFromInfo();
	}
	bool ParticleSystemResourceInfo::LoadFromInfo()
	{
		tParticleSystemInfo.iBlendInfo = (tParticleSystemInfo.iBlendInfo >> 16) & 0x00000003u;
		eBlendMode = BlendMode::MulAlpha;
		if (tParticleSystemInfo.iBlendInfo & 0x1)  // ADD
		{
			if (tParticleSystemInfo.iBlendInfo & 0x2)  // ALPHA
				eBlendMode = BlendMode::AddAlpha;
			else
				eBlendMode = BlendMode::AddAdd;
		}
		else  // MUL
		{
			if (tParticleSystemInfo.iBlendInfo & 0x2)  // ALPHA
				eBlendMode = BlendMode::MulAlpha;
			else
				eBlendMode = BlendMode::MulAdd;
		}
		return true;
	}

	bool ResourceParticleImpl::CreateInstance(IParticlePool** pp_pool)
	{
		ParticlePoolImpl* p_pool = static_cast<ParticlePoolImpl*>(s_particle_pool_res.allocate(sizeof(ParticlePoolImpl), alignof(ParticlePoolImpl)));
		new(p_pool) ParticlePoolImpl(core::SmartReference<IResourceParticle>(this));
		*pp_pool = p_pool;
		return true;
	}
	void ResourceParticleImpl::DestroyInstance(IParticlePool* p_pool)
	{
		assert(p_pool);
		auto* p_class = static_cast<ParticlePoolImpl*>(p_pool);
		p_class->~ParticlePoolImpl();
		s_particle_pool_res.deallocate(p_class, sizeof(ParticlePoolImpl), alignof(ParticlePoolImpl));
	}

	ResourceParticleImpl::ResourceParticleImpl(const char* name, const hgeParticleSystemInfo& pinfo, core::Graphics::ISprite* sprite, double a, double b, bool rect)
		: ResourceBaseImpl(ResourceType::Particle, name)
		, m_HalfSizeX(a)
		, m_HalfSizeY(b)
		, m_bRectangle(rect)
	{
		m_Info.tParticleSystemInfo = pinfo;
		m_Info.pSprite = sprite;
		m_Info.LoadFromInfo(); // 加载混合模式
	}

	float ParticlePoolImpl::RandomFloat(float a, float b)
	{
		if (b > a) std::swap(a, b);
		float const c = std::nextafterf(b, std::numeric_limits<float>::max()) - a;
		return a + c * random::to_float(m_Random());
	}

	ParticlePoolImpl::ParticlePoolImpl(core::SmartReference<IResourceParticle> ref)
	{
		m_Res = ref;
		m_Info = static_cast<ResourceParticleImpl*>(ref.get())->GetResourceInfo();
		SetSeed(uint32_t(std::rand()));
	}
	size_t ParticlePoolImpl::GetAliveCount() { return m_iAlive; }
	BlendMode ParticlePoolImpl::GetBlendMode() { return m_Info.eBlendMode; }
	void ParticlePoolImpl::SetBlendMode(BlendMode m) { m_Info.eBlendMode = m; }
	core::Color4B ParticlePoolImpl::GetVertexColor()
	{
		return core::Color4B(
			(uint8_t)std::clamp(m_Info.colVertexColor[0] * 255.0f, 0.0f, 255.0f),
			(uint8_t)std::clamp(m_Info.colVertexColor[1] * 255.0f, 0.0f, 255.0f),
			(uint8_t)std::clamp(m_Info.colVertexColor[2] * 255.0f, 0.0f, 255.0f),
			(uint8_t)std::clamp(m_Info.colVertexColor[3] * 255.0f, 0.0f, 255.0f)
		);
	}
	void ParticlePoolImpl::SetVertexColor(core::Color4B c)
	{
		m_Info.colVertexColor[0] = (float)c.r / 255.0f;
		m_Info.colVertexColor[1] = (float)c.g / 255.0f;
		m_Info.colVertexColor[2] = (float)c.b / 255.0f;
		m_Info.colVertexColor[3] = (float)c.a / 255.0f;
	}
	int ParticlePoolImpl::GetEmission() { return m_Info.tParticleSystemInfo.nEmission; }
	void ParticlePoolImpl::SetEmission(int e) { m_Info.tParticleSystemInfo.nEmission = e; }
	uint32_t ParticlePoolImpl::GetSeed() { return m_RandomSeed; }
	void ParticlePoolImpl::SetSeed(uint32_t seed)
	{
		m_RandomSeed = seed;
		m_Random.seed(seed);
	}
	bool ParticlePoolImpl::IsActived() { return m_iStatus == Status::Alive; }
	void ParticlePoolImpl::SetActive(bool v)
	{
		if (v)
		{
			m_iStatus = Status::Alive;
			m_fAge = 0.f;
		}
		else
		{
			m_iStatus = Status::Sleep;
		}
	}
	void ParticlePoolImpl::SetCenter(core::Vector2F pos)
	{
		if (m_iStatus == Status::Alive)
			m_vPrevCenter = m_vCenter;
		else
			m_vPrevCenter = pos;
		m_vCenter = pos;
	}
	core::Vector2F ParticlePoolImpl::GetCenter() { return m_vCenter; }
	void ParticlePoolImpl::SetRotation(float r) { m_fDirection = r; }
	float ParticlePoolImpl::GetRotation() { return m_fDirection; }
	void ParticlePoolImpl::Update(float delta)
	{
		hgeParticleSystemInfo const& pInfo = m_Info.tParticleSystemInfo;

		if (m_iStatus == Status::Alive)
		{
			m_fAge += delta;
			if (m_fAge >= pInfo.fLifetime && pInfo.fLifetime >= 0.0f)
			{
				m_iStatus = Status::Sleep;
			}
		}

		// 更新所有粒子
		for (size_t i = 0; i < m_iAlive; i += 1)
		{
			hgeParticle& tInst = m_ParticlePool[i];
			tInst.fAge += delta;
			if (tInst.fAge >= tInst.fTerminalAge)
			{
				m_iAlive -= 1;
				if (i < m_iAlive)
				{
					// 需要拷贝最后一个粒子到当前位置
					std::memcpy(&tInst, &m_ParticlePool[m_iAlive], sizeof(hgeParticle));
				}
				// 回溯索引
				i -= 1;
				continue;
			}

			// 计算线加速度和切向加速度
			core::Vector2F vecAccel = (tInst.vecLocation - m_vCenter).normalized();
			core::Vector2F vecAccel2 = vecAccel;
			vecAccel *= tInst.fRadialAccel;
			// 相当于旋转向量 vecAccel2.Rotate(M_PI_2);
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
		}

		// 产生新的粒子
		if (m_iStatus == Status::Alive)
		{
			float const fParticlesNeeded = (float)pInfo.nEmission * delta + m_fEmissionResidue;
			uint32_t const nParticlesCreated = (uint32_t)fParticlesNeeded;
			m_fEmissionResidue = fParticlesNeeded - (float)nParticlesCreated;

			for (uint32_t i = 0; i < nParticlesCreated; ++i)
			{
				if (m_iAlive >= m_ParticlePool.size())
					break;

				hgeParticle& tInst = m_ParticlePool[m_iAlive];
				m_iAlive += 1;

				tInst.fAge = 0.0f;
				tInst.fTerminalAge = RandomFloat(pInfo.fParticleLifeMin, pInfo.fParticleLifeMax);

				tInst.vecLocation = m_vPrevCenter + (m_vCenter - m_vPrevCenter) * RandomFloat(0.0f, 1.0f);
				tInst.vecLocation.x += RandomFloat(-2.0f, 2.0f);
				tInst.vecLocation.y += RandomFloat(-2.0f, 2.0f);

				float ang = 0.0f;
				if (m_bOldBehavior)
				{
					// LuaSTG Plus、LuaSTG Ex Plus、LuaSTG-x 的代码
					ang = /* pInfo.fDirection */ (m_fDirection - L_PI_HALF_F) - L_PI_HALF_F + RandomFloat(0.0f, pInfo.fSpread) - pInfo.fSpread / 2.0f;
				}
				else
				{
					// 来自 HGE 的原始代码，但是似乎 HGE 的坐标系 y 轴是向下的，直接拿来用并不可行
					//float ang = pInfo.fDirection - L_PI_HALF_F + RandomFloat(0.0f, pInfo.fSpread) - pInfo.fSpread / 2.0f;
					// 修改后的正确的代码应该是这个
					ang = -pInfo.fDirection + L_PI_HALF_F + RandomFloat(0.0f, pInfo.fSpread) - pInfo.fSpread / 2.0f;
					if (pInfo.bRelative)
					{
						ang += (m_vPrevCenter - m_vCenter).angle() + L_PI_HALF_F;
					}
					// 此外，我们还有自己的旋转量
					ang += m_fDirection;
				}

				tInst.vecVelocity.x = std::cos(ang);
				tInst.vecVelocity.y = std::sin(ang);
				tInst.vecVelocity *= RandomFloat(pInfo.fSpeedMin, pInfo.fSpeedMax);

				tInst.fGravity = RandomFloat(pInfo.fGravityMin, pInfo.fGravityMax);
				tInst.fRadialAccel = RandomFloat(pInfo.fRadialAccelMin, pInfo.fRadialAccelMax);
				tInst.fTangentialAccel = RandomFloat(pInfo.fTangentialAccelMin, pInfo.fTangentialAccelMax);

				tInst.fSize = RandomFloat(pInfo.fSizeStart, pInfo.fSizeStart + (pInfo.fSizeEnd - pInfo.fSizeStart) * pInfo.fSizeVar);
				tInst.fSizeDelta = (pInfo.fSizeEnd - tInst.fSize) / tInst.fTerminalAge;

				// TODO: 删除
				//tInst.fSpin = /* pInfo.fSpinStart */ m_fRotation + s_ParticleRandomizer.GetRandFloat(0, pInfo.fSpinEnd) - pInfo.fSpinEnd / 2.0f;
				//tInst.fSpinDelta = pInfo.fSpinVar;

				tInst.fSpin = RandomFloat(pInfo.fSpinStart, pInfo.fSpinStart + (pInfo.fSpinEnd - pInfo.fSpinStart) * pInfo.fSpinVar);
				tInst.fSpinDelta = (pInfo.fSpinEnd - tInst.fSpin) / tInst.fTerminalAge;

				tInst.colColor[0] = RandomFloat(pInfo.colColorStart[0], pInfo.colColorStart[0] + (pInfo.colColorEnd[0] - pInfo.colColorStart[0]) * pInfo.fColorVar);
				tInst.colColor[1] = RandomFloat(pInfo.colColorStart[1], pInfo.colColorStart[1] + (pInfo.colColorEnd[1] - pInfo.colColorStart[1]) * pInfo.fColorVar);
				tInst.colColor[2] = RandomFloat(pInfo.colColorStart[2], pInfo.colColorStart[2] + (pInfo.colColorEnd[2] - pInfo.colColorStart[2]) * pInfo.fColorVar);
				tInst.colColor[3] = RandomFloat(pInfo.colColorStart[3], pInfo.colColorStart[3] + (pInfo.colColorEnd[3] - pInfo.colColorStart[3]) * pInfo.fAlphaVar);

				tInst.colColorDelta[0] = (pInfo.colColorEnd[0] - tInst.colColor[0]) / tInst.fTerminalAge;
				tInst.colColorDelta[1] = (pInfo.colColorEnd[1] - tInst.colColor[1]) / tInst.fTerminalAge;
				tInst.colColorDelta[2] = (pInfo.colColorEnd[2] - tInst.colColor[2]) / tInst.fTerminalAge;
				tInst.colColorDelta[3] = (pInfo.colColorEnd[3] - tInst.colColor[3]) / tInst.fTerminalAge;
			}
		}

		m_vPrevCenter = m_vCenter;
	}
	void ParticlePoolImpl::Render(float scaleX, float scaleY)
	{
		core::Graphics::ISprite* pSprite = m_Info.pSprite.get();
		hgeParticleSystemInfo const& pInfo = m_Info.tParticleSystemInfo;
		core::Color4B const tVertexColor = GetVertexColor();
		for (size_t i = 0; i < m_iAlive; i += 1)
		{
			hgeParticle const& pInst = m_ParticlePool[i];
			if (pInfo.colColorStart[0] < 0) // r < 0
			{
				pSprite->setColor(core::Color4B(
					tVertexColor.r,
					tVertexColor.g,
					tVertexColor.b,
					(uint8_t)std::clamp(pInst.colColor[3] * (float)tVertexColor.a, 0.0f, 255.0f)
				));
			}
			else
			{
				pSprite->setColor(core::Color4B(
					(uint8_t)std::clamp(pInst.colColor[0] * (float)tVertexColor.r, 0.0f, 255.0f),
					(uint8_t)std::clamp(pInst.colColor[1] * (float)tVertexColor.g, 0.0f, 255.0f),
					(uint8_t)std::clamp(pInst.colColor[2] * (float)tVertexColor.b, 0.0f, 255.0f),
					(uint8_t)std::clamp(pInst.colColor[3] * (float)tVertexColor.a, 0.0f, 255.0f)
				));
			}
			pSprite->draw(
				core::Vector2F(pInst.vecLocation.x, pInst.vecLocation.y),
				core::Vector2F(scaleX * pInst.fSize, scaleY * pInst.fSize),
				pInst.fSpin);
		}
	}
}
