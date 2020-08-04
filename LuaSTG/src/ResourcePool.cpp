#include "ResourceMgr.h"
#include "AppFrame.h"
#include "Utility.h"

#include <io.h>
#include "ESC.h"

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

using namespace std;
using namespace LuaSTGPlus;

#if (defined LDEVVERSION) || (defined LDEBUG)
#define LDEBUG_RESOURCETIMER float tResourceLoadingTime
#define LDEBUG_RESOURCESCOPE TimerScope tLoadingTimer(tResourceLoadingTime)
#define LDEBUG_RESOURCEHINT(t, path) \
    LAPP.SendResourceLoadedHint(t, m_iType, name, path, tResourceLoadingTime)
#else
#define LDEBUG_RESOURCETIMER
#define LDEBUG_RESOURCESCOPE
#define LDEBUG_RESOURCEHINT
#endif

void ResourcePool::Clear()LNOEXCEPT
{
	m_TexturePool.clear();
	m_SpritePool.clear();
	m_AnimationPool.clear();
	m_MusicPool.clear();
	m_SoundSpritePool.clear();
	m_ParticlePool.clear();
	m_SpriteFontPool.clear();
	m_TTFFontPool.clear();
	m_FXPool.clear();

#if (defined LDEVVERSION) || (defined LDEBUG)
	LAPP.SendResourceClearedHint(m_iType);
#endif
}

void ResourcePool::RemoveResource(ResourceType t, const char* name)LNOEXCEPT
{
	switch (t)
	{
	case ResourceType::Texture:
		removeResource(m_TexturePool, name);
		break;
	case ResourceType::Sprite:
		removeResource(m_SpritePool, name);
		break;
	case ResourceType::Animation:
		removeResource(m_AnimationPool, name);
		break;
	case ResourceType::Music:
		removeResource(m_MusicPool, name);
		break;
	case ResourceType::SoundEffect:
		removeResource(m_SoundSpritePool, name);
		break;
	case ResourceType::Particle:
		removeResource(m_ParticlePool, name);
		break;
	case ResourceType::SpriteFont:
		removeResource(m_SpriteFontPool, name);
		break;
	case ResourceType::TrueTypeFont:
		removeResource(m_TTFFontPool, name);
		break;
	case ResourceType::FX:
		removeResource(m_FXPool, name);
		break;
	default:
		return;
	}

#if (defined LDEVVERSION) || (defined LDEBUG)
	LAPP.SendResourceRemovedHint(t, m_iType, name);
#endif
}

