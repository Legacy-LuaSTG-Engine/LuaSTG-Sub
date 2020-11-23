/// @file AppFrame.h
/// @brief 定义应用程序框架
#pragma once
#include "Global.h"
#include "SplashWindow.h"
#include "ResourceMgr.h"
#include "GameObjectPool.h"
#include "UnicodeStringEncoding.h"

#include "ESC.h"

#if (defined LDEVVERSION) || (defined LDEBUG)
#include "RemoteDebuggerClient.h"
#endif

#include "E2DFileManager.hpp"

namespace LuaSTGPlus
{
	/// @brief 应用程序状态
	enum class AppStatus
	{
		NotInitialized,
		Initializing,
		Initialized,
		Running,
		Aborted,
		Destroyed
	};

	/// @brief 当前激活的渲染器
	enum class GraphicsType
	{
		Graph2D,
		Graph3D
	};

	/// @brief 应用程序框架
	class AppFrame : public f2dEngineEventListener
	{
	public:
		static LNOINLINE AppFrame& GetInstance();
	private:
		class GdiPlusScope
		{
		private:
			ULONG_PTR m_gdiplusToken;
		public:
			GdiPlusScope()
			{
				Gdiplus::GdiplusStartupInput StartupInput;
				GdiplusStartup(&m_gdiplusToken, &StartupInput, NULL);
			}
			~GdiPlusScope()
			{
				Gdiplus::GdiplusShutdown(m_gdiplusToken);
			}
		};
	private:
		AppStatus m_iStatus = AppStatus::NotInitialized;
		
#if (defined LDEVVERSION) || (defined LDEBUG)
		// 远端调试器
		std::unique_ptr<RemoteDebuggerClient> m_DebuggerClient;

		// 性能计数器
		float m_UpdateTimer = 0.f;
		float m_RenderTimer = 0.f;

		float m_PerformanceUpdateTimer = 0.f;  // 记录性能参数的累计采样时间
		float m_PerformanceUpdateCounter = 0.f;  // 记录采样次数
		float m_FPSTotal = 0.f;  // 记录在采样时间内累计的FPS
		float m_ObjectTotal = 0.f;  // 记录在采样时间内累计的对象数
		float m_UpdateTimerTotal = 0.f;  // 记录在采样时间内累计的更新时间
		float m_RenderTimerTotal = 0.f;  // 记录在采样时间内累计的渲染时间
#endif
		
		// 载入窗口
		GdiPlusScope m_GdiScope;
		SplashWindow m_SplashWindow;
		
		// 资源管理器
		Eyes2D::IO::FileManager m_FileManager;
		ResourceMgr m_ResourceMgr;
		
		// 对象池
		std::unique_ptr<GameObjectPool> m_GameObjectPool;
		
		// Lua虚拟机
		lua_State* L = nullptr;
		
		// 选项与值
		bool m_bSplashWindowEnabled = false;
		bool m_OptionWindowed = true;
		fuInt m_OptionFPSLimit = 60;
		bool m_OptionVsync = false;
		fcyVec2 m_OptionResolution = fcyVec2(640.f, 480.f);
		bool m_OptionSplash = true;
		std::wstring m_OptionTitle = L"LuaSTG";
		fDouble m_fFPS = 0.;
		
		// 引擎
		fcyRefPointer<f2dEngine> m_pEngine;
		f2dWindow* m_pMainWindow = nullptr;
		f2dRenderer* m_pRenderer = nullptr;
		f2dRenderDevice* m_pRenderDev = nullptr;
		f2dSoundSys* m_pSoundSys = nullptr;
		f2dInputSys* m_pInputSys = nullptr;
		
		//渲染状态
		GraphicsType m_GraphType = GraphicsType::Graph2D;
		bool m_bRenderStarted = false;
		bool m_bLockBlendMode = false;
		BlendMode m_Graph2DLastBlendMode;//2D
		f2dBlendState m_Graph2DBlendState;//2D
		F2DGRAPH2DBLENDTYPE m_Graph2DColorBlendState;//2D
		BlendMode m_Graph3DLastBlendMode;//3D
		f2dBlendState m_Graph3DBlendState;//3D
		
		//渲染器
		fcyRefPointer<f2dGeometryRenderer> m_GRenderer;//2D
		fcyRefPointer<f2dFontRenderer> m_FontRenderer;//2D
		fcyRefPointer<f2dGraphics2D> m_Graph2D;//2D
		fcyRefPointer<f2dGraphics3D> m_Graph3D;//3D
		
