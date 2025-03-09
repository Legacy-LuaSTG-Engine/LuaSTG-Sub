#include "AppFrame.h"
#include "Core/FileManager.hpp"
#include "Platform/XInput.hpp"
#include "Utility/Utility.h"
#include "Debugger/ImGuiExtension.h"
#include "LuaBinding/LuaAppFrame.hpp"
#include "utf8.hpp"
#include "resource.h"
#include "core/Configuration.hpp"

using namespace LuaSTGPlus;

////////////////////////////////////////////////////////////////////////////////
/// AppFrame
////////////////////////////////////////////////////////////////////////////////

AppFrame& AppFrame::GetInstance()
{
	static AppFrame s_Instance;
	return s_Instance;
}
AppFrame::AppFrame() noexcept = default;
AppFrame::~AppFrame() noexcept
{
	if (m_iStatus != AppStatus::NotInitialized && m_iStatus != AppStatus::Destroyed)
	{
		// 若没有销毁框架，则执行销毁
		Shutdown();
	}
}

#pragma region 脚本接口

void AppFrame::SetFPS(uint32_t v)noexcept
{
	m_target_fps = std::max(1u, v); // 最低也得有1FPS每秒
}
void AppFrame::SetSEVolume(float v) {
	if (m_pAppModel) {
		m_pAppModel->getAudioDevice()->setMixChannelVolume(Core::Audio::MixChannel::SoundEffect, v);
	}
	else {
		core::ConfigurationLoader::getInstance().getAudioSystemRef().setSoundEffectVolume(v);
	}
}
void AppFrame::SetBGMVolume(float v) {
	if (m_pAppModel) {
		m_pAppModel->getAudioDevice()->setMixChannelVolume(Core::Audio::MixChannel::Music, v);
	}
	else {
		core::ConfigurationLoader::getInstance().getAudioSystemRef().setMusicVolume(v);
	}
}
float AppFrame::GetSEVolume() {
	if (m_pAppModel) {
		return m_pAppModel->getAudioDevice()->getMixChannelVolume(Core::Audio::MixChannel::SoundEffect);
	}
	else {
		return core::ConfigurationLoader::getInstance().getAudioSystem().getSoundEffectVolume();
	}
}
float AppFrame::GetBGMVolume() {
	if (m_pAppModel) {
		return m_pAppModel->getAudioDevice()->getMixChannelVolume(Core::Audio::MixChannel::Music);
	}
	else {
		return core::ConfigurationLoader::getInstance().getAudioSystem().getMusicVolume();
	}
}
void AppFrame::SetTitle(const char* v)noexcept
{
	if (m_pAppModel) {
		m_pAppModel->getWindow()->setTitleText(v);
	}
	else {
		auto& win = core::ConfigurationLoader::getInstance().getWindowRef();
		win.setTitle(v);
	}
}
void AppFrame::SetPreferenceGPU(const char* v) noexcept
{
	if (m_pAppModel) {
		// TODO
	}
	else {
		auto& gs = core::ConfigurationLoader::getInstance().getGraphicsSystemRef();
		gs.setPreferredDeviceName(v);
	}
}
void AppFrame::SetSplash(bool v)noexcept
{
	if (m_pAppModel) {
		m_pAppModel->getWindow()->setCursor(v ? Core::Graphics::WindowCursor::Arrow : Core::Graphics::WindowCursor::None);
	}
	else {
		auto& win = core::ConfigurationLoader::getInstance().getWindowRef();
		win.setCursorVisible(v);
	}
}

int AppFrame::LoadTextFile(lua_State* L_, const char* path, const char* packname)noexcept
{
	if (ResourceMgr::GetResourceLoadingLog()) {
		if (packname)
			spdlog::info("[luastg] 在资源包'{}'中读取文本文件'{}'", packname, path);
		else
			spdlog::info("[luastg] 读取文本文件'{}'", path);
	}
	bool loaded = false;
	std::vector<uint8_t> src;
	if (packname)
	{
		auto& arc = GFileManager().getFileArchive(packname);
		if (!arc.empty())
		{
			loaded = arc.load(path, src);
		}
	}
	else
	{
		loaded = GFileManager().loadEx(path, src);
	}
	if (!loaded) {
		spdlog::error("[luastg] 无法加载文件'{}'", path);
		return 0;
	}
	lua_pushlstring(L_, (char*)src.data(), src.size());
	return 1;
}

#pragma endregion

#pragma region 框架函数

