#include <string>
#include <fstream>
#include "resource.h"
#include "AppFrame.h"
#include "Utility.h"
#include "ImGuiExtension.h"
#include "LuaWrapper/LuaAppFrame.hpp"
#include "LConfig.h"
#include "Core/FileManager.hpp"
#include "AdapterPolicy.hpp"
#include "utility/encoding.hpp"
#include "platform/DetectCPU.hpp"

class f2dGraphic2dAdapter
	: public f2dGraphics2D
	, public f2dRenderDeviceEventListener
{
private:
	fcyMatrix4 m_tMat;
	f2dBlendState m_tState = {};
	LuaSTG::Core::Renderer* m_Renderer = nullptr;
	inline void applyTexure(f2dTexture2D* pTex)
	{
		if (pTex)
		{
			m_Renderer->setTextureAlphaType(pTex->IsPremultipliedAlpha() ? LuaSTG::Core::TextureAlphaType::PremulAlpha : LuaSTG::Core::TextureAlphaType::Normal);
			m_Renderer->setTexture(LuaSTG::Core::TextureID(pTex->GetHandle()));
		}
		else
		{
			m_Renderer->setTexture({});
		}
	}
public:
	// 用不上
	void AddRef() {}
	void Release() {}

	fBool IsGraphics3D() { return false; }

	fBool IsInRender() { return true; }
	fResult Begin() { return FCYERR_OK; }
	fResult Flush()
	{
		return m_Renderer->flush() ? FCYERR_OK : FCYERR_INTERNALERR;
	}
	fResult End()
	{
		return Flush();
	}

	// 用不上
	const fcyMatrix4& GetWorldTransform() { return m_tMat; }
	const fcyMatrix4& GetViewTransform() { return m_tMat; }
	const fcyMatrix4& GetProjTransform() { return m_tMat; }

	// 用不上
	void SetWorldTransform(const fcyMatrix4& Mat) {}
	void SetViewTransform(const fcyMatrix4& Mat) {}
	void SetProjTransform(const fcyMatrix4& Mat) { m_tMat = Mat; }

	// 用不上
	const f2dBlendState& GetBlendState() { return m_tState; }
	void SetBlendState(const f2dBlendState& State) { m_tState = State; }

	// 用不上
	F2DGRAPH2DBLENDTYPE GetColorBlendType()
	{
		return F2DGRAPH2DBLENDTYPE_MODULATE;
	}
	fResult SetColorBlendType(F2DGRAPH2DBLENDTYPE Type)
	{
		return FCYERR_OK;
	}

	fResult DrawQuad(f2dTexture2D* pTex, const f2dGraphics2DVertex& v1, const f2dGraphics2DVertex& v2, const f2dGraphics2DVertex& v3, const f2dGraphics2DVertex& v4, fBool bAutoFixCoord = true)
	{
		applyTexure(pTex);
		m_Renderer->drawQuad(
			(LuaSTG::Core::DrawVertex2D const&)(v1),
			(LuaSTG::Core::DrawVertex2D const&)(v2),
			(LuaSTG::Core::DrawVertex2D const&)(v3),
			(LuaSTG::Core::DrawVertex2D const&)(v4));
		return FCYERR_OK;
	}
	fResult DrawQuad(f2dTexture2D* pTex, const f2dGraphics2DVertex* arr, fBool bAutoFixCoord = true)
	{
		applyTexure(pTex);
		m_Renderer->drawQuad((LuaSTG::Core::DrawVertex2D*)arr);
		return FCYERR_OK;
	}
	fResult DrawRaw(f2dTexture2D* pTex, fuInt VertCount, fuInt IndexCount, const f2dGraphics2DVertex* VertArr, const fuShort* IndexArr, fBool bAutoFixCoord = true)
	{
		applyTexure(pTex);
		m_Renderer->drawRaw((LuaSTG::Core::DrawVertex2D*)VertArr, (uint16_t)VertCount, (LuaSTG::Core::DrawIndex2D*)IndexArr, (uint16_t)IndexCount);
		return FCYERR_OK;
	}
	
	void OnRenderDeviceLost()
	{
		m_Renderer->detachDevice();
	}
	void OnRenderDeviceReset()
	{
		m_Renderer->attachDevice(LAPP.GetRenderDev()->GetHandle());
	}
	
	f2dGraphic2dAdapter() : m_Renderer(nullptr) {}
	f2dGraphic2dAdapter(LuaSTG::Core::Renderer* r2d) : m_Renderer(r2d) {}
public:
	static f2dGraphic2dAdapter& get()
	{
		static f2dGraphic2dAdapter i;
		return i;
	}
};