		// PostEffect控制
		bool m_bPostEffectCaptureStarted = false;
		fcyRefPointer<f2dTexture2D> m_PostEffectBuffer;//全局临时RenderTarget
		std::vector<fcyRefPointer<f2dTexture2D>> m_stRenderTargetStack;// RenderTarget控制
		
		//输入设备
		fcyRefPointer<f2dInputMouse> m_Mouse;
		fcyRefPointer<f2dInputKeyboard> m_Keyboard;
		fcyRefPointer<f2dInputKeyboard> m_Keyboard2;
		fcyRefPointer<f2dInputJoystick> m_Joystick[2];
		fCharW m_LastChar;
		fInt m_LastKey;
		fBool m_KeyStateMap[256];
		fcyVec2 m_MousePosition;
		void resetKeyStatus()LNOEXCEPT {
			m_LastChar = 0;
			m_LastKey = 0;
			::memset(m_KeyStateMap, 0, sizeof(m_KeyStateMap));
		}
	private:
		void updateGraph2DBlendMode(BlendMode m);
		void updateGraph3DBlendMode(BlendMode m);
#if (defined LDEVVERSION) || (defined LDEBUG)
	public: // 调试用接口
		void SendResourceLoadedHint(ResourceType Type, ResourcePoolType PoolType, const char* Name, const wchar_t* Path, float LoadingTime)
		{
			if (m_DebuggerClient)
				m_DebuggerClient->SendResourceLoadedHint(Type, PoolType, Name, Path, LoadingTime);
		}
		void SendResourceRemovedHint(ResourceType Type, ResourcePoolType PoolType, const char* Name)
		{
			if (m_DebuggerClient)
				m_DebuggerClient->SendResourceRemovedHint(Type, PoolType, Name);
		}
		void SendResourceClearedHint(ResourcePoolType PoolType)
		{
			if (m_DebuggerClient)
				m_DebuggerClient->SendResourceClearedHint(PoolType);
		}
#endif
	public: // 脚本调用接口，含义参见API文档
		LNOINLINE void ShowSplashWindow(const char* imgPath = nullptr)LNOEXCEPT;  // UTF8编码
		void SetWindowed(bool v)LNOEXCEPT;
		void SetFPS(fuInt v)LNOEXCEPT;
		void SetVsync(bool v)LNOEXCEPT;
		void SetResolution(fuInt width, fuInt height)LNOEXCEPT;
		void SetSplash(bool v)LNOEXCEPT;
		LNOINLINE void SetTitle(const char* v)LNOEXCEPT;  // UTF8编码
	public:
		/// @brief 使用新的视频参数更新显示模式
		/// @note 若切换失败则进行回滚
		LNOINLINE bool ChangeVideoMode(int width, int height, bool windowed, bool vsync)LNOEXCEPT;

		LNOINLINE bool UpdateVideoMode()LNOEXCEPT;
		/// @brief 获取当前的FPS
		double GetFPS()LNOEXCEPT { return m_fFPS; }

		/// @brief 执行资源包中的文件
		/// @note 该函数为脚本系统使用
		LNOINLINE void LoadScript(const char* path,const char *packname)LNOEXCEPT;

		//读取资源包中的文本文件
		//也能读取其他类型的文件，但是会得到无意义的结果
		LNOINLINE int LoadTextFile(lua_State* L, const char* path, const char *packname)LNOEXCEPT;

		//检查按键是否按下
		fBool GetKeyState(int VKCode)LNOEXCEPT;

		//检查键盘按键是否按下，Dinput KeyCode
		fBool GetKeyboardState(DWORD VKCode)LNOEXCEPT;

		//检查键盘按键是否按下，使用的是GetAsyncKeyState
		//和GetKeyboardState不同，这个检测的不是按下过的，而是现在被按住的键
		bool GetAsyncKeyState(int VKCode)LNOEXCEPT;

		/// @brief 获得最后一次字符输入（UTF-8）
		LNOINLINE int GetLastChar(lua_State* L)LNOEXCEPT;

		/// @brief 获得最后一次按键输入
		int GetLastKey()LNOEXCEPT { return m_LastKey; }

		/// @brief 获取鼠标位置（以窗口左下角为原点）
		fcyVec2 GetMousePosition()LNOEXCEPT { return m_MousePosition; }

		/// @brief 获取鼠标滚轮增量
		fInt GetMouseWheelDelta()LNOEXCEPT { return m_Mouse->GetOffsetZ(); }