bool AppFrame::Init()noexcept
{
	assert(m_iStatus == AppStatus::NotInitialized);

	spdlog::info(LUASTG_INFO);
	spdlog::info("[luastg] 初始化引擎");
	m_iStatus = AppStatus::Initializing;

	//////////////////////////////////////// 基础

	// 初始化文件系统
	if (auto const& resources = core::ConfigurationLoader::getInstance().getFileSystem().getResources(); !resources.empty()) {
		auto& file_manager = GFileManager();
		for (auto const& resource : resources) {
			using Type = core::ConfigurationLoader::FileSystem::ResourceFileSystem::Type;
			switch (resource.getType()) {
			case Type::directory:
				file_manager.addSearchPath(resource.getPath());
				break;
			case Type::archive:
				file_manager.loadFileArchive(resource.getPath());
				break;
			}
		}
	}

	//////////////////////////////////////// Lua 引擎

	spdlog::info("[luastg] 初始化luajit引擎");

	// 开启Lua引擎
	if (!OnOpenLuaEngine())
	{
		spdlog::info("[luastg] 初始化luajit引擎失败");
		return false;
	}

	CLR = new CLRHost(L".\\Managed\\net8.0\\LuaSTG.runtimeconfig.json");
	if (!CLR->init()) 
	{
		spdlog::info("[luastg] 初始化coreclr失败");
		return false;
	}

	// 加载初始化脚本（可选）
	if (!OnLoadLaunchScriptAndFiles())
	{
		return false;
	}

	//////////////////////////////////////// 应用程序模型、窗口子系统、图形子系统、音频子系统等

	{
		if (!Core::IApplicationModel::create(this, ~m_pAppModel))
			return false;
		if (!Core::Graphics::ITextRenderer::create(m_pAppModel->getRenderer(), ~m_pTextRenderer))
			return false;
		if (!InitializationApplySettingStage1())
			return false;

		// 为对象池分配空间
		spdlog::info("[luastg] 初始化对象池，容量{}", LOBJPOOL_SIZE);
		try
		{
			m_GameObjectPool = std::make_unique<GameObjectPool>(L);
		}
		catch (const std::bad_alloc&)
		{
			spdlog::error("[luastg] 无法为对象池分配内存");
			return false;
		}

		// 渲染器适配器
		m_bRenderStarted = false;

		OpenInput();

		// 创建手柄输入
		try
		{
			m_DirectInput = std::make_unique<Platform::DirectInput>((ptrdiff_t)m_pAppModel->getWindow()->getNativeHandle());
			{
				m_DirectInput->refresh(); // 这里因为窗口还没显示，所以应该会出现一个Aquire设备失败的错误信息，忽略即可
				uint32_t cnt = m_DirectInput->count();
				for (uint32_t i = 0; i < cnt; i += 1)
				{
					spdlog::info("[luastg] 检测到 {} 控制器 产品名称：{} 设备名称：{}",
						m_DirectInput->isXInputDevice(i) ? "XInput" : "DirectInput",
						utf8::to_string(m_DirectInput->getProductName(i)),
						utf8::to_string(m_DirectInput->getDeviceName(i))
					);
				}
				spdlog::info("[luastg] 成功创建了 {} 个控制器", cnt);
			}
		}
		catch (const std::bad_alloc&)
		{
			spdlog::error("[luastg] 无法为 DirectInput 分配内存");
		}

		// 初始化ImGui
	#ifdef USING_DEAR_IMGUI
		imgui::bindEngine();
	#endif

		if (!InitializationApplySettingStage2())
			return false;
	}

	// 装载main脚本
	if (!OnLoadMainScriptAndFiles())
	{
		return false;
	}

	//////////////////////////////////////// 初始化完成
	m_iStatus = AppStatus::Initialized;
	spdlog::info("[luastg] 初始化完成");

	//////////////////////////////////////// 调用GameInit
	if (!SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_EngineInit)) {
		return false;
	}

	return true;
}
void AppFrame::Shutdown()noexcept
{
	if (L) {
		SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_EngineStop);
	}

	m_GameObjectPool = nullptr;
	spdlog::info("[luastg] 清空对象池");

	if (L)
	{
		lua_close(L);
		L = nullptr;
		spdlog::info("[luastg] 关闭luajit引擎");
	}

	m_stRenderTargetStack.clear();
	m_ResourceMgr.ClearAllResource();
	spdlog::info("[luastg] 清空所有游戏资源");

	// 卸载ImGui
#ifdef USING_DEAR_IMGUI
	imgui::unbindEngine();
#endif

	GFileManager().unloadAllFileArchive();
	spdlog::info("[luastg] 卸载所有资源包");

	CloseInput();
	m_DirectInput = nullptr;
	m_pTextRenderer = nullptr;
	m_pAppModel = nullptr;

	m_iStatus = AppStatus::Destroyed;
	spdlog::info("[luastg] 引擎关闭");
}
void AppFrame::Run()noexcept
{
	assert(m_iStatus == AppStatus::Initialized);
	spdlog::info("[luastg] 开始更新&渲染循环");

	m_pAppModel->getWindow()->addEventListener(this);
	onSwapChainCreate(); // 手动触发一次，让自动尺寸的RenderTarget设置为正确的尺寸
	m_pAppModel->getSwapChain()->addEventListener(this);

	m_pAppModel->getFrameRateController()->setTargetFPS(m_target_fps);
	m_pAppModel->run();

	m_pAppModel->getSwapChain()->removeEventListener(this);
	m_pAppModel->getWindow()->removeEventListener(this);

	spdlog::info("[luastg] 结束更新&渲染循环");
}

