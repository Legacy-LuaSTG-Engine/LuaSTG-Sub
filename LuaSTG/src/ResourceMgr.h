#pragma once
#include "Global.h"
#include "Dictionary.hpp"
#include "ResourceTexture.hpp"
#include "ResourceSprite.hpp"
#include "ResourceAnimation.hpp"
#include "ResourceParticle.hpp"
#include "ResourceFont.hpp"
#include "ResourceFX.hpp"
#include "ResourceAudio.hpp"
#include "ResourceModel.hpp"

#ifdef LoadImage
#undef LoadImage
#endif

namespace LuaSTGPlus
{
	class ResourceMgr;

	// 资源池类型
	enum class ResourcePoolType
	{
		None = 0,
		Global,
		Stage
	};

	// 资源池
	class ResourcePool
	{
	private:
		ResourceMgr* m_pMgr;
		ResourcePoolType m_iType;

		Dictionary<fcyRefPointer<ResTexture>> m_TexturePool;
		Dictionary<fcyRefPointer<ResSprite>> m_SpritePool;
		Dictionary<fcyRefPointer<ResAnimation>> m_AnimationPool;
		Dictionary<fcyRefPointer<ResSound>> m_MusicPool;
		Dictionary<fcyRefPointer<ResMusic>> m_SoundSpritePool;
		Dictionary<fcyRefPointer<ResParticle>> m_ParticlePool;
		Dictionary<fcyRefPointer<ResFont>> m_SpriteFontPool;
		Dictionary<fcyRefPointer<ResFont>> m_TTFFontPool;
		Dictionary<fcyRefPointer<ResFX>> m_FXPool;
		Dictionary<fcyRefPointer<ResModel>> m_ModelPool;
	private:
		const wchar_t* getResourcePoolTypeName()
		{
			switch (m_iType)
			{
			case ResourcePoolType::Global:
				return L"全局资源池";
			case ResourcePoolType::Stage:
				return L"关卡资源池";
			default:
				return nullptr;
			}
		}
		template <typename T>
		void removeResource(Dictionary<fcyRefPointer<T>>& pool, const char* name)//bakaCHU漏了一个&草
		{
			auto i = pool.find(name);
			if (i == pool.end())
			{
				LWARNING("RemoveResource: 试图移除一个不存在的资源'%m'", name);
				return;
			}
			pool.erase(i);
#ifdef LSHOWRESLOADINFO
			LINFO("RemoveResource: 资源'%m'已卸载", name);
#endif
		}
	public:
		/// @brief 清空对象池
		void Clear()LNOEXCEPT;

		/// @brief 移除某个资源类型的资源
		void RemoveResource(ResourceType t, const char* name)LNOEXCEPT;

		/// @brief 检查资源是否存在
		/// @warning 注意t可以是非法枚举量
		bool CheckResourceExists(ResourceType t, const std::string& name)const LNOEXCEPT
		{
			switch (t)
			{
			case ResourceType::Texture:
				return m_TexturePool.find(name.c_str()) != m_TexturePool.end();
			case ResourceType::Sprite:
				return m_SpritePool.find(name.c_str()) != m_SpritePool.end();
			case ResourceType::Animation:
				return m_AnimationPool.find(name.c_str()) != m_AnimationPool.end();
			case ResourceType::Music:
				return m_MusicPool.find(name.c_str()) != m_MusicPool.end();
			case ResourceType::SoundEffect:
				return m_SoundSpritePool.find(name.c_str()) != m_SoundSpritePool.end();
			case ResourceType::Particle:
				return m_ParticlePool.find(name.c_str()) != m_ParticlePool.end();
			case ResourceType::SpriteFont:
				return m_SpriteFontPool.find(name.c_str()) != m_SpriteFontPool.end();
			case ResourceType::TrueTypeFont:
				return m_TTFFontPool.find(name.c_str()) != m_TTFFontPool.end();
			case ResourceType::FX:
				return m_FXPool.find(name.c_str()) != m_FXPool.end();
			default:
				break;
			}

			return false;
		}

		/// @brief 导出资源表
		/// @note 在L的堆栈上放置一个table用以存放ResourceType中的资源名称
		/// @warning 注意t可以是非法枚举量
		void ExportResourceList(lua_State* L, ResourceType t)const LNOEXCEPT;