		/// @brief 检查鼠标是否按下
		fBool GetMouseState(int button)LNOEXCEPT
		{
			switch (button) {
			case 0:
				return m_Mouse->IsLeftBtnDown();
			case 1:
				return m_Mouse->IsMiddleBtnDown();
			case 2:
				return m_Mouse->IsRightBtnDown();
			default:
				break;
			}
			if (button >= 3 && button <= 7) {
				return m_Mouse->IsAdditionBtnDown(button - 3);//对齐额外键索引（不包含左中右键）
			}
			return false;
		}
	public:  // 渲染器接口
		/// @brief 通知开始渲染
		bool BeginScene()LNOEXCEPT;

		/// @brief 通知结束渲染
		bool EndScene()LNOEXCEPT;

		/// @brief 清屏
		void ClearScreen(const fcyColor& c)LNOEXCEPT
		{
			m_pRenderDev->Clear(c);
		}

		/// @brief 设置视口
		bool SetViewport(double left, double right, double bottom, double top)LNOEXCEPT
		{
			if (FCYFAILED(m_pRenderDev->SetViewport(fcyRect(
				static_cast<float>((int)left),
				static_cast<float>((int)m_pRenderDev->GetBufferHeight() - (int)top),
				static_cast<float>((int)right),
				static_cast<float>((int)m_pRenderDev->GetBufferHeight() - (int)bottom)
			))))
			{
				LERROR("设置视口(left: %lf, right: %lf, bottom: %lf, top: %lf)失败", left, right, bottom, top);
				return false;
			}
			return true;
		}

		/// @brief 设置正投影矩阵。
		void SetOrtho(float left, float right, float bottom, float top, float znear = 8.0f, float zfar = 2048.0f)LNOEXCEPT
		{
			if (m_GraphType == GraphicsType::Graph2D)
			{
				// luastg的lua部分已经做了坐标修正
				// m_Graph2D->SetWorldTransform(fcyMatrix4::GetTranslateMatrix(fcyVec3(-0.5f, -0.5f, 0.f)));
				m_Graph2D->SetWorldTransform(fcyMatrix4::GetIdentity());
				//m_Graph2D->SetViewTransform(fcyMatrix4::GetIdentity());
				//允许正交投影下下可以饶原点3D旋转图片精灵
				m_Graph2D->SetViewTransform(fcyMatrix4::GetTranslateMatrix(fcyVec3(0.0f, 0.0f, znear + (zfar - znear) / 2.0f)));
				m_Graph2D->SetProjTransform(fcyMatrix4::GetOrthoOffCenterLH(left, right, bottom, top, znear, zfar));
			}
		}
		
		/// @brief 设置透视投影矩阵
		void SetPerspective(float eyeX, float eyeY, float eyeZ, float atX, float atY, float atZ, 
			float upX, float upY, float upZ, float fovy, float aspect, float zn, float zf)LNOEXCEPT
		{
			if (m_GraphType == GraphicsType::Graph2D)
			{
				m_Graph2D->SetWorldTransform(fcyMatrix4::GetIdentity());
				m_Graph2D->SetViewTransform(fcyMatrix4::GetLookAtLH(fcyVec3(eyeX, eyeY, eyeZ), fcyVec3(atX, atY, atZ), fcyVec3(upX, upY, upZ)));
				m_Graph2D->SetProjTransform(fcyMatrix4::GetPespctiveLH(aspect, fovy, zn, zf));
			}
		}

		/// @brief 设置雾值
		/// @note 扩展方法，视情况移除。
		void SetFog(float start, float end, fcyColor color);

		// 开启或关闭zbuffer
		void SetZBufferEnable(bool enable)LNOEXCEPT
		{
			if (m_GraphType == GraphicsType::Graph2D)
			{
				m_Graph2D->Flush();
				m_pRenderDev->SetZBufferEnable(enable);
			}
		}

		// 用指定的值清空zbuffer
		void ClearZBuffer(float z)LNOEXCEPT
		{
			if (m_GraphType == GraphicsType::Graph2D)
			{
				m_pRenderDev->ClearZBuffer(z);
			}
		}

		/// @brief 渲染图像
		bool Render(ResSprite* p, float x, float y, float rot = 0, float hscale = 1, float vscale = 1, float z = 0.5)LNOEXCEPT
		{
			LASSERT(p);
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("Render: 只有2D渲染器可以执行该方法");
				return false;
			}

			// 设置混合
			updateGraph2DBlendMode(p->GetBlendMode());

