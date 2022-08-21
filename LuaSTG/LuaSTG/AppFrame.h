/// @file AppFrame.h
/// @brief 定义应用程序框架
#pragma once
#include "Core/ApplicationModel.hpp"
#include "Core/Graphics/Font.hpp"
#include "GameResource/ResourceMgr.h"
#include "GameObject/GameObjectPool.h"
#include "platform/DirectInput.hpp"

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
	class AppFrame
		: public Core::IApplicationEventListener
		, public Core::Graphics::IWindowEventListener
	{
	private:
		AppStatus m_iStatus = AppStatus::NotInitialized;
		
		// 应用程序框架
		Core::ScopeObject<Core::IApplicationModel> m_pAppModel;
		Core::ScopeObject<Core::Graphics::ITextRenderer> m_pTextRenderer;

		// 资源管理器
		ResourceMgr m_ResourceMgr;
		
		// 对象池
		std::unique_ptr<GameObjectPool> m_GameObjectPool;
		
		// Lua虚拟机
		lua_State* L = nullptr;
		
		// 选项与值
		bool m_OptionWindowed = true;
		Core::Graphics::WindowFrameStyle m_OptionWindowStyle = Core::Graphics::WindowFrameStyle::Fixed;
		uint32_t m_OptionFPSLimit = 60;
		bool m_OptionVsync = false;
		Core::Vector2F m_OptionResolution = Core::Vector2F(640.f, 480.f);
		uint32_t m_OptionRefreshRateA = 0;
		uint32_t m_OptionRefreshRateB = 0;
		std::wstring m_OptionGPU;
		std::string m_OptionTitle = LUASTG_INFO;
		bool m_OptionCursor = true;
		double m_fFPS = 0.;
		double m_fAvgFPS = 0.;
		float m_gSEVol = 1.0f;
		float m_gBGMVol = 1.0f;

		//渲染状态
		bool m_bRenderStarted = false;
		
		// 渲染目标栈
		std::vector<fcyRefPointer<ResTexture>> m_stRenderTargetStack;
		
		//输入设备
		std::unique_ptr<platform::DirectInput> m_DirectInput;
		
	public:
		/// @brief 保护模式执行脚本
		/// @note 该函数仅限框架调用，为主逻辑最外层调用。若脚本运行时发生错误，该函数负责截获错误发出错误消息。
		bool SafeCallScript(const char* source, size_t len, const char* desc)noexcept;
		
		/// @brief 不保护调用全局函数
		/// @note 该函数仅限框架调用，为主逻辑最外层调用
		bool UnsafeCallGlobalFunction(const char* name, int retc = 0)noexcept;
		
		/// @brief 保护模式调用全局函数
		/// @note 该函数仅限框架调用，为主逻辑最外层调用。若脚本运行时发生错误，该函数负责截获错误发出错误消息。调用者负责维持栈平衡。
		bool SafeCallGlobalFunction(const char* name, int retc = 0)noexcept;
		
		/// @brief 保护模式调用全局函数
		/// @note 该函数仅限框架调用，为主逻辑最外层调用。若脚本运行时发生错误，该函数负责截获错误发出错误消息。调用者负责维持栈平衡。
		bool SafeCallGlobalFunctionB(const char* name, int argc = 0, int retc = 0)noexcept;
		
		/// @brief 执行资源包中的文件
		/// @note 该函数为脚本系统使用
		void LoadScript(const char* path,const char *packname)noexcept;
		
		bool OnOpenLuaEngine();
		bool OnLoadLaunchScriptAndFiles();
		bool OnLoadMainScriptAndFiles();
		
	public:  // 输入系统接口
		void OpenInput();
		void CloseInput();
		void UpdateInput();
		bool WantSwitchFullScreenMode();
		void ResetKeyboardInput();
		void ResetMouseInput();

		//检查按键是否按下
		bool GetKeyState(int VKCode)noexcept;
		
		/// @brief 获得最后一次按键输入
		int GetLastKey()noexcept;
		
		/// @brief 获取鼠标位置（以窗口左下角为原点）
		Core::Vector2F GetMousePosition(bool no_flip = false)noexcept;
		
		/// @brief 获取鼠标滚轮增量
		int32_t GetMouseWheelDelta()noexcept;
		
		/// @brief 检查鼠标是否按下
		bool GetMouseState_legacy(int button) noexcept;
		bool GetMouseState(int button) noexcept;
		
	public: // 脚本调用接口，含义参见API文档
		void SetWindowed(bool v)noexcept;
		void SetDefaultWindowStyle(Core::Graphics::WindowFrameStyle v) { m_OptionWindowStyle = v; };
		void SetVsync(bool v)noexcept;
		void SetResolution(uint32_t width, uint32_t height, uint32_t A = 0, uint32_t B = 0)noexcept;
		void SetTitle(const char* v)noexcept;
		void SetPreferenceGPU(const char* v, bool dGPU_trick = false)noexcept;
		void SetSplash(bool v)noexcept;
		void SetSEVolume(float v);
		void SetBGMVolume(float v);
		float GetSEVolume() { return m_gSEVol; }
		float GetBGMVolume() { return m_gBGMVol; }
		
	public:
		void SetAdapterPolicy(bool enable);

		bool ChangeVideoMode(int width, int height, bool windowed, bool vsync)noexcept;
		bool ChangeVideoMode2(int width, int height, bool windowed, bool vsync, int hza, int hzb, bool flip)noexcept;
		bool UpdateVideoMode()noexcept;
		
		void SetFPS(uint32_t v)noexcept;
		
		/// @brief 获取当前的FPS
		double GetFPS()noexcept { return m_fAvgFPS; }
		
		//读取资源包中的文本文件
		//也能读取其他类型的文件，但是会得到无意义的结果
		int LoadTextFile(lua_State* L, const char* path, const char *packname)noexcept;
		
	public:  // 渲染器接口
		void updateGraph2DBlendMode(BlendMode m);

		/// @brief 渲染图像
		bool Render(ResSprite* p, float x, float y, float rot = 0, float hscale = 1, float vscale = 1, float z = 0.5)noexcept;
		
		/// @brief 渲染动画
		bool Render(ResAnimation* p, int ani_timer, float x, float y, float rot = 0, float hscale = 1, float vscale = 1)noexcept;
		
		/// @brief 渲染粒子
		bool Render(ResParticle::ParticlePool* p, float hscale = 1, float vscale = 1)noexcept;
		
		/// @brief 渲染图像
		bool Render(const char* name, float x, float y, float rot = 0, float hscale = 1, float vscale = 1, float z = 0.5)noexcept;
		
		/// @brief 渲染动画
		bool RenderAnimation(const char* name, int timer, float x, float y, float rot = 0, float hscale = 1, float vscale = 1)noexcept;
		
		/// @brief 渲染纹理（注：UV 坐标会被归一化）
		bool RenderTexture(ResTexture* tex, BlendMode blend, const Core::Graphics::IRenderer::DrawVertex vertex[])noexcept;
		
		/// @brief 渲染纹理（注：UV 坐标会被归一化）
		bool RenderTexture(const char* name, BlendMode blend, Core::Graphics::IRenderer::DrawVertex vertex[])noexcept;
		
		/// @brief 渲染文字
		bool RenderText(ResFont* p, wchar_t* strBuf, Core::RectF rect, Core::Vector2F scale, ResFont::FontAlignHorizontal halign, ResFont::FontAlignVertical valign, bool bWordBreak)noexcept;
		
		Core::Vector2F CalcuTextSize(ResFont* p, const wchar_t* strBuf, Core::Vector2F scale)noexcept;
		
		bool RenderText(const char* name, const char* str, float x, float y, float scale, ResFont::FontAlignHorizontal halign, ResFont::FontAlignVertical valign)noexcept;
		
		bool RenderTTF(const char* name, const char* str, float left, float right, float bottom, float top, float scale, int format, Core::Color4B c)noexcept;

		void SnapShot(const char* path)noexcept;
		void SaveTexture(const char* tex_name, const char* path)noexcept;

		bool CheckRenderTargetInUse(ResTexture* rt)noexcept;
		bool PushRenderTarget(ResTexture* rt)noexcept;
		bool PopRenderTarget()noexcept;
		Core::Vector2U GetCurrentRenderTargetSize();

		void DebugSetGeometryRenderState();
		void DebugDrawCircle(float const x, float const y, float const r, Core::Color4B const color);
		void DebugDrawRect(float const x, float const y, float const a, float const b, float const rot, Core::Color4B const color);
		void DebugDrawEllipse(float const x, float const y, float const a, float const b, float const rot, Core::Color4B const color);

	public:
		// 文字渲染器包装
		bool FontRenderer_SetFontProvider(const char* name);
		void FontRenderer_SetScale(const Core::Vector2F& s);
		
		Core::RectF FontRenderer_MeasureTextBoundary(const char* str, size_t len);
		Core::Vector2F FontRenderer_MeasureTextAdvance(const char* str, size_t len);
		bool FontRenderer_RenderText(const char* str, size_t len, Core::Vector2F& pos, const float z, const BlendMode blend, Core::Color4B const& color);
		bool FontRenderer_RenderTextInSpace(const char* str, size_t len, Core::Vector3F& pos, Core::Vector3F const& rvec, Core::Vector3F const& dvec, const BlendMode blend, Core::Color4B const& color);
		
		float FontRenderer_GetFontLineHeight();
		float FontRenderer_GetFontAscender();
		float FontRenderer_GetFontDescender();
		
	public:
		// 获取框架对象
		lua_State* GetLuaEngine()noexcept { return L; }

		ResourceMgr& GetResourceMgr()noexcept { return m_ResourceMgr; }

		GameObjectPool& GetGameObjectPool()noexcept{ return *m_GameObjectPool.get(); }

		platform::DirectInput* GetDInput()noexcept { return m_DirectInput.get(); }
		
		Core::IApplicationModel* GetAppModel() { return *m_pAppModel; }
		Core::Graphics::IRenderer* GetRenderer2D() { return m_pAppModel->getRenderer(); }

	public:
		/// @brief 初始化框架
		/// @note 该函数必须在一开始被调用，且仅能调用一次
		/// @return 失败返回false
		bool Init()noexcept;
		/// @brief 终止框架并回收资源
		/// @note 该函数可以由框架自行调用，且仅能调用一次
		void Shutdown()noexcept;
		
		/// @brief 执行框架，进入游戏循环
		void Run()noexcept;
		
	protected:
		std::atomic_int m_window_active_changed{ 0 };

		void onWindowActive();
		void onWindowInactive();
		void onDeviceChange();

		void onUpdate();
		void onRender();
	public:
		AppFrame()noexcept;
		~AppFrame()noexcept;
	public:
		static AppFrame& GetInstance();
	};
}

#define LAPP (LuaSTGPlus::AppFrame::GetInstance())
#define LPOOL (LAPP.GetGameObjectPool())
#define LRES (LAPP.GetResourceMgr())