		/// @brief 装载纹理
		/// @param name 名称
		/// @param path 路径
		/// @param mipmaps 纹理链
		bool LoadTexture(const char* name, const std::wstring& path, bool mipmaps = true)LNOEXCEPT;

		bool LoadModel(const char* name, const std::wstring& path)LNOEXCEPT;

		bool LoadModel(const char* name, const char* path)LNOEXCEPT;

		/// @brief 装载纹理（UTF-8）
		LNOINLINE bool LoadTexture(const char* name, const char* path, bool mipmaps = true)LNOEXCEPT;

		/// @brief 装载图像
		LNOINLINE bool LoadImage(const char* name, const char* texname,
			double x, double y, double w, double h, double a, double b, bool rect = false)LNOEXCEPT;

		LNOINLINE bool LoadAnimation(const char* name, const char* texname,
			double x, double y, double w, double h, int n, int m, int intv, double a, double b, bool rect = false)LNOEXCEPT;

		/// @brief 装载背景音乐
		bool LoadMusic(const char* name, const std::wstring& path, double start, double end)LNOEXCEPT;

		LNOINLINE bool LoadMusic(const char* name, const char* path, double start, double end)LNOEXCEPT;

		/// @brief 装载音效
		bool LoadSound(const char* name, const std::wstring& path)LNOEXCEPT;

		LNOINLINE bool LoadSound(const char* name, const char* path)LNOEXCEPT;

		/// @brief 装载粒子
		bool LoadParticle(const char* name, const std::wstring& path, const char* img_name, double a, double b, bool rect = false)LNOEXCEPT;

		bool LoadParticle(const char* name, const ResParticle::ParticleInfo& info, const char* img_name, double a, double b, bool rect = false)LNOEXCEPT;

		LNOINLINE bool LoadParticle(const char* name, const char* path, const char* img_name, double a, double b, bool rect = false)LNOEXCEPT;

		/// @brief 装载纹理字体(HGE)
		bool LoadSpriteFont(const char* name, const std::wstring& path, bool mipmaps = true)LNOEXCEPT;

		/// @brief 装载纹理字体(fancy2d)
		bool LoadSpriteFont(const char* name, const std::wstring& path, const std::wstring& tex_path, bool mipmaps = true)LNOEXCEPT;

		LNOINLINE bool LoadSpriteFont(const char* name, const char* path, bool mipmaps = true)LNOEXCEPT;

		LNOINLINE bool LoadSpriteFont(const char* name, const char* path, const char* tex_path, bool mipmaps = true)LNOEXCEPT;

		/// @brief 装载TTF字体
		bool LoadTTFFont(const char* name, const std::wstring& path, float width, float height, float bboxwidth, float bboxheight)LNOEXCEPT;

		LNOINLINE bool LoadTTFFont(const char* name, const char* path, float width, float height, float bboxwidth, float bboxheight)LNOEXCEPT;

		bool LoadTTFFont(const char* name, fcyStream* stream, float width, float height, float bboxwidth, float bboxheight)LNOEXCEPT;

		/// @brief 装载FX
		bool LoadFX(const char* name, const std::wstring& path)LNOEXCEPT;

		LNOINLINE bool LoadFX(const char* name, const char* path)LNOEXCEPT;

		/// @brief 构造RenderTarget
		LNOINLINE bool CreateRenderTarget(const char* name)LNOEXCEPT;