void ResourcePool::ExportResourceList(lua_State* L, ResourceType t)const LNOEXCEPT
{
	int cnt = 1;
	switch (t)
	{
	case ResourceType::Texture:
		lua_newtable(L);  // t
		for (auto& i : m_TexturePool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::Sprite:
		lua_newtable(L);  // t
		for (auto& i : m_SpritePool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::Animation:
		lua_newtable(L);  // t
		for (auto& i : m_AnimationPool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::Music:
		lua_newtable(L);  // t
		for (auto& i : m_MusicPool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::SoundEffect:
		lua_newtable(L);  // t
		for (auto& i : m_SoundSpritePool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::Particle:
		lua_newtable(L);  // t
		for (auto& i : m_ParticlePool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::SpriteFont:
		lua_newtable(L);  // t
		for (auto& i : m_SpriteFontPool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::TrueTypeFont:
		lua_newtable(L);  // t
		for (auto& i : m_TTFFontPool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::FX:
		lua_newtable(L);  // t
		for (auto& i : m_FXPool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	default:
		lua_pushnil(L);
		break;
	}
}

bool ResourcePool::LoadTexture(const char* name, const std::wstring& path, bool mipmaps)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderDev());

		if (m_TexturePool.find(name) != m_TexturePool.end())
		{
			LWARNING("LoadTexture: 纹理'%m'已存在，试图使用'%s'加载的操作已被取消", name, path.c_str());
			return true;
		}

		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
			return false;

		fcyRefPointer<f2dTexture2D> tTexture;
		if (FCYFAILED(LAPP.GetRenderDev()->CreateTextureFromMemory((fcData)tDataBuf->GetInternalBuffer(), tDataBuf->GetLength(), 0, 0, false, mipmaps, &tTexture)))
		{
			LERROR("LoadTexture: 从文件'%s'创建纹理'%m'失败", path.c_str(), name);
			return false;
		}

		try
		{
			fcyRefPointer<ResTexture> tRes;
			tRes.DirectSet(new ResTexture(name, tTexture));
			m_TexturePool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadTexture: 内存不足");
			return false;
		}

#ifdef LSHOWRESLOADINFO
		LINFO("LoadTexture: 纹理'%s'已装载 -> '%m' (%s)", path.c_str(), name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::Texture, path.c_str());
	return true;
}

bool ResourcePool::LoadModel(const char* name, const std::wstring& path)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderDev());

		if (m_TexturePool.find(name) != m_TexturePool.end())
		{
			LWARNING("LoadModel: 模型'%m'已存在，试图使用'%s'加载的操作已被取消", name, path.c_str());
			return true;
		}

		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
			return false;
		fcyRefPointer<fcyMemStream> tDataBuf2;
		std::wstring path2 = path;
		int i = path2.length();
		path2[i - 3] = 'm';
		path2[i - 2] = 't';
		path2[i - 1] = 'l';
		if (!m_pMgr->LoadFile(path2.c_str(), tDataBuf2))
			return false;
		void* model = NULL;
		void* LoadObj(const string & id, const string & path, const string & path2);
		string buf((char*)tDataBuf->GetInternalBuffer(), tDataBuf->GetLength());
		string buf2((char*)tDataBuf2->GetInternalBuffer(), tDataBuf2->GetLength());
		model = LoadObj(name, buf, buf2);

		try
		{
			fcyRefPointer<ResModel> tRes;
			tRes.DirectSet(new ResModel(name, model));
			m_ModelPool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadModel: 内存不足");
			return false;
		}

#ifdef LSHOWRESLOADINFO
		LINFO("LoadTexture: 纹理'%s'已装载 -> '%m' (%s)", path.c_str(), name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::Texture, path.c_str());
	return true;
}

bool ResourcePool::LoadModel(const char* name, const char* path)LNOEXCEPT
{
	try
	{
		return LoadModel(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8));
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadModel: 转换编码时无法分配内存");
		return false;
	}
}

bool ResourcePool::LoadTexture(const char* name, const char* path, bool mipmaps)LNOEXCEPT
{
	try
	{
		return LoadTexture(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8), mipmaps);
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadTexture: 转换编码时无法分配内存");
		return false;
	}
}

bool ResourcePool::LoadImage(const char* name, const char* texname,
	double x, double y, double w, double h, double a, double b, bool rect)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderer());

		if (m_SpritePool.find(name) != m_SpritePool.end())
		{
			LWARNING("LoadImage: 图像'%m'已存在，加载操作已被取消", name);
			return true;
		}

		fcyRefPointer<ResTexture> pTex = m_pMgr->FindTexture(texname);
		if (!pTex)
		{
			LWARNING("LoadImage: 加载图像'%m'失败, 无法找到纹理'%m'", name, texname);
			return false;
		}

		fcyRefPointer<f2dSprite> pSprite;
		fcyRect tRect((float)x, (float)y, (float)(x + w), (float)(y + h));
		if (FCYFAILED(LAPP.GetRenderer()->CreateSprite2D(pTex->GetTexture(), tRect, &pSprite)))
		{
			LERROR("LoadImage: 无法从纹理'%m'加载图像'%m' (CreateSprite2D failed)", texname, name);
			return false;
		}

		try
		{
			fcyRefPointer<ResSprite> tRes;
			tRes.DirectSet(new ResSprite(name, pSprite, a, b, rect));
			m_SpritePool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadImage: 内存不足");
			return false;
		}

#ifdef LSHOWRESLOADINFO
		LINFO("LoadImage: 图像'%m'已装载 (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::Sprite, L"N/A");
	return true;
}

bool ResourcePool::LoadAnimation(const char* name, const char* texname,
	double x, double y, double w, double h, int n, int m, int intv, double a, double b, bool rect)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		if (m_AnimationPool.find(name) != m_AnimationPool.end())
		{
			LWARNING("LoadAnimation: 动画'%m'已存在，加载操作已被取消", name);
			return true;
		}

		fcyRefPointer<ResTexture> pTex = m_pMgr->FindTexture(texname);
		if (!pTex)
		{
			LWARNING("LoadAnimation: 加载动画'%m'失败, 无法找到纹理'%m'", name, texname);
			return false;
		}

		try
		{
			fcyRefPointer<ResAnimation> tRes;
			tRes.DirectSet(new ResAnimation(name, pTex, (float)x, (float)y, (float)w, (float)h, n, m, intv, a, b, rect));
			m_AnimationPool.emplace(name, tRes);
		}
		catch (const fcyException&)
		{
			LERROR("LoadAnimation: 构造动画'%m'时失败", name);
			return false;
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadAnimation: 内存不足");
			return false;
		}

#ifdef LSHOWRESLOADINFO
		LINFO("LoadAnimation: 动画'%m'已装载 (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::Animation, L"N/A");
	return true;
}

bool ResourcePool::LoadMusic(const char* name, const std::wstring& path, double start, double end)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		//检查音频系统是否已经初始化
		LASSERT(LAPP.GetSoundSys());

		//加载音频文件流，加载到内存
		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
			return false;

		try
		{
			//加载解码器，优先OGG解码器，加载失败则使用WAV解码器，都失败则error糊脸
			fcyRefPointer<f2dSoundDecoder> tDecoder;
			if (FCYFAILED(LAPP.GetSoundSys()->CreateOGGVorbisDecoder(tDataBuf, &tDecoder)))
			{
				tDataBuf->SetPosition(FCYSEEKORIGIN_BEG, 0);
				if (FCYFAILED(LAPP.GetSoundSys()->CreateWaveDecoder(tDataBuf, &tDecoder)))
				{
					LERROR("LoadMusic: 无法解码文件'%s'", path.c_str());
					return false;
				}
			}

			//加载解码器
			fcyRefPointer<ResMusic::BGMWrapper> tWrapperedBuffer;
			tWrapperedBuffer.DirectSet(new ResMusic::BGMWrapper(tDecoder, start, end));

			//加载音频缓冲曲，动态缓冲区
			fcyRefPointer<f2dSoundBuffer> tBuffer;
			if (FCYFAILED(LAPP.GetSoundSys()->CreateDynamicBuffer(tWrapperedBuffer, LSOUNDGLOBALFOCUS, &tBuffer)))
			{
				LERROR("LoadMusic: 无法创建音频缓冲区，文件'%s' (f2dSoundSys::CreateDynamicBuffer failed.)", path.c_str());
				return false;
			}

			//存入资源池
			fcyRefPointer<ResMusic> tRes;
			tRes.DirectSet(new ResMusic(name, tBuffer));
			m_MusicPool.emplace(name, tRes);
		}
		catch (const fcyException& e)
		{
			LERROR("LoadMusic: 解码文件'%s'的音频数据时发生错误，格式不支持？ (异常信息'%m' 源'%m')", path.c_str(), e.GetDesc(), e.GetSrc());
			return false;
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadMusic: 内存不足");
			return false;
		}

#ifdef LSHOWRESLOADINFO
		LINFO("LoadMusic: BGM'%m'已装载 (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::Music, path.c_str());
	return true;
}

LNOINLINE bool ResourcePool::LoadMusic(const char* name, const char* path, double start, double end)LNOEXCEPT
{
	try
	{
		return LoadMusic(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8), start, end);
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadMusic: 转换编码时无法分配内存");
		return false;
	}
}

bool ResourcePool::LoadSound(const char* name, const std::wstring& path)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetSoundSys());

		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
			return false;

		try
		{
			fcyRefPointer<f2dSoundDecoder> tDecoder;
			if (FCYFAILED(LAPP.GetSoundSys()->CreateWaveDecoder(tDataBuf, &tDecoder)))
			{
				tDataBuf->SetPosition(FCYSEEKORIGIN_BEG, 0);
				if (FCYFAILED(LAPP.GetSoundSys()->CreateOGGVorbisDecoder(tDataBuf, &tDecoder)))
				{
					LERROR("LoadSound: 无法解码文件'%s'", path.c_str());
					return false;
				}
			}

			fcyRefPointer<f2dSoundBuffer> tBuffer;
			if (FCYFAILED(LAPP.GetSoundSys()->CreateStaticBuffer(tDecoder, LSOUNDGLOBALFOCUS, &tBuffer)))
			{
				LERROR("LoadSound: 无法创建音频缓冲区，文件'%s' (f2dSoundSys::CreateStaticBuffer failed.)", path.c_str());
				return false;
			}

			fcyRefPointer<ResSound> tRes;
			tRes.DirectSet(new ResSound(name, tBuffer));
			m_SoundSpritePool.emplace(name, tRes);
		}
		catch (const fcyException& e)
		{
			LERROR("LoadSound: 解码文件'%s'的音频数据时发生错误，格式不支持？ (异常信息'%m' 源'%m')", path.c_str(), e.GetDesc(), e.GetSrc());
			return false;
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadSound: 内存不足");
			return false;
		}

