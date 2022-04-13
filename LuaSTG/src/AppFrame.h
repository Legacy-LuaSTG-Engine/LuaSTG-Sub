/// @file AppFrame.h
/// @brief 定义应用程序框架
#pragma once
#include "Global.h"
#include "ResourceMgr.h"
#include "GameObjectPool.h"
#include "Common/DirectInput.h"
#include "LuaSTG/Core/Renderer.hpp"
#include "LConfig.h"

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
	private:
		AppStatus m_iStatus = AppStatus::NotInitialized;
		
		// 资源管理器
		ResourceMgr m_ResourceMgr;
		
		// 对象池
		std::unique_ptr<GameObjectPool> m_GameObjectPool;
		
		// Lua虚拟机
		lua_State* L = nullptr;
		
		// 选项与值
		bool m_OptionWindowed = true;
		F2DWINBORDERTYPE m_OptionWindowStyle = F2DWINBORDERTYPE_FIXED;
		fuInt m_OptionFPSLimit = 60;
		bool m_OptionVsync = false;
		fcyVec2 m_OptionResolution = fcyVec2(640.f, 480.f);
		fuInt m_OptionRefreshRateA = 0;
		fuInt m_OptionRefreshRateB = 0;
		std::wstring m_OptionGPU = L"";
		std::wstring m_OptionTitle = L"" LUASTG_INFO;
		bool m_OptionCursor = true;
		fDouble m_fFPS = 0.;
		fFloat m_gSEVol = 1.0f;
		fFloat m_gBGMVol = 1.0f;

		// 引擎
		fcyRefPointer<f2dEngine> m_pEngine;
		f2dWindow* m_pMainWindow = nullptr;
		f2dRenderer* m_pRenderer = nullptr;
		f2dRenderDevice* m_pRenderDev = nullptr;
		f2dSoundSys* m_pSoundSys = nullptr;
		
		//渲染状态
		GraphicsType m_GraphType = GraphicsType::Graph2D;
		bool m_bRenderStarted = false;
		
		//渲染器
		fcyRefPointer<f2dGeometryRenderer> m_GRenderer;//2D
		fcyRefPointer<f2dFontRenderer> m_FontRenderer;//2D
		fcyRefPointer<f2dGraphics2D> m_Graph2D;//2D
		fcyRefPointer<f2dGraphics3D> m_Graph3D;//3D
		LuaSTG::Core::Renderer m_NewRenderer2D;
		
		// PostEffect控制
		std::vector<fcyRefPointer<f2dTexture2D>> m_stRenderTargetStack;// RenderTarget控制
		std::vector<fcyRefPointer<f2dDepthStencilSurface>> m_stDepthStencilStack;// DepthStencil控制
		
		//输入设备
		std::unique_ptr<native::DirectInput> m_DirectInput;
		fInt m_LastKey = 0;
		bool m_InputTextEnable = false;
		bool m_LastInputTextEnable = false;
		std::wstring m_InputTextBuffer;
		fBool m_KeyStateMap[256];
		void resetKeyStatus()LNOEXCEPT;
		
	public:
		/// @brief 保护模式执行脚本
		/// @note 该函数仅限框架调用，为主逻辑最外层调用。若脚本运行时发生错误，该函数负责截获错误发出错误消息。
		bool SafeCallScript(const char* source, size_t len, const char* desc)LNOEXCEPT;
		
		/// @brief 不保护调用全局函数
		/// @note 该函数仅限框架调用，为主逻辑最外层调用
		bool UnsafeCallGlobalFunction(const char* name, int retc = 0)LNOEXCEPT;
		
		/// @brief 保护模式调用全局函数
		/// @note 该函数仅限框架调用，为主逻辑最外层调用。若脚本运行时发生错误，该函数负责截获错误发出错误消息。调用者负责维持栈平衡。
		bool SafeCallGlobalFunction(const char* name, int retc = 0)LNOEXCEPT;
		
		/// @brief 保护模式调用全局函数
		/// @note 该函数仅限框架调用，为主逻辑最外层调用。若脚本运行时发生错误，该函数负责截获错误发出错误消息。调用者负责维持栈平衡。
		bool SafeCallGlobalFunctionB(const char* name, int argc = 0, int retc = 0)LNOEXCEPT;
		
		/// @brief 执行资源包中的文件
		/// @note 该函数为脚本系统使用
		void LoadScript(const char* path,const char *packname)LNOEXCEPT;
		
		bool OnOpenLuaEngine();
		bool OnLoadLaunchScriptAndFiles();
		bool OnLoadMainScriptAndFiles();
		
	public:  // 输入系统接口
		//检查按键是否按下
		fBool GetKeyState(int VKCode)LNOEXCEPT;
		
		/// @brief 获得最后一次按键输入
		int GetLastKey()LNOEXCEPT;
		
		void OnTextInputDeleteFront();
		void OnTextInputDeleteBack();
		void OnTextInputPasting();
		void OnTextInputChar(fCharW c);
		
		void SetTextInputEnable(bool enable = false)LNOEXCEPT;
		
		/// @brief 获得输入的文本
		fcStrW GetTextInput()LNOEXCEPT;
		
		/// @brief 清空输入的文本
		void ClearTextInput()LNOEXCEPT;
		
		/// @brief 获取鼠标位置（以窗口左下角为原点）
		fcyVec2 GetMousePosition(bool no_flip = false)LNOEXCEPT;
		
		/// @brief 获取鼠标滚轮增量
		fInt GetMouseWheelDelta()LNOEXCEPT;
		
		/// @brief 检查鼠标是否按下
		fBool GetMouseState(int button) LNOEXCEPT;
		
		void OpenInput();
		void CloseInput();
		void UpdateInput();

	public: // 脚本调用接口，含义参见API文档
		void SetWindowed(bool v)LNOEXCEPT;
		void SetDefaultWindowStyle(F2DWINBORDERTYPE v) { m_OptionWindowStyle = v; };
		void SetVsync(bool v)LNOEXCEPT;
		void SetResolution(fuInt width, fuInt height, fuInt A = 0, fuInt B = 0)LNOEXCEPT;
		void SetTitle(const char* v)LNOEXCEPT;
		void SetPreferenceGPU(const char* v, bool dGPU_trick = false)LNOEXCEPT;
		void SetSplash(bool v)LNOEXCEPT;
		void SetSEVolume(float v);
		void SetBGMVolume(float v);
		
	public:
		/// @brief 使用新的视频参数更新显示模式
		/// @note 若切换失败则进行回滚
		LNOINLINE bool ChangeVideoMode(int width, int height, bool windowed, bool vsync)LNOEXCEPT;
		LNOINLINE bool ChangeVideoMode2(int width, int height, bool windowed, bool vsync, int hza, int hzb, bool flip)LNOEXCEPT;
		
		LNOINLINE bool UpdateVideoMode()LNOEXCEPT;
		
		void SetFPS(fuInt v)LNOEXCEPT;
		
		/// @brief 获取当前的FPS
		double GetFPS()LNOEXCEPT { return m_fFPS; }
		
		//读取资源包中的文本文件
		//也能读取其他类型的文件，但是会得到无意义的结果
		LNOINLINE int LoadTextFile(lua_State* L, const char* path, const char *packname)LNOEXCEPT;
		
	private:
		void updateGraph2DBlendMode(BlendMode m);
		void updateGraph3DBlendMode(BlendMode m);
		
	public:  // 渲染器接口
		/// @brief 通知开始渲染
		bool BeginScene() LNOEXCEPT;
		
		/// @brief 通知结束渲染
		bool EndScene() LNOEXCEPT;
		
		/// @brief 清屏
		void ClearScreen(const fcyColor& c) LNOEXCEPT;
		
		/// @brief 设置视口
		bool SetViewport(double left, double right, double bottom, double top) LNOEXCEPT;
		
		/// @brief 设置裁剪矩形
		bool SetScissorRect(double left, double right, double bottom, double top) LNOEXCEPT;
		
		/// @brief 设置正投影矩阵。
		void SetOrtho(
			float left, float right, float bottom, float top,
			float znear = 8.0f, float zfar = 2048.0f) LNOEXCEPT;
		
		/// @brief 设置透视投影矩阵
		void SetPerspective(
			float eyeX, float eyeY, float eyeZ,
			float atX, float atY, float atZ, 
			float upX, float upY, float upZ,
			float fovy, float aspect,
			float zn, float zf) LNOEXCEPT;
		
		/// @brief 设置雾值
		/// @note 扩展方法，视情况移除。
		void SetFog(float start, float end, fcyColor color);
		
		// 开启或关闭zbuffer
		void SetZBufferEnable(bool enable) LNOEXCEPT;
		
		// 用指定的值清空zbuffer
		void ClearZBuffer(float z) LNOEXCEPT;
		
		/// @brief 渲染图像
		bool Render(ResSprite* p, float x, float y, float rot = 0, float hscale = 1, float vscale = 1, float z = 0.5) LNOEXCEPT;
		
		/// @brief 渲染动画
		bool Render(ResAnimation* p, int ani_timer, float x, float y, float rot = 0, float hscale = 1, float vscale = 1) LNOEXCEPT;
		
		/// @brief 渲染粒子
		bool Render(ResParticle::ParticlePool* p, float hscale = 1, float vscale = 1) LNOEXCEPT;
		
		/// @brief 渲染图像
		bool Render(const char* name, float x, float y, float rot = 0, float hscale = 1, float vscale = 1, float z = 0.5) LNOEXCEPT;
		
		/// @brief 渲染动画
		bool RenderAnimation(const char* name, int timer, float x, float y, float rot = 0, float hscale = 1, float vscale = 1) LNOEXCEPT;
		
		/// @brief 渲染图像
		bool RenderRect(const char* name, float x1, float y1, float x2, float y2) LNOEXCEPT;
		
		/// @brief 渲染图像
		bool Render4V(
			const char* name,
			float x1, float y1, float z1,
			float x2, float y2, float z2, 
			float x3, float y3, float z3,
			float x4, float y4, float z4) LNOEXCEPT;
		
		/// @brief 渲染纹理
		bool RenderTexture(ResTexture* tex, BlendMode blend, const f2dGraphics2DVertex vertex[]) LNOEXCEPT;
		
		/// @brief 渲染纹理
		bool RenderTexture(const char* name, BlendMode blend, f2dGraphics2DVertex vertex[]) LNOEXCEPT;
		
		// 渲染纹理，多顶点，不修正uv坐标
		bool RenderTexture(
			const char* name, BlendMode blend,
			int vcount, const f2dGraphics2DVertex vertex[],
			int icount, const unsigned short indexs[]) LNOEXCEPT;
		
		/// @brief 渲染文字
		bool RenderText(ResFont* p, wchar_t* strBuf, fcyRect rect, fcyVec2 scale, ResFont::FontAlignHorizontal halign, ResFont::FontAlignVertical valign, bool bWordBreak)LNOEXCEPT;
		
		fcyVec2 CalcuTextSize(ResFont* p, const wchar_t* strBuf, fcyVec2 scale)LNOEXCEPT;
		
		bool RenderText(const char* name, const char* str, float x, float y, float scale, ResFont::FontAlignHorizontal halign, ResFont::FontAlignVertical valign)LNOEXCEPT;
		
		bool RenderTTF(const char* name, const char* str, float left, float right, float bottom, float top, float scale, int format, fcyColor c)LNOEXCEPT;

		void SnapShot(const char* path)LNOEXCEPT;
		void SaveTexture(f2dTexture2D* Tex, const char* path)LNOEXCEPT;
		void SaveTexture(const char* tex_name, const char* path)LNOEXCEPT;

		bool CheckRenderTargetInUse(fcyRefPointer<f2dTexture2D> rt)LNOEXCEPT;

		bool CheckRenderTargetInUse(ResTexture* rt)LNOEXCEPT;

		bool PushRenderTarget(fcyRefPointer<f2dTexture2D> rt)LNOEXCEPT;

		bool PushRenderTarget(ResTexture* rt)LNOEXCEPT;

		bool PopRenderTarget()LNOEXCEPT;

		bool PostEffect(fcyRefPointer<f2dTexture2D> rt, ResFX* shader, BlendMode blend)LNOEXCEPT;

		bool PostEffect(ResTexture* rt, ResFX* shader, BlendMode blend)LNOEXCEPT;
		
		// 渲染扇形，通过纹理+uv范围渲染
		bool RenderSector(const char* name, fcyRect uv, bool tran, BlendMode blend, fcyColor color1, fcyColor color2,
			fcyVec2 pos, float rot, float exp, float r1, float r2, int div);
		
		// 渲染环，通过纹理+uv范围渲染
		bool RenderAnnulus(const char* name, fcyRect uv, bool tran, BlendMode blend, fcyColor color1, fcyColor color2,
			fcyVec2 pos, float rot, float r1, float r2, int div, int rep);
		
	public:
		// 文字渲染器包装
		bool FontRenderer_SetFontProvider(const char* name);
		void FontRenderer_SetFlipType(const F2DSPRITEFLIP t);
		void FontRenderer_SetScale(const fcyVec2& s);
		fcyRect FontRenderer_MeasureString(const char* str, bool strict = true);
		float FontRenderer_MeasureStringWidth(const char* str);
		bool FontRenderer_DrawTextW2(const char* str, fcyVec2& pos, const float z, const BlendMode blend, const fcyColor& color);
		
		fcyRect FontRenderer_MeasureTextBoundary(const char* str, size_t len);
		fcyVec2 FontRenderer_MeasureTextAdvance(const char* str, size_t len);
		bool FontRenderer_RenderText(const char* str, size_t len, fcyVec2& pos, const float z, const BlendMode blend, const fcyColor& color);
		bool FontRenderer_RenderTextInSpace(const char* str, size_t len, fcyVec3& pos, const fcyVec3& rvec, const fcyVec3& dvec, const BlendMode blend, const fcyColor& color);
		
		float FontRenderer_GetFontLineHeight();
		float FontRenderer_GetFontAscender();
		float FontRenderer_GetFontDescender();
		
	public:
		// 获取框架对象
		ResourceMgr& GetResourceMgr() LNOEXCEPT { return m_ResourceMgr; }
		GameObjectPool& GetGameObjectPool() LNOEXCEPT{ return *m_GameObjectPool.get(); }
		f2dEngine* GetEngine() LNOEXCEPT { return m_pEngine; }
		f2dWindow* GetWindow() LNOEXCEPT { return m_pMainWindow; }
		native::DirectInput* GetDInput() LNOEXCEPT { return m_DirectInput.get(); }
		f2dRenderer* GetRenderer() LNOEXCEPT { return m_pRenderer; }
		f2dRenderDevice* GetRenderDev() LNOEXCEPT { return m_pRenderDev; }
		f2dSoundSys* GetSoundSys() LNOEXCEPT { return m_pSoundSys; }
		fcyRefPointer<f2dGeometryRenderer> GetGeometryRenderer() LNOEXCEPT { return m_GRenderer; }
		GraphicsType GetGraphicsType() LNOEXCEPT { return m_GraphType; }
		fcyRefPointer<f2dGraphics3D> GetGraphics3D() LNOEXCEPT { return m_Graph3D; }
		fcyRefPointer<f2dGraphics2D> GetGraphics2D() LNOEXCEPT { return m_Graph2D; }
		LuaSTG::Core::Renderer& GetRenderer2D() { return m_NewRenderer2D; }
		lua_State* GetLuaEngine() LNOEXCEPT { return L; }
	public:
		/// @brief 初始化框架
		/// @note 该函数必须在一开始被调用，且仅能调用一次
		/// @return 失败返回false
		bool Init()LNOEXCEPT;
		/// @brief 终止框架并回收资源
		/// @note 该函数可以由框架自行调用，且仅能调用一次
		void Shutdown()LNOEXCEPT;
		
		/// @brief 执行框架，进入游戏循环
		void Run()LNOEXCEPT;
		
	protected:  // fancy2d逻辑循环回调
		fBool OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump);
		fBool OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController);
	public:
		AppFrame()LNOEXCEPT;
		~AppFrame()LNOEXCEPT;
	public:
		static LNOINLINE AppFrame& GetInstance();
	};
}

#define LAPP (LuaSTGPlus::AppFrame::GetInstance())
#define LPOOL (LAPP.GetGameObjectPool())
#define LRES (LAPP.GetResourceMgr())