		/// @brief 获取纹理
		fcyRefPointer<ResTexture> GetTexture(const char* name)LNOEXCEPT
		{
			auto i = m_TexturePool.find(name);
			if (i == m_TexturePool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief 获取精灵
		fcyRefPointer<ResSprite> GetSprite(const char* name)LNOEXCEPT
		{
			auto i = m_SpritePool.find(name);
			if (i == m_SpritePool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief 获取动画
		fcyRefPointer<ResAnimation> GetAnimation(const char* name)LNOEXCEPT
		{
			auto i = m_AnimationPool.find(name);
			if (i == m_AnimationPool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief 获取背景音
		fcyRefPointer<ResMusic> GetMusic(const char* name)LNOEXCEPT
		{
			auto i = m_MusicPool.find(name);
			if (i == m_MusicPool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief 获取音效
		fcyRefPointer<ResSound> GetSound(const char* name)LNOEXCEPT
		{
			auto i = m_SoundSpritePool.find(name);
			if (i == m_SoundSpritePool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief 获取粒子系统
		fcyRefPointer<ResParticle> GetParticle(const char* name)LNOEXCEPT
		{
			auto i = m_ParticlePool.find(name);
			if (i == m_ParticlePool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief 获取纹理字体
		fcyRefPointer<ResFont> GetSpriteFont(const char* name)LNOEXCEPT
		{
			auto i = m_SpriteFontPool.find(name);
			if (i == m_SpriteFontPool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief 获取TTF字体
		fcyRefPointer<ResFont> GetTTFFont(const char* name)LNOEXCEPT
		{
			auto i = m_TTFFontPool.find(name);
			if (i == m_TTFFontPool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief 获取FX
		fcyRefPointer<ResFX> GetFX(const char* name)LNOEXCEPT
		{
			auto i = m_FXPool.find(name);
			if (i == m_FXPool.end())
				return nullptr;
			else
				return i->second;
		}
	private:
		ResourcePool& operator=(const ResourcePool&);
		ResourcePool(const ResourcePool&);
	public:
		ResourcePool(ResourceMgr* mgr, ResourcePoolType t)
			: m_pMgr(mgr), m_iType(t) {}
		};
	
	// 资源管理器
	class ResourceMgr
	{
	private:
		float m_GlobalImageScaleFactor = 1.0f;
		float m_GlobalSoundEffectVolume = 1.0f;
		float m_GlobalMusicVolume = 1.0f;

		ResourcePoolType m_ActivedPool = ResourcePoolType::Global;
		ResourcePool m_GlobalResourcePool;
		ResourcePool m_StageResourcePool;
	public:
		float GetGlobalImageScaleFactor()const LNOEXCEPT { return m_GlobalImageScaleFactor; }
		void SetGlobalImageScaleFactor(float s)LNOEXCEPT { m_GlobalImageScaleFactor = s; }
		float GetGlobalSoundEffectVolume()const LNOEXCEPT { return m_GlobalSoundEffectVolume; }
		void SetGlobalSoundEffectVolume(float s)LNOEXCEPT { m_GlobalSoundEffectVolume = s; }
		float GetGlobalMusicVolume()const LNOEXCEPT { return m_GlobalMusicVolume; }
		void SetGlobalMusicVolume(float s)LNOEXCEPT { m_GlobalMusicVolume = s; }

		/// @brief 获得当前激活的资源池类型
		ResourcePoolType GetActivedPoolType()LNOEXCEPT
		{
			return m_ActivedPool;
		}

		/// @brief 设置当前激活的资源池类型
		void SetActivedPoolType(ResourcePoolType t)LNOEXCEPT
		{
			m_ActivedPool = t;
		}

		/// @brief 获得当前激活的资源池
		ResourcePool* GetActivedPool()LNOEXCEPT
		{
			return GetResourcePool(m_ActivedPool);
		}

		/// @brief 获得资源池
		ResourcePool* GetResourcePool(ResourcePoolType t)LNOEXCEPT
		{
			switch (t)
			{
			case ResourcePoolType::Global:
				return &m_GlobalResourcePool;
			case ResourcePoolType::Stage:
				return &m_StageResourcePool;
			default:
				return nullptr;
			}
		}

		/// @brief 卸载所有资源并重置状态
		void ClearAllResource()LNOEXCEPT;

		/// @brief 装载文件
		/// @param[in] path 路径
		/// @param[out] outBuf 输出缓冲
		LNOINLINE bool LoadFile(const wchar_t* path, fcyRefPointer<fcyMemStream>& outBuf, const wchar_t* pack = nullptr) LNOEXCEPT;

		/// @brief 装载文件（UTF8）
		/// @param[in] path 路径
		/// @param[out] outBuf 输出缓冲
		LNOINLINE bool LoadFile(const char* path, fcyRefPointer<fcyMemStream>& outBuf, const char* pack = nullptr) LNOEXCEPT;

		/// @brief 解压资源文件
		/// @param[in] path 路径
		/// @param[in] target 目的地
		bool ExtractRes(const wchar_t* path, const wchar_t* target)LNOEXCEPT;

		//查找文件
		LNOINLINE bool FindFiles(lua_State* L, const char* path, const char* ext, const char* packname)LNOEXCEPT;

		/// @brief 解压资源文件（UTF8）
		/// @param[in] path 路径
		/// @param[in] target 目的地
		LNOINLINE bool ExtractRes(const char* path, const char* target)LNOEXCEPT;

		/// @brief 寻找纹理
		fcyRefPointer<ResTexture> FindTexture(const char* texname)LNOEXCEPT
		{
			fcyRefPointer<ResTexture> tRet;
			if (!(tRet = m_StageResourcePool.GetTexture(texname)))
				tRet = m_GlobalResourcePool.GetTexture(texname);
			return tRet;
		}

		/// @brief 获取纹理大小
		bool GetTextureSize(const char* texname, fcyVec2& out)LNOEXCEPT
		{
			fcyRefPointer<ResTexture> tRet = FindTexture(texname);
			if (!tRet)
				return false;
			out.x = static_cast<float>(tRet->GetTexture()->GetWidth());
			out.y = static_cast<float>(tRet->GetTexture()->GetHeight());
			return true;
		}

		// 缓存字形
		void CacheTTFFontString(const char* ttf, const char* text) {
			try
			{
				std::wstring t = fcyStringHelper::MultiByteToWideChar(text, CP_UTF8);
				fcyRefPointer<ResFont> f = FindTTFFont(ttf);
				if (f) f->GetFontProvider()->CacheString(t.c_str());
				else LWARNING("ResourceMgr: 缓存字形贴图时未找到指定字体");
			}
			catch (const std::bad_alloc&)
			{
				LERROR("ResourceMgr: 转换字符编码时无法分配内存");
			}
		}

		/// @brief 寻找精灵
		fcyRefPointer<ResSprite> FindSprite(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResSprite> tRet;
			if (!(tRet = m_StageResourcePool.GetSprite(name)))
				tRet = m_GlobalResourcePool.GetSprite(name);
			return tRet;
		}

		/// @brief 寻找动画
		fcyRefPointer<ResAnimation> FindAnimation(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResAnimation> tRet;
			if (!(tRet = m_StageResourcePool.GetAnimation(name)))
				tRet = m_GlobalResourcePool.GetAnimation(name);
			return tRet;
		}

		/// @brief 寻找音乐
		fcyRefPointer<ResMusic> FindMusic(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResMusic> tRet;
			if (!(tRet = m_StageResourcePool.GetMusic(name)))
				tRet = m_GlobalResourcePool.GetMusic(name);
			return tRet;
		}

		/// @brief 寻找音效
		fcyRefPointer<ResSound> FindSound(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResSound> tRet;
			if (!(tRet = m_StageResourcePool.GetSound(name)))
				tRet = m_GlobalResourcePool.GetSound(name);
			return tRet;
		}

		/// @brief 寻找粒子
		fcyRefPointer<ResParticle> FindParticle(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResParticle> tRet;
			if (!(tRet = m_StageResourcePool.GetParticle(name)))
				tRet = m_GlobalResourcePool.GetParticle(name);
			return tRet;
		}

		/// @brief 寻找字体
		fcyRefPointer<ResFont> FindSpriteFont(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResFont> tRet;
			if (!(tRet = m_StageResourcePool.GetSpriteFont(name)))
				tRet = m_GlobalResourcePool.GetSpriteFont(name);
			return tRet;
		}

		/// @brief 寻找字体
		fcyRefPointer<ResFont> FindTTFFont(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResFont> tRet;
			if (!(tRet = m_StageResourcePool.GetTTFFont(name)))
				tRet = m_GlobalResourcePool.GetTTFFont(name);
			return tRet;
		}

		/// @brief 寻找shader
		fcyRefPointer<ResFX> FindFX(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResFX> tRet;
			if (!(tRet = m_StageResourcePool.GetFX(name)))
				tRet = m_GlobalResourcePool.GetFX(name);
			return tRet;
		}
	public:
		ResourceMgr();
	};
}