#ifdef LSHOWRESLOADINFO
		LINFO("LoadSound: 音效'%m'已装载 (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::SoundEffect, path.c_str());
	return true;
}

LNOINLINE bool ResourcePool::LoadSound(const char* name, const char* path)LNOEXCEPT
{
	try
	{
		return LoadSound(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8));
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadSound: 转换编码时无法分配内存");
		return false;
	}
}

bool ResourcePool::LoadParticle(const char* name, const std::wstring& path, const char* img_name, double a, double b, bool rect)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderer());

		if (m_ParticlePool.find(name) != m_ParticlePool.end())
		{
			LWARNING("LoadParticle: 粒子'%m'已存在，加载操作已被取消", name);
			return true;
		}

		fcyRefPointer<ResSprite> pSprite = m_pMgr->FindSprite(img_name);
		fcyRefPointer<f2dSprite> pClone;
		if (!pSprite)
		{
			LWARNING("LoadParticle: 加载粒子'%m'失败, 无法找到精灵'%m'", name, img_name);
			return false;
		}
		else
		{
			// 克隆一个精灵对象
			if (FCYFAILED(LAPP.GetRenderer()->CreateSprite2D(pSprite->GetSprite()->GetTexture(), pSprite->GetSprite()->GetTexRect(), pSprite->GetSprite()->GetHotSpot(), &pClone)))
			{
				LERROR("LoadParticle: 克隆图片'%m'失败", img_name);
				return false;
			}
			pClone->SetColor(0, pSprite->GetSprite()->GetColor(0U));
			pClone->SetColor(1, pSprite->GetSprite()->GetColor(1U));
			pClone->SetColor(2, pSprite->GetSprite()->GetColor(2U));
			pClone->SetColor(3, pSprite->GetSprite()->GetColor(3U));
			pClone->SetZ(pSprite->GetSprite()->GetZ());
		}

		fcyRefPointer<fcyMemStream> outBuf;
		if (!LRES.LoadFile(path.c_str(), outBuf))
			return false;
		if (outBuf->GetLength() != sizeof(ResParticle::ParticleInfo))
		{
			LERROR("LoadParticle: 粒子定义文件'%s'格式不正确", path.c_str());
			return false;
		}

		try
		{
			ResParticle::ParticleInfo tInfo;
			memcpy(&tInfo, outBuf->GetInternalBuffer(), sizeof(ResParticle::ParticleInfo));
			tInfo.iBlendInfo = (tInfo.iBlendInfo >> 16) & 0x00000003;

			BlendMode tBlendInfo = BlendMode::AddAlpha;
			if (tInfo.iBlendInfo & 1)  // ADD
			{
				if (tInfo.iBlendInfo & 2)  // ALPHA
					tBlendInfo = BlendMode::AddAlpha;
				else
					tBlendInfo = BlendMode::AddAdd;
			}
			else  // MUL
			{
				if (tInfo.iBlendInfo & 2)  // ALPHA
					tBlendInfo = BlendMode::MulAlpha;
				else
					tBlendInfo = BlendMode::MulAdd;
			}

			fcyRefPointer<ResParticle> tRes;
			tRes.DirectSet(new ResParticle(name, tInfo, pClone, tBlendInfo, a, b, rect));
			m_ParticlePool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadParticle: 内存不足");
			return false;
		}
