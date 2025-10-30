#include "LuaBinding/LuaWrapper.hpp"

void luastg::binding::Spine::Register(lua_State* L) noexcept
{
#define GETUDATA(p, i) spine::Skeleton* (p) = static_cast<spine::Skeleton*>(luaL_checkudata(L, (i), LUASTG_LUA_TYPENAME_SPINE));
	struct Wrapper
	{
		static int CreateSpineInstance(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			
			core::SmartReference<IResourceSpineSkeleton> pRes;
			if (!pRes) return luaL_error(L, "could not find spineSkeleton '%s'.", name);

			luastg::binding::Spine::CreateAndPush(L, pRes->getSkeletonData());
			return 1;
		}
		static int __gc(lua_State* L) noexcept
		{
			GETUDATA(data, 1);
			data->~Skeleton();
			return 0;
		}


	};
#undef GETUDATA
	luaL_Reg const lib[] = {
		{ NULL, NULL },
	};

	luaL_Reg const mt[] = {
		{ "__gc", &Wrapper::__gc },
		{ NULL, NULL },
	};

	luaL_Reg const ins[] = {
		{ "CreateSpineInstance", Wrapper::CreateSpineInstance },
		{ NULL, NULL }
	};

	luaL_register(L, "lstg", ins); // ??? lstg
	RegisterClassIntoTable(L, ".Spine", lib, LUASTG_LUA_TYPENAME_SPINE, mt);
	lua_pop(L, 1);
}

void luastg::binding::Spine::CreateAndPush(lua_State* L, spine::SkeletonData* data)
{
	spine::Skeleton* p = static_cast<spine::Skeleton*>(lua_newuserdata(L, sizeof(spine::Skeleton))); // udata
	new(p) spine::Skeleton(data);
	luaL_getmetatable(L, LUASTG_LUA_TYPENAME_SPINE); // udata mt
	lua_setmetatable(L, -2); // udata
}