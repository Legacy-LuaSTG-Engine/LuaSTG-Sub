#include "LuaBinding/LuaWrapper.hpp"
#include <spine/Version.h>

using SpineInstance = spine::LuaSTGSpineInstance;

namespace
{
	enum class Mapper
	{
		x, y,
		vscale, hscale,
		getExistBones,
		getBoneInfo,
		update, reset,
		render,
		getExistAnimations,
		addAnimation, setAnimation,
	};

	const std::unordered_map<std::string_view, Mapper> KeyMapper
	{
		{ "x", Mapper::x },
		{ "y", Mapper::y },
		{ "vscale", Mapper::vscale },
		{ "hscale", Mapper::hscale },
		{ "getExistBones", Mapper::getExistBones },
		{ "getBoneInfo", Mapper::getBoneInfo },
		{ "update", Mapper::update },
		{ "reset", Mapper::reset },
		{ "render", Mapper::render },
		{ "getExistAnimations", Mapper::getExistAnimations },
		{ "addAnimation", Mapper::addAnimation },
		{ "setAnimation", Mapper::setAnimation },
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
			case Mapper::update :
				lua_pushcfunction(L, Wrapper::update);					break;
			case Mapper::reset :
				lua_pushcfunction(L, Wrapper::reset);					break;
			case Mapper::render :
				lua_pushcfunction(L, Wrapper::render);					break;
			case Mapper::getExistAnimations :
				lua_pushcfunction(L, Wrapper::getExistAnimations);		break;
			case Mapper::addAnimation :
				lua_pushcfunction(L, Wrapper::addAnimation);			break;
			case Mapper::setAnimation :
				lua_pushcfunction(L, Wrapper::setAnimation);			break;
			default :
				lua_pushnil(L);											break;
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
				data->getSkeleton()->setX(luaL_checknumber(L, 3));			break;
			case Mapper::y:
				data->getSkeleton()->setY(luaL_checknumber(L, 3));			break;
			case Mapper::vscale:
				data->getSkeleton()->setScaleX(luaL_checknumber(L, 3));		break;
			case Mapper::hscale:
				data->getSkeleton()->setScaleY(luaL_checknumber(L, 3));		break;
			
			default:
				break;
			}
			return 0;
		}

		static int update(lua_State* L) 
		{
			GETUDATA(data, 1);
			auto delta_t = luaL_checknumber(L, 2);
			
#if SPINE_MAJOR_VERSION >= 4
			int physic_state = 0;
			if (lua_gettop(L) >= 3) physic_state = luaL_checkint(L, 3);
			
			spine::Physics physics;
			switch (physic_state)
			{
			case 0:		physics = spine::Physics_Update;	break;
			case 1:		physics = spine::Physics_None;		break;
			case 2:		physics = spine::Physics_Pose;		break;
			case 3:		physics = spine::Physics_Reset;		break;
			default:	physics = spine::Physics_Update;	break;
			}	
#endif

			auto animation_state = data->getAnimationState();
			auto skeleton = data->getSkeleton();
			animation_state->update(delta_t);
			animation_state->apply(*skeleton);
#if SPINE_MAJOR_VERSION >= 4
			skeleton->updateWorldTransform(physics);
#else
			skeleton->updateWorldTramsform();
#endif
			return 0;
		}
		static int reset(lua_State* L)
		{
			GETUDATA(data, 1);
			auto animationState = data->getAnimationState();
			auto skeleton = data->getSkeleton();

			skeleton->setToSetupPose();
			skeleton->setSlotsToSetupPose();

			animationState->clearTracks();
			animationState->setTimeScale(1.0f);
			animationState->setListener(spine::LuaSTGdummyOnAnimationEventFunc);
			animationState->update(0);
			animationState->apply(*skeleton);
#if SPINE_MAJOR_VERSION >= 4
			skeleton->updateWorldTransform(spine::Physics_Reset);
#else
			skeleton->updateWorldTransform();
#endif

			return 0;
		}
		static int render(lua_State* L)
		{
			GETUDATA(data, 1);
			
			auto* ctx = LAPP.GetAppModel()->getRenderer();
			core::Graphics::IRenderer::DrawVertex vertex[3];
			vertex[0].z = 0.5f; vertex[1].z = 0.5f; vertex[2].z = 0.5f;
			
			auto skeleton = data->getSkeleton();
			auto& skeletonRenderer = spine::LuaSTGSkeletonRenderer::Instance();
			spine::RenderCommand* command = skeletonRenderer.render(*skeleton);
			while (command) {
				float* positions = command->positions;
				float* uvs = command->uvs;
				uint32_t* colors = command->colors;
				uint16_t* indices = command->indices;
				int32_t n_indices = command->numIndices;
				int32_t n_vertices = command->numVertices;
				
				core::Graphics::ITexture2D* texture = (core::Graphics::ITexture2D*)command->texture;
				ctx->setTexture(texture);

				luastg::BlendMode mode;
				switch (command->blendMode)
				{
				case spine::BlendMode_Additive:
					mode = luastg::BlendMode::AddAdd;			break;
				case spine::BlendMode_Multiply:
					mode = luastg::BlendMode::AddMutiply;		break;
				case spine::BlendMode_Screen:
					mode = luastg::BlendMode::AddScreen;		break;
				case spine::BlendMode_Normal:	// fall through
				default: mode = luastg::BlendMode::MulAlpha;	break;
				}
				LAPP.updateGraph2DBlendMode(mode);
				
				auto vertices = new core::Graphics::IRenderer::DrawVertex[n_vertices];
				for (int i = 0, n = command->numVertices; i < n; i++)
				{
					vertices[i].x = positions[i << 1];
					vertices[i].y = positions[(i << 1) + 1];
					vertices[i].z = 0.5f;
					vertices[i].u = uvs[i << 1];
					vertices[i].v = uvs[(i << 1) + 1];
					vertices[i].color = colors[i];
				}
				ctx->drawRaw(vertices, n_vertices, indices, n_indices);
				delete[] vertices;
				command = command->next;
			}

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
		static int getExistAnimations(lua_State* L)
		{
			GETUDATA(data, 1);
			auto& animations = data->getAllAnimations();

			lua_createtable(L, animations.size(), 0);		// ..args t

			int count = 1;
			for (const auto& [k, _] : animations)
			{
				lua_pushlstring(L, k.data(), k.length());	// ..args t v
				lua_rawseti(L, -2, count++);				// ..args t
			}

			return 1;
		}
		static int addAnimation(lua_State* L)
		{
			GETUDATA(data, 1);
			const char* ani_name = luaL_checkstring(L, 2);
			const size_t track_index = luaL_checkinteger(L, 3);
			const float delay = luaL_checknumber(L, 4);
			const bool loop = lua_toboolean(L, 5);


			spine::Animation* ani = data->findAnimation(ani_name);
			if (!ani) return luaL_error(L, "could not find animation '%s' from spine skeleton '%s'.", ani_name, std::string(data->getName()).c_str());

			auto animationState = data->getAnimationState();
			animationState->addAnimation(track_index, ani, loop, delay);

			return 0;
		}
		static int setAnimation(lua_State* L)
		{
			GETUDATA(data, 1);
			const char* ani_name = luaL_checkstring(L, 2);
			const size_t track_index = luaL_checkinteger(L, 3);
			const float delay = luaL_checknumber(L, 4);
			const bool loop = lua_toboolean(L, 5);


			spine::Animation* ani = data->findAnimation(ani_name);
			if (!ani) return luaL_error(L, "could not find animation '%s' from spine skeleton '%s'.", ani_name, std::string(data->getName()).c_str());

			auto animationState = data->getAnimationState();
			animationState->setAnimation(track_index, ani, loop);

			return 0;
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
		{ "update", &Wrapper::update },
		{ "reset", &Wrapper::reset },
		{ "render", &Wrapper::render },
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