#pragma endregion

#pragma region 游戏循环

void AppFrame::onWindowCreate()
{
	OpenInput();
	m_DirectInput = std::make_unique<Platform::DirectInput>((ptrdiff_t)m_pAppModel->getWindow()->getNativeHandle());
	{
		m_DirectInput->refresh(); // 这里因为窗口还没显示，所以应该会出现一个Aquire设备失败的错误信息，忽略即可
		uint32_t cnt = m_DirectInput->count();
		for (uint32_t i = 0; i < cnt; i += 1)
		{
			spdlog::info("[luastg] 检测到 {} 控制器 产品名称：{} 设备名称：{}",
				m_DirectInput->isXInputDevice(i) ? "XInput" : "DirectInput",
				utf8::to_string(m_DirectInput->getProductName(i)),
				utf8::to_string(m_DirectInput->getDeviceName(i))
			);
		}
		spdlog::info("[luastg] 成功创建了 {} 个控制器", cnt);
	}
}
void AppFrame::onWindowDestroy()
{
	m_DirectInput = nullptr;
	CloseInput();
}
void AppFrame::onWindowActive()
{
	Platform::XInput::setEnable(true);
	m_window_active_changed.fetch_or(0x1);
}
void AppFrame::onWindowInactive()
{
	Platform::XInput::setEnable(false);
	m_window_active_changed.fetch_or(0x2);
}
void AppFrame::onWindowSize(Core::Vector2U size)
{
	m_win32_window_size = size;
}
void AppFrame::onDeviceChange()
{
	m_window_active_changed.fetch_or(0x4);
}

bool AppFrame::onUpdate()
{
	m_fFPS = m_pAppModel->getFrameRateController()->getFPS();
	m_fAvgFPS = m_pAppModel->getFrameRateController()->getAvgFPS();
	m_pAppModel->getFrameRateController()->setTargetFPS(m_target_fps);

	bool result = true;

	{
		tracy_zone_scoped_with_name("OnUpdate-Event");

		int window_active_changed = m_window_active_changed.exchange(0);
		if (window_active_changed & 0x2)
		{
			if (m_DirectInput)
				m_DirectInput->reset();

			lua_pushinteger(L, (lua_Integer)LuaSTG::LuaEngine::EngineEvent::WindowActive);
			lua_pushboolean(L, false);
			SafeCallGlobalFunctionB(LuaSTG::LuaEngine::G_CALLBACK_EngineEvent, 2, 0);

			if (!SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_FocusLoseFunc))
			{
				result = false;
				m_pAppModel->requestExit();
			}
		}
		if (window_active_changed & 0x1)
		{
			if (m_DirectInput)
				m_DirectInput->reset();

			lua_pushinteger(L, (lua_Integer)LuaSTG::LuaEngine::EngineEvent::WindowActive);
			lua_pushboolean(L, true);
			SafeCallGlobalFunctionB(LuaSTG::LuaEngine::G_CALLBACK_EngineEvent, 2, 0);

			if (!SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_FocusGainFunc))
			{
				result = false;
				m_pAppModel->requestExit();
			}
		}
		if (window_active_changed & 0x4)
		{
			if (m_DirectInput)
				m_DirectInput->refresh();
		}

		UpdateInput();
	}

#if (defined(_DEBUG) && defined(LuaSTG_enable_GameObjectManager_Debug))
	static uint64_t _frame_count = 0;
	spdlog::debug("[frame] ---------- {} ----------", _frame_count);
	_frame_count += 1;
#endif

	if (result)
	{
		tracy_zone_scoped_with_name("OnUpdate-LuaCallback");
		// 执行帧函数
		imgui::cancelSetCursor();
		m_GameObjectPool->DebugNextFrame();
		if (!SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_EngineUpdate, 1))
		{
			result = false;
			m_pAppModel->requestExit();
		}
		bool tAbort = lua_toboolean(L, -1) != 0;
		lua_pop(L, 1);
		if (tAbort)
			m_pAppModel->requestExit();
		m_ResourceMgr.UpdateSound();
	}

	return result;
}
bool AppFrame::onRender()
{
	m_bRenderStarted = true;

	GetRenderTargetManager()->BeginRenderTargetStack();

	// 执行渲染函数
	bool result = SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_EngineDraw);
	if (!result)
		m_pAppModel->requestExit();

	GetRenderTargetManager()->EndRenderTargetStack();

	m_bRenderStarted = false;
	return result;
}

#pragma endregion
