#include "Global.h"
#include "ResourcePassword.hpp"
#include "LuaWrapper\LuaWrapper.hpp"
#include "AppFrame.h"
#include <filesystem>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef LoadImage
#undef LoadImage
#endif

#ifdef PlaySound
#undef PlaySound
#endif

using namespace std;
using namespace LuaSTGPlus;

void BuiltInFunctionWrapper::Register(lua_State* L)LNOEXCEPT
{
	//警告：函数的返回值为向lua栈推入的变量数量
	
	struct WrapperImplement
	{
		#pragma region 框架函数
		// 框架函数
		static int SetWindowed(lua_State* L)LNOEXCEPT
		{
			LAPP.SetWindowed(lua_toboolean(L, 1) == 0 ? false : true);
			return 0;
		}
		static int SetVsync(lua_State* L)LNOEXCEPT
		{
			LAPP.SetVsync(lua_toboolean(L, 1) == 0 ? false : true);
			return 0;
		}
		static int SetResolution(lua_State* L)LNOEXCEPT
		{
			LAPP.SetResolution(static_cast<fuInt>(::max((int)luaL_checkinteger(L, 1), 0)),
			static_cast<fuInt>(::max((int)luaL_checkinteger(L, 2), 0)));
			return 0;
		}
		static int SetFPS(lua_State* L)LNOEXCEPT
		{
			int v = luaL_checkinteger(L, 1);
			if (v <= 0)
				v = 60;
			LAPP.SetFPS(static_cast<fuInt>(v));
			return 0;
		}
		static int GetFPS(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, LAPP.GetFPS());
			return 1;
		}
		static int Log(lua_State* L)LNOEXCEPT
		{
			lua_Integer _level = luaL_checkinteger(L, 1);
			LuaSTGPlus::LogType level = LuaSTGPlus::LogType::Information;
			switch (_level)
			{
			case 1:
				level = LuaSTGPlus::LogType::Debug;
				break;
			case 2:
				level = LuaSTGPlus::LogType::Information;
				break;
			case 3:
				level = LuaSTGPlus::LogType::Warning;
				break;
			case 4:
				level = LuaSTGPlus::LogType::Error;
				break;
			case 5:
				level = LuaSTGPlus::LogType::Fatal;
				break;
			default:
				level = LuaSTGPlus::LogType::Information;
				break;
			}
			try {
				std::wstring msg = fcyStringHelper::MultiByteToWideChar(luaL_checkstring(L, 2), CP_UTF8);
				LLOGGER.Log(level, msg.c_str());
			}
			catch (...) {
				return luaL_error(L, "Failed to write log message.");
			}
			return 0;
		}
		static int SystemLog(lua_State* L)LNOEXCEPT
		{
			LINFO("脚本日志：%m", luaL_checkstring(L, 1));
			return 0;
		}
		static int Print(lua_State* L)LNOEXCEPT
		{
			int n = lua_gettop(L);
			lua_getglobal(L, "tostring"); // ... f
			lua_pushstring(L, ""); // ... f s
			for (int i = 1; i <= n; i++)
			{
				if (i > 1)
				{
					lua_pushstring(L, "\t"); // ... f s s
					lua_concat(L, 2); // ... f s
				}
				lua_pushvalue(L, -2); // ... f s f
				lua_pushvalue(L, i); // ... f s f arg[i]
				lua_call(L, 1, 1); // ... f s ret
				const char* x = luaL_checkstring(L, -1);
				lua_concat(L, 2); // ... f s
			}
			LINFO("脚本日志：%m", luaL_checkstring(L, -1));
			//lua_pop(L, 2);
			return 1;
		}
		static int LoadPack(lua_State* L)LNOEXCEPT
		{
			const char* p = luaL_checkstring(L, 1);
			const char* pwd = nullptr;
			if (lua_isstring(L, 2))
				pwd = luaL_checkstring(L, 2);
			if (!LFMGR.LoadArchive(p, 0, pwd))
				LWARNING("无法装载资源包'%m'，文件不存在或不是合法的资源包格式", p);
			return 0;
		}
		static int LoadPackSub(lua_State* L)LNOEXCEPT
		{
			const char* p = luaL_checkstring(L, 1);
			std::string pw = LuaSTGPlus::GetGameName();
			if (!LFMGR.LoadArchive(p, 0, pw.c_str()))
				LWARNING("无法装载资源包'%m'，文件不存在或不是合法的资源包格式", p);
			return 0;
		}
		static int UnloadPack(lua_State* L)LNOEXCEPT
		{
			const char* p = luaL_checkstring(L, 1);
			LFMGR.UnloadArchive(p);
			return 0;
		}
		static int ExtractRes(lua_State* L)LNOEXCEPT
		{
			const char* pArgPath = luaL_checkstring(L, 1);
			const char* pArgTarget = luaL_checkstring(L, 2);
			if (!LRES.ExtractRes(pArgPath, pArgTarget))
				return luaL_error(L, "failed to extract resource '%s' to '%s'.", pArgPath, pArgTarget);
			return 0;
		}
		static int DoFile(lua_State* L)LNOEXCEPT
		{
			int args = lua_gettop(L);//获取此时栈上的值的数量
			LAPP.LoadScript(luaL_checkstring(L, 1),luaL_optstring(L,2,NULL));
			return (lua_gettop(L)- args);
		}
		static int LoadTextFile(lua_State* L)LNOEXCEPT
		{
			return LAPP.LoadTextFile(L, luaL_checkstring(L, 1), luaL_optstring(L, 2, NULL));
		}
		static int FindFiles(lua_State* L)LNOEXCEPT
		{
			// searchpath extendname packname
			LRES.FindFiles(L, luaL_checkstring(L, 1), luaL_optstring(L, 2, ""), luaL_optstring(L, 3, ""));
			return 1;
		}
		#pragma endregion
		
		#pragma region 窗口与交换链控制函数
		// 窗口与交换链控制函数
		static int SetSplash(lua_State* L)LNOEXCEPT
		{
			LAPP.SetSplash(lua_toboolean(L, 1) == 0 ? false : true);
			return 0;
		}
		static int SetTitle(lua_State* L)LNOEXCEPT
		{
			LAPP.SetTitle(luaL_checkstring(L, 1));
			return 0;
		}
		static int ChangeVideoMode(lua_State* L)LNOEXCEPT
		{
			lua_pushboolean(L, LAPP.ChangeVideoMode(
				luaL_checkinteger(L, 1),
				luaL_checkinteger(L, 2),
				lua_toboolean(L, 3) == 0 ? false : true,
				lua_toboolean(L, 4) == 0 ? false : true
			));
			return 1;
		}
		static int MoveWindowToCenter(lua_State* L)LNOEXCEPT
		{
			LAPP.GetWindow()->MoveToCenter();
			return 0;
		}
		static int SetWindowStyle(lua_State* L)LNOEXCEPT
		{
			LAPP.GetWindow()->SetBorderType(
				(F2DWINBORDERTYPE)luaL_checkinteger(L, 1));
			return 0;
		}
		static int SetWindowSize(lua_State* L)LNOEXCEPT
		{
			fResult result = LAPP.GetWindow()->SetClientRect(
				fcyRect(
					0.0f, 0.0f,
					(float)luaL_checkinteger(L, 1),
					(float)luaL_checkinteger(L, 2)));
			lua_pushboolean(L, result == FCYERR_OK);
			return 1;
		}
		static int SetWindowTopMost(lua_State* L)LNOEXCEPT
		{
			LAPP.GetWindow()->SetTopMost(lua_toboolean(L, 1));
			return 0;
		}
		static int SetSwapChainSize(lua_State* L)LNOEXCEPT
		{
			fResult result = LAPP.GetRenderDev()->SetBufferSize(
				(fuInt)luaL_checkinteger(L, 1),
				(fuInt)luaL_checkinteger(L, 2),
				(fBool)lua_toboolean(L, 3),
				(fBool)lua_toboolean(L, 4),
				F2DAALEVEL_NONE);
			lua_pushboolean(L, result == FCYERR_OK);
			return 1;
		}
		static int EnumResolutions(lua_State* L) {
			//返回一个lua表，该表中又包含多个表，分别储存着所支持的屏幕分辨率宽和屏幕分辨率的高，均为整数
			//例如{ {1920,1080}, {1600,900}, ...  }
			if (LAPP.GetRenderDev()) {
				auto count = LAPP.GetRenderDev()->GetSupportResolutionCount();
				lua_createtable(L, count, 0);		// t
				for (auto index = 0; index < count; index++) {
					auto res = LAPP.GetRenderDev()->EnumSupportResolution(index);
					lua_createtable(L, 2, 0);		// t t
					lua_pushinteger(L, res.x);		// t t x
					lua_rawseti(L, -2, 1);			// t t
					lua_pushinteger(L, res.y);		// t t y
					lua_rawseti(L, -2, 2);			// t t
					lua_rawseti(L, -2, index + 1);	// t
				}
				return 1;
			}
			else {
				return luaL_error(L, "The fancy2D Engine is null.");
			}
		}
		#pragma endregion
		
		#pragma region 对象控制函数
		// 对象控制函数（这些方法将被转发到对象池）
		static int GetnObj(lua_State* L)LNOEXCEPT
		{
			lua_pushinteger(L, (lua_Integer)LPOOL.GetObjectCount());
			return 1;
		}
		static int ObjFrame(lua_State* L)LNOEXCEPT
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.DoFrame();
			return 0;
		}
		static int ObjRender(lua_State* L)LNOEXCEPT
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.DoRender();
			return 0;
		}
		static int BoundCheck(lua_State* L)LNOEXCEPT
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.BoundCheck();
			return 0;
		}
		static int SetBound(lua_State* L)LNOEXCEPT
		{
			LPOOL.SetBound(
				luaL_checkinteger(L, 1),
				luaL_checkinteger(L, 2),
				luaL_checkinteger(L, 3),
				luaL_checkinteger(L, 4)
			);
			return 0;
		}
		static int CollisionCheck(lua_State* L)LNOEXCEPT
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.CollisionCheck(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2));
			return 0;
		}
		static int UpdateXY(lua_State* L)LNOEXCEPT
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.UpdateXY();
			return 0;
		}
		static int AfterFrame(lua_State* L)LNOEXCEPT
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.AfterFrame();
			return 0;
		}
		static int ResetObject(lua_State* L)LNOEXCEPT
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "Invalid lstg.GameObject for 'ResetObject'.");
			lua_rawgeti(L, 1, 2);  // t(object) ??? id
			if (!LPOOL.DirtResetObject((size_t)luaL_checkinteger(L, -1)))
				return luaL_error(L, "invalid lstg.GameObject for 'ResetObject'.");
			return 0;
		}
		static int New(lua_State* L)LNOEXCEPT
		{
			return LPOOL.New(L);
		}
		static int Del(lua_State* L)LNOEXCEPT
		{
			return LPOOL.Del(L);
		}
		static int Kill(lua_State* L)LNOEXCEPT
		{
			return LPOOL.Kill(L);
		}
		static int IsValid(lua_State* L)LNOEXCEPT
		{
			return LPOOL.IsValid(L);
		}
		static int Angle(lua_State* L)LNOEXCEPT
		{
			if (lua_gettop(L) == 2)
			{
				if (!lua_istable(L, 1) || !lua_istable(L, 2))
					return luaL_error(L, "invalid lstg object for 'Angle'.");
				lua_rawgeti(L, 1, 2);  // t(object) t(object) ??? id
				lua_rawgeti(L, 2, 2);  // t(object) t(object) ??? id id
				double tRet;
				if (!LPOOL.Angle((size_t)luaL_checkint(L, -2), (size_t)luaL_checkint(L, -1), tRet))
					return luaL_error(L, "invalid lstg object for 'Angle'.");
				lua_pushnumber(L, tRet);
			}
			else if (lua_gettop(L) == 3) {
				// a b c ，此时是未知的
				if (lua_istable(L, 1)) {
					//如果第一个参数是object
					lua_rawgeti(L, 1, 2);  // t(object) x y ??? id
					GameObject* p = LPOOL.GetPooledObject((size_t)luaL_checkint(L, -1));
					if (!p) {
						return luaL_error(L, "invalid lstg object for 'Angle'.");
					}
					lua_pushnumber(L,
						atan2(luaL_checknumber(L, 3) - p->y, luaL_checknumber(L, 2) - p->x) * LRAD2DEGREE
					);
				}
				else if (lua_istable(L, 3)) {
					//如果第三个参数是object
					lua_rawgeti(L, 3, 2);  // x y t(object) ??? id
					GameObject* p = LPOOL.GetPooledObject((size_t)luaL_checkint(L, -1));
					if (!p) {
						return luaL_error(L, "invalid lstg object for 'Angle'.");
					}
					lua_pushnumber(L,
						atan2(p->y - luaL_checknumber(L, 2), p->x - luaL_checknumber(L, 1)) * LRAD2DEGREE
					);
				}
				else {
					return luaL_error(L, "invalid lstg object for 'Angle'.");
				}
			}
			else
			{
				lua_pushnumber(L, 
					atan2(luaL_checknumber(L, 4) - luaL_checknumber(L, 2), luaL_checknumber(L, 3) - luaL_checknumber(L, 1)) * LRAD2DEGREE
				);
			}
			return 1;
		}
		static int ColliCheck(lua_State* L) {
			//对两个对象进行单独的碰撞检测
			// t(object) t(object) ???
			if (!lua_istable(L, 1) || !lua_istable(L, 2))
				return luaL_error(L, "invalid lstg object for 'ColliCheck'.");
			bool ignoreWorldMask = false;
			if (lua_gettop(L) == 3) {
				//有第三个参数则检查第三个参数
				// t(object) t(object) ignoreWorldMask ???
				ignoreWorldMask = lua_toboolean(L, 3) == 0 ? false : true;
			}
			lua_rawgeti(L, 1, 2);
			lua_rawgeti(L, 2, 2);
			bool pass;
			if (!LPOOL.ColliCheck((size_t)luaL_checkint(L, -2), (size_t)luaL_checkint(L, -1), ignoreWorldMask, pass))
				return luaL_error(L, "invalid lstg object for 'ColliCheck'.");
			lua_pushboolean(L, pass);
			return 1;
		}
		static int Dist(lua_State* L)LNOEXCEPT
		{
			if (lua_gettop(L) == 2)
			{
				if (!lua_istable(L, 1) || !lua_istable(L, 2))
					return luaL_error(L, "invalid lstg object for 'Dist'.");
				lua_rawgeti(L, 1, 2);  // t(object) t(object) id
				lua_rawgeti(L, 2, 2);  // t(object) t(object) id id
				double tRet;
				if (!LPOOL.Dist((size_t)luaL_checkint(L, -2), (size_t)luaL_checkint(L, -1), tRet))
					return luaL_error(L, "invalid lstg object for 'Dist'.");
				lua_pushnumber(L, tRet);
			}
			else if (lua_gettop(L) == 3) {
				// a b c ，此时是未知的
				if (lua_istable(L, 1)) {
					//如果第一个参数是object
					lua_rawgeti(L, 1, 2);  // t(object) x y ??? id
					GameObject* p = LPOOL.GetPooledObject((size_t)luaL_checkint(L, -1));
					if (!p) {
						return luaL_error(L, "invalid lstg object for 'Dist'.");
					}
					lua_Number dx = luaL_checknumber(L, 2) - p->x;
					lua_Number dy = luaL_checknumber(L, 3) - p->y;
					lua_pushnumber(L,
						sqrt(dx * dx + dy * dy)
					);
				}
				else if (lua_istable(L, 3)) {
					//如果第三个参数是object
					lua_rawgeti(L, 3, 2);  // x y t(object) ??? id
					GameObject* p = LPOOL.GetPooledObject((size_t)luaL_checkint(L, -1));
					if (!p) {
						return luaL_error(L, "invalid lstg object for 'Dist'.");
					}
					lua_Number dx = p->x - luaL_checknumber(L, 1);
					lua_Number dy = p->y - luaL_checknumber(L, 2);
					lua_pushnumber(L,
						sqrt(dx * dx + dy * dy)
					);
				}
				else {
					return luaL_error(L, "invalid lstg object for 'Dist'.");
				}
			}
			else
			{
				lua_Number dx = luaL_checknumber(L, 3) - luaL_checknumber(L, 1);
				lua_Number dy = luaL_checknumber(L, 4) - luaL_checknumber(L, 2);
				lua_pushnumber(L, sqrt(dx*dx + dy*dy));
			}
			return 1;
		}
		static int GetV(lua_State* L)LNOEXCEPT
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'GetV'.");
			double v, a;
			lua_rawgeti(L, 1, 2);  // t(object) ??? id
			if (!LPOOL.GetV((size_t)luaL_checkinteger(L, -1), v, a))
				return luaL_error(L, "invalid lstg object for 'GetV'.");
			lua_pushnumber(L, v);
			lua_pushnumber(L, a);
			return 2;
		}
		static int SetV(lua_State* L)LNOEXCEPT
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'SetV'.");
			if (lua_gettop(L) == 3)
			{
				lua_rawgeti(L, 1, 2);  // t(object) 'v' 'a' ??? id
				if (!LPOOL.SetV((size_t)luaL_checkinteger(L, -1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), false))
					return luaL_error(L, "invalid lstg object for 'SetV'.");
			}
			else if (lua_gettop(L) == 4)
			{
				lua_rawgeti(L, 1, 2);  // t(object) 'v' 'a' 'rot' ??? id
				if (!LPOOL.SetV((size_t)luaL_checkinteger(L, -1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), lua_toboolean(L, 4) == 0 ? false : true))
					return luaL_error(L, "invalid lstg object for 'SetV'.");
			}
			else
				return luaL_error(L, "invalid argument count for 'SetV'.");
			return 0;
		}
		static int SetImgState(lua_State* L)LNOEXCEPT
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'SetImgState'.");
			lua_rawgeti(L, 1, 2);  // t(object) ??? id
			size_t id = (size_t)luaL_checkinteger(L, -1);
			lua_pop(L, 1);

			BlendMode m = TranslateBlendMode(L, 2);
			fcyColor c(luaL_checkinteger(L, 3), luaL_checkinteger(L, 4), luaL_checkinteger(L, 5), luaL_checkinteger(L, 6));
			if (!LPOOL.SetImgState(id, m, c))
				return luaL_error(L, "invalid lstg object for 'SetImgState'.");
			return 0;
		}
		static int SetParState(lua_State* L)LNOEXCEPT
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'SetParState'.");
			lua_rawgeti(L, 1, 2);  // t(object) ??? id
			size_t id = (size_t)luaL_checkinteger(L, -1);
			lua_pop(L, 1);

			BlendMode m = TranslateBlendMode(L, 2);
			fcyColor c(luaL_checkinteger(L, 3), luaL_checkinteger(L, 4), luaL_checkinteger(L, 5), luaL_checkinteger(L, 6));
			if (!LPOOL.SetParState(id, m, c))
				return luaL_error(L, "invalid lstg object for 'SetParState'.");
			return 0;
		}
		static int BoxCheck(lua_State* L)LNOEXCEPT
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'BoxCheck'.");
			lua_rawgeti(L, 1, 2);  // t(object) 'l' 'r' 't' 'b' ??? id
			bool tRet;
			if (!LPOOL.BoxCheck(
				(size_t)luaL_checkinteger(L, -1),
				luaL_checknumber(L, 2),
				luaL_checknumber(L, 3),
				luaL_checknumber(L, 4),
				luaL_checknumber(L, 5),
				tRet))
			{
				return luaL_error(L, "invalid lstg object for 'BoxCheck'.");
			}	
			lua_pushboolean(L, tRet);
			return 1;
		}
		static int ResetPool(lua_State* L)LNOEXCEPT
		{
			LPOOL.ResetPool();
			return 0;
		}
		static int DefaultRenderFunc(lua_State* L)LNOEXCEPT
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'DefaultRenderFunc'.");
			lua_rawgeti(L, 1, 2);  // t(object) ??? id
			if (!LPOOL.DoDefaultRender(luaL_checkinteger(L, -1)))
				return luaL_error(L, "invalid lstg object for 'DefaultRenderFunc'.");
			return 0;
		}
		static int NextObject(lua_State* L)LNOEXCEPT
		{
			return LPOOL.NextObject(L);
		}
		static int ObjList(lua_State* L)LNOEXCEPT
		{
			/*
			int g = luaL_checkinteger(L, 1);					// i(groupId)
			lua_pushcfunction(L, WrapperImplement::NextObject);	// i(groupId) next(f)
			lua_pushinteger(L, g);								// i(groupId) next(f) i(groupId)
			lua_pushinteger(L, LPOOL.FirstObject(g));			// i(groupId) next(f) i(groupId) id(firstobj) 最后的两个参数作为迭代器参数传入
			//*/
			int g = luaL_checkinteger(L, 1);					// i(groupId)
			lua_pushcfunction(L, WrapperImplement::NextObject);	// i(groupId) next(f)
			lua_insert(L, 1);									// next(f) i(groupId)
			lua_pushinteger(L, LPOOL.FirstObject(g));			// next(f) i(groupId) id(firstobj) 最后的两个参数作为迭代器参数传入
			return 3;
		}
		static int ObjTable(lua_State* L)LNOEXCEPT
		{
			return LPOOL.GetObjectTable(L);
		}
		static int ObjMetaIndex(lua_State* L)LNOEXCEPT
		{
			return LPOOL.GetAttr(L);
		}
		static int ObjMetaNewIndex(lua_State* L)LNOEXCEPT
		{
			return LPOOL.SetAttr(L);
		}
		static int ParticleStop(lua_State* L)LNOEXCEPT
		{
			return LPOOL.ParticleStop(L);
		}
		static int ParticleFire(lua_State* L)LNOEXCEPT
		{
			return LPOOL.ParticleFire(L);
		}
		static int ParticleGetn(lua_State* L)LNOEXCEPT
		{
			return LPOOL.ParticleGetn(L);
		}
		static int ParticleGetEmission(lua_State* L)LNOEXCEPT
		{
			return LPOOL.ParticleGetEmission(L);
		}
		static int ParticleSetEmission(lua_State* L)LNOEXCEPT
		{
			return LPOOL.ParticleSetEmission(L);
		}
		//EX+
		static int GetCurrentObject(lua_State* L)LNOEXCEPT
		{
			return LPOOL.PushCurrentObject(L);
		}
		//EX+ 对象更新相关，影响frame回调函数以及对象更新
		static int SetSuperPause(lua_State* L)LNOEXCEPT
		{
			LPOOL.SetNextFrameSuperPauseTime(luaL_checkinteger(L, 1));
			return 0;
		}
		static int AddSuperPause(lua_State* L)LNOEXCEPT
		{
			lua_Integer a = luaL_checkinteger(L, 1);
			LPOOL.SetNextFrameSuperPauseTime(LPOOL.GetNextFrameSuperPauseTime() + a);
			return 0;
		}
		static int GetSuperPause(lua_State* L)LNOEXCEPT
		{
			lua_pushinteger(L, LPOOL.GetNextFrameSuperPauseTime());
			return 1;
		}
		static int GetCurrentSuperPause(lua_State* L)LNOEXCEPT
		{
			lua_pushinteger(L, LPOOL.GetSuperPauseTime());
			return 1;
		}
		//EX+ multi world  world mask
		static int GetWorldFlag(lua_State* L)LNOEXCEPT
		{
			lua_pushinteger(L, LPOOL.GetWorldFlag());
			return 1;
		}
		static int SetWorldFlag(lua_State* L)LNOEXCEPT
		{
			LPOOL.SetWorldFlag(luaL_checkinteger(L, 1));
			return 1;
		}
		static int IsSameWorld(lua_State* L)LNOEXCEPT
		{
			int a = luaL_checkinteger(L, 1);
			int b = luaL_checkinteger(L, 2);
			lua_pushboolean(L, GameObjectPool::CheckWorld(a, b));
			return 1;
		}
		static int ActiveWorlds(lua_State* L)LNOEXCEPT
		{
			int a1 = luaL_optinteger(L, 1, 0);
			int a2 = luaL_optinteger(L, 2, 0);
			int a3 = luaL_optinteger(L, 3, 0);
			int a4 = luaL_optinteger(L, 4, 0);
			LPOOL.ActiveWorlds(a1, a2, a3, a4);
			return 0;
		}
		static int CheckWorlds(lua_State* L)LNOEXCEPT
		{
			int a1 = luaL_checkinteger(L, 1);
			int a2 = luaL_checkinteger(L, 2);
			lua_pushboolean(L, LPOOL.CheckWorlds(a1, a2));
			return 1;
		}
		#pragma endregion
		
		#pragma region 资源控制函数
		// 资源控制函数
		static int SetResourceStatus(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			if (strcmp(s, "global") == 0)
				LRES.SetActivedPoolType(ResourcePoolType::Global);
			else if (strcmp(s, "stage") == 0)
				LRES.SetActivedPoolType(ResourcePoolType::Stage);
			else if (strcmp(s, "none") == 0)
				LRES.SetActivedPoolType(ResourcePoolType::None);
			else
				return luaL_error(L, "invalid argument #1 for 'SetResourceStatus', requires 'stage', 'global' or 'none'.");
			return 0;
		}
		static int GetResourceStatus(lua_State* L)LNOEXCEPT
		{
			switch (LRES.GetActivedPoolType()) {
			case ResourcePoolType::Global:
				lua_pushstring(L, "global");
				break;
			case ResourcePoolType::Stage:
				lua_pushstring(L, "stage");
				break;
			case ResourcePoolType::None:
				lua_pushstring(L, "none");
				break;
			default:
				return luaL_error(L, "can't get resource pool status at this time.");
			}
			return 1;
		}
		static int LoadTexture(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			if (!pActivedPool->LoadTexture(name, path, lua_toboolean(L, 3) == 0 ? false : true))
				return luaL_error(L, "can't load texture from file '%s'.", path);
			return 0;
		}
		static int LoadImage(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* texname = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->LoadImage(
				name,
				texname,
				luaL_checknumber(L, 3),
				luaL_checknumber(L, 4),
				luaL_checknumber(L, 5),
				luaL_checknumber(L, 6),
				luaL_optnumber(L, 7, 0.),
				luaL_optnumber(L, 8, 0.),
				lua_toboolean(L, 9) == 0 ? false : true
			))
			{
				return luaL_error(L, "load image failed (name='%s', tex='%s').", name, texname);
			}
			return 0;
		}
		static int LoadAnimation(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* texname = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			
			if (!pActivedPool->LoadAnimation(
				name,
				texname,
				luaL_checknumber(L, 3),
				luaL_checknumber(L, 4),
				luaL_checknumber(L, 5),
				luaL_checknumber(L, 6),
				luaL_checkinteger(L, 7),
				luaL_checkinteger(L, 8),
				luaL_checkinteger(L, 9),
				luaL_optnumber(L, 10, 0.0f),
				luaL_optnumber(L, 11, 0.0f),
				lua_toboolean(L, 12) == 0 ? false : true
			))
			{
				return luaL_error(L, "load animation failed (name='%s', tex='%s').", name, texname);
			}

			return 0;
		}
		static int LoadPS(lua_State* L)LNOEXCEPT
		{
			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			
			const char* name = luaL_checkstring(L, 1);
			const char* img_name = luaL_checkstring(L, 3);
			if (lua_type(L, 2) == LUA_TTABLE) {
				ResParticle::ParticleInfo info;
				bool ret = TranslateTableToParticleInfo(L, 2, info);
				if (!ret) return luaL_error(L, "load particle failed (name='%s', define=?, img='%s').", name, img_name);
				if (!pActivedPool->LoadParticle(
					name,
					info,
					img_name,
					luaL_optnumber(L, 4, 0.0f),
					luaL_optnumber(L, 5, 0.0f),
					lua_toboolean(L, 6) == 0 ? false : true
				))
				{
					return luaL_error(L, "load particle failed (name='%s', define=table, img='%s').", name, img_name);
				}
				return 0;
			}
			else {
				const char* path = luaL_checkstring(L, 2);

				if (!pActivedPool->LoadParticle(
					name,
					path,
					img_name,
					luaL_optnumber(L, 4, 0.0f),
					luaL_optnumber(L, 5, 0.0f),
					lua_toboolean(L, 6) == 0 ? false : true
				))
				{
					return luaL_error(L, "load particle failed (name='%s', file='%s', img='%s').", name, path, img_name);
				}
				return 0;
			}
		}
		static int LoadSound(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->LoadSound(name, path))
				return luaL_error(L, "load sound failed (name=%s, path=%s)", name, path);
			return 0;
		}
		static int LoadMusic(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			double loop_end = luaL_checknumber(L, 3);
			double loop_duration = luaL_checknumber(L, 4);
			double loop_start = max(0., loop_end - loop_duration);

			if (!pActivedPool->LoadMusic(
				name,
				path,
				loop_start,
				loop_end
				))
			{
				return luaL_error(L, "load music failed (name=%s, path=%s, loop=%f~%f)", name, path, loop_start, loop_end);
			}
			return 0;
		}
		static int LoadFont(lua_State* L)LNOEXCEPT
		{
			bool bSucceed = false;
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (lua_gettop(L) == 2)
			{
				// HGE字体 mipmap=true
				bSucceed = pActivedPool->LoadSpriteFont(name, path);
			}
			else
			{
				if (lua_isboolean(L, 3))
				{
					// HGE字体 mipmap=user_defined
					bSucceed = pActivedPool->LoadSpriteFont(name, path, lua_toboolean(L, 3) == 0 ? false : true);
				}
				else
				{
					// fancy2d字体
					const char* texpath = luaL_checkstring(L, 3);
					if (lua_gettop(L) == 4)
						bSucceed = pActivedPool->LoadSpriteFont(name, path, texpath, lua_toboolean(L, 4) == 0 ? false : true);
					else
						bSucceed = pActivedPool->LoadSpriteFont(name, path, texpath);
				}
			}

			if (!bSucceed)
				return luaL_error(L, "can't load font from file '%s'.", path);
			return 0;
		}
		static int LoadTTF(lua_State* L)LNOEXCEPT
		{
			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool) {
				return luaL_error(L, "can't load resource at this time.");
			}

			const char* name = luaL_checkstring(L, 1);
			int t = lua_type(L, 2);
			if (LUA_TSTRING == t) {
				const char* path = luaL_checkstring(L, 2);
				bool result = pActivedPool->LoadTTFFont(name, path, (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), (float)luaL_optnumber(L, 5, 0.0), (float)luaL_optnumber(L, 6, 0.0));
				lua_pushboolean(L, result);
			}
			else {
				using sw = LuaSTGPlus::LuaWrapper::IO::StreamWrapper::Wrapper;
				sw* data = (sw*)luaL_checkudata(L, 2, LUASTG_LUA_TYPENAME_IO_STREAM);
				bool result = pActivedPool->LoadTTFFont(name, data->handle, (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), (float)luaL_optnumber(L, 5, 0.0), (float)luaL_optnumber(L, 6, 0.0));
				lua_pushboolean(L, result);
			}
			return 1;
		}
		static int LoadTrueTypeFont(lua_State* L)LNOEXCEPT
		{
			// 先检查有没有资源池
			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool) {
				return luaL_error(L, "can't load resource at this time.");
			}
			
			// 第一个参数，资源名
			const char* name = luaL_checkstring(L, 1);
			
			// 第二个参数，字体加载配置
			f2dFontProviderParam param;
			{
				param.font_bbox = fcyVec2(0.0f, 0.0f);
				param.glyph_count = 1024;
				param.texture_size = 2048;
			}
			if (lua_istable(L, 2))
			{
				lua_getfield(L, 2, "glyph_bbox_x");
				if (lua_type(L, -1) == LUA_TNUMBER) {
					param.font_bbox.x = (fFloat)luaL_checknumber(L, -1);
				}
				lua_pop(L, 1);
				
				lua_getfield(L, 2, "glyph_bbox_y");
				if (lua_type(L, -1) == LUA_TNUMBER) {
					param.font_bbox.y = (fFloat)luaL_checknumber(L, -1);
				}
				lua_pop(L, 1);
				
				lua_getfield(L, 2, "glyph_count");
				if (lua_type(L, -1) == LUA_TNUMBER) {
					param.glyph_count = (fuInt)std::max(0, luaL_checkinteger(L, -1));
				}
				lua_pop(L, 1);
				
				lua_getfield(L, 2, "texture_size");
				if (lua_type(L, -1) == LUA_TNUMBER) {
					param.texture_size = (fuInt)std::max(0, luaL_checkinteger(L, -1));
				}
				lua_pop(L, 1);
			}
			
			// 第三个参数，字体
			std::vector<f2dTrueTypeFontParam> fonts;
			std::vector<fcyRefPointer<fcyStream>> streams;
			if (lua_istable(L, 3))
			{
				using sw = LuaSTGPlus::LuaWrapper::IO::StreamWrapper::Wrapper;
				int cnt = (int)lua_objlen(L, 3);
												// name param fonts
				for (int i = 1; i <= cnt; i += 1) {
					f2dTrueTypeFontParam font;
					{
						font.font_file = nullptr; // 不会用上
						font.font_source = nullptr;
						font.font_face = 0;
						font.font_size = fcyVec2(0.0f, 0.0f);
					}
					
					lua_pushinteger(L, i);		// name param fonts i
					lua_gettable(L, 3);			// name param fonts font
					
					lua_getfield(L, -1, "face_index"); // name param fonts font ?
					if (lua_type(L, -1) == LUA_TNUMBER) // name param fonts font v
					{
						font.font_face = (fInt)luaL_checkinteger(L, -1);
					}
					lua_pop(L, 1);				// name param fonts font
					
					lua_getfield(L, -1, "width"); // name param fonts font ?
					if (lua_type(L, -1) == LUA_TNUMBER) // name param fonts font v
					{
						font.font_size.x = (fFloat)luaL_checknumber(L, -1);
					}
					lua_pop(L, 1);				// name param fonts font
					
					lua_getfield(L, -1, "height"); // name param fonts font ?
					if (lua_type(L, -1) == LUA_TNUMBER) // name param fonts font v
					{
						font.font_size.y = (fFloat)luaL_checknumber(L, -1);
					}
					lua_pop(L, 1);				// name param fonts font
					
					lua_getfield(L, -1, "source"); // name param fonts font ?
					if (lua_type(L, -1) == LUA_TUSERDATA) // name param fonts font v
					{
						sw* data = (sw*)luaL_checkudata(L, -1, LUASTG_LUA_TYPENAME_IO_STREAM);
						font.font_source = (fcyMemStream*)data->handle;
					}
					lua_pop(L, 1);				// name param fonts font
					
					lua_getfield(L, -1, "file"); // name param fonts font ?
					if (lua_type(L, -1) == LUA_TSTRING) // name param fonts font v
					{
						const char* filename = luaL_checkstring(L, -1);
						bool loaded = false;
						// 先从文件加载试试看
						try {
							std::wstring wfilename = fcyStringHelper::MultiByteToWideChar(filename, CP_UTF8);
							if (std::filesystem::is_regular_file(wfilename)) {
								try {
									fcyFileStream* stream = new fcyFileStream(wfilename.c_str(), false);
									
									streams.push_back(fcyRefPointer<fcyStream>());
									streams.back().DirectSet(stream);
									loaded = true;
									
									font.font_source = nullptr; // 这个不要
									font.font_file = (f2dStream*)stream;
								}
								catch (...) {}
							}
						}
						catch (...) {}
						// 没有……那只能从FMGR加载了
						if (!loaded) {
							fcyMemStream* stream = (fcyMemStream*)LFMGR.LoadFile(filename);
							if (stream != nullptr) {
								streams.push_back(fcyRefPointer<fcyStream>());
								streams.back().DirectSet(stream);
								loaded = true;
								
								font.font_source = stream;
							}
						}
					}
					lua_pop(L, 1);				// name param fonts font
					
					lua_pop(L, 1);				// name param fonts
					fonts.emplace_back(std::move(font));
				}
			}
			
			bool result = pActivedPool->LoadTrueTypeFont(name, param, fonts.data(), fonts.size());
			lua_pushboolean(L, result);
			
			return 1;
		}
		static int LoadFX(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->LoadFX(name, path))
				return luaL_error(L, "load fx failed (name=%s, path=%s)", name, path);
			return 0;
		}
		static int CreateRenderTarget(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->CreateRenderTarget(name))
				return luaL_error(L, "can't create render target with name '%s'.", name);
			return 0;
		}
		static int IsRenderTarget(lua_State* L)LNOEXCEPT
		{
			ResTexture* p = LRES.FindTexture(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "render target '%s' not found.", luaL_checkstring(L, 1));
			lua_pushboolean(L, p->IsRenderTarget());
			return 1;
		}
		static int GetTextureSize(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			fcyVec2 size;
			if (!LRES.GetTextureSize(name, size))
				return luaL_error(L, "texture '%s' not found.", name);
			lua_pushnumber(L, size.x);
			lua_pushnumber(L, size.y);
			return 2;
		}
		static int RemoveResource(lua_State* L)LNOEXCEPT
		{
			ResourcePoolType t;
			const char* s = luaL_checkstring(L, 1);
			if (strcmp(s, "global") == 0)
				t = ResourcePoolType::Global;
			else if (strcmp(s, "stage") == 0)
				t = ResourcePoolType::Stage;
			else if (strcmp(s, "none") != 0)
				t = ResourcePoolType::None;
			else
				return luaL_error(L, "invalid argument #1 for 'RemoveResource', requires 'stage', 'global' or 'none'.");

			if (lua_gettop(L) == 1)
			{
				switch (t)
				{
				case ResourcePoolType::Stage:
					LRES.GetResourcePool(ResourcePoolType::Stage)->Clear();
					LINFO("关卡资源池已清空");
					break;
				case ResourcePoolType::Global:
					LRES.GetResourcePool(ResourcePoolType::Global)->Clear();
					LINFO("全局资源池已清空");
					break;
				default:
					break;
				}
			}
			else
			{
				ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 2));
				const char* tResourceName = luaL_checkstring(L, 3);

				switch (t)
				{
				case ResourcePoolType::Stage:
					LRES.GetResourcePool(ResourcePoolType::Stage)->RemoveResource(tResourceType, tResourceName);
					break;
				case ResourcePoolType::Global:
					LRES.GetResourcePool(ResourcePoolType::Global)->RemoveResource(tResourceType, tResourceName);
					break;
				default:
					break;
				}
			}
			
			return 0;
		}
		static int CheckRes(lua_State* L)LNOEXCEPT
		{
			ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 1));
			const char* tResourceName = luaL_checkstring(L, 2);
			// 先在全局池中寻找再到关卡池中找
			if (LRES.GetResourcePool(ResourcePoolType::Global)->CheckResourceExists(tResourceType, tResourceName))
				lua_pushstring(L, "global");
			else if (LRES.GetResourcePool(ResourcePoolType::Stage)->CheckResourceExists(tResourceType, tResourceName))
				lua_pushstring(L, "stage");
			else
				lua_pushnil(L);
			return 1;
		}
		static int EnumRes(lua_State* L)LNOEXCEPT
		{
			ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 1));
			LRES.GetResourcePool(ResourcePoolType::Global)->ExportResourceList(L, tResourceType);
			LRES.GetResourcePool(ResourcePoolType::Stage)->ExportResourceList(L, tResourceType);
			return 2;
		}
		static int SetImageScale(lua_State* L)LNOEXCEPT
		{
			float x = static_cast<float>(luaL_checknumber(L, 1));
			if (x == 0.f)
				return luaL_error(L, "invalid argument #1 for 'SetImageScale'.");
			LRES.SetGlobalImageScaleFactor(x);
			return 0;
		}
		static int GetImageScale(lua_State* L)LNOEXCEPT
		{
			lua_Number ret = LRES.GetGlobalImageScaleFactor();
			lua_pushnumber(L, ret);
			return 1;
		}
		static int SetImageState(lua_State* L)LNOEXCEPT
		{
			ResSprite* p = LRES.FindSprite(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));

			p->SetBlendMode(TranslateBlendMode(L, 2));
			if (lua_gettop(L) == 3)
				p->GetSprite()->SetColor(*static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR)));
			else if (lua_gettop(L) == 6)
			{
				fcyColor tColors[] = {
					*static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 4, LUASTG_LUA_TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 5, LUASTG_LUA_TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 6, LUASTG_LUA_TYPENAME_COLOR))
				};
				p->GetSprite()->SetColor(tColors);
			}
			return 0;
		}
		static int SetFontState(lua_State* L)LNOEXCEPT
		{
			ResFont* p = LRES.FindSpriteFont(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "sprite font '%s' not found.", luaL_checkstring(L, 1));

			p->SetBlendMode(TranslateBlendMode(L, 2));
			if (lua_gettop(L) == 3)
			{
				fcyColor c = *static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR));
				p->SetBlendColor(c);
			}
			return 0;
		}
		static int SetAnimationState(lua_State* L)LNOEXCEPT
		{
			ResAnimation* p = LRES.FindAnimation(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "animation '%s' not found.", luaL_checkstring(L, 1));

			p->SetBlendMode(TranslateBlendMode(L, 2));
			if (lua_gettop(L) == 3)
			{
				fcyColor c = *static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR));
				for (size_t i = 0; i < p->GetCount(); ++i)
					p->GetSprite(i)->SetColor(c);
			}
			else if (lua_gettop(L) == 6)
			{
				fcyColor tColors[] = {
					*static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 4, LUASTG_LUA_TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 5, LUASTG_LUA_TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 6, LUASTG_LUA_TYPENAME_COLOR))
				};
				for (size_t i = 0; i < p->GetCount(); ++i)
					p->GetSprite(i)->SetColor(tColors);
			}
			return 0;
		}
		static int SetImageCenter(lua_State* L)LNOEXCEPT
		{
			ResSprite* p = LRES.FindSprite(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));
			p->GetSprite()->SetHotSpot(fcyVec2(
				static_cast<float>(luaL_checknumber(L, 2) + p->GetSprite()->GetTexRect().a.x),
				static_cast<float>(luaL_checknumber(L, 3) + p->GetSprite()->GetTexRect().a.y)));
			return 0;
		}
		static int SetAnimationCenter(lua_State* L)LNOEXCEPT
		{
			ResAnimation* p = LRES.FindAnimation(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "animation '%s' not found.", luaL_checkstring(L, 1));
			for (size_t i = 0; i < p->GetCount(); ++i)
			{
				p->GetSprite(i)->SetHotSpot(fcyVec2(
					static_cast<float>(luaL_checknumber(L, 2) + p->GetSprite(i)->GetTexRect().a.x),
					static_cast<float>(luaL_checknumber(L, 3) + p->GetSprite(i)->GetTexRect().a.y)));
			}
			return 0;
		}
		static int CacheTTFString(lua_State* L) {
			LRES.CacheTTFFontString(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
			return 0;
		}
		//EX+ model
		static int LoadModel(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* texname = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			bool LoadObj(string id, string path);
			if (!pActivedPool->LoadModel(
				name,
				texname))
			{
				return luaL_error(L, "load model failed (name='%s', tex='%s').", name, texname);
			}
			return 0;
		}
		#pragma endregion
		
		#pragma region 绘图函数
		// 绘图函数
		static int BeginScene(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.BeginScene())
				return luaL_error(L, "can't invoke 'BeginScene'.");
			return 0;
		}
		static int EndScene(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.EndScene())
				return luaL_error(L, "can't invoke 'EndScene'.");
			return 0;
		}
		static int RenderClear(lua_State* L)LNOEXCEPT
		{
			fcyColor* c = static_cast<fcyColor*>(luaL_checkudata(L, 1, LUASTG_LUA_TYPENAME_COLOR));
			LAPP.ClearScreen(*c);
			return 0;
		}
		static int SetViewport(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.SetViewport(
				luaL_checknumber(L, 1),
				luaL_checknumber(L, 2),
				luaL_checknumber(L, 3),
				luaL_checknumber(L, 4)
			))
			{
				return luaL_error(L, "invalid arguments for 'SetViewport'.");
			}
			return 0;
		}
		static int SetScissorRect(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.SetScissorRect(
				luaL_checknumber(L, 1),
				luaL_checknumber(L, 2),
				luaL_checknumber(L, 3),
				luaL_checknumber(L, 4)
			))
			{
				return luaL_error(L, "invalid arguments for 'SetScissorRect'.");
			}
			return 0;
		}
		static int SetOrtho(lua_State* L)LNOEXCEPT
		{
			int top_n = lua_gettop(L);
			switch (top_n) {
			case 4:
				LAPP.SetOrtho(
					static_cast<float>(luaL_checknumber(L, 1)),
					static_cast<float>(luaL_checknumber(L, 2)),
					static_cast<float>(luaL_checknumber(L, 3)),
					static_cast<float>(luaL_checknumber(L, 4))
				);
				break;
			case 6:
				LAPP.SetOrtho(
					static_cast<float>(luaL_checknumber(L, 1)),
					static_cast<float>(luaL_checknumber(L, 2)),
					static_cast<float>(luaL_checknumber(L, 3)),
					static_cast<float>(luaL_checknumber(L, 4)),
					static_cast<float>(luaL_checknumber(L, 5)),
					static_cast<float>(luaL_checknumber(L, 6))
				);
				break;
			}
			return 0;
		}
		static int SetPerspective(lua_State* L)LNOEXCEPT
		{
			LAPP.SetPerspective(
				static_cast<float>(luaL_checknumber(L, 1)),
				static_cast<float>(luaL_checknumber(L, 2)),
				static_cast<float>(luaL_checknumber(L, 3)),
				static_cast<float>(luaL_checknumber(L, 4)),
				static_cast<float>(luaL_checknumber(L, 5)),
				static_cast<float>(luaL_checknumber(L, 6)),
				static_cast<float>(luaL_checknumber(L, 7)),
				static_cast<float>(luaL_checknumber(L, 8)),
				static_cast<float>(luaL_checknumber(L, 9)),
				static_cast<float>(luaL_checknumber(L, 10)),
				static_cast<float>(luaL_checknumber(L, 11)),
				static_cast<float>(luaL_checknumber(L, 12)),
				static_cast<float>(luaL_checknumber(L, 13))
			);
			return 0;
		}
		static int SetTextureSamplerState(lua_State* L)LNOEXCEPT
		{
			bool ret;
			if (lua_gettop(L) == 2) {
				// string string
				const char* s = luaL_checkstring(L, 1);
				if (strcmp(s, "address") == 0) {
					// "address" string
					fcyColor color(0,0,0,0);
					ret = (FCYERR_OK == LAPP.GetRenderDev()->SetTextureAddress(TranslateTextureSamplerAddress(L, 2), color));
					if (!ret) {
						return luaL_error(L, "Failed to set texture sampler address mode.");
					}
				}
				else if (strcmp(s, "filter") == 0) {
					// "filter" string
					ret = (FCYERR_OK == LAPP.GetRenderDev()->SetTextureFilter(TranslateTextureSamplerFilter(L, 2)));
					if (!ret) {
						return luaL_error(L, "Failed to set texture sampler filter type.");
					}
				}
				else {
					return luaL_error(L, "Invalid argument '%m'.",s);
				}
			}
			else if (lua_gettop(L) == 3) {
				// "address" string color
				fcyColor* p = static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR));
				ret = (FCYERR_OK == LAPP.GetRenderDev()->SetTextureAddress(TranslateTextureSamplerAddress(L, 2), *p));
				if (!ret) {
					return luaL_error(L, "Failed to set texture sampler address mode.");
				}
			}
			return 0;
		}
		static int Render(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.Render(
				luaL_checkstring(L, 1),
				static_cast<float>(luaL_checknumber(L, 2)),
				static_cast<float>(luaL_checknumber(L, 3)),
				static_cast<float>(luaL_optnumber(L, 4, 0.) * LDEGREE2RAD),
				static_cast<float>(luaL_optnumber(L, 5, 1.) * LRES.GetGlobalImageScaleFactor()),
				static_cast<float>(luaL_optnumber(L, 6, luaL_optnumber(L, 5, 1.)) * LRES.GetGlobalImageScaleFactor()),
				static_cast<float>(luaL_optnumber(L, 7, 0.5))
			))
			{
				return luaL_error(L, "can't render '%s'", luaL_checkstring(L, 1));
			}
			return 0;
		}
		static int RenderAnimation(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.RenderAnimation(
				luaL_checkstring(L, 1),
				luaL_checkinteger(L,2),
				static_cast<float>(luaL_checknumber(L, 3)),
				static_cast<float>(luaL_checknumber(L, 4)),
				static_cast<float>(luaL_optnumber(L, 5, 0.) * LDEGREE2RAD),
				static_cast<float>(luaL_optnumber(L, 6, 1.) * LRES.GetGlobalImageScaleFactor()),
				static_cast<float>(luaL_optnumber(L, 7, luaL_optnumber(L, 6, 1.)) * LRES.GetGlobalImageScaleFactor())
			))
			{
				return luaL_error(L, "can't render '%s'", luaL_checkstring(L, 1));
			}
			return 0;
		}
		static int RenderRect(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.RenderRect(
				luaL_checkstring(L, 1),
				static_cast<float>(luaL_checknumber(L, 2)),
				static_cast<float>(luaL_checknumber(L, 5)),
				static_cast<float>(luaL_checknumber(L, 3)),
				static_cast<float>(luaL_checknumber(L, 4))
			))
			{
				return luaL_error(L, "can't render '%s'", luaL_checkstring(L, 1));
			}
			return 0;
		}
		static int Render4V(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.Render4V(
				luaL_checkstring(L, 1),
				static_cast<float>(luaL_checknumber(L, 2)),
				static_cast<float>(luaL_checknumber(L, 3)),
				static_cast<float>(luaL_checknumber(L, 4)),
				static_cast<float>(luaL_checknumber(L, 5)),
				static_cast<float>(luaL_checknumber(L, 6)),
				static_cast<float>(luaL_checknumber(L, 7)),
				static_cast<float>(luaL_checknumber(L, 8)),
				static_cast<float>(luaL_checknumber(L, 9)),
				static_cast<float>(luaL_checknumber(L, 10)),
				static_cast<float>(luaL_checknumber(L, 11)),
				static_cast<float>(luaL_checknumber(L, 12)),
				static_cast<float>(luaL_checknumber(L, 13))
			))
			{
				return luaL_error(L, "can't render '%s'.", luaL_checkstring(L, 1));
			}
			return 0;
		}
		static int RenderText(lua_State* L)LNOEXCEPT
		{
			ResFont::FontAlignHorizontal halign = ResFont::FontAlignHorizontal::Center;
			ResFont::FontAlignVertical valign = ResFont::FontAlignVertical::Middle;
			if (lua_gettop(L) == 6)
				TranslateAlignMode(L, 6, halign, valign);
			if (!LAPP.RenderText(
				luaL_checkstring(L, 1),
				luaL_checkstring(L, 2),
				(float)luaL_checknumber(L, 3),
				(float)luaL_checknumber(L, 4),
				(float)(luaL_optnumber(L, 5, 1.0) * LRES.GetGlobalImageScaleFactor()),
				halign,
				valign
				))
			{
				return luaL_error(L, "can't draw text '%s'.", luaL_checkstring(L, 1));
			}	
			return 0;
		}
		static int RenderTexture(lua_State* L)LNOEXCEPT
		{
			const char* tex_name = luaL_checkstring(L, 1);
			BlendMode blend = TranslateBlendMode(L, 2);
			f2dGraphics2DVertex vertex[4];

			for (int i = 0; i < 4; ++i)
			{
				lua_pushinteger(L, 1);
				lua_gettable(L, 3 + i);
				vertex[i].x = (float)lua_tonumber(L, -1);

				lua_pushinteger(L, 2);
				lua_gettable(L, 3 + i);
				vertex[i].y = (float)lua_tonumber(L, -1);
				
				lua_pushinteger(L, 3);
				lua_gettable(L, 3 + i);
				vertex[i].z = (float)lua_tonumber(L, -1);

				lua_pushinteger(L, 4);
				lua_gettable(L, 3 + i);
				vertex[i].u = (float)lua_tonumber(L, -1);

				lua_pushinteger(L, 5);
				lua_gettable(L, 3 + i);
				vertex[i].v = (float)lua_tonumber(L, -1);

				lua_pushinteger(L, 6);
				lua_gettable(L, 3 + i);
				vertex[i].color = static_cast<fcyColor*>(luaL_checkudata(L, -1, LUASTG_LUA_TYPENAME_COLOR))->argb;

				lua_pop(L, 6);
			}

			if (!LAPP.RenderTexture(tex_name, blend, vertex))
				return luaL_error(L, "can't render texture '%s'.", tex_name);
			return 0;
		}
		static int RenderTTF(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.RenderTTF(
				luaL_checkstring(L, 1),
				luaL_checkstring(L, 2),
				(float)luaL_checknumber(L, 3),
				(float)luaL_checknumber(L, 4),
				(float)luaL_checknumber(L, 5),
				(float)luaL_checknumber(L, 6),
				LRES.GetGlobalImageScaleFactor() * (float)luaL_optnumber(L, 9, 1.0),
				luaL_checkinteger(L, 7),
				*static_cast<fcyColor*>(luaL_checkudata(L, 8, LUASTG_LUA_TYPENAME_COLOR))
			))
			{
				return luaL_error(L, "can't render font '%s'.", luaL_checkstring(L, 1));
			}	
			return 0;
		}
		static int SetFog(lua_State* L)LNOEXCEPT
		{
			if (lua_gettop(L) == 3)
				LAPP.SetFog(
					static_cast<float>(luaL_checknumber(L, 1)),
					static_cast<float>(luaL_checknumber(L, 2)),
					*(static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR)))
				);
			else if (lua_gettop(L) == 2)
				LAPP.SetFog(
					static_cast<float>(luaL_checknumber(L, 1)),
					static_cast<float>(luaL_checknumber(L, 2)),
					0xFF000000
				);
			else
				LAPP.SetFog(0.0f, 0.0f, 0x00FFFFFF);
			return 0;
		}
		static int PushRenderTarget(lua_State* L)LNOEXCEPT
		{
			ResTexture* p = LRES.FindTexture(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "rendertarget '%s' not found.", luaL_checkstring(L, 1));
			if (!p->IsRenderTarget())
				return luaL_error(L, "'%s' is a texture.", luaL_checkstring(L, 1));

			if (!LAPP.PushRenderTarget(p))
				return luaL_error(L, "push rendertarget '%s' failed.", luaL_checkstring(L, 1));
			return 0;
		}
		static int PopRenderTarget(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.PopRenderTarget())
				return luaL_error(L, "pop rendertarget failed.");
			return 0;
		}
		static int PostEffect(lua_State* L)LNOEXCEPT
		{
			const char* texture = luaL_checkstring(L, 1);
			const char* name = luaL_checkstring(L, 2);
			BlendMode blend = TranslateBlendMode(L, 3);

			// 获取纹理
			ResTexture* rt = LRES.FindTexture(luaL_checkstring(L, 1));
			if (!rt)
				return luaL_error(L, "texture '%s' not found.", texture);

			// 获取fx
			ResFX* p = LRES.FindFX(name);
			if (!p)
				return luaL_error(L, "PostEffect: can't find effect '%s'.", name);
			if (lua_istable(L, 4))
			{
				// 设置table上的参数到fx
				lua_pushnil(L);  // s s t ... nil
				while (0 != lua_next(L, 4))
				{
					// s s t ... nil key value
					const char* key = luaL_checkstring(L, -2);
					if (lua_isnumber(L, -1))
						p->SetValue(key, (float)lua_tonumber(L, -1));
					else if (lua_isstring(L, -1))
					{
						ResTexture* pTex = LRES.FindTexture(lua_tostring(L, -1));
						if (!pTex)
							return luaL_error(L, "PostEffect: can't find texture '%s'.", lua_tostring(L, -1));
						p->SetValue(key, pTex->GetTexture());
					}
					else if (lua_isuserdata(L, -1))
					{
						fcyColor c = *static_cast<fcyColor*>(luaL_checkudata(L, -1, LUASTG_LUA_TYPENAME_COLOR));
						p->SetValue(key, c);
					}
					else
						return luaL_error(L, "PostEffect: invalid data type.");

					lua_pop(L, 1);  // s s t ... nil key
				}
			}

			if (!LAPP.PostEffect(rt, p, blend))
				return luaL_error(L, "PostEffect failed.");
			return 0;
		}
		//EX+
		static int SetZBufferEnable(lua_State* L)LNOEXCEPT
		{
			LAPP.SetZBufferEnable(luaL_checkinteger(L, 1) != 0);
			return 0;
		}
		static int ClearZBuffer(lua_State* L)LNOEXCEPT
		{
			LAPP.ClearZBuffer(luaL_optnumber(L, 1, 1.0f));
			return 0;
		}
		static int RenderModel(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.RenderModel(
				luaL_checkstring(L, 1),
				static_cast<float>(luaL_checknumber(L, 2)),
				static_cast<float>(luaL_checknumber(L, 3)),
				static_cast<float>(luaL_checknumber(L, 4)),
				static_cast<float>(luaL_optnumber(L, 5, 0)),
				static_cast<float>(luaL_optnumber(L, 6, 0)),
				static_cast<float>(luaL_optnumber(L, 7, 0)),
				static_cast<float>(luaL_optnumber(L, 8, 0)),
				static_cast<float>(luaL_optnumber(L, 9, 0)),
				static_cast<float>(luaL_optnumber(L, 10, 0))
			))
			{
				return luaL_error(L, "can't render '%s'.", luaL_checkstring(L, 1));
			}
			return 0;
		}
		static int DrawCollider(lua_State* L)LNOEXCEPT // t(list) [n] <length> <rate>
		{
			LPOOL.DrawCollider();
			return 1;
		}
		//ETC
		static int RenderGroupCollider(lua_State* L) {
			// group color
			LPOOL.DrawGroupCollider2(
				luaL_checkinteger(L, 1),
				fcyColor(static_cast<fcyColor*>(luaL_checkudata(L, 2, LUASTG_LUA_TYPENAME_COLOR))->argb)
			);
			return 0;
		}
		static int RenderTextureSector(lua_State* L) {
			lua_pushboolean(L, LAPP.RenderSector(
				luaL_checkstring(L, 1),
				fcyRect(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5)),
				lua_toboolean(L, 6),
				TranslateBlendMode(L, 7),
				*static_cast<fcyColor*>(luaL_checkudata(L, 8, LUASTG_LUA_TYPENAME_COLOR)),
				*static_cast<fcyColor*>(luaL_checkudata(L, 9, LUASTG_LUA_TYPENAME_COLOR)),
				fcyVec2(luaL_checknumber(L, 10), luaL_checknumber(L, 11)),
				luaL_checknumber(L, 12) * LDEGREE2RAD,
				luaL_checknumber(L, 13) * LDEGREE2RAD,
				luaL_checknumber(L, 14),
				luaL_checknumber(L, 15),
				luaL_checkinteger(L, 16)
			));
			return 1;
		}
		static int RenderTextureAnnulus(lua_State* L) {
			lua_pushboolean(L, LAPP.RenderAnnulus(
				luaL_checkstring(L, 1),
				fcyRect(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5)),
				lua_toboolean(L, 6),
				TranslateBlendMode(L, 7),
				*static_cast<fcyColor*>(luaL_checkudata(L, 8, LUASTG_LUA_TYPENAME_COLOR)),
				*static_cast<fcyColor*>(luaL_checkudata(L, 9, LUASTG_LUA_TYPENAME_COLOR)),
				fcyVec2(luaL_checknumber(L, 10), luaL_checknumber(L, 11)),
				luaL_checknumber(L, 12) * LDEGREE2RAD,
				luaL_checknumber(L, 13),
				luaL_checknumber(L, 14),
				luaL_checkinteger(L, 15),
				luaL_checkinteger(L, 16)
			));
			return 1;
		}
		#pragma endregion

		#pragma region 声音控制函数
		// 声音控制函数
		static int PlaySound(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Play((float)luaL_checknumber(L, 2) * LRES.GetGlobalSoundEffectVolume(), (float)luaL_optnumber(L, 3, 0.));
			return 0;
		}
		static int StopSound(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Stop();
			return 0;
		}
		static int PauseSound(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Pause();
			return 0;
		}
		static int ResumeSound(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Resume();
			return 0;
		}
		static int GetSoundState(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			if (p->IsPlaying())
				lua_pushstring(L, "playing");
			else if (p->IsStopped())
				lua_pushstring(L, "stopped");
			else
				lua_pushstring(L, "paused");
			return 1;
		}
		static int PlayMusic(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Play((float)luaL_optnumber(L, 2, 1.) * LRES.GetGlobalMusicVolume(), luaL_optnumber(L, 3, 0.));
			return 0;
		}
		static int StopMusic(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Stop();
			return 0;
		}
		static int PauseMusic(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Pause();
			return 0;
		}
		static int ResumeMusic(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Resume();
			return 0;
		}
		static int GetMusicState(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			if (p->IsPlaying())
				lua_pushstring(L, "playing");
			else if (p->IsPaused())
				lua_pushstring(L, "paused");
			//else if (p->IsStopped())
				//lua_pushstring(L, "stopped");
			else
				lua_pushstring(L, "stopped");
				//lua_pushstring(L, "paused");
			return 1;
		}
		static int UpdateSound(lua_State* L)LNOEXCEPT
		{
			// 否决的方法
			return 0;
		}
		static int SetSEVolume(lua_State* L)LNOEXCEPT
		{
			float x = static_cast<float>(luaL_checknumber(L, 1));
			LRES.SetGlobalSoundEffectVolume(max(min(x, 1.f), 0.f));
			return 0;
		}
		static int GetSEVolume(lua_State* L) {
			lua_pushnumber(L, LRES.GetGlobalSoundEffectVolume());
			return 1;
		}
		static int SetBGMVolume(lua_State* L)LNOEXCEPT
		{
			if (lua_gettop(L) == 1)
			{
				float x = static_cast<float>(luaL_checknumber(L, 1));
				LRES.SetGlobalMusicVolume(max(min(x, 1.f), 0.f));
			}
			else
			{
				const char* s = luaL_checkstring(L, 1);
				float x = static_cast<float>(luaL_checknumber(L, 2));
				ResMusic* p = LRES.FindMusic(s);
				if (!p)
					return luaL_error(L, "music '%s' not found.", s);
				p->SetVolume(x * LRES.GetGlobalMusicVolume());
			}
			return 0;
		}
		static int GetBGMVolume(lua_State* L)LNOEXCEPT
		{
			float GV = LRES.GetGlobalMusicVolume();
			if (lua_gettop(L) == 0)
			{
				lua_pushnumber(L, GV);
			}
			else if (lua_gettop(L) == 1)
			{
				const char* s = luaL_checkstring(L, 1);
				ResMusic* p = LRES.FindMusic(s);
				if (!p)
					return luaL_error(L, "music '%s' not found.", s);
				lua_pushnumber(L, p->GetVolume() / GV);
			}
			return 1;
		}
		static int SetSESpeed(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			float speed = luaL_checknumber(L, 2);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			if (!p->SetSpeed(speed))
				return luaL_error(L, "Can't set sound('%s') playing speed.", s);
			return 0;
		}
		static int GetSESpeed(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			lua_pushnumber(L, p->GetSpeed());
			return 1;
		}
		static int SetBGMSpeed(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			float speed = luaL_checknumber(L, 2);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			if (!p->SetSpeed(speed))
				return luaL_error(L, "Can't set music('%s') playing speed.", s);
			return 0;
		}
		static int GetBGMSpeed(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			lua_pushnumber(L, p->GetSpeed());
			return 1;
		}
		#pragma endregion

		#pragma region 输入控制函数
		// 输入控制函数
		static int GetKeyState(lua_State* L)LNOEXCEPT
		{
			lua_pushboolean(L, LAPP.GetKeyState(luaL_checkinteger(L, -1)));
			return 1;
		}
		static int GetLastKey(lua_State* L)LNOEXCEPT
		{
			lua_pushinteger(L, LAPP.GetLastKey());
			return 1;
		}
		static int GetLastChar(lua_State* L)LNOEXCEPT
		{
			return LAPP.GetLastChar(L);
		}
		static int GetMousePosition(lua_State* L)LNOEXCEPT
		{
			fcyVec2 tPos = LAPP.GetMousePosition(lua_toboolean(L, 1));
			lua_pushnumber(L, tPos.x);
			lua_pushnumber(L, tPos.y);
			return 2;
		}
		static int GetMouseWheelDelta(lua_State* L)LNOEXCEPT
		{
			//返回有符号整数，每次获取的是自上次获取以来所有的滚轮操作，有的鼠标可能没有滚轮
			lua_pushinteger(L, (lua_Integer)LAPP.GetMouseWheelDelta());
			return 1;
		}
		static int GetMouseState(lua_State* L)LNOEXCEPT
		{
			//参数为整数，索引从1到8，返回bool值，索引1到3分别为鼠标左中右键，有的鼠标可能没有中键，剩下的按键看实际硬件情况
			lua_pushboolean(L, LAPP.GetMouseState(luaL_checkinteger(L, 1)));
			return 1;
		}
		//ETC Raw Input
		static int GetKeyboardState(lua_State* L)LNOEXCEPT
		{
			//检查键盘按键是否按下，Dinput KeyCode
			lua_pushboolean(L, LAPP.GetKeyboardState(luaL_checkinteger(L, -1)));
			return 1;
		}
		static int GetAsyncKeyState(lua_State* L)LNOEXCEPT
		{
			//检查键盘按键是否按下，使用微软VKcode，通过GetAsyncKeyState函数获取
			//和GetKeyboardState不同，这个检测的不是按下过的，而是现在被按住的键
			lua_pushboolean(L, LAPP.GetAsyncKeyState(luaL_checkinteger(L, -1)));
			return 1;
		}
		#pragma endregion
		
		#pragma region 杂项
		static int Snapshot(lua_State* L)LNOEXCEPT
		{
			LAPP.SnapShot(luaL_checkstring(L, 1));
			return 0;
		}
		static int SaveTexture(lua_State* L)LNOEXCEPT
		{
			const char* tex_name = luaL_checkstring(L, 1);
			fcyRefPointer<ResTexture> resTex = LRES.FindTexture(tex_name);
			if (!resTex)
			{
				LERROR("RenderTexture: 找不到纹理资源'%m'", tex_name);
				return false;
			}
			LAPP.SaveTexture(resTex->GetTexture(), luaL_checkstring(L, 2));
			return 0;
		}
		static int Execute(lua_State* L)LNOEXCEPT
		{
			struct Detail_
			{
				LNOINLINE static bool Execute(const char* path, const char* args, const char* directory, bool bWait, bool bShow)LNOEXCEPT
				{
					wstring tPath, tArgs, tDirectory;

					try
					{
						tPath = fcyStringHelper::MultiByteToWideChar(path, CP_UTF8);
						tArgs = fcyStringHelper::MultiByteToWideChar(args, CP_UTF8);
						if (directory)
							tDirectory = fcyStringHelper::MultiByteToWideChar(directory, CP_UTF8);

						SHELLEXECUTEINFO tShellExecuteInfo;
						memset(&tShellExecuteInfo, 0, sizeof(SHELLEXECUTEINFO));

						tShellExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFO);
						tShellExecuteInfo.fMask = bWait ? SEE_MASK_NOCLOSEPROCESS : 0;
						tShellExecuteInfo.lpVerb = L"open";
						tShellExecuteInfo.lpFile = tPath.c_str();
						tShellExecuteInfo.lpParameters = tArgs.c_str();
						tShellExecuteInfo.lpDirectory = directory ? tDirectory.c_str() : nullptr;
						tShellExecuteInfo.nShow = bShow ? SW_SHOWDEFAULT : SW_HIDE;
						
						if (FALSE == ShellExecuteEx(&tShellExecuteInfo))
							return false;

						if (bWait)
						{
							WaitForSingleObject(tShellExecuteInfo.hProcess, INFINITE);
							CloseHandle(tShellExecuteInfo.hProcess);
						}
						return true;
					}
					catch (const std::bad_alloc&)
					{
						return false;
					}
				}
			};

			const char* path = luaL_checkstring(L, 1);
			const char* args = luaL_optstring(L, 2, "");
			const char* directory = luaL_optstring(L, 3, NULL);
			bool bWait = true;
			bool bShow = true;
			if (lua_gettop(L) >= 4)
				bWait = lua_toboolean(L, 4) == 0 ? false : true;
			if (lua_gettop(L) >= 5)
				bShow = lua_toboolean(L, 5) == 0 ? false : true;
			
			lua_pushboolean(L, Detail_::Execute(path, args, directory, bWait, bShow));
			return 1;
		}
		#pragma endregion
		
		#pragma region 内置数学库
		static int Sin(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, sin(luaL_checknumber(L, 1) * LDEGREE2RAD));
			return 1;
		}
		static int Cos(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, cos(luaL_checknumber(L, 1) * LDEGREE2RAD));
			return 1;
		}
		static int ASin(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, asin(luaL_checknumber(L, 1)) * LRAD2DEGREE);
			return 1;
		}
		static int ACos(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, acos(luaL_checknumber(L, 1)) * LRAD2DEGREE);
			return 1;
		}
		static int Tan(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, tan(luaL_checknumber(L, 1) * LDEGREE2RAD));
			return 1;
		}
		static int ATan(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, atan(luaL_checknumber(L, 1)) * LRAD2DEGREE);
			return 1;
		}
		static int ATan2(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, atan2(luaL_checknumber(L, 1), luaL_checknumber(L, 2)) * LRAD2DEGREE);
			return 1;
		}
		#pragma endregion
	};
	
	luaL_Reg tFunctions[] =
	{
		#pragma region 框架函数
		{ "SetWindowed", &WrapperImplement::SetWindowed },
		{ "SetFPS", &WrapperImplement::SetFPS },
		{ "GetFPS", &WrapperImplement::GetFPS },
		{ "SetVsync", &WrapperImplement::SetVsync },
		{ "SetResolution", &WrapperImplement::SetResolution },
		{ "Log", &WrapperImplement::Log },
		{ "SystemLog", &WrapperImplement::SystemLog },
		{ "Print", &WrapperImplement::Print },
		{ "LoadPack", &WrapperImplement::LoadPack },
		{ "LoadPackSub", &WrapperImplement::LoadPackSub },
		{ "UnloadPack", &WrapperImplement::UnloadPack },
#ifndef USING_ENCRYPTION
		{ "ExtractRes", &WrapperImplement::ExtractRes },
#endif // !USING_ENCRYPTION
		{ "DoFile", &WrapperImplement::DoFile },
		{ "LoadTextFile", &WrapperImplement::LoadTextFile },
		{ "FindFiles", &WrapperImplement::FindFiles },
		#pragma endregion
		
		#pragma region 窗口与交换链控制函数
		{ "SetSplash", &WrapperImplement::SetSplash },
		{ "SetTitle", &WrapperImplement::SetTitle },
		{ "ChangeVideoMode", &WrapperImplement::ChangeVideoMode },
		{ "MoveWindowToCenter", &WrapperImplement::MoveWindowToCenter },
		{ "SetWindowStyle", &WrapperImplement::SetWindowStyle },
		{ "SetWindowSize", &WrapperImplement::SetWindowSize },
		{ "SetWindowTopMost", &WrapperImplement::SetWindowTopMost },
		{ "SetSwapChainSize", &WrapperImplement::SetSwapChainSize },
		{ "EnumResolutions", &WrapperImplement::EnumResolutions },
		#pragma endregion
		
		#pragma region 游戏对象
		//对象池管理
		{ "GetnObj", &WrapperImplement::GetnObj },
		{ "ObjFrame", &WrapperImplement::ObjFrame },
		{ "ObjRender", &WrapperImplement::ObjRender },
		{ "BoundCheck", &WrapperImplement::BoundCheck },
		{ "SetBound", &WrapperImplement::SetBound },
		{ "CollisionCheck", &WrapperImplement::CollisionCheck },
		{ "UpdateXY", &WrapperImplement::UpdateXY },
		{ "AfterFrame", &WrapperImplement::AfterFrame },
		{ "ResetPool", &WrapperImplement::ResetPool },
		{ "NextObject", &WrapperImplement::NextObject },
		{ "ObjList", &WrapperImplement::ObjList },
		{ "ObjTable", &WrapperImplement::ObjTable },
		// 对象控制函数
		{ "BoxCheck", &WrapperImplement::BoxCheck },
		{ "ResetObject", &WrapperImplement::ResetObject },
		{ "New", &WrapperImplement::New },
		{ "Del", &WrapperImplement::Del },
		{ "Kill", &WrapperImplement::Kill },
		{ "IsValid", &WrapperImplement::IsValid },
		{ "Angle", &WrapperImplement::Angle },
		{ "Dist", &WrapperImplement::Dist },
		{ "ColliCheck", &WrapperImplement::ColliCheck },
		{ "GetV", &WrapperImplement::GetV },
		{ "SetV", &WrapperImplement::SetV },
		{ "DefaultRenderFunc", &WrapperImplement::DefaultRenderFunc },
		{ "GetAttr", &WrapperImplement::ObjMetaIndex },
		{ "SetAttr", &WrapperImplement::ObjMetaNewIndex },
		//对象资源控制
		{ "SetImgState", &WrapperImplement::SetImgState },
		{ "SetParState", &WrapperImplement::SetParState },
		{ "ParticleStop", &WrapperImplement::ParticleStop },
		{ "ParticleFire", &WrapperImplement::ParticleFire },
		{ "ParticleGetn", &WrapperImplement::ParticleGetn },
		{ "ParticleGetEmission", &WrapperImplement::ParticleGetEmission },
		{ "ParticleSetEmission", &WrapperImplement::ParticleSetEmission },
		//ESC
		{ "SetSuperPause", &WrapperImplement::SetSuperPause },
		{ "GetSuperPause", &WrapperImplement::GetSuperPause },
		{ "AddSuperPause", &WrapperImplement::AddSuperPause },
		{ "GetCurrentSuperPause", &WrapperImplement::GetCurrentSuperPause },
		{ "GetWorldFlag", &WrapperImplement::GetWorldFlag },
		{ "SetWorldFlag", &WrapperImplement::SetWorldFlag },
		{ "IsSameWorld", &WrapperImplement::CheckWorlds },
		{ "IsInWorld", &WrapperImplement::IsSameWorld },
		{ "GetCurrentObject", &WrapperImplement::GetCurrentObject },
		{ "ActiveWorlds", &WrapperImplement::ActiveWorlds },
		#pragma endregion

		#pragma region 资源控制函数
		{ "SetResourceStatus", &WrapperImplement::SetResourceStatus },
		{ "GetResourceStatus", &WrapperImplement::GetResourceStatus },
		{ "LoadTexture", &WrapperImplement::LoadTexture },
		{ "LoadImage", &WrapperImplement::LoadImage },
		{ "LoadAnimation", &WrapperImplement::LoadAnimation },
		{ "LoadPS", &WrapperImplement::LoadPS },
		{ "LoadSound", &WrapperImplement::LoadSound },
		{ "LoadMusic", &WrapperImplement::LoadMusic },
		{ "LoadFont", &WrapperImplement::LoadFont },
		{ "LoadTTF", &WrapperImplement::LoadTTF },
		{ "LoadTrueTypeFont", &WrapperImplement::LoadTrueTypeFont },
		{ "LoadFX", &WrapperImplement::LoadFX },
		{ "CreateRenderTarget", &WrapperImplement::CreateRenderTarget },
		{ "IsRenderTarget", &WrapperImplement::IsRenderTarget },
		{ "GetTextureSize", &WrapperImplement::GetTextureSize },
		{ "RemoveResource", &WrapperImplement::RemoveResource },
		{ "CheckRes", &WrapperImplement::CheckRes },
		{ "EnumRes", &WrapperImplement::EnumRes },
		{ "SetImageScale", &WrapperImplement::SetImageScale },
		{ "GetImageScale", &WrapperImplement::GetImageScale },
		{ "SetImageState", &WrapperImplement::SetImageState },//这个不一样
		{ "SetFontState", &WrapperImplement::SetFontState },
		{ "SetAnimationState", &WrapperImplement::SetAnimationState },
		{ "SetImageCenter", &WrapperImplement::SetImageCenter },
		{ "SetAnimationCenter", &WrapperImplement::SetAnimationCenter },
		{ "CacheTTFString", &WrapperImplement::CacheTTFString },
		//ESC
		{ "LoadModel", &WrapperImplement::LoadModel },
		#pragma endregion

		#pragma region 绘图函数
		{ "BeginScene", &WrapperImplement::BeginScene },
		{ "EndScene", &WrapperImplement::EndScene },
		{ "RenderClear", &WrapperImplement::RenderClear },
		{ "SetViewport", &WrapperImplement::SetViewport },
		{ "SetScissorRect", &WrapperImplement::SetScissorRect },
		{ "SetOrtho", &WrapperImplement::SetOrtho },
		{ "SetPerspective", &WrapperImplement::SetPerspective },
		{ "SetTextureSamplerState", &WrapperImplement::SetTextureSamplerState },
		{ "Render", &WrapperImplement::Render },
		{ "RenderAnimation", &WrapperImplement::RenderAnimation },
		{ "RenderRect", &WrapperImplement::RenderRect },
		{ "Render4V", &WrapperImplement::Render4V },
		{ "RenderText", &WrapperImplement::RenderText },
		{ "RenderTexture", &WrapperImplement::RenderTexture },
		{ "RenderTTF", &WrapperImplement::RenderTTF },
		{ "SetFog", &WrapperImplement::SetFog },
		{ "PushRenderTarget", &WrapperImplement::PushRenderTarget },
		{ "PopRenderTarget", &WrapperImplement::PopRenderTarget },
		{ "PostEffect", &WrapperImplement::PostEffect },
		//ESC
		{ "SetZBufferEnable", &WrapperImplement::SetZBufferEnable },
		{ "ClearZBuffer", &WrapperImplement::ClearZBuffer },
		{ "RenderModel", &WrapperImplement::RenderModel },
		{ "DrawCollider", &WrapperImplement::DrawCollider },
		//ETC
		{ "RenderGroupCollider", &WrapperImplement::RenderGroupCollider },
		{ "RenderTextureSector", &WrapperImplement::RenderTextureSector },
		{ "RenderTextureAnnulus", &WrapperImplement::RenderTextureAnnulus },
		#pragma endregion
		
		#pragma region 声音控制函数
		{ "PlaySound", &WrapperImplement::PlaySound },
		{ "StopSound", &WrapperImplement::StopSound },
		{ "PauseSound", &WrapperImplement::PauseSound },
		{ "ResumeSound", &WrapperImplement::ResumeSound },
		{ "GetSoundState", &WrapperImplement::GetSoundState },
		{ "PlayMusic", &WrapperImplement::PlayMusic },
		{ "StopMusic", &WrapperImplement::StopMusic },
		{ "PauseMusic", &WrapperImplement::PauseMusic },
		{ "ResumeMusic", &WrapperImplement::ResumeMusic },
		{ "GetMusicState", &WrapperImplement::GetMusicState },
		{ "UpdateSound", &WrapperImplement::UpdateSound },
		{ "SetSEVolume", &WrapperImplement::SetSEVolume },
		{ "SetBGMVolume", &WrapperImplement::SetBGMVolume },
		{ "GetSEVolume", &WrapperImplement::GetSEVolume },
		{ "GetBGMVolume", &WrapperImplement::GetBGMVolume },
		{ "SetSESpeed", &WrapperImplement::SetSESpeed },
		{ "GetSESpeed", &WrapperImplement::GetSESpeed },
		{ "SetBGMSpeed", &WrapperImplement::SetBGMSpeed },
		{ "GetBGMSpeed", &WrapperImplement::GetBGMSpeed },
		#pragma endregion
		
		#pragma region 输入控制函数
		{ "GetKeyState", &WrapperImplement::GetKeyState },
		{ "GetLastKey", &WrapperImplement::GetLastKey },
		{ "GetLastChar", &WrapperImplement::GetLastChar },
		{ "GetMousePosition", &WrapperImplement::GetMousePosition },
		{ "GetMouseWheelDelta", &WrapperImplement::GetMouseWheelDelta },
		{ "GetMouseState", &WrapperImplement::GetMouseState },
		//Raw Input
		{ "GetKeyboardState", &WrapperImplement::GetKeyboardState },
		{ "GetAsyncKeyState", &WrapperImplement::GetAsyncKeyState },
		#pragma endregion
		
		#pragma region 内置数学函数
		{ "sin", &WrapperImplement::Sin },
		{ "cos", &WrapperImplement::Cos },
		{ "asin", &WrapperImplement::ASin },
		{ "acos", &WrapperImplement::ACos },
		{ "tan", &WrapperImplement::Tan },
		{ "atan", &WrapperImplement::ATan },
		{ "atan2", &WrapperImplement::ATan2 },
		#pragma endregion
		
		#pragma region 杂项
		{ "Snapshot", &WrapperImplement::Snapshot },
		{ "SaveTexture", &WrapperImplement::SaveTexture },
		{ "Execute", &WrapperImplement::Execute },
		#pragma endregion
		
		{ NULL, NULL }
	};
	
	lua_getglobal(L, "lstg");				// ... t
	::luaL_register(L, NULL, tFunctions);	// ... t
	lua_pop(L, 1);							// ...
}
