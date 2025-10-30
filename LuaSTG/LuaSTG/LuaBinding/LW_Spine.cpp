#include "LuaBinding/LuaWrapper.hpp"
#include <spine/Version.h>

namespace spine
{
	class LuaSTGSpineInstance
	{
	private:
		std::string_view resname;
		std::unique_ptr<Skeleton> skeleton;
		std::unique_ptr<AnimationState> anistate;
		std::unordered_map<std::string_view, Bone*> bonecache;
		std::unordered_map<std::string_view, Animation*> animationcache;
		int animation_callback = LUA_NOREF;
		int event_callback = LUA_NOREF;
		int self_weak_table = LUA_NOREF;
	public:
		const std::string_view& getName();
		const std::unordered_map<std::string_view, Bone*>& getAllBones();
		const std::unordered_map<std::string_view, Animation*>& getAllAnimations();
		Skeleton* getSkeleton();
		AnimationState* getAnimationState();
		Bone* findBone(const char* name);
		Animation* findAnimation(const char* name);
		void setAnimationCallback(lua_State* L, int position);
		void setEventCallback(lua_State* L, int position);
		bool pushAnimationCallback(lua_State* L);
		bool pushEventCallback(lua_State* L);
		void makeSelfWeakTable(lua_State* L, int self_position);
		bool pushSelf(lua_State* L);
	public:
		LuaSTGSpineInstance(const std::string_view& name, spine::SkeletonData* skeldata, spine::AnimationStateData* anidata);
		~LuaSTGSpineInstance();
	};

	LuaSTGSpineInstance::LuaSTGSpineInstance(const std::string_view& name, spine::SkeletonData* skeldata, spine::AnimationStateData* anidata)
		: resname(name)
		, skeleton(new spine::Skeleton(skeldata))
		, anistate(new spine::AnimationState(anidata))
	{
		// name -> bone mapping
		const auto& bones = skeleton->getBones();
		const auto bone_size = bones.size();
		for (int i = 0; i < bone_size; i++) bonecache[bones[i]->getData().getName().buffer()] = bones[i];
		// name -> ani mapping
		const auto& animations = skeldata->getAnimations();
		const auto ani_size = animations.size();
		for (int i = 0; i < ani_size; i++) animationcache[animations[i]->getName().buffer()] = animations[i];
	};
	const std::string_view& LuaSTGSpineInstance::getName() { return resname; }
	const std::unordered_map<std::string_view, Bone*>& LuaSTGSpineInstance::getAllBones() { return bonecache; }
	const std::unordered_map<std::string_view, Animation*>& LuaSTGSpineInstance::getAllAnimations() { return animationcache; }
	Skeleton* LuaSTGSpineInstance::getSkeleton() { return skeleton.get(); }
	AnimationState* LuaSTGSpineInstance::getAnimationState() { return anistate.get(); }
	Bone* LuaSTGSpineInstance::findBone(const char* name) { return bonecache.contains(name) ? bonecache[name] : nullptr; }
	Animation* LuaSTGSpineInstance::findAnimation(const char* name) { return animationcache.contains(name) ? animationcache[name] : nullptr; }
	void LuaSTGSpineInstance::setAnimationCallback(lua_State* L, int position)
	{
		if (animation_callback != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, animation_callback);
		lua_pushvalue(L, position);
		animation_callback = luaL_ref(L, LUA_REGISTRYINDEX);
	};
	void LuaSTGSpineInstance::setEventCallback(lua_State* L, int position)
	{
		if (event_callback != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, event_callback);
		lua_pushvalue(L, position);
		event_callback = luaL_ref(L, LUA_REGISTRYINDEX);
	};
	bool LuaSTGSpineInstance::pushAnimationCallback(lua_State* L)
	{
		if (animation_callback == LUA_NOREF) return false;
		lua_rawgeti(L, LUA_REGISTRYINDEX, animation_callback);
		return true;
	}
	bool LuaSTGSpineInstance::pushEventCallback(lua_State* L)
	{
		if (event_callback == LUA_NOREF) return false;
		lua_rawgeti(L, LUA_REGISTRYINDEX, event_callback);
		return true;
	}
	LuaSTGSpineInstance::~LuaSTGSpineInstance()
	{
		lua_State* L = LAPP.GetLuaEngine();
		if (animation_callback != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, animation_callback);
		if (event_callback != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, event_callback);
		if (self_weak_table != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, self_weak_table);
	}
	void LuaSTGSpineInstance::makeSelfWeakTable(lua_State* L, int self_position)
	{
		if (self_weak_table != LUA_NOREF) return;

		lua_newtable(L);									// ..arg t
		lua_newtable(L);									// ..arg t mt
		lua_pushstring(L, "v");								// ..arg t mt "v"
		lua_setfield(L, -2, "__mode");						// ..arg t mt
		lua_setmetatable(L, -2);							// ..arg t
		lua_pushvalue(L, self_position);					// ..arg t self
		lua_setfield(L, -2, "self");						// ..arg t
		self_weak_table = luaL_ref(L, LUA_REGISTRYINDEX);	// ..arg
	}
	bool LuaSTGSpineInstance::pushSelf(lua_State* L)
	{
		if (self_weak_table == LUA_NOREF) return false;

		lua_rawgeti(L, LUA_REGISTRYINDEX, self_weak_table);		// ..arg t
		lua_getfield(L, -1, "self");							// ..arg t self
		lua_remove(L, -2);										// ..arg self

		return true;
	}
}

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
		addAnimation, setAnimation, getCurrentAnimation,
		getExistEvents, setEventListener,
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
		{ "getCurrentAnimation", Mapper::getCurrentAnimation },
		{ "getExistEvents", Mapper::getExistEvents },
		{ "setEventListener", Mapper::setEventListener },
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
			case Mapper::setAnimation:
				lua_pushcfunction(L, Wrapper::setAnimation);			break;
			case Mapper::getCurrentAnimation:
				lua_pushcfunction(L, Wrapper::getCurrentAnimation);		break;
			case Mapper::getExistEvents:
				lua_pushcfunction(L, Wrapper::getExistEvents);			break;
			case Mapper::setEventListener:
				lua_pushcfunction(L, Wrapper::setEventListener);		break;
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
		static int getCurrentAnimation(lua_State* L)
		{
			GETUDATA(data, 1);
			const size_t track_index = luaL_checkinteger(L, 2);

			auto animationState = data->getAnimationState();
			auto current = animationState->getCurrent(track_index);

			if (!current) 
			{
				lua_pushnil(L);
				return 1;
			}

			const auto& ani_name = current->getAnimation()->getName();
			lua_pushlstring(L, ani_name.buffer(), ani_name.length());

			return 1;
		}
		static int getExistEvents(lua_State* L)
		{
			GETUDATA(data, 1);
			const auto& events = data->getSkeleton()->getData()->getEvents();
		
			const auto size = events.size();
			lua_createtable(L, size, 0);		// ..args t

			int count = 1;
			for (int i = 0; i < size; i++)
			{
				const auto& event = events[i]->getName();
				lua_pushlstring(L, event.buffer(), event.length());	// ..args t v
				lua_rawseti(L, -2, count++);						// ..args t
			}

			return 1;
		}
		static int setEventListener(lua_State* L)
		{
			GETUDATA(data, 1);
			luaL_checktype(L, 2, LUA_TFUNCTION);	// animation callback
			luaL_checktype(L, 3, LUA_TFUNCTION);	// event callback
			
			data->makeSelfWeakTable(L, 1);
			data->setAnimationCallback(L, 2);
			data->setEventCallback(L, 3);
			
			auto listener = [data](spine::AnimationState* state, spine::EventType type, spine::TrackEntry* entry, spine::Event* event)
			{
				lua_State* L = LAPP.GetLuaEngine();
				int stack_top = lua_gettop(L);
				
				if (type == spine::EventType_Event) 
				{
					// spine name time fval ival  sval ba vol
					data->pushEventCallback(L);
					data->pushSelf(L);
					const auto& name = event->getData().getName();
					lua_pushlstring(L, name.buffer(), name.length());
					lua_pushinteger(L, event->getTime());
					lua_pushinteger(L, event->getFloatValue());
					lua_pushinteger(L, event->getIntValue());
					const auto& str = event->getStringValue();
					lua_pushlstring(L, str.buffer(), str.length());
					lua_pushnumber(L, event->getBalance());
					lua_pushnumber(L, event->getVolume());

					lua_call(L, 8, 0);
					lua_settop(L, stack_top);
					return;
				}
				
				data->pushAnimationCallback(L);
				data->pushSelf(L);
				const auto& name = entry->getAnimation()->getName();
				lua_pushlstring(L, name.buffer(), name.length());
				switch (type) {
				case spine::EventType_Start:
					lua_pushstring(L, "start");			break;
				case spine::EventType_Interrupt:
					lua_pushstring(L, "interrupt");		break;
				case spine::EventType_End:
					lua_pushstring(L, "end");			break;
				case spine::EventType_Complete:
					lua_pushstring(L, "complete");		break;
				case spine::EventType_Dispose:
					lua_pushstring(L, "dispose");		break;
				default:
					break;
				};
				lua_call(L, 3, 0);
				lua_settop(L, stack_top);
				
				return;
			};
			
			data->getAnimationState()->setListener(listener);


			return 1;
		}


	};
#undef GETUDATA

	luaL_Reg const lib[] = {
		{ "update", &Wrapper::update },
		{ "reset", &Wrapper::reset },
		{ "render", &Wrapper::render },
		{ "getExistBones", &Wrapper::getExistBones },
		{ "getBoneInfo", &Wrapper::getBoneInfo },
		{ "update", &Wrapper::update },
		{ "reset", &Wrapper::reset },
		{ "render", &Wrapper::render },
		{ "getExistAnimations", &Wrapper::getExistAnimations },
		{ "addAnimation", &Wrapper::addAnimation },
		{ "setAnimation", &Wrapper::setAnimation },
		{ "getCurrentAnimation", &Wrapper::getCurrentAnimation },
		{ "getExistEvents", &Wrapper::getExistEvents },
		{ "setEventListener", &Wrapper::setEventListener },
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