			// 渲染
			f2dSprite* pSprite = p->GetSprite();
			pSprite->SetZ(z);
			pSprite->Draw2(m_Graph2D, fcyVec2(x, y), fcyVec2(hscale, vscale), rot, false);
			return true;
		}

		/// @brief 渲染动画
		bool Render(ResAnimation* p, int ani_timer, float x, float y, float rot = 0, float hscale = 1, float vscale = 1)LNOEXCEPT
		{
			LASSERT(p);
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("Render: 只有2D渲染器可以执行该方法");
				return false;
			}

			// 设置混合
			updateGraph2DBlendMode(p->GetBlendMode());

			// 渲染
			f2dSprite* pSprite = p->GetSprite(((fuInt)ani_timer / p->GetInterval()) % p->GetCount());
			pSprite->Draw2(m_Graph2D, fcyVec2(x, y), fcyVec2(hscale, vscale), rot, false);
			return true;
		}

		/// @brief 渲染粒子
		bool Render(ResParticle::ParticlePool* p, float hscale = 1, float vscale = 1)LNOEXCEPT
		{
			LASSERT(p);
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("Render: 只有2D渲染器可以执行该方法");
				return false;
			}

			// 设置混合
			updateGraph2DBlendMode(p->GetBlendMode());

			// 渲染
			p->Render(m_Graph2D, hscale, vscale);
			return true;
		}

		/// @brief 渲染图像
		bool Render(const char* name, float x, float y, float rot = 0, float hscale = 1, float vscale = 1, float z = 0.5)LNOEXCEPT
		{
			fcyRefPointer<ResSprite> p = m_ResourceMgr.FindSprite(name);
			if (!p)
			{
				LERROR("Render: 找不到图像资源'%m'", name);
				return false;
			}
			return Render(p, x, y, rot, hscale, vscale, z);
		}

		/// @brief 渲染动画
		bool RenderAnimation(const char* name, int timer, float x, float y, float rot = 0, float hscale = 1, float vscale = 1)LNOEXCEPT
		{
			fcyRefPointer<ResAnimation> p = m_ResourceMgr.FindAnimation(name);
			if (!p)
			{
				LERROR("Render: 找不到动画资源'%m'", name);
				return false;
			}
			return Render(p, timer, x, y, rot, hscale, vscale);
		}

		/// @brief 渲染图像
		bool RenderRect(const char* name, float x1, float y1, float x2, float y2)LNOEXCEPT
		{
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("RenderRect: 只有2D渲染器可以执行该方法");
				return false;
			}

			fcyRefPointer<ResSprite> p = m_ResourceMgr.FindSprite(name);
			if (!p)
			{
				LERROR("RenderRect: 找不到图像资源'%m'", name);
				return false;
			}

			// 设置混合
			updateGraph2DBlendMode(p->GetBlendMode());

			// 渲染
			f2dSprite* pSprite = p->GetSprite();
			pSprite->SetZ(0.5f);
			pSprite->Draw(m_Graph2D, fcyRect(x1, y1, x2, y2), false);
			return true;
		}

		/// @brief 渲染图像
		bool Render4V(const char* name, float x1, float y1, float z1, float x2, float y2, float z2, 
			float x3, float y3, float z3, float x4, float y4, float z4)LNOEXCEPT
		{
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("Render4V: 只有2D渲染器可以执行该方法");
				return false;
			}

			fcyRefPointer<ResSprite> p = m_ResourceMgr.FindSprite(name);
			if (!p)
			{
				LERROR("Render4V: 找不到图像资源'%m'", name);
				return false;
			}
			
			// 设置混合
			updateGraph2DBlendMode(p->GetBlendMode());

			f2dSprite* pSprite = p->GetSprite();
			pSprite->SetZ(0.5f);
			pSprite->Draw(m_Graph2D, fcyVec3(x1, y1, z1), fcyVec3(x2, y2, z2), fcyVec3(x3, y3, z3), fcyVec3(x4, y4, z4), false);
			return true;
		}

		/// @brief 渲染纹理
		bool RenderTexture(ResTexture* tex, BlendMode blend, const f2dGraphics2DVertex vertex[])LNOEXCEPT
		{
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("RenderTexture: 只有2D渲染器可以执行该方法");
				return false;
			}
			
			// 设置混合
			updateGraph2DBlendMode(blend);

			// 复制坐标
			f2dGraphics2DVertex tVertex[4];
			memcpy(tVertex, vertex, sizeof(tVertex));

			// 修正UV到[0,1]区间
			for (int i = 0; i < 4; ++i)
			{
				tVertex[i].u /= (float)tex->GetTexture()->GetWidth();
				tVertex[i].v /= (float)tex->GetTexture()->GetHeight();
			}

			m_Graph2D->DrawQuad(tex->GetTexture(), tVertex, false);
			return true;
		}

		/// @brief 渲染纹理
		bool RenderTexture(const char* name, BlendMode blend, f2dGraphics2DVertex vertex[])LNOEXCEPT
		{
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("RenderTexture: 只有2D渲染器可以执行该方法");
				return false;
			}

			fcyRefPointer<ResTexture> p = m_ResourceMgr.FindTexture(name);
			if (!p)
			{
				LERROR("RenderTexture: 找不到纹理资源'%m'", name);
				return false;
			}

			// 设置混合
			updateGraph2DBlendMode(blend);

			// 修正UV到[0,1]区间
			const auto w = (float)p->GetTexture()->GetWidth(), h = (float)p->GetTexture()->GetHeight();
			for (int i = 0; i < 4; ++i)
			{
				vertex[i].u /= w;
				vertex[i].v /= h;
			}

			m_Graph2D->DrawQuad(p->GetTexture(), vertex, false);
			return true;
		}

		// 渲染纹理，多顶点，不修正uv坐标
		bool RenderTexture(const char* name, BlendMode blend, int vcount, const f2dGraphics2DVertex vertex[], int icount, const unsigned short indexs[])LNOEXCEPT
		{
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("RenderTexture: 只有2D渲染器可以执行该方法");
				return false;
			}

			fcyRefPointer<ResTexture> p = m_ResourceMgr.FindTexture(name);
			if (!p)
			{
				LERROR("RenderTexture: 找不到纹理资源'%m'", name);
				return false;
			}

			// 设置混合
			updateGraph2DBlendMode(blend);

			m_Graph2D->DrawRaw(p->GetTexture(), vcount, icount, vertex, indexs, false);
			return true;
		}

		// 渲染模型
		bool RenderModel(const char* name, float x, float y, float z, float sx, float sy, float sz,float rx,float ry ,float rz)LNOEXCEPT
		{
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("Render4V: 只有2D渲染器可以执行该方法");
				return false;
			}
			void RenderObj(std::string id);

			fcyMatrix4 f0 = m_Graph2D->GetWorldTransform();
			fcyMatrix4 f1 = fcyMatrix4::GetTranslateMatrix(fcyVec3(x, y, z));
			f1 = fcyMatrix4::GetScaleMatrix(fcyVec3(sx, sy, sz)) * f1;
			if (rx || ry || rz){
				f1 = fcyMatrix4::GetRotationYawPitchRoll(rx,ry,rz)*f1;
			}

			m_Graph2D->SetWorldTransform(f1);
			RenderObj(name);
			m_Graph2D->SetWorldTransform(f0);

			return true;
		}

		/// @brief 渲染文字
		bool RenderText(ResFont* p, wchar_t* strBuf, fcyRect rect, fcyVec2 scale, ResFont::FontAlignHorizontal halign, ResFont::FontAlignVertical valign, bool bWordBreak)LNOEXCEPT;

		fcyVec2 CalcuTextSize(ResFont* p, const wchar_t* strBuf, fcyVec2 scale)LNOEXCEPT;

		LNOINLINE bool RenderText(const char* name, const char* str, float x, float y, float scale, ResFont::FontAlignHorizontal halign, ResFont::FontAlignVertical valign)LNOEXCEPT;

		LNOINLINE bool RenderTTF(const char* name, const char* str, float left, float right, float bottom, float top, float scale, int format, fcyColor c)LNOEXCEPT;

		LNOINLINE void SnapShot(const char* path)LNOEXCEPT;

		LNOINLINE void SaveTexture(f2dTexture2D* Tex, const char* path)LNOEXCEPT;

		bool CheckRenderTargetInUse(fcyRefPointer<f2dTexture2D> rt)LNOEXCEPT;

		bool CheckRenderTargetInUse(ResTexture* rt)LNOEXCEPT;

		bool PushRenderTarget(fcyRefPointer<f2dTexture2D> rt)LNOEXCEPT;

		LNOINLINE bool PushRenderTarget(ResTexture* rt)LNOEXCEPT;

		LNOINLINE bool PopRenderTarget()LNOEXCEPT;

		bool PostEffect(fcyRefPointer<f2dTexture2D> rt, ResFX* shader, BlendMode blend)LNOEXCEPT;

		LNOINLINE bool PostEffect(ResTexture* rt, ResFX* shader, BlendMode blend)LNOEXCEPT;

		LNOINLINE bool PostEffectCapture()LNOEXCEPT;
		
		LNOINLINE bool PostEffectApply(ResFX* shader, BlendMode blend)LNOEXCEPT;
	public: // 拓展方法
		// 渲染扇形，通过纹理+uv范围渲染
		bool RenderSector(const char* name, fcyRect uv, bool tran, BlendMode blend, fcyColor color1, fcyColor color2,
			fcyVec2 pos, float rot, float exp, float r1, float r2, int div);
		
		// 渲染环，通过纹理+uv范围渲染
		bool RenderAnnulus(const char* name, fcyRect uv, bool tran, BlendMode blend, fcyColor color1, fcyColor color2,
			fcyVec2 pos, float rot, float r1, float r2, int div, int rep);
		
	public: // 文字渲染器包装
		bool FontRenderer_SetFontProvider(const char* name);
		void FontRenderer_SetFlipType(const F2DSPRITEFLIP t);
		void FontRenderer_SetScale(const fcyVec2& s);
		fcyRect FontRenderer_MeasureString(const char* str, bool strict = true);
		float FontRenderer_MeasureStringWidth(const char* str);
		bool FontRenderer_DrawTextW2(const char* str, fcyVec2& pos, const float z, const BlendMode blend, const fcyColor& color);
	public:
		ResourceMgr& GetResourceMgr()LNOEXCEPT { return m_ResourceMgr; }
		Eyes2D::IO::FileManager& GetFileManager() noexcept { return m_FileManager; }
		GameObjectPool& GetGameObjectPool()LNOEXCEPT{ return *m_GameObjectPool.get(); }
		f2dEngine* GetEngine()LNOEXCEPT { return m_pEngine; }
		f2dWindow* GetWindow()LNOEXCEPT { return m_pMainWindow; }
		f2dRenderer* GetRenderer()LNOEXCEPT { return m_pRenderer; }
		f2dRenderDevice* GetRenderDev()LNOEXCEPT { return m_pRenderDev; }
		f2dSoundSys* GetSoundSys()LNOEXCEPT { return m_pSoundSys; }
		fcyRefPointer<f2dGeometryRenderer> GetGeometryRenderer()LNOEXCEPT { return m_GRenderer; }
		GraphicsType GetGraphicsType() { return m_GraphType; }
		fcyRefPointer<f2dGraphics3D> GetGraphics3D()LNOEXCEPT { return m_Graph3D; }
		fcyRefPointer<f2dGraphics2D> GetGraphics2D()LNOEXCEPT { return m_Graph2D; }
		lua_State* GetLuaEngine() { return L; }
		
		/// @brief 初始化框架
		/// @note 该函数必须在一开始被调用，且仅能调用一次
		/// @return 失败返回false
		bool Init()LNOEXCEPT;
		/// @brief 终止框架并回收资源
		/// @note 该函数可以由框架自行调用，且仅能调用一次
		void Shutdown()LNOEXCEPT;

		/// @brief 执行框架，进入游戏循环
		void Run()LNOEXCEPT;

		/// @brief 保护模式执行脚本
		/// @note 该函数仅限框架调用，为主逻辑最外层调用。若脚本运行时发生错误，该函数负责截获错误发出错误消息。
		bool SafeCallScript(const char* source, size_t len, const char* desc)LNOEXCEPT;

		/// @brief 保护模式调用全局函数
		/// @note 该函数仅限框架调用，为主逻辑最外层调用。若脚本运行时发生错误，该函数负责截获错误发出错误消息。调用者负责维持栈平衡。
		bool SafeCallGlobalFunction(const char* name, int retc = 0)LNOEXCEPT;
		
		/// @brief 保护模式调用全局函数
		/// @note 该函数仅限框架调用，为主逻辑最外层调用。若脚本运行时发生错误，该函数负责截获错误发出错误消息。调用者负责维持栈平衡。
		bool SafeCallGlobalFunctionB(const char* name, int argc = 0, int retc = 0)LNOEXCEPT;
	protected:  // fancy2d逻辑循环回调
		fBool OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump);
		fBool OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController);
	public:
		AppFrame()LNOEXCEPT;
		~AppFrame()LNOEXCEPT;
	public:
		IExInputControl *m_Input;
	};
}