#ifdef LSHOWRESLOADINFO
		LINFO("LoadParticle: 粒子'%m'已装载 (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::Particle, path.c_str());
	return true;
}

bool ResourcePool::LoadParticle(const char* name, const ResParticle::ParticleInfo& info, const char* img_name, double a, double b, bool rect)LNOEXCEPT {
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderer());

		if (m_ParticlePool.find(name) != m_ParticlePool.end())
		{
			LWARNING("LoadParticle: 粒子'%m'已存在，加载操作已被取消", name);
			return true;
		}

		fcyRefPointer<ResSprite> pSprite = m_pMgr->FindSprite(img_name);
		fcyRefPointer<f2dSprite> pClone;
		if (!pSprite)
		{
			LWARNING("LoadParticle: 加载粒子'%m'失败, 无法找到精灵'%m'", name, img_name);
			return false;
		}
		else
		{
			// 克隆一个精灵对象
			if (FCYFAILED(LAPP.GetRenderer()->CreateSprite2D(pSprite->GetSprite()->GetTexture(), pSprite->GetSprite()->GetTexRect(), pSprite->GetSprite()->GetHotSpot(), &pClone)))
			{
				LERROR("LoadParticle: 克隆图片'%m'失败", img_name);
				return false;
			}
			pClone->SetColor(0, pSprite->GetSprite()->GetColor(0U));
			pClone->SetColor(1, pSprite->GetSprite()->GetColor(1U));
			pClone->SetColor(2, pSprite->GetSprite()->GetColor(2U));
			pClone->SetColor(3, pSprite->GetSprite()->GetColor(3U));
			pClone->SetZ(pSprite->GetSprite()->GetZ());
		}

		try
		{
			ResParticle::ParticleInfo tInfo = info;
			tInfo.iBlendInfo = (tInfo.iBlendInfo >> 16) & 0x00000003;

			BlendMode tBlendInfo = BlendMode::AddAlpha;
			if (tInfo.iBlendInfo & 1)  // ADD
			{
				if (tInfo.iBlendInfo & 2)  // ALPHA
					tBlendInfo = BlendMode::AddAlpha;
				else
					tBlendInfo = BlendMode::AddAdd;
			}
			else  // MUL
			{
				if (tInfo.iBlendInfo & 2)  // ALPHA
					tBlendInfo = BlendMode::MulAlpha;
				else
					tBlendInfo = BlendMode::MulAdd;
			}

			fcyRefPointer<ResParticle> tRes;
			tRes.DirectSet(new ResParticle(name, tInfo, pClone, tBlendInfo, a, b, rect));
			m_ParticlePool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadParticle: 内存不足");
			return false;
		}
