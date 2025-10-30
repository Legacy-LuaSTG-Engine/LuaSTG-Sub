#include "LuaBinding/LuaWrapper.hpp"

using SpineInstance = spine::LuaSTGSpineInstance;

namespace
{
	enum class Mapper
	{
		x, y,
		vscale, hscale,
		getExistBones,
		getBoneInfo,
	};

	const std::unordered_map<std::string_view, Mapper> KeyMapper
	{
		{ "x", Mapper::x },
		{ "y", Mapper::y },
		{ "vscale", Mapper::vscale },
		{ "hscale", Mapper::hscale },
		{ "getExistBones", Mapper::getExistBones },
		{ "getBoneInfo", Mapper::getBoneInfo },

	};
}

void luastg::binding::Spine::Register(lua_State* L) noexcept
{
#define GETUDATA(p, i) SpineInstance* (p) = static_cast<SpineInstance*>(luaL_checkudata(L, (i), LUASTG_LUA_TYPENAME_SPINE));	
	struct Wrapper
	{
		static int CreateSpineInstance(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			
			core::SmartReference<IResourceSpineSkeleton> pRes = LRES.FindSpineSkeleton(name);
			if (!pRes) return luaL_error(L, "could not find spine skeleton '%s'.", name);

			luastg::binding::Spine::CreateAndPush(L, pRes.get());
			return 1;
		}
		static int __gc(lua_State* L) noexcept
		{
			GETUDATA(data, 1);
			data->~SpineInstance();

			return 0;
		}
		static int __index(lua_State* L)
		{
			GETUDATA(data, 1);
			const char* key = luaL_checkstring(L, 2);

			if (!KeyMapper.contains(key)) { lua_pushnil(L); return 1; }
			
			switch (KeyMapper.at(key))
			{
			case Mapper::x :
				lua_pushnumber(L, data->getSkeleton()->getX());			break;
			case Mapper::y :
				lua_pushnumber(L, data->getSkeleton()->getY());			break;
			case Mapper::vscale :
				lua_pushnumber(L, data->getSkeleton()->getScaleX());	break;
			case Mapper::hscale :
				lua_pushnumber(L, data->getSkeleton()->getScaleY());	break;
			case Mapper::getExistBones :
				lua_pushcfunction(L, Wrapper::getExistBones);			break;
			case Mapper::getBoneInfo :
				lua_pushcfunction(L, Wrapper::getBoneInfo);				break;
			
			default :
				;
			};
			
			return 1;
		}
		static int __newindex(lua_State* L)
		{
			GETUDATA(data, 1);
			const char* key = luaL_checkstring(L, 2);

			if (!KeyMapper.contains(key)) { lua_pushnil(L); return 1; }

			switch (KeyMapper.at(key))
			{
			case Mapper::x:
				data->getSkeleton()->setX(luaL_checknumber(L, 3));
			case Mapper::y:
				data->getSkeleton()->setY(luaL_checknumber(L, 3));
			case Mapper::vscale:
				data->getSkeleton()->setScaleX(luaL_checknumber(L, 3));
			case Mapper::hscale:
				data->getSkeleton()->setScaleY(luaL_checknumber(L, 3));
			
			default:
				;
			};
			return 0;
		}

		static int getExistBones(lua_State* L)
		{
			GETUDATA(data, 1);
			auto& bones = data->getAllBones();

			lua_createtable(L, bones.size(), 0);			// ..args t

			int count = 1;
			for (const auto& [k, _] : bones)
			{
				lua_pushlstring(L, k.data(), k.length());	// ..args t v
				lua_rawseti(L, -2, count++);				// ..args t
			}

			return 1;
		}
		static int getBoneInfo(lua_State* L)
		{
			GETUDATA(data, 1);
			const char* bone_name = luaL_checkstring(L, 2);

			spine::Bone* bone = data->findBone(bone_name);
			if (!bone) return luaL_error(L, "could not find bone '%s' from spine skeleton '%s'.", bone_name, std::string(data->getName()).c_str());
			
			// 不知道这些信息哪些有用 总之全给了.jpg	
			lua_createtable(L, 0, 13);						// ..args t
			lua_pushnumber(L, bone->getX());				// ..args t val
			lua_setfield(L, -2, "x");						// ..args t
			lua_pushnumber(L, bone->getY());				// ..args t val
			lua_setfield(L, -2, "y");						// ..args t
			lua_pushnumber(L, bone->getWorldX());			// ..args t val
			lua_setfield(L, -2, "world_x");					// ..args t
			lua_pushnumber(L, bone->getWorldY());			// ..args t val
			lua_setfield(L, -2, "world_y");					// ..args t
			lua_pushnumber(L, bone->getRotation());			// ..args t val
			lua_setfield(L, -2, "rot");						// ..args t
			lua_pushnumber(L, bone->getWorldRotationX());	// ..args t val
			lua_setfield(L, -2, "world_rot_x");				// ..args t
			lua_pushnumber(L, bone->getWorldRotationY());	// ..args t val
			lua_setfield(L, -2, "world_rot_y");				// ..args t
			lua_pushnumber(L, bone->getScaleX());			// ..args t val
			lua_setfield(L, -2, "vscale");					// ..args t
			lua_pushnumber(L, bone->getScaleY());			// ..args t val
			lua_setfield(L, -2, "hscale");					// ..args t
			lua_pushnumber(L, bone->getWorldScaleX());		// ..args t val
			lua_setfield(L, -2, "world_vscale");			// ..args t
			lua_pushnumber(L, bone->getWorldScaleY());		// ..args t val
			lua_setfield(L, -2, "world_hscale");			// ..args t
			lua_pushnumber(L, bone->getShearX());			// ..args t val
			lua_setfield(L, -2, "shear_x");					// ..args t
			lua_pushnumber(L, bone->getShearY());			// ..args t val
			lua_setfield(L, -2, "shear_y");					// ..args t

			return 1;
		}
		static int GetEvent()
		{

		}
		static int SetEventListener()
		{

		}


	};
#undef GETUDATA

	luaL_Reg const lib[] = {
		{ "getExistBones", &Wrapper::getExistBones },
		{ "getBoneInfo", &Wrapper::getBoneInfo },
		{ NULL, NULL },
	};

	luaL_Reg const mt[] = {
		{ "__gc", &Wrapper::__gc },
		{ "__index", &Wrapper::__index },
		{ "__newindex", &Wrapper::__newindex },
		{ NULL, NULL },
	};

	luaL_Reg const ins[] = {
		{ "CreateSpineInstance", Wrapper::CreateSpineInstance },
		{ NULL, NULL }
	};

	luaL_register(L, "lstg", ins); // ??? lstg
	RegisterClassIntoTable2(L, ".Spine", lib, LUASTG_LUA_TYPENAME_SPINE, mt);
	lua_pop(L, 1);
}

void luastg::binding::Spine::CreateAndPush(lua_State* L, IResourceSpineSkeleton* data)
{
	SpineInstance* p = static_cast<SpineInstance*>(lua_newuserdata(L, sizeof(SpineInstance))); // udata
	new(p) SpineInstance(data->GetResName(), data->getSkeletonData(), data->getAnimationStateData());
	luaL_getmetatable(L, LUASTG_LUA_TYPENAME_SPINE); // udata mt
	lua_setmetatable(L, -2); // udata
}