using namespace LuaSTGPlus;

////////////////////////////////////////////////////////////////////////////////
/// AppFrame
////////////////////////////////////////////////////////////////////////////////

LNOINLINE AppFrame& AppFrame::GetInstance()
{
	static AppFrame s_Instance;
	return s_Instance;
}
AppFrame::AppFrame() LNOEXCEPT
{
}
AppFrame::~AppFrame() LNOEXCEPT
{
	if (m_iStatus != AppStatus::NotInitialized && m_iStatus != AppStatus::Destroyed)
	{
		// 若没有销毁框架，则执行销毁
		Shutdown();
	}
}

#pragma region 脚本接口

void AppFrame::SetWindowed(bool v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionWindowed = v;
	else if (m_iStatus == AppStatus::Running)
		spdlog::warn("[luastg] SetWindowed: 试图在运行时更改窗口化模式");
}
void AppFrame::SetFPS(fuInt v)LNOEXCEPT
{
	m_OptionFPSLimit = (v > 1u) ? v : 1u; // 最低也得有1FPS每秒
}
void AppFrame::SetVsync(bool v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionVsync = v;
	else if (m_iStatus == AppStatus::Running)
		spdlog::warn("[luastg] SetVsync: 试图在运行时更改垂直同步模式");
}
void AppFrame::SetResolution(fuInt width, fuInt height, fuInt A, fuInt B)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
	{
		m_OptionResolution.Set((float)width, (float)height);
		m_OptionRefreshRateA = A;
		m_OptionRefreshRateB = B;
	}
	else if (m_iStatus == AppStatus::Running)
		spdlog::warn("[luastg] SetResolution: 试图在运行时更改分辨率");
}
void AppFrame::SetSEVolume(float v)
{
	if (m_pSoundSys)
		m_pSoundSys->SetSoundEffectChannelVolume(v);
	else
		m_gSEVol = v;
}
void AppFrame::SetBGMVolume(float v)
{
	if (m_pSoundSys)
		m_pSoundSys->SetMusicChannelVolume(v);
	else
		m_gBGMVol = v;
}
void AppFrame::SetTitle(const char* v)LNOEXCEPT
{
	try
	{
		m_OptionTitle = v;
		if (m_pMainWindow)
			m_pMainWindow->SetCaption(v);
	}
	catch (const std::bad_alloc&)
	{
		spdlog::error("[luastg] SetTitle: 内存不足");
	}
}
void AppFrame::SetPreferenceGPU(const char* v, bool dGPU_trick)LNOEXCEPT
{
	try
	{
		m_OptionGPU = std::move(utility::encoding::to_wide(v));
		SetAdapterPolicy(dGPU_trick);
	}
	catch (const std::bad_alloc&)
	{
		spdlog::error("[luastg] SetPreferenceGPU: 内存不足");
	}
}
void AppFrame::SetSplash(bool v)LNOEXCEPT
{
	m_OptionCursor = v;
	if (m_pMainWindow)
		m_pMainWindow->HideMouse(!m_OptionCursor);
}

LNOINLINE bool AppFrame::ChangeVideoMode(int width, int height, bool windowed, bool vsync)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initialized)
	{
		if (windowed)
		{
			bool bResult = true;
			if (FCYOK(m_pRenderDev->SetBufferSize((fuInt)width, (fuInt)height, windowed, vsync, false, F2DAALEVEL_NONE)))
			{
				spdlog::info("[luastg] 显示模式切换成功 ({}x{} Vsync:{} Windowed:{}) -> ({}x{} Vsync:{} Windowed:{})",
					(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed,
					width, height, vsync, windowed);
			}
			else
			{
				spdlog::error("[luastg] 显示模式切换失败 ({}x{} Vsync:{} Windowed:{}) -> ({}x{} Vsync:{} Windowed:{})",
					(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed,
					width, height, vsync, windowed);
				bResult = false;
			}
			
			m_pMainWindow->SetBorderType(m_OptionWindowStyle);
			m_pMainWindow->SetClientRect(fcyRect(0.0f, 0.0f, (fFloat)width, (fFloat)height));
			m_pMainWindow->SetTopMost(false); // 强制取消窗口置顶
			m_pMainWindow->MoveToCenter();
			
			m_OptionResolution.Set((fFloat)width, (fFloat)height);
			m_OptionWindowed = windowed;
			m_OptionVsync = vsync;
			
			return bResult;
		}
		else
		{
			m_pMainWindow->SetBorderType(F2DWINBORDERTYPE_NONE);
			m_pMainWindow->SetClientRect(fcyRect(0.0f, 0.0f, (fFloat)width, (fFloat)height));
			//m_pMainWindow->SetTopMost(true);
			//m_pMainWindow->MoveToCenter();
			
			if (FCYOK(m_pRenderDev->SetBufferSize((fuInt)width, (fuInt)height, windowed, vsync, false, F2DAALEVEL_NONE)))
			{
				spdlog::info("[luastg] 显示模式切换成功 ({}x{} Vsync:{} Windowed:{}) -> ({}x{} Vsync:{} Windowed:{})",
					(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed,
					width, height, vsync, windowed);
				
				m_OptionResolution.Set((fFloat)width, (fFloat)height);
				m_OptionWindowed = windowed;
				m_OptionVsync = vsync;
				
				return true;
			}
			else
			{
				spdlog::error("[luastg] 显示模式切换失败 ({}x{} Vsync:{} Windowed:{}) -> ({}x{} Vsync:{} Windowed:{})",
					(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed,
					width, height, vsync, windowed);
				
				windowed = true; // 强制窗口化
				m_pRenderDev->SetBufferSize((fuInt)width, (fuInt)height, true, vsync, false, F2DAALEVEL_NONE); // 出错也不用管了
				
				m_pMainWindow->SetBorderType(m_OptionWindowStyle);
				m_pMainWindow->SetClientRect(fcyRect(0.0f, 0.0f, (fFloat)width, (fFloat)height));
				m_pMainWindow->SetTopMost(false); // 强制取消窗口置顶
				m_pMainWindow->MoveToCenter();
				
				m_OptionResolution.Set((fFloat)width, (fFloat)height);
				m_OptionWindowed = windowed;
				m_OptionVsync = vsync;
				
				return false;
			}
		}
	}
	return false;
}
LNOINLINE bool AppFrame::ChangeVideoMode2(int width, int height, bool windowed, bool vsync, int hza, int hzb, bool flip)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initialized)
	{
		if (windowed)
		{
			if (m_OptionWindowed)
			{
				// 窗口模式下，先改变窗口设置再修改交换链
				m_pMainWindow->SetBorderType(m_OptionWindowStyle);
				m_pMainWindow->SetClientRect(fcyRect(0.0f, 0.0f, (fFloat)width, (fFloat)height));
				m_pMainWindow->SetTopMost(false); // 强制取消窗口置顶
				m_pMainWindow->MoveToCenter();
			}
			
			bool bResult = true;
			if (FCYOK(m_pRenderDev->SetDisplayMode((fuInt)width, (fuInt)height, (fuInt)hza, (fuInt)hzb, windowed, vsync, flip)))
			{
				spdlog::info("[luastg] 显示模式切换成功 ({}x{} Vsync:{} Windowed:{}) -> ({}x{} Vsync:{} Windowed:{})",
					(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed,
					width, height, vsync, windowed);
			}
			else
			{
				spdlog::error("[luastg] 显示模式切换失败 ({}x{} Vsync:{} Windowed:{}) -> ({}x{} Vsync:{} Windowed:{})",
					(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed,
					width, height, vsync, windowed);
				bResult = false;
			}
			
			if (!m_OptionWindowed)
			{
				// 全屏模式切回窗口模式，先修改交换链再更新窗口，避免出现winxp、win7风格窗口
				m_pMainWindow->SetBorderType(m_OptionWindowStyle);
				m_pMainWindow->SetClientRect(fcyRect(0.0f, 0.0f, (fFloat)width, (fFloat)height));
				m_pMainWindow->SetTopMost(false); // 强制取消窗口置顶
				m_pMainWindow->MoveToCenter();
			}
			
			m_OptionResolution.Set((fFloat)width, (fFloat)height);
			m_OptionWindowed = windowed;
			m_OptionVsync = vsync;
			
			return bResult;
		}
		else
		{
			m_pMainWindow->SetBorderType(F2DWINBORDERTYPE_NONE);
			m_pMainWindow->SetClientRect(fcyRect(0.0f, 0.0f, (fFloat)width, (fFloat)height));
			//m_pMainWindow->SetTopMost(true);
			//m_pMainWindow->MoveToCenter();
			
			if (FCYOK(m_pRenderDev->SetDisplayMode((fuInt)width, (fuInt)height, (fuInt)hza, (fuInt)hzb, windowed, vsync, flip)))
			{
				spdlog::info("[luastg] 显示模式切换成功 ({}x{} Vsync:{} Windowed:{}) -> ({}x{}@{} Vsync:{} Windowed:{})",
					(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed,
					width, height, (float)hza / (float)hzb, vsync, windowed);
				
				m_OptionResolution.Set((fFloat)width, (fFloat)height);
				m_OptionWindowed = windowed;
				m_OptionVsync = vsync;
				
				return true;
			}
			else
			{
				spdlog::error("[luastg] 显示模式切换失败 ({}x{} Vsync:{} Windowed:{}) -> ({}x{}@{} Vsync:{} Windowed:{})",
					(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed,
					width, height, (float)hza / (float)hzb, vsync, windowed);
				
				windowed = true; // 强制窗口化
				m_pRenderDev->SetDisplayMode((fuInt)width, (fuInt)height, (fuInt)hza, (fuInt)hzb, windowed, vsync, flip); // 出错也不用管了
				
				m_pMainWindow->SetBorderType(m_OptionWindowStyle);
				m_pMainWindow->SetClientRect(fcyRect(0.0f, 0.0f, (fFloat)width, (fFloat)height));
				m_pMainWindow->SetTopMost(false); // 强制取消窗口置顶
				m_pMainWindow->MoveToCenter();
				
				m_OptionResolution.Set((fFloat)width, (fFloat)height);
				m_OptionWindowed = windowed;
				m_OptionVsync = vsync;
				
				return false;
			}
		}
	}
	return false;
}
LNOINLINE bool AppFrame::UpdateVideoMode()LNOEXCEPT
{
	return ChangeVideoMode(m_OptionResolution.x, m_OptionResolution.y, m_OptionWindowed, m_OptionVsync);
}

LNOINLINE int AppFrame::LoadTextFile(lua_State* L, const char* path, const char *packname)LNOEXCEPT
{
	if (ResourceMgr::GetResourceLoadingLog()) {
		if (packname)
			spdlog::info("[luastg] 在资源包'{}'中读取文本文件'{}'", packname, path);
		else
			spdlog::info("[luastg] 读取文本文件'{}'", path);
	}
	bool loaded = false;
	fcyRefPointer<fcyMemStream> tMemStream;
	if (packname)
	{
		auto& arc = GFileManager().getFileArchive(packname);
		if (!arc.empty())
		{
			loaded = arc.load(path, ~tMemStream);
		}
	}
	else
	{
		loaded = GFileManager().loadEx(path, ~tMemStream);
	}
	if (!loaded) {
		spdlog::error("[luastg] 无法加载文件'{}'", path);
		return 0;
	}
	lua_pushlstring(L, (char*)tMemStream->GetInternalBuffer(), (size_t)tMemStream->GetLength());
	tMemStream = nullptr;
	return 1;
}

#pragma endregion

#pragma region 框架函数

bool AppFrame::Init()LNOEXCEPT
{
	assert(m_iStatus == AppStatus::NotInitialized);
	
	spdlog::info(LUASTG_INFO);
	spdlog::info("[luastg] 初始化引擎");
	m_iStatus = AppStatus::Initializing;
	
	////////////////////////////////////////

	if (!LuaSTG::Core::IApplicationModel::create(this, ~m_pAppModel))
		return false;
	
	//////////////////////////////////////// Lua初始化部分
	
	spdlog::info("[luastg] 初始化luajit引擎");
	
	// 开启Lua引擎
	if (!OnOpenLuaEngine())
	{
		spdlog::info("[luastg] 初始化luajit引擎失败");
		return false;
	}
	
	// 加载初始化脚本（可选）
	if (!OnLoadLaunchScriptAndFiles())
	{
		return false;
	}
	
	//////////////////////////////////////// 初始化引擎
	{
		// 配置窗口
		{
			using namespace LuaSTG::Core;
			auto* p_window = m_pAppModel->getWindow();
			p_window->setSize(Vector2I(m_OptionResolution.x, m_OptionResolution.y));
			if (m_OptionWindowed)
				p_window->setFrameStyle(Graphics::WindowFrameStyle::Fixed);
			else
				p_window->setFrameStyle(Graphics::WindowFrameStyle::None);
			p_window->setTitleText(m_OptionTitle);
			p_window->setNativeIcon((void*)(ptrdiff_t)IDI_APPICON);
			p_window->setCursor(m_OptionCursor ? Graphics::WindowCursor::Arrow : Graphics::WindowCursor::None);
		}

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
		
		// 初始化fancy2d引擎
		spdlog::info("[fancy2d] 初始化，窗口分辨率：{}x{}，垂直同步：{}，窗口化：{}",
			(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed);
		
		struct : public f2dInitialErrListener
		{
			void OnErr(fuInt TimeTick, fcStr Src, fcStr Desc)
			{
				spdlog::error("[fancy2d] [{}] {}", Src, Desc);
			}
		} tErrListener;
		
		f2dEngineRenderWindowParam render_window_def = {
			.title = L"",
			.windowed = m_OptionWindowed,
			.vsync = m_OptionVsync,
			.mode = f2dDisplayMode{
				.width = (fuInt)m_OptionResolution.x,
				.height = (fuInt)m_OptionResolution.y,
				.refresh_rate = f2dRational{
					.numerator = m_OptionRefreshRateA,
					.denominator = m_OptionRefreshRateB,
				},
				.format = 0, // 让引擎自动决定
				.scanline_ordering = 0, // 让引擎自动决定
				.scaling = 0, // 让引擎自动决定
			},
			.gpu = m_OptionGPU.c_str(),
			.appmodel = *m_pAppModel,
		};
		if (FCYFAILED(CreateF2DEngineAndInit(
			F2DVERSION,
			&render_window_def,
			~m_pEngine,
			&tErrListener
			)))
		{
			spdlog::error("[fancy2d] 初始化失败");
			return false;
		}
		
		// 获取组件
		m_pMainWindow = m_pEngine->GetMainWindow();
		m_pRenderer = m_pEngine->GetRenderer();
		m_pRenderDev = m_pRenderer->GetDevice();
		m_pSoundSys = m_pEngine->GetSoundSys();
		
		// 配置音量
		m_pSoundSys->SetSoundEffectChannelVolume(m_gSEVol);
		m_pSoundSys->SetMusicChannelVolume(m_gBGMVol);

		// 打印设备信息
		spdlog::info("[fancy2d] CPU {} {}", InstructionSet::Vendor(), InstructionSet::Brand());
		spdlog::info("[fancy2d] GPU {}", m_pRenderDev->GetDeviceName());
		
		// 渲染器
		spdlog::info("[luastg] 创建2D渲染器");
		if (!m_NewRenderer2D.attachDevice(m_pRenderDev->GetHandle()))
		{
			spdlog::info("[luastg] 创建2D渲染器失败");
			return false;
		}
		// 渲染器适配器
		spdlog::info("[luastg] 创建2D渲染器适配器");
		f2dGraphic2dAdapter::get() = f2dGraphic2dAdapter(&m_NewRenderer2D);
		m_Graph2D = &f2dGraphic2dAdapter::get();
		m_pRenderDev->AttachListener(&f2dGraphic2dAdapter::get());
		m_bRenderStarted = false;
		
		// 创建文字渲染器
		spdlog::info("[fancy2d] 创建文本渲染器");
		if (FCYFAILED(m_pRenderer->CreateFontRenderer(nullptr, ~m_FontRenderer)))
		{
			spdlog::error("[fancy2d] [fcyRenderer::CreateFontRenderer] 创建文本渲染器失败");
			return false;
		}
		m_FontRenderer->SetZ(0.5f);
		
		// 创建图元渲染器
		spdlog::info("[fancy2d] 创建平面几何渲染器");
		if (FCYFAILED(m_pRenderer->CreateGeometryRenderer(~m_GRenderer)))
		{
			spdlog::error("[fancy2d] [fcyRenderer::CreateGeometryRenderer] 创建平面几何渲染器失败");
			return false;
		}
		
		OpenInput();

		// 创建手柄输入
		try
		{
			m_DirectInput = std::make_unique<platform::DirectInput>((ptrdiff_t)m_pMainWindow->GetHandle());
			{
				m_DirectInput->refresh(); // 这里因为窗口还没显示，所以应该会出现一个Aquire设备失败的错误信息，忽略即可
				uint32_t cnt = m_DirectInput->count();
				for (uint32_t i = 0; i < cnt; i += 1)
				{
					spdlog::info("[luastg] 检测到 {} 控制器 产品名称：{} 设备名称：{}",
						m_DirectInput->isXInputDevice(i) ? "XInput" : "DirectInput",
						utility::encoding::to_utf8(m_DirectInput->getProductName(i)),
						utility::encoding::to_utf8(m_DirectInput->getDeviceName(i))
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
		
		// 显示窗口
		{
			using namespace LuaSTG::Core;
			auto* p_window = m_pAppModel->getWindow();
			if (m_OptionWindowed)
				p_window->setCentered();
			p_window->setLayer(Graphics::WindowLayer::Normal);
		}
		// 启动交换链
		{
			using namespace LuaSTG::Core;
			auto* p_swapchain = m_pAppModel->getSwapChain();
			p_swapchain->setVSync(m_OptionVsync);
			if (m_OptionWindowed)
			{
				p_swapchain->setWindowMode(m_OptionResolution.x, m_OptionResolution.y, false);
			}
			else
			{
				Graphics::DisplayMode mode = {
					.width = (uint32_t)m_OptionResolution.x,
					.height = (uint32_t)m_OptionResolution.y,
					.refresh_rate = Rational{
						.numerator = m_OptionRefreshRateA,
						.denominator = m_OptionRefreshRateB,
					},
					.format = Graphics::Format::B8G8R8A8_UNORM, // 未使用
				};
				p_swapchain->findBestMatchDisplayMode(mode);
				p_swapchain->setExclusiveFullscreenMode(mode);
			}
			p_swapchain->refreshDisplayMode();
		}
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
void AppFrame::Shutdown()LNOEXCEPT
{
	SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_EngineStop);
	
	m_GameObjectPool = nullptr;
	spdlog::info("[luastg] 清空对象池");

	m_ResourceMgr.ClearAllResource();
	spdlog::info("[luastg] 清空所有游戏资源");
	
	// 卸载ImGui
	#ifdef USING_DEAR_IMGUI
		imgui::unbindEngine();
	#endif
	
	// 关闭渲染器
	m_pRenderDev->RemoveListener(&f2dGraphic2dAdapter::get());
	m_NewRenderer2D.detachDevice();
	
	CloseInput();
	m_DirectInput = nullptr;
	m_GRenderer = nullptr;
	m_FontRenderer = nullptr;
	m_Graph2D = nullptr;
	m_pSoundSys = nullptr;
	m_pRenderDev = nullptr;
	m_pRenderer = nullptr;
	m_pMainWindow = nullptr;
	spdlog::info("[fancy2d] 卸载所有组件");
	
	if (L)
	{
		lua_close(L);
		L = nullptr;
		spdlog::info("[luastg] 关闭luajit引擎");
	}
	
	GFileManager().unloadAllFileArchive();
	spdlog::info("[luastg] 卸载所有资源包");
	
	m_pEngine = nullptr;

	m_pAppModel = nullptr;

	m_iStatus = AppStatus::Destroyed;
	spdlog::info("[luastg] 引擎关闭");
}
void AppFrame::Run()LNOEXCEPT
{
	assert(m_iStatus == AppStatus::Initialized);
	spdlog::info("[luastg] 开始更新&渲染循环");
	
	m_pAppModel->getWindow()->addEventListener(this);

	m_pAppModel->getFrameRateController()->setTargetFPS(m_OptionFPSLimit);
	m_pAppModel->run();
	
	m_pAppModel->getWindow()->removeEventListener(this);

	spdlog::info("[luastg] 结束更新&渲染循环");
}

#pragma endregion

#pragma region 游戏循环

void AppFrame::onWindowActive()
{
	m_window_active_changed.fetch_or(0x1);
}
void AppFrame::onWindowInactive()
{
	m_window_active_changed.fetch_or(0x2);
}
void AppFrame::onDeviceChange()
{
	m_window_active_changed.fetch_or(0x4);
}

void AppFrame::onUpdate()
{
	m_fFPS = m_pAppModel->getFrameRateController()->getFPS();
	m_fAvgFPS = m_pAppModel->getFrameRateController()->getAvgFPS();
	m_pAppModel->getFrameRateController()->setTargetFPS(m_OptionFPSLimit);

	int window_active_changed = m_window_active_changed.exchange(0);
	if (window_active_changed & 0x2)
	{
		if (m_DirectInput)
			m_DirectInput->reset();

		lua_pushinteger(L, (lua_Integer)LuaSTG::LuaEngine::EngineEvent::WindowActive);
		lua_pushboolean(L, false);
		SafeCallGlobalFunctionB(LuaSTG::LuaEngine::G_CALLBACK_EngineEvent, 2, 0);

		if (!SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_FocusLoseFunc))
			m_pAppModel->requestExit();
	}
	if (window_active_changed & 0x1)
	{
		if (m_DirectInput)
			m_DirectInput->reset();

		lua_pushinteger(L, (lua_Integer)LuaSTG::LuaEngine::EngineEvent::WindowActive);
		lua_pushboolean(L, true);
		SafeCallGlobalFunctionB(LuaSTG::LuaEngine::G_CALLBACK_EngineEvent, 2, 0);

		if (!SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_FocusGainFunc))
			m_pAppModel->requestExit();
	}
	if (window_active_changed & 0x4)
	{
		if (m_DirectInput)
			m_DirectInput->refresh();
	}

	UpdateInput();
	
#ifdef USING_CTRL_ENTER_SWITCH
	if (WantSwitchFullScreenMode())
	{
		ChangeVideoMode((int)m_OptionResolution.x, (int)m_OptionResolution.y, !m_OptionWindowed, m_OptionVsync);
	}
#endif

#if (defined(_DEBUG) && defined(LuaSTG_enable_GameObjectManager_Debug))
	static uint64_t _frame_count = 0;
	spdlog::debug("[frame] ---------- {} ----------", _frame_count);
	_frame_count += 1;
#endif

	// 执行帧函数
	if (!SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_EngineUpdate, 1))
		m_pAppModel->requestExit();
	bool tAbort = lua_toboolean(L, -1) == 0 ? false : true;
	lua_pop(L, 1);
	if (tAbort)
		m_pAppModel->requestExit();
}
void AppFrame::onRender()
{
	// 执行渲染函数
	m_bRenderStarted = true;
	if (!SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_EngineDraw))
		m_pAppModel->requestExit();
	m_bRenderStarted = false;
	// 发出警告
	if (!m_stRenderTargetStack.empty() || !m_stDepthStencilStack.empty())
	{
		spdlog::error("[luastg] [AppFrame::OnRender] 渲染结束时 RenderTarget 栈不为空，可能缺少对 lstg.PopRenderTarget 的调用");
		while (!m_stRenderTargetStack.empty() || !m_stDepthStencilStack.empty())
			PopRenderTarget();
	}
}

#pragma endregion
