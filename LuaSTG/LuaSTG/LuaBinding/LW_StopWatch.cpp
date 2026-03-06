#include "LuaBinding/LuaWrapper.hpp"
#include "windows/CleanWindows.hpp"

////////////////////////////////////////////////////////////////////////////////
/// @brief 高精度停表类
////////////////////////////////////////////////////////////////////////////////
class fcyStopWatch
{
private:
	int64_t m_cFreq;      ///< @brief CPU频率
	int64_t m_cLast;      ///< @brief 上一次时间
	int64_t m_cFixStart;  ///< @brief 暂停时的时间修复参数
	int64_t m_cFixAll;    ///< @brief 暂停时的时间修复参数
public:
	void Pause();        ///< @brief 暂停
	void Resume();       ///< @brief 继续
	void Reset();        ///< @brief 归零
	double GetElapsed(); ///< @brief 获得流逝时间
	///< @note  以秒为单位
public:
	fcyStopWatch();
	~fcyStopWatch();
};

fcyStopWatch::fcyStopWatch(void)
{
	LARGE_INTEGER freq = {};
	QueryPerformanceFrequency(&freq); // 初始化
	m_cFreq = freq.QuadPart;
	Reset();
}

fcyStopWatch::~fcyStopWatch(void)
{
}

void fcyStopWatch::Pause()
{
	LARGE_INTEGER t = {};
	QueryPerformanceCounter(&t);
	m_cFixStart = t.QuadPart;
}

void fcyStopWatch::Resume()
{
	LARGE_INTEGER t = {};
	QueryPerformanceCounter(&t);
	m_cFixAll += t.QuadPart - m_cFixStart;
}

void fcyStopWatch::Reset()
{
	LARGE_INTEGER t = {};
	QueryPerformanceCounter(&t);
	m_cLast = t.QuadPart;
	m_cFixAll = 0;
}

double fcyStopWatch::GetElapsed()
{
	LARGE_INTEGER t = {};
	QueryPerformanceCounter(&t);
	return ((double)(t.QuadPart - m_cLast - m_cFixAll)) / ((double)m_cFreq);
}

namespace luastg::binding
{
	void StopWatch::Register(lua_State* L)noexcept
	{
		struct Function
		{
		#define GETUDATA(p, i) fcyStopWatch* (p) = static_cast<fcyStopWatch*>(luaL_checkudata(L, (i), LUASTG_LUA_TYPENAME_STOPWATCH));
			static int Reset(lua_State* L)
			{
				GETUDATA(p, 1);
				p->Reset();
				return 1;
			}
			static int Pause(lua_State* L)
			{
				GETUDATA(p, 1);
				p->Pause();
				return 1;
			}
			static int Resume(lua_State* L)
			{
				GETUDATA(p, 1);
				p->Resume();
				return 1;
			}
			static int GetElapsed(lua_State* L)
			{
				GETUDATA(p, 1);
				lua_pushnumber(L, (lua_Number)p->GetElapsed());
				return 1;
			}
			static int Meta_ToString(lua_State* L)noexcept
			{
				::lua_pushfstring(L, LUASTG_LUA_TYPENAME_STOPWATCH);
				return 1;
			}
		#undef GETUDATA
		};

		luaL_Reg tMethods[] =
		{
			{ "Reset", &Function::Reset },
			{ "Pause", &Function::Pause },
			{ "Resume", &Function::Resume },
			{ "GetElapsed", &Function::GetElapsed },
			{ NULL, NULL }
		};
		luaL_Reg tMetaTable[] =
		{
			{ "__tostring", &Function::Meta_ToString },
			{ NULL, NULL }
		};

		RegisterClassIntoTable(L, ".StopWatch", tMethods, LUASTG_LUA_TYPENAME_STOPWATCH, tMetaTable);
	}

	void StopWatch::CreateAndPush(lua_State* L)
	{
		fcyStopWatch* p = static_cast<fcyStopWatch*>(lua_newuserdata(L, sizeof(fcyStopWatch))); // udata
		new(p) fcyStopWatch();
		luaL_getmetatable(L, LUASTG_LUA_TYPENAME_STOPWATCH); // udata mt
		lua_setmetatable(L, -2); // udata
	}
}