#ifdef LSHOWRESLOADINFO
		LINFO("LoadParticle: 粒子'%m'已装载 (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::Particle, L"binary");
	return true;
}

LNOINLINE bool ResourcePool::LoadParticle(const char* name, const char* path, const char* img_name, double a, double b, bool rect)LNOEXCEPT
{
	try
	{
		return LoadParticle(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8), img_name, a, b, rect);
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadParticle: 转换编码时无法分配内存");
		return false;
	}
}

bool ResourcePool::LoadSpriteFont(const char* name, const std::wstring& path, bool mipmaps)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderer());

		if (m_SpriteFontPool.find(name) != m_SpriteFontPool.end())
		{
			LWARNING("LoadFont: 字体'%m'已存在，加载操作已被取消", name);
			return true;
		}

		std::unordered_map<wchar_t, f2dGlyphInfo> tOutputCharset;
		std::wstring tOutputTextureName;

		// 读取文件
		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf)) {
			LERROR("LoadFont: 无法加载字体定义文件 ");
			return false;
		}

		// 转换编码
		wstring tFileData;
		try
		{
			if (tDataBuf->GetLength() > 0)
			{
				// stupid
				tFileData = fcyStringHelper::MultiByteToWideChar(string((const char*)tDataBuf->GetInternalBuffer(), (size_t)tDataBuf->GetLength()), CP_UTF8);
			}
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFont: 转换编码时无法分配内存");
			return false;
		}

		// 读取HGE字体定义
		try
		{
			ResFont::HGEFont::ReadDefine(tFileData, tOutputCharset, tOutputTextureName);
		}
		catch (const fcyException& e)
		{
			LERROR("LoadFont: 装载HGE字体定义文件'%s'失败 (异常信息'%m' 源'%m')", path.c_str(), e.GetDesc(), e.GetSrc());
			return false;
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFont: 内存不足");
			return false;
		}

		// 装载纹理
		try
		{
			if (!m_pMgr->LoadFile((fcyPathParser::GetPath(path) + tOutputTextureName).c_str(), tDataBuf)) {
				LERROR("LoadFont: 无法加载字体纹理图片");
				return false;
			}
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFont: 内存不足");
			return false;
		}

		fcyRefPointer<f2dTexture2D> tTexture;
		if (FCYFAILED(LAPP.GetRenderDev()->CreateTextureFromMemory((fcData)tDataBuf->GetInternalBuffer(), tDataBuf->GetLength(), 0, 0, false, mipmaps, &tTexture)))
		{
			LERROR("LoadFont: 从文件'%s'创建纹理'%m'失败", tOutputTextureName.c_str(), name);
			return false;
		}

		// 创建定义
		try
		{
			fcyRefPointer<f2dFontProvider> tFontProvider;
			tFontProvider.DirectSet(new ResFont::HGEFont(std::move(tOutputCharset), tTexture));

			fcyRefPointer<ResFont> tRes;
			tRes.DirectSet(new ResFont(name, tFontProvider));
			m_SpriteFontPool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFont: 内存不足");
			return false;
		}
