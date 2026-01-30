#pragma once
#include <set>
#include <vector>
#include "core/Application.hpp"
#include "core/FrameRateController.hpp"
#include "core/Window.hpp"
#include "core/GraphicsDevice.hpp"
#include "core/Graphics/SwapChain.hpp"
#include "core/Graphics/Renderer.hpp"
#include "core/Graphics/Font.hpp"
#include "core/AudioEngine.hpp"
#include "GameResource/ResourceManager.h"
#include "GameObject/GameObjectPool.h"
#include "windows/DirectInput.hpp"
#include "Debugger/FrameQuery.hpp"

namespace luastg {
	/// @brief 应用程序状态
	enum class AppStatus {
		NotInitialized,
		Initializing,
		Initialized,
		Running,
		Aborted,
		Destroyed,
	};

	struct IRenderTargetManager {
		// 渲染目标栈

		virtual bool BeginRenderTargetStack() = 0;
		virtual bool EndRenderTargetStack() = 0;
		virtual bool PushRenderTarget(IResourceTexture* rt) = 0;
		virtual bool PopRenderTarget() = 0;
		virtual bool IsRenderTargetStackEmpty() = 0;
		virtual bool CheckRenderTargetInUse(IResourceTexture* rt) = 0;
		virtual core::Vector2U GetTopRenderTargetSize() = 0;

		// 维护自动大小的渲染目标

		virtual void AddAutoSizeRenderTarget(IResourceTexture* rt) = 0;
		virtual void RemoveAutoSizeRenderTarget(IResourceTexture* rt) = 0;
		virtual core::Vector2U GetAutoSizeRenderTargetSize() = 0;
		virtual bool ResizeAutoSizeRenderTarget(core::Vector2U size) = 0;
	};

