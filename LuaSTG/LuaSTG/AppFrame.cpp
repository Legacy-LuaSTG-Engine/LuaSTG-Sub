#include "AppFrame.h"
#include "ApplicationRestart.hpp"
#include "core/FileSystem.hpp"
#include "windows/WindowsVersion.hpp"
#include "windows/ProcessorInfo.hpp"
#include "windows/XInput.hpp"
#include "Utility/Utility.h"
#include "Debugger/ImGuiExtension.h"
#include "LuaBinding/LuaAppFrame.hpp"
#include "utf8.hpp"
#include "core/Configuration.hpp"
#include "GameResource/SharedSpriteRenderer.hpp"

using namespace luastg;

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
	if (m_audio_engine) {
		m_audio_engine->setMixingChannelVolume(core::AudioMixingChannel::sound_effect, v);
	}
	else {
		core::ConfigurationLoader::getInstance().getAudioSystemRef().setSoundEffectVolume(v);
	}
}
void AppFrame::SetBGMVolume(float v) {
	if (m_audio_engine) {
		m_audio_engine->setMixingChannelVolume(core::AudioMixingChannel::music, v);
	}
	else {
		core::ConfigurationLoader::getInstance().getAudioSystemRef().setMusicVolume(v);
	}
}
float AppFrame::GetSEVolume() {
	if (m_audio_engine) {
		return m_audio_engine->getMixingChannelVolume(core::AudioMixingChannel::sound_effect);
	}
	else {
		return core::ConfigurationLoader::getInstance().getAudioSystem().getSoundEffectVolume();
	}
}
float AppFrame::GetBGMVolume() {
	if (m_audio_engine) {
		return m_audio_engine->getMixingChannelVolume(core::AudioMixingChannel::music);
	}
	else {
		return core::ConfigurationLoader::getInstance().getAudioSystem().getMusicVolume();
	}
}
void AppFrame::SetTitle(const char* v) noexcept {
	if (m_window) {
		m_window->setTitleText(v);
	}
	else {
		auto& win = core::ConfigurationLoader::getInstance().getWindowRef();
		win.setTitle(v);
	}
}
void AppFrame::SetPreferenceGPU(const char* v) noexcept {
	if (m_graphics_device) {
		// TODO
	}
	else {
		auto& gs = core::ConfigurationLoader::getInstance().getGraphicsSystemRef();
		gs.setPreferredDeviceName(v);
	}
}
void AppFrame::SetSplash(bool v) noexcept {
	if (m_window) {
		m_window->setCursor(v ? core::WindowCursor::Arrow : core::WindowCursor::None);
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
	core::SmartReference<core::IData> src;
	if (packname)
	{
		core::SmartReference<core::IFileSystemArchive> archive;
		if (core::FileSystemManager::getFileSystemArchiveByPath(packname, archive.put())) {
			loaded = archive->readFile(path, src.put());
		}
	}
	else
	{
		loaded = core::FileSystemManager::readFile(path, src.put());
	}
	if (!loaded) {
		spdlog::error("[luastg] 无法加载文件'{}'", path);
		return 0;
	}
	lua_pushlstring(L_, (char*)src->data(), src->size());
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
		for (auto const& resource : resources) {
			using Type = core::ConfigurationLoader::FileSystem::ResourceFileSystem::Type;
			switch (resource.getType()) {
			case Type::directory:
				core::FileSystemManager::addSearchPath(resource.getPath());
				break;
			case Type::archive:
				do {
					core::SmartReference<core::IFileSystemArchive> archive;
					if (core::IFileSystemArchive::createFromFile(resource.getPath(), archive.put())) {
						core::FileSystemManager::addFileSystem(resource.getName(), archive.get());
					}
				}
				while (false);
				break;
			}
		}
	}

	//////////////////////////////////////// 游戏对象池

	// 为对象池分配空间
	spdlog::info("[luastg] 初始化对象池，容量{}", LOBJPOOL_SIZE);
	try {
		m_GameObjectPool = std::make_unique<GameObjectPool>();
	}
	catch (const std::bad_alloc&) {
		spdlog::error("[luastg] 无法为对象池分配内存");
		return false;
	}

	//////////////////////////////////////// Lua 引擎

	spdlog::info("[luastg] 初始化luajit引擎");

	// 开启Lua引擎
	if (!OnOpenLuaEngine()) {
		spdlog::info("[luastg] 初始化luajit引擎失败");
		return false;
	}

	// 加载初始化脚本（可选）
	if (!OnLoadLaunchScriptAndFiles()) {
		return false;
	}

	//////////////////////////////////////// 应用程序模型、窗口子系统、图形子系统、音频子系统等

	{
		if (auto& window_config = core::ConfigurationLoader::getInstance().getWindowRef(); !window_config.hasTitle()) {
			window_config.setTitle(LUASTG_INFO);
		}

		spdlog::info("[core] System: {}", Platform::WindowsVersion::GetName());
		spdlog::info("[core] Kernel: {}", Platform::WindowsVersion::GetKernelVersionString());
		spdlog::info("[core] CPU: {}", Platform::ProcessorInfo::name());
		//get_system_memory_status();
		if (!core::IWindow::create(m_window.put()))
			return false;
		auto const& gpu = core::ConfigurationLoader::getInstance().getGraphicsSystem().getPreferredDeviceName();
		if (!core::Graphics::IDevice::create(gpu, m_graphics_device.put()))
			return false;
		m_render_statistics.reserve(4);
		for (int i = 0; i < 4; i += 1) {
			m_render_statistics.emplace_back(m_graphics_device.get());
		}
		if (!core::Graphics::ISwapChain::create(m_window.get(), m_graphics_device.get(), m_swap_chain.put()))
			return false;
		if (!core::Graphics::IRenderer::create(m_graphics_device.get(), m_renderer.put()))
			return false;
		if (!core::Graphics::ITextRenderer::create(m_renderer.get(), m_text_renderer.put()))
			return false;
		if (!core::IAudioEngine::create(m_audio_engine.put()))
			return false;
		if (!SharedSpriteRenderer::create())
			return false;
		if (!InitializationApplySettingStage1())
			return false;

		// 渲染器适配器
		m_bRenderStarted = false;

		OpenInput();

		// 创建手柄输入
		try
		{
			m_DirectInput = std::make_unique<Platform::DirectInput>((ptrdiff_t)m_window->getNativeHandle());
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
	if (!SafeCallGlobalFunction(LuaEngine::G_CALLBACK_EngineInit)) {
		return false;
	}

	return true;
}
void AppFrame::Shutdown()noexcept
{
	if (L) {
		SafeCallGlobalFunction(LuaEngine::G_CALLBACK_EngineStop);
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

	core::FileSystemManager::removeAllFileSystem();
	spdlog::info("[luastg] 卸载所有资源包");

	CloseInput();
	m_DirectInput = nullptr;
	m_window.reset();
	m_graphics_device.reset();
	m_render_statistics.clear();
	m_swap_chain.reset();
	m_renderer.reset();
	m_text_renderer.reset();
	m_audio_engine.reset();
	SharedSpriteRenderer::destroy();

	m_iStatus = AppStatus::Destroyed;
	spdlog::info("[luastg] 引擎关闭");
}
void AppFrame::Run()noexcept
{
	assert(m_iStatus == AppStatus::Initialized);
	spdlog::info("[luastg] 开始更新&渲染循环");

	m_window->addEventListener(this);
	onSwapChainCreate(); // 手动触发一次，让自动尺寸的RenderTarget设置为正确的尺寸
	m_swap_chain->addEventListener(this);

	m_frame_rate_controller->setFrameRate(m_target_fps);
	core::ApplicationManager::run(this);

	m_swap_chain->removeEventListener(this);
	m_window->removeEventListener(this);

	spdlog::info("[luastg] 结束更新&渲染循环");
}

#pragma endregion

#pragma region 游戏循环

void AppFrame::onWindowCreate()
{
	OpenInput();
	m_DirectInput = std::make_unique<Platform::DirectInput>((ptrdiff_t)m_window->getNativeHandle());
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
void AppFrame::onWindowSize(core::Vector2U size)
{
	m_win32_window_size = size;
}
void AppFrame::onDeviceChange()
{
	m_window_active_changed.fetch_or(0x4);
}

bool AppFrame::onCreate() { return true; }
void AppFrame::onBeforeUpdate() {
	const auto frame_statistics_index = (m_frame_statistics_index + 1) % 2;
	auto& d = m_frame_statistics[frame_statistics_index];

	{
		core::ScopeTimer t(&d.wait_time);
		if (m_swap_chain) {
			m_swap_chain->waitFrameLatency();
		}
		m_frame_rate_controller->update();
	}

	m_fFPS = 1.0 / m_frame_rate_controller->getStatistics()->getDuration(0);
	m_fAvgFPS = 1.0 / m_frame_rate_controller->getStatistics()->getAverage(10);
	m_frame_rate_controller->setFrameRate(m_target_fps);
	m_message_timer = core::ScopeTimer(&m_message_time);
}
bool AppFrame::onUpdate() {
	m_message_timer = core::ScopeTimer();

	const auto frame_statistics_index = (m_frame_statistics_index + 1) % 2;
	auto& d = m_frame_statistics[frame_statistics_index];

	const auto render_statistics_index = (m_render_statistics_index + 1) % m_render_statistics.size();
	auto& r = m_render_statistics[render_statistics_index];

	{
		core::ScopeTimer t(&d.update_time);
		if (!onUpdateInternal()) {
			return false;
		}
	}

	{
		core::ScopeTimer t(&d.render_time);
		r.begin();
		m_swap_chain->applyRenderAttachment();
		m_swap_chain->clearRenderAttachment();
		const auto result = onRenderInternal();
		r.end();
		if (!result) {
			return false;
		}
	}

	{
		core::ScopeTimer t(&d.present_time);
		m_swap_chain->present();
	}

	d.update_time += m_message_time;
	d.total_time = d.wait_time + d.update_time + d.render_time + d.present_time;
	m_frame_statistics_index = frame_statistics_index; // move next
	m_render_statistics_index = render_statistics_index;
	return true;
}
void AppFrame::onDestroy() {}

bool AppFrame::onUpdateInternal()
{
	bool result = true;

	// pre-check
	if (ApplicationRestart::hasRestart()) {
		core::ApplicationManager::requestExit();
		result = false;
	}

	if (result) {
		tracy_zone_scoped_with_name("OnUpdate-Event");

		int window_active_changed = m_window_active_changed.exchange(0);
		if (window_active_changed & 0x2)
		{
			if (m_DirectInput)
				m_DirectInput->reset();

			lua_pushinteger(L, (lua_Integer)LuaEngine::EngineEvent::WindowActive);
			lua_pushboolean(L, false);
			SafeCallGlobalFunctionB(LuaEngine::G_CALLBACK_EngineEvent, 2, 0);

			if (!SafeCallGlobalFunction(LuaEngine::G_CALLBACK_FocusLoseFunc))
			{
				result = false;
				core::ApplicationManager::requestExit();
			}
		}
		if (window_active_changed & 0x1)
		{
			if (m_DirectInput)
				m_DirectInput->reset();

			lua_pushinteger(L, (lua_Integer)LuaEngine::EngineEvent::WindowActive);
			lua_pushboolean(L, true);
			SafeCallGlobalFunctionB(LuaEngine::G_CALLBACK_EngineEvent, 2, 0);

			if (!SafeCallGlobalFunction(LuaEngine::G_CALLBACK_FocusGainFunc))
			{
				result = false;
				core::ApplicationManager::requestExit();
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

	if (result) {
		tracy_zone_scoped_with_name("OnUpdate-LuaCallback");
		// 执行帧函数
		imgui::cancelSetCursor();
		m_GameObjectPool->DebugNextFrame();
		if (!SafeCallGlobalFunction(LuaEngine::G_CALLBACK_EngineUpdate, 1))
		{
			result = false;
			core::ApplicationManager::requestExit();
		}
		bool tAbort = lua_toboolean(L, -1) != 0;
		lua_pop(L, 1);
		if (tAbort)
			core::ApplicationManager::requestExit();
		m_ResourceMgr.UpdateSound();
	}

	// check again after FrameFunc
	if (ApplicationRestart::hasRestart()) {
		core::ApplicationManager::requestExit();
		result = false;
	}

	return result;
}
bool AppFrame::onRenderInternal()
{
	m_bRenderStarted = true;

	GetRenderTargetManager()->BeginRenderTargetStack();

	// 执行渲染函数
	bool result = SafeCallGlobalFunction(LuaEngine::G_CALLBACK_EngineDraw);
	if (!result)
		core::ApplicationManager::requestExit();

	GetRenderTargetManager()->EndRenderTargetStack();

	m_bRenderStarted = false;
	return result;
}

#pragma endregion