#ifdef LSHOWRESLOADINFO
		LINFO("LoadFont: 纹理字体'%m'已装载 (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::SpriteFont, path.c_str());
	return true;
}

bool ResourcePool::LoadSpriteFont(const char* name, const std::wstring& path, const std::wstring& tex_path, bool mipmaps)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderer());

		if (m_SpriteFontPool.find(name) != m_SpriteFontPool.end())
		{
			LWARNING("LoadFont: 字体'%m'已存在，加载操作已被取消", name);
			return true;
		}

		// 读取文件
		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
			return false;

		// 转换编码
		wstring tFileData;
		try
		{
			if (tDataBuf->GetLength() > 0)
			{
				// stupid
				tFileData = fcyStringHelper::MultiByteToWideChar(string((const char*)tDataBuf->GetInternalBuffer(), (size_t)tDataBuf->GetLength()), CP_UTF8);
			}
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFont: 转换编码时无法分配内存");
			return false;
		}

		// 装载纹理
		try
		{
			if (!m_pMgr->LoadFile((fcyPathParser::GetPath(path) + tex_path).c_str(), tDataBuf))
			{
				if (!m_pMgr->LoadFile(tex_path.c_str(), tDataBuf))
					return false;
			}
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFont: 内存不足");
			return false;
		}

		fcyRefPointer<f2dTexture2D> tTexture;
		if (FCYFAILED(LAPP.GetRenderDev()->CreateTextureFromMemory((fcData)tDataBuf->GetInternalBuffer(), tDataBuf->GetLength(), 0, 0, false, mipmaps, &tTexture)))
		{
			LERROR("LoadFont: 从文件'%s'创建纹理'%m'失败", tex_path.c_str(), name);
			return false;
		}

		// 创建定义
		try
		{
			fcyRefPointer<f2dFontProvider> tFontProvider;
			if (FCYFAILED(LAPP.GetRenderer()->CreateFontFromTex(tFileData.c_str(), tTexture, &tFontProvider)))
			{
				LERROR("LoadFont: 从文件'%s'创建纹理字体失败", path.c_str());
				return false;
			}

			fcyRefPointer<ResFont> tRes;
			tRes.DirectSet(new ResFont(name, tFontProvider));
			m_SpriteFontPool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFont: 内存不足");
			return false;
		}