	/// @brief 应用程序框架
	class AppFrame
		: public core::IApplication
		, public core::IWindowEventListener
		, public core::Graphics::ISwapChainEventListener
		, public IRenderTargetManager
	{
	public:
		// 统计数据

		struct FrameStatistics {
			double total_time{};
			double wait_time{};
			double update_time{};
			double render_time{};
			double present_time{};
		};

		struct FrameRenderStatistics {
			double render_time{};
		};

	private:
		AppStatus m_iStatus = AppStatus::NotInitialized;

		// 应用程序框架
		core::IFrameRateController* m_frame_rate_controller{ core::IFrameRateController::getInstance() };
		core::SmartReference<core::IWindow> m_window;
		core::SmartReference<core::IGraphicsDevice> m_graphics_device;
		core::SmartReference<core::Graphics::ISwapChain> m_swap_chain;
		core::SmartReference<core::Graphics::IRenderer> m_renderer;
		core::SmartReference<core::Graphics::ITextRenderer> m_text_renderer;
		core::SmartReference<core::IAudioEngine> m_audio_engine;

		// 统计数据
		size_t m_frame_statistics_index{};
		FrameStatistics m_frame_statistics[2]{};
		double m_message_time{};
		core::ScopeTimer m_message_timer;

		// 图形统计数据
		size_t m_render_statistics_index{};
		std::vector<FrameQuery> m_render_statistics;

		// 资源管理器
		ResourceMgr m_ResourceMgr;

		// 对象池
		std::unique_ptr<GameObjectPool> m_GameObjectPool;

		// Lua虚拟机
		lua_State* L = nullptr;

		// 目标帧率
		uint32_t m_target_fps{ 60 };
		// 测量值
		double m_fFPS = 0.;
		double m_fAvgFPS = 0.;

		// 渲染状态
		bool m_bRenderStarted = false;

		// 输入设备
		std::unique_ptr<Platform::DirectInput> m_DirectInput;

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
		void LoadScript(lua_State* L, const char* path, const char* packname);

		bool OnOpenLuaEngine();
		bool OnLoadLaunchScriptAndFiles();
		bool OnLoadMainScriptAndFiles();

	public:  // 输入系统接口

		void OpenInput();
		void CloseInput();
		void UpdateInput();
		void ResetKeyboardInput();
		void ResetMouseInput();

		//检查按键是否按下
		bool GetKeyState(int VKCode)noexcept;

		/// @brief 获得最后一次按键输入
		int GetLastKey()noexcept;

		/// @brief 获取鼠标位置（以窗口左下角为原点）
		core::Vector2F GetMousePosition(bool no_flip = false)noexcept;

		core::Vector2F GetCurrentWindowSizeF();
		core::Vector4F GetMousePositionTransformF();

		/// @brief 获取鼠标滚轮增量
		int32_t GetMouseWheelDelta()noexcept;

		/// @brief 检查鼠标是否按下
		bool GetMouseState_legacy(int button) noexcept;
		bool GetMouseState(int button) noexcept;

	public: // 脚本调用接口，含义参见API文档

		void SetTitle(const char* v) noexcept;
		void SetPreferenceGPU(const char* v) noexcept;
		void SetSplash(bool v) noexcept;
		void SetSEVolume(float v);
		void SetBGMVolume(float v);
		float GetSEVolume();
		float GetBGMVolume();

	public: // 窗口和交换链

		// 以窗口模式显示  
		// 当 monitor_rect 为空矩形时，窗口自动挑选最近的显示器来居中，否则根据显示器矩形选择匹配的显示器居中  
		bool SetDisplayModeWindow(core::Vector2U window_size, bool vsync);

		// 以独占全屏显示  
		// 当 refresh_rate 为全 0 时，自动选择合适的匹配的刷新率  
		bool SetDisplayModeExclusiveFullscreen(core::Vector2U window_size, bool vsync, core::Rational refresh_rate);

		bool InitializationApplySettingStage1();

		bool InitializationApplySettingStage2();

		void SetWindowed(bool v);

		void SetVsync(bool v);

		void SetResolution(uint32_t width, uint32_t height);

	public: // 其他框架方法

		// 设置目标 FPS
		void SetFPS(uint32_t v) noexcept;

		// 获取当前平均 FPS
		double GetFPS() const noexcept { return m_fAvgFPS; }

		// 读取资源包中的文本文件
		// 也能读取其他类型的文件，但是会得到无意义的结果
		int LoadTextFile(lua_State* L, const char* path, const char* packname) noexcept;

	public: // 渲染器接口

		// 应用混合模式
		void updateGraph2DBlendMode(BlendMode blend);

		/// @brief 渲染粒子
		bool Render(IParticlePool* p, float hscale = 1, float vscale = 1)noexcept;

		/// @brief 渲染文字
		bool RenderText(IResourceFont* p, wchar_t* strBuf, core::RectF rect, core::Vector2F scale, FontAlignHorizontal halign, FontAlignVertical valign, bool bWordBreak)noexcept;

		core::Vector2F CalcuTextSize(IResourceFont* p, const wchar_t* strBuf, core::Vector2F scale)noexcept;

		bool RenderText(const char* name, const char* str, float x, float y, float scale, FontAlignHorizontal halign, FontAlignVertical valign)noexcept;

		bool RenderTTF(const char* name, const char* str, float left, float right, float bottom, float top, float scale, int format, core::Color4B c)noexcept;

		void SnapShot(const char* path)noexcept;
		void SaveTexture(const char* tex_name, const char* path)noexcept;

		// ---------- 绘制常用形状 ----------

	public:
		void DebugSetGeometryRenderState();
		void DebugDrawCircle(float x, float y, float r, core::Color4B color);
		void DebugDrawRect(float x, float y, float a, float b, float rot, core::Color4B color);
		void DebugDrawEllipse(float x, float y, float a, float b, float rot, core::Color4B color);

		// ---------- 渲染目标管理 ----------

	private:
		std::vector<core::SmartReference<IResourceTexture>> m_stRenderTargetStack;
		std::set<IResourceTexture*> m_AutoSizeRenderTarget;
		core::Vector2U m_AutoSizeRenderTargetSize;
	private:
		// 渲染目标栈

		bool BeginRenderTargetStack();
		bool EndRenderTargetStack();
		bool PushRenderTarget(IResourceTexture* rt);
		bool PopRenderTarget();
		bool IsRenderTargetStackEmpty();
		bool CheckRenderTargetInUse(IResourceTexture* rt);
		core::Vector2U GetTopRenderTargetSize();

		// 维护自动大小的渲染目标

		void AddAutoSizeRenderTarget(IResourceTexture* rt);
		void RemoveAutoSizeRenderTarget(IResourceTexture* rt);
		core::Vector2U GetAutoSizeRenderTargetSize();
		bool ResizeAutoSizeRenderTarget(core::Vector2U size);

	public:
		// 事件监听

		void onSwapChainCreate() override;
		void onSwapChainDestroy() override;

		// 接口

		IRenderTargetManager* GetRenderTargetManager();

	public:
		// 文字渲染器包装
		bool FontRenderer_SetFontProvider(const char* name);
		void FontRenderer_SetScale(const core::Vector2F& s);

		core::RectF FontRenderer_MeasureTextBoundary(const char* str, size_t len);
		core::Vector2F FontRenderer_MeasureTextAdvance(const char* str, size_t len);
		bool FontRenderer_RenderText(const char* str, size_t len, core::Vector2F& pos, float z, BlendMode blend, core::Color4B const& color);
		bool FontRenderer_RenderTextInSpace(const char* str, size_t len, core::Vector3F& pos, core::Vector3F const& rvec, core::Vector3F const& dvec, BlendMode blend, core::Color4B const& color);

		float FontRenderer_GetFontLineHeight();
		float FontRenderer_GetFontAscender();
		float FontRenderer_GetFontDescender();

	public:
		// 获取框架对象
		lua_State* GetLuaEngine()noexcept { return L; }

		ResourceMgr& GetResourceMgr()noexcept { return m_ResourceMgr; }

		GameObjectPool& GetGameObjectPool()noexcept { return *m_GameObjectPool; }

		Platform::DirectInput* GetDInput()noexcept { return m_DirectInput.get(); }

		const FrameStatistics& getFrameStatistics() { return m_frame_statistics[m_frame_statistics_index]; }
		const FrameRenderStatistics& getFrameRenderStatistics() {
			// TODO: stupid implement
			static FrameRenderStatistics s;
			s.render_time = m_render_statistics[m_render_statistics_index].getTime();
			return s;
		}
		core::IWindow* getWindow() const noexcept { return m_window.get(); }
		core::IGraphicsDevice* getGraphicsDevice() const noexcept { return m_graphics_device.get(); }
		core::Graphics::ISwapChain* getSwapChain() const noexcept { return m_swap_chain.get(); }
		core::Graphics::IRenderer* getRenderer2D() const noexcept { return m_renderer.get(); }
		core::Graphics::ITextRenderer* getTextRenderer() const noexcept { return m_text_renderer.get(); }
		core::IAudioEngine* getAudioEngine() const noexcept { return m_audio_engine.get(); }

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
		core::Vector2U m_win32_window_size;

		void onWindowCreate() override;
		void onWindowDestroy() override;
		void onWindowActive() override;
		void onWindowInactive() override;
		void onWindowSize(core::Vector2U size) override;
		void onDeviceChange() override;

		// IApplication

		bool onCreate() override;
		void onBeforeUpdate() override;
		bool onUpdate() override;
		void onDestroy() override;

		// Application

		bool onUpdateInternal();
		bool onRenderInternal();
	public:
		AppFrame()noexcept;
		~AppFrame()noexcept;
	public:
		static AppFrame& GetInstance();
	};
}

#define LAPP (luastg::AppFrame::GetInstance())
#define LPOOL (LAPP.GetGameObjectPool())
#define LRES (LAPP.GetResourceMgr())