#ifdef LSHOWRESLOADINFO
		LINFO("LoadFont: 纹理字体'%m'已装载 (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::SpriteFont, path.c_str());
	return true;
}

LNOINLINE bool ResourcePool::LoadSpriteFont(const char* name, const char* path, bool mipmaps)LNOEXCEPT
{
	try
	{
		return LoadSpriteFont(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8), mipmaps);
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadSpriteFont: 转换编码时无法分配内存");
		return false;
	}
}

LNOINLINE bool ResourcePool::LoadSpriteFont(const char* name, const char* path, const char* tex_path, bool mipmaps)LNOEXCEPT
{
	try
	{
		return LoadSpriteFont(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8), fcyStringHelper::MultiByteToWideChar(tex_path, CP_UTF8), mipmaps);
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadSpriteFont: 转换编码时无法分配内存");
		return false;
	}
}

bool ResourcePool::LoadTTFFont(const char* name, const std::wstring& path, float width, float height, float bboxwidth, float bboxheight)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderer());

		if (m_TTFFontPool.find(name) != m_TTFFontPool.end())
		{
			LWARNING("LoadTTFFont: 字体'%m'已存在，加载操作已被取消", name);
			return true;
		}

		fcyRefPointer<f2dFontProvider> tFontProvider;

		// 读取文件
		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
		{
			LINFO("LoadTTFFont: 无法在路径'%s'上加载字体，尝试以系统字体对待并加载系统字体", path.c_str());
			if (FCYFAILED(LAPP.GetRenderer()->CreateSystemFont(path.c_str(), 0, fcyVec2(width, height), F2DFONTFLAG_NONE, &tFontProvider)))
			{
				//LERROR("LoadTTFFont: 尝试失败，无法从路径'%s'上加载字体", path.c_str());
				LWARNING("LoadTTFFont: 尝试失败，无法从系统字体库加载字体'%s'", path.c_str());//向lua层返回错误，而不是直接崩游戏
				return false;
			}
		}

		// 创建定义
		try
		{
			if (!tFontProvider)
			{
				if (FCYFAILED(LAPP.GetRenderer()->CreateFontFromFile(tDataBuf, 0, fcyVec2(width, height), fcyVec2(bboxwidth, bboxheight), F2DFONTFLAG_NONE, &tFontProvider)))
				{
					LERROR("LoadTTFFont: 从文件'%s'创建TrueType字体失败", path.c_str());
					return false;
				}
			}
#ifdef LSHOWRESLOADINFO
			LINFO("字形缓存数量：%d，字形缓存贴图大小：%dx%d", tFontProvider->GetCacheCount(), tFontProvider->GetCacheTexSize(), tFontProvider->GetCacheTexSize());
#endif
			fcyRefPointer<ResFont> tRes;
			tRes.DirectSet(new ResFont(name, tFontProvider));
			tRes->SetBlendMode(BlendMode::AddAlpha);
			m_TTFFontPool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadTTFFont: 内存不足");
			return false;
		}
#ifdef LSHOWRESLOADINFO
		LINFO("LoadTTFFont: truetype字体'%m'已装载 (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::TrueTypeFont, path.c_str());
	return true;
}

LNOINLINE bool ResourcePool::LoadTTFFont(const char* name, const char* path, float width, float height, float bboxwidth, float bboxheight)LNOEXCEPT
{
	try
	{
		return LoadTTFFont(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8), width, height, bboxwidth, bboxheight);
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadTTFFont: 转换编码时无法分配内存");
		return false;
	}
}

bool ResourcePool::LoadTTFFont(const char* name, fcyStream* stream, float width, float height, float bboxwidth, float bboxheight)LNOEXCEPT {
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderer());

		if (m_TTFFontPool.find(name) != m_TTFFontPool.end())
		{
			LWARNING("LoadTTFFont: 字体'%m'已存在，加载操作已被取消", name);
			return true;
		}

		fcyRefPointer<f2dFontProvider> tFontProvider;
		// 创建定义
		try
		{
			if (!tFontProvider)
			{
				if (FCYFAILED(LAPP.GetRenderer()->CreateFontFromFile(stream, 0, fcyVec2(width, height), fcyVec2(bboxwidth, bboxheight), F2DFONTFLAG_NONE, &tFontProvider)))
				{
					LERROR("LoadTTFFont: 从内存创建TrueType字体失败");
					return false;
				}
			}
#ifdef LSHOWRESLOADINFO
			LINFO("字形缓存数量：%d，字形缓存贴图大小：%dx%d", tFontProvider->GetCacheCount(), tFontProvider->GetCacheTexSize(), tFontProvider->GetCacheTexSize());
#endif
			fcyRefPointer<ResFont> tRes;
			tRes.DirectSet(new ResFont(name, tFontProvider));
			tRes->SetBlendMode(BlendMode::AddAlpha);
			m_TTFFontPool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadTTFFont: 内存不足");
			return false;
		}
#ifdef LSHOWRESLOADINFO
		LINFO("LoadTTFFont: truetype字体'%m'已装载 (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::TrueTypeFont, L"Memory");
	return true;
}

bool ResourcePool::LoadFX(const char* name, const std::wstring& path)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderDev());

		if (m_FXPool.find(name) != m_FXPool.end())
		{
			LWARNING("LoadFX: FX'%m'已存在，加载操作已被取消", name);
			return true;
		}

		// 读取文件
		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
			return false;

		try
		{
			fcyRefPointer<f2dEffect> tEffect;
			if (FCYFAILED(LAPP.GetRenderDev()->CreateEffect(tDataBuf, false, &tEffect)))
			{
				LERROR("LoadFX: 加载shader于文件'%s'失败 (lasterr=%m)", path.c_str(), LAPP.GetEngine()->GetLastErrDesc());
				return false;
			}

			fcyRefPointer<ResFX> tRes;
			tRes.DirectSet(new ResFX(name, tEffect));
			m_FXPool.emplace(name, tRes);
		}
		catch (const fcyException& e)
		{
			LERROR("LoadFX: 绑定变量于文件'%s'失败 (异常信息'%m' 源'%m')", path.c_str(), e.GetDesc(), e.GetSrc());
			return false;
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFX: 内存不足");
			return false;
		}
#ifdef LSHOWRESLOADINFO
		LINFO("LoadFX: FX'%m'已装载 (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::FX, path.c_str());
	return true;
}

LNOINLINE bool ResourcePool::LoadFX(const char* name, const char* path)LNOEXCEPT
{
	try
	{
		return LoadFX(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8));
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadFX: 转换编码时无法分配内存");
		return false;
	}
}

LNOINLINE bool ResourcePool::CreateRenderTarget(const char* name)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderDev());

		if (m_TexturePool.find(name) != m_TexturePool.end())
		{
			LWARNING("CreateRenderTarget: '%m'已存在，创建操作已被取消", name);
			return true;
		}

		fcyRefPointer<f2dTexture2D> tTexture;
		if (FCYFAILED(LAPP.GetRenderDev()->CreateRenderTarget(LAPP.GetRenderDev()->GetBufferWidth(),
			LAPP.GetRenderDev()->GetBufferHeight(), true, &tTexture)))
		{
			LERROR("CreateRenderTarget: 创建渲染目标'%m'失败", name);
			return false;
		}

		try
		{
			fcyRefPointer<ResTexture> tRes;
			tRes.DirectSet(new ResTexture(name, tTexture));
			m_TexturePool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("CreateRenderTarget: 内存不足");
			return false;
		}

#ifdef LSHOWRESLOADINFO
		LINFO("CreateRenderTarget: '%m'已创建 (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::Texture, L"[RenderTarget]");
	return true;
}
