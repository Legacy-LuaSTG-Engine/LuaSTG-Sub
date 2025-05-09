#include "GameObject/GameObjectPool.h"
#include "LuaBinding/LuaWrapper.hpp"
#include "lua/plus.hpp"
#include "AppFrame.h"

#define LOBJPOOL_SIZE_INTERNAL (LOBJPOOL_SIZE + 1)
#define LOBJPOOL_METATABLE_IDX (LOBJPOOL_SIZE_INTERNAL)

namespace luastg
{
	// --------------------------------------------------------------------------------

	static GameObjectPool* g_GameObjectPool = nullptr;

	GameObjectPool::GameObjectPool(lua_State* pL, ManagedAPI* clr_fn)
	{
		assert(g_GameObjectPool == nullptr);
		g_GameObjectPool = this;
		// Lua_State
		G_L = pL;
		CLR_fn = clr_fn;
		// 初始化对象链表
		_ClearLinkList();
		m_RenderList.clear();
		// ex+
#ifdef USING_MULTI_GAME_WORLD
		m_pCurrentObject = nullptr;
#endif // USING_MULTI_GAME_WORLD
		m_superpause = 0;
		m_nextsuperpause = 0;
		// lua
		_PrepareLuaObjectTable();
	}
	GameObjectPool::~GameObjectPool()
	{
		ResetPool();
		g_GameObjectPool = nullptr;
	}

	void GameObjectPool::_ClearLinkList()
	{
		m_UpdateLinkList.first.pUpdateNext = &m_UpdateLinkList.second;
		m_UpdateLinkList.first.pColliNext = &m_UpdateLinkList.second;
		m_UpdateLinkList.second.pUpdatePrev = &m_UpdateLinkList.first;
		m_UpdateLinkList.second.pColliPrev = &m_UpdateLinkList.first;
		m_UpdateLinkList.first.status = GameObjectStatus::Free;
		m_UpdateLinkList.first.uid = 0;
		m_UpdateLinkList.second.status = GameObjectStatus::Free;
		m_UpdateLinkList.second.uid = UINT64_MAX;
		for (size_t i = 0; i < LOBJPOOL_GROUPN; i += 1)
		{
			m_ColliLinkList[i].first.pUpdateNext = &m_ColliLinkList[i].second;
			m_ColliLinkList[i].first.pColliNext = &m_ColliLinkList[i].second;
			m_ColliLinkList[i].second.pUpdatePrev = &m_ColliLinkList[i].first;
			m_ColliLinkList[i].second.pColliPrev = &m_ColliLinkList[i].first;
			m_ColliLinkList[i].first.status = GameObjectStatus::Free;
			m_ColliLinkList[i].first.uid = 0;
			m_ColliLinkList[i].first.group = (lua_Integer)i;
			m_ColliLinkList[i].second.status = GameObjectStatus::Free;
			m_ColliLinkList[i].second.uid = UINT64_MAX;
			m_ColliLinkList[i].second.group = (lua_Integer)i;
		}
	}

	void GameObjectPool::_InsertToUpdateLinkList(GameObject* p)
	{
		GameObject* prev = m_UpdateLinkList.second.pUpdatePrev;
		GameObject* next = &m_UpdateLinkList.second;
		prev->pUpdateNext = p;
		p->pUpdatePrev = prev;
		p->pUpdateNext = next;
		next->pUpdatePrev = p;
	}
	void GameObjectPool::_RemoveFromUpdateLinkList(GameObject* p)
	{
		GameObject* prev = p->pUpdatePrev;
		GameObject* next = p->pUpdateNext;
		prev->pUpdateNext = next;
		next->pUpdatePrev = prev;
		p->pUpdatePrev = nullptr;
		p->pUpdateNext = nullptr;
	}

	void GameObjectPool::_InsertToColliLinkList(GameObject* p, size_t group)
	{
		GameObject* prev = m_ColliLinkList[group].second.pColliPrev;
		GameObject* next = &m_ColliLinkList[group].second;
		prev->pColliNext = p;
		p->pColliPrev = prev;
		p->pColliNext = next;
		next->pColliPrev = p;
	}
	void GameObjectPool::_RemoveFromColliLinkList(GameObject* p)
	{
		assert(p != m_LockObjectA && p != m_LockObjectB);
		GameObject* prev = p->pColliPrev;
		GameObject* next = p->pColliNext;
		prev->pColliNext = next;
		next->pColliPrev = prev;
		p->pColliPrev = nullptr;
		p->pColliNext = nullptr;
	}
	void GameObjectPool::_MoveToColliLinkList(GameObject* p, size_t group)
	{
		_RemoveFromColliLinkList(p);
		_InsertToColliLinkList(p, group);
	}

	void GameObjectPool::_InsertToRenderList(GameObject* p)
	{
		m_RenderList.insert(p);
	}
	void GameObjectPool::_RemoveFromRenderList(GameObject* p)
	{
		m_RenderList.erase(p);
	}
	void GameObjectPool::_SetObjectLayer(GameObject* object, lua_Number layer)
	{
		m_RenderList.erase(object);
		object->layer = layer;
		m_RenderList.insert(object);
	}

	void GameObjectPool::_PrepareLuaObjectTable()
	{
		luaL_Reg const mt[3] = {
			{ "__index", &api_GetAttr },
			{ "__newindex", &api_SetAttr },
			{ NULL, NULL },
		};

		// 创建一个全局表用于存放所有对象
		lua_pushlightuserdata(G_L, this);					// ??? p
		lua_createtable(G_L, LOBJPOOL_SIZE_INTERNAL, 0);	// ??? p ot

		// 创建对象元表
		lua_createtable(G_L, 0, 2);							// ??? p ot mt
		luaL_register(G_L, NULL, mt);						// ??? p ot mt
		lua_rawseti(G_L, -2, LOBJPOOL_METATABLE_IDX);		// ??? p ot

		// 保存对象表
		lua_settable(G_L, LUA_REGISTRYINDEX);				// ???
	}

	GameObject* GameObjectPool::_AllocObject()
	{
		size_t id = 0;
		if (!m_ObjectPool.alloc(id))
		{
			return nullptr;
		}
		GameObject* p = m_ObjectPool.object(id);
		p->Reset();
		p->status = GameObjectStatus::Active;
		p->id = id;
		p->uid = m_iUid;
		m_iUid++;
#ifdef USING_MULTI_GAME_WORLD
		if (m_pCurrentObject)
		{
			p->world = m_pCurrentObject->world;
		}
#endif // USING_MULTI_GAME_WORLD
		_InsertToUpdateLinkList(p);
		_InsertToRenderList(p);
		_InsertToColliLinkList(p, (size_t)p->group);
		m_DbgData[m_DbgIdx].object_alloc += 1;
		return p;
	}
	GameObject* GameObjectPool::_ReleaseObject(GameObject* object)
	{
		m_DbgData[m_DbgIdx].object_free += 1;
		GameObject* ret = object->pUpdateNext;
		_RemoveFromUpdateLinkList(object);
		_RemoveFromRenderList(object);
		_RemoveFromColliLinkList(object);
#ifdef USING_MULTI_GAME_WORLD
		if (m_pCurrentObject == object)
		{
			m_pCurrentObject = nullptr;
		}
#endif // USING_MULTI_GAME_WORLD
		object->status = GameObjectStatus::Free;
		m_ObjectPool.free(object->id);
		return ret;
	}
	GameObject* GameObjectPool::_FreeObject(GameObject* p, int ot_at) noexcept
	{
		int const index = (int)p->id + 1;
		int ot_stk = ot_at;

#if (defined(_DEBUG) && defined(LuaSTG_enable_GameObjectManager_Debug))
		static std::string _name("<null>");
		spdlog::debug("[object] free {}-{} (img = {})", p->id, p->uid, p->res ? p->res->GetResName() : _name);
#endif

		// 删除lua对象表中元素
		if (ot_at <= 0)
		{
			GetObjectTable(G_L);				// ot
			ot_stk = lua_gettop(G_L);
		}
		lua_rawgeti(G_L, ot_stk, index);		// ot object
#ifdef LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
		p->ReleaseLuaRC(G_L, lua_gettop(G_L));	// ot object				// 释放可能的粒子系统
#endif // LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
		lua_pushlightuserdata(G_L, nullptr);	// ot object nullptr
		lua_rawseti(G_L, -2, 3);				// ot object
		lua_pop(G_L, 1);						// ot
		lua_pushnil(G_L);						// ot nil
		lua_rawseti(G_L, ot_stk, index);		// ot
		if (ot_at <= 0)
		{
			lua_pop(G_L, 1);					// 
		}

		// 释放引用的资源
		p->ReleaseResource();

		CLR_fn->DetachGameObject(p->id);

		GameObject* pRet = _ReleaseObject(p);

		return pRet;
	}

	GameObject* GameObjectPool::_ToGameObject(lua_State* L, int idx)
	{
		if (!lua_istable(L, idx))
		{
			luaL_error(L, "invalid lstg object");
			return nullptr;
		}
		return _TableToGameObject(L, idx);
	}
	GameObject* GameObjectPool::_TableToGameObject(lua_State* L, int idx)
	{
#if (defined(_DEBUG) && defined(LuaSTG_enable_GameObjectManager_Debug))
		lua_rawgeti(L, idx, 2);
		size_t oidx = (size_t)(luaL_checkinteger(L, -1));
		lua_pop(L, 1);
		GameObject* pukn = m_ObjectPool.object(oidx);
#endif
		lua_rawgeti(L, idx, 3);
		GameObject* p = (GameObject*)lua_touserdata(L, -1);
		lua_pop(L, 1);
		if (!p)
			luaL_error(L, "invalid lstg object");
		return p;
	}

	void GameObjectPool::_GameObjectCallback(lua_State* L, int otidx, GameObject* p, int cbidx)
	{
		lua_rawgeti(L, otidx, (int)p->id + 1);	// ??? ot object
		lua_rawgeti(L, -1, 1);					// ??? ot object class
		lua_rawgeti(L, -1, cbidx);				// ??? ot object class frame
		lua_pushvalue(L, -3);					// ??? ot object class frame object
		lua_call(L, 1, 0);						// ??? ot object class
		lua_pop(L, 2);							// ??? ot
	}

	int GameObjectPool::Del(lua_State* L, bool kill_mode) noexcept
	{
		GameObject* p = _ToGameObject(L, 1);
		Del(p, kill_mode);
		return 0;
	}

	// --------------------------------------------------------------------------------

	void GameObjectPool::DebugNextFrame()
	{
		m_DbgIdx = (m_DbgIdx + 1) % std::size(m_DbgData);
		m_DbgData[m_DbgIdx].object_alloc = 0;
		m_DbgData[m_DbgIdx].object_free = 0;
		m_DbgData[m_DbgIdx].object_alive = m_ObjectPool.size();
		m_DbgData[m_DbgIdx].object_colli_check = 0;
		m_DbgData[m_DbgIdx].object_colli_callback = 0;
	}
	GameObjectPool::FrameStatistics GameObjectPool::DebugGetFrameStatistics()
	{
		size_t const n = std::size(m_DbgData);
		size_t const i = (m_DbgIdx + n - 1) % n;
		return m_DbgData[i];
	}

	int GameObjectPool::GetObjectTable(lua_State* L) noexcept
	{
		lua_pushlightuserdata(L, this);
		lua_gettable(L, LUA_REGISTRYINDEX);
		return 1;
	}
#ifdef USING_MULTI_GAME_WORLD
	int GameObjectPool::PushCurrentObject(lua_State* L)  noexcept
	{
		if (!m_pCurrentObject)
		{
			lua_pushnil(L);
			return 1;
		}
		GetObjectTable(L);
		lua_rawgeti(L, -1, (int)m_pCurrentObject->id + 1);  // ot t(object)
		return 1;
	}
#endif // USING_MULTI_GAME_WORLD
	GameObject* GameObjectPool::CastGameObject(lua_State* L, int idx)
	{
		return _ToGameObject(L, idx);
	}

	void GameObjectPool::ResetPool() noexcept
	{
		// 回收已分配的对象和更新链表
		GetObjectTable(G_L);
		int const ot_at = lua_gettop(G_L);
		for (GameObject* p = m_UpdateLinkList.first.pUpdateNext; p != &m_UpdateLinkList.second;)
		{
			p = _FreeObject(p, ot_at);
		}
#if (defined(_DEBUG) && defined(LuaSTG_enable_GameObjectManager_Debug))
		for (int i = 1; i <= LOBJPOOL_SIZE; i += 1)
		{
			// 确保所有 lua 侧对象都被正确回收
			lua_rawgeti(G_L, ot_at, i);
			assert(!lua_istable(G_L, -1));
			lua_pop(G_L, 1);
		}
#endif
		lua_pop(G_L, 1);
		// 重置其他链表
		_ClearLinkList();
		m_RenderList.clear();
		// 重置整个对象池，恢复为线性状态
		m_ObjectPool.clear();
		// 重置其他数据
#ifdef USING_MULTI_GAME_WORLD
		m_iWorld = 15;
		m_Worlds = { 15, 0, 0, 0 };
		m_pCurrentObject = nullptr;
#endif // USING_MULTI_GAME_WORLD
		m_LockObjectA = nullptr;
		m_LockObjectB = nullptr;
		m_superpause = 0;
		m_nextsuperpause = 0;
		// 清理内存
		local_memory_resource.release();
	}
	void GameObjectPool::updateMovementsLegacy(int32_t objects_index, lua_State* L) {
		tracy_zone_scoped_with_name("LOBJMGR.ObjFrame");

		int superpause = UpdateSuperPause(); // 更新超级暂停
		for (GameObject* p = m_UpdateLinkList.first.pUpdateNext; p != &m_UpdateLinkList.second; p = p->pUpdateNext) {
			if (superpause <= 0 || p->ignore_superpause) {
				if (p->features.has_callback_update) {
#ifdef USING_MULTI_GAME_WORLD
					m_pCurrentObject = p;
#endif // USING_MULTI_GAME_WORLD
					_GameObjectCallback(L, objects_index, p, LGOBJ_CC_FRAME);
#ifdef USING_MULTI_GAME_WORLD
					m_pCurrentObject = nullptr;
#endif // USING_MULTI_GAME_WORLD
				}
#ifdef USING_MULTI_GAME_WORLD
				m_pCurrentObject = p;
#endif // USING_MULTI_GAME_WORLD
				CLR_fn->CallOnFrame(p->id);
#ifdef USING_MULTI_GAME_WORLD
				m_pCurrentObject = nullptr;
#endif // USING_MULTI_GAME_WORLD
				p->Update();
			}
		}
	}
	void GameObjectPool::updateMovements(int32_t objects_index, lua_State* L) {
		tracy_zone_scoped_with_name("LOBJMGR.ObjFrame(New)");

		int superpause = GetSuperPauseTime();
		for (GameObject* p = m_UpdateLinkList.first.pUpdateNext; p != &m_UpdateLinkList.second; p = p->pUpdateNext) {
			if (superpause <= 0 || p->ignore_superpause) {
				if (!p->features.has_callback_update) {
					continue;
				}
#ifdef USING_MULTI_GAME_WORLD
				m_pCurrentObject = p;
#endif // USING_MULTI_GAME_WORLD
				_GameObjectCallback(L, objects_index, p, LGOBJ_CC_FRAME);
				CLR_fn->CallOnFrame(p->id);
#ifdef USING_MULTI_GAME_WORLD
				m_pCurrentObject = nullptr;
#endif // USING_MULTI_GAME_WORLD
			}
		}

		for (GameObject* p = m_UpdateLinkList.first.pUpdateNext; p != &m_UpdateLinkList.second; p = p->pUpdateNext) {
			if (superpause <= 0 || p->ignore_superpause) {
				p->UpdateV2();
			}
		}
	}
	void GameObjectPool::DoRender() noexcept
	{
		GetObjectTable(G_L); // ot
		int const ot_idx = lua_gettop(G_L);

		m_IsRendering = true;

#ifdef USING_MULTI_GAME_WORLD
		m_pCurrentObject = nullptr;
		lua_Integer world = GetWorldFlag();
#endif // USING_MULTI_GAME_WORLD

		for (auto& p : m_RenderList) {
#ifdef USING_MULTI_GAME_WORLD
			if (!p->hide && CheckWorld(p->world, world)) { // 只渲染可见对象
				m_pCurrentObject = p;
#else // USING_MULTI_GAME_WORLD
			if (!p->hide) {  // 只渲染可见对象
#endif // USING_MULTI_GAME_WORLD
				if (p->features.has_callback_render) {
					_GameObjectCallback(G_L, ot_idx, p, LGOBJ_CC_RENDER);
					CLR_fn->CallOnRender(p->id);
				}
				else {
					p->Render();
				}
			}
		}

#ifdef USING_MULTI_GAME_WORLD
		m_pCurrentObject = nullptr;
#endif // USING_MULTI_GAME_WORLD
		m_IsRendering = false;

		lua_pop(G_L, 1);
	}
	void GameObjectPool::UpdateXY() noexcept
	{
		tracy_zone_scoped_with_name("LOBJMGR.UpdateXY");

		int superpause = GetSuperPauseTime();
		for (GameObject* p = m_UpdateLinkList.first.pUpdateNext; p != &m_UpdateLinkList.second; p = p->pUpdateNext)
		{
			if (superpause <= 0 || p->ignore_superpause)
			{
				p->UpdateLast();
			}
		}
	}
	void GameObjectPool::updateNextLegacy(int32_t objects_index, lua_State*)
	{
		tracy_zone_scoped_with_name("LOBJMGR.AfterFrame");

		int superpause = GetSuperPauseTime();
		for (GameObject* p = m_UpdateLinkList.first.pUpdateNext; p != &m_UpdateLinkList.second;)
		{
			if (superpause <= 0 || p->ignore_superpause)
			{
				p->UpdateTimer();
				if (p->status != GameObjectStatus::Active)
				{
					p = _FreeObject(p, objects_index); // 再下一个
				}
				else
				{
					p = p->pUpdateNext;
				}
			}
			else
			{
				p = p->pUpdateNext;
			}
		}
	}
	void GameObjectPool::updateNext(int32_t objects_index, lua_State*) {
		tracy_zone_scoped_with_name("LOBJMGR.AfterFrame(New)");

		int superpause = UpdateSuperPause(); // 更新超级暂停
		for (GameObject* p = m_UpdateLinkList.first.pUpdateNext; p != &m_UpdateLinkList.second;)
		{
			if (superpause <= 0 || p->ignore_superpause)
			{
				p->UpdateLastV2();
				if (p->status != GameObjectStatus::Active)
				{
					p = _FreeObject(p, objects_index); // 再下一个
				}
				else
				{
					p = p->pUpdateNext;
				}
			}
			else
			{
				p = p->pUpdateNext;
			}
		}
	}
	void GameObjectPool::detectOutOfWorldBoundLegacy(int32_t objects_index, lua_State* L) {
		tracy_zone_scoped_with_name("LOBJMGR.BoundCheck");

#ifdef USING_MULTI_GAME_WORLD
		auto const world = GetWorldFlag();
#endif // USING_MULTI_GAME_WORLD

		for (GameObject* p = m_UpdateLinkList.first.pUpdateNext; p != &m_UpdateLinkList.second; p = p->pUpdateNext) {
#ifdef USING_MULTI_GAME_WORLD
			if (!CheckWorld(p->world, world)) {
				continue;
			}
#endif // USING_MULTI_GAME_WORLD
			if (_ObjectBoundCheck(p)) {
				continue;
			}
			p->status = GameObjectStatus::Dead; // 产生副作用
			// 调用 del 回调
			if (p->features.has_callback_destroy) {
#ifdef USING_MULTI_GAME_WORLD
				m_pCurrentObject = p;
#endif // USING_MULTI_GAME_WORLD
				_GameObjectCallback(L, objects_index, p, LGOBJ_CC_DEL);
				CLR_fn->CallOnDestroy(p->id, CLR_DESTROY_BOUNDS);
#ifdef USING_MULTI_GAME_WORLD
				m_pCurrentObject = nullptr;
#endif // USING_MULTI_GAME_WORLD
			}
		}
	}
	void GameObjectPool::detectOutOfWorldBound(int32_t objects_index, lua_State* L) {
		tracy_zone_scoped_with_name("LOBJMGR.BoundCheck(New)");

		struct OutOfWorldBoundDetectionResult {
			uint64_t uid{};
			GameObject* game_object{};
		};

		std::pmr::deque<OutOfWorldBoundDetectionResult> cache{ &local_memory_resource };

#ifdef USING_MULTI_GAME_WORLD
		auto const world = GetWorldFlag();
#endif // USING_MULTI_GAME_WORLD

		for (GameObject* p = m_UpdateLinkList.first.pUpdateNext; p != &m_UpdateLinkList.second; p = p->pUpdateNext) {
#ifdef USING_MULTI_GAME_WORLD
			if (!CheckWorld(p->world, world)) {
				continue;
			}
#endif // USING_MULTI_GAME_WORLD
			if (_ObjectBoundCheck(p)) {
				continue;
			}
			p->status = GameObjectStatus::Dead; // 产生副作用
			// 需要调用 del 回调
			if (p->features.has_callback_destroy) {
				cache.push_back(OutOfWorldBoundDetectionResult{
					.uid = p->uid,
					.game_object = p,
				});
			}
		}

		for (auto const& [uid, game_object] : cache) {
			if (game_object->uid != uid) {
				assert(false); continue; // 理论上不太可能发生
			}
#ifdef USING_MULTI_GAME_WORLD
			m_pCurrentObject = game_object;
#endif // USING_MULTI_GAME_WORLD
			//_GameObjectCallback(L, objects_index, object, LGOBJ_CC_DEL);
			lua_rawgeti(L, objects_index, game_object->id + 1);	// ... object
			lua_rawgeti(L, -1, 1);								// ... object class
			lua_rawgeti(L, -1, LGOBJ_CC_DEL);					// ... object class colli
			lua_pushvalue(L, -3);								// ... object class colli object
			lua_pushstring(L, "luastg:leave_world_border");		// ... object class colli object "luastg:leave_world_border"
			lua_call(L, 2, 0);									// ... object class
			lua_pop(L, 2);										// ...
#ifdef USING_MULTI_GAME_WORLD
			CLR_fn->CallOnDestroy(object->id, CLR_DESTROY_BOUNDS);
			m_pCurrentObject = nullptr;
#endif // USING_MULTI_GAME_WORLD
		}
	}
	void GameObjectPool::detectIntersectionLegacy(uint32_t group1_, uint32_t group2_, int32_t objects_index, lua_State* L) {
		tracy_zone_scoped_with_name("LOBJMGR.CollisionCheck");
		auto& debug_data = m_DbgData[m_DbgIdx];
		auto& group1 = m_ColliLinkList[group1_];
		auto& group2 = m_ColliLinkList[group2_];
		for (GameObject* ptrA = group1.first.pColliNext; ptrA != &group1.second;) {
			GameObject* pA = ptrA;
			ptrA = ptrA->pColliNext;
			for (GameObject* ptrB = group2.first.pColliNext; ptrB != &group2.second;) {
				GameObject* pB = ptrB;
				ptrB = ptrB->pColliNext;
				if (!pA->features.has_callback_trigger) {
					continue;
				}
#ifdef USING_MULTI_GAME_WORLD
				if (!CheckWorlds(pA->world, pB->world)) {
					continue;
				}
#endif // USING_MULTI_GAME_WORLD
				debug_data.object_colli_check += 1;
				if (!CollisionCheck(pA, pB)) {
					continue;
				}
				debug_data.object_colli_callback += 1;
#ifdef USING_MULTI_GAME_WORLD
				m_pCurrentObject = pA;
#endif // USING_MULTI_GAME_WORLD
				m_LockObjectA = ptrA;
				m_LockObjectB = ptrB;
				// 根据id获取对象的lua绑定table、拿到class再拿到collifunc
				lua_rawgeti(L, objects_index, pA->id + 1);	// ... object1
				lua_rawgeti(L, -1, 1);						// ... object1 class1
				lua_rawgeti(L, -1, LGOBJ_CC_COLLI);			// ... object1 class1 colli1
				lua_pushvalue(L, -3);						// ... object1 class1 colli1 object1
				lua_rawgeti(L, objects_index, pB->id + 1);	// ... object1 class1 colli1 object1 object2
				lua_call(L, 2, 0);							// ... object1 class1
				lua_pop(L, 2);								// ...
				CLR_fn->CallOnColli(pA->id, pB->id);
#ifdef USING_MULTI_GAME_WORLD
				m_pCurrentObject = nullptr;
#endif // USING_MULTI_GAME_WORLD
				m_LockObjectA = nullptr;
				m_LockObjectB = nullptr;
			}
		}
	}
	void GameObjectPool::detectIntersection(std::pmr::vector<IntersectionDetectionGroupPair> const& group_pairs, int32_t objects_index, lua_State* L) {
		tracy_zone_scoped_with_name("LOBJMGR.CollisionCheck(New)");
		std::pmr::deque<IntersectionDetectionResult> cache{ &local_memory_resource };
		for (auto const& group_pair : group_pairs) {
			auto& debug_data = m_DbgData[m_DbgIdx];
			auto& group1 = m_ColliLinkList[group_pair.group1];
			auto& group2 = m_ColliLinkList[group_pair.group2];
			for (GameObject* object1 = group1.first.pColliNext; object1 != &group1.second; object1 = object1->pColliNext) {
				for (GameObject* object2 = group2.first.pColliNext; object2 != &group2.second; object2 = object2->pColliNext) {
					if (!object1->features.has_callback_trigger) {
						continue;
					}
#ifdef USING_MULTI_GAME_WORLD
					if (!CheckWorlds(object1->world, object2->world)) {
						continue;
					}
#endif // USING_MULTI_GAME_WORLD
					debug_data.object_colli_check += 1;
					if (!CollisionCheck(object1, object2)) {
						continue;
					}
					cache.push_back(IntersectionDetectionResult{
						.id1 = object1->uid,
						.id2 = object2->uid,
						.index1 = static_cast<uint32_t>(object1->id),
						.index2 = static_cast<uint32_t>(object2->id),
						});
				}
			}
		}
		if (objects_index <= 0 || L == nullptr) {
			return;
		}
		auto& debug_data = m_DbgData[m_DbgIdx];
		for (auto const& result : cache) {
			auto* object1 = m_ObjectPool.object(result.index1);
			auto* object2 = m_ObjectPool.object(result.index2);
			if (object1->uid != result.id1 || object2->uid != result.id2) {
				assert(false); continue; // 理论上不太可能发生
			}
			debug_data.object_colli_callback += 1;
#ifdef USING_MULTI_GAME_WORLD
			m_pCurrentObject = object1;
#endif // USING_MULTI_GAME_WORLD
			m_LockObjectA = object1;
			m_LockObjectB = object2;
			lua_rawgeti(L, objects_index, object1->id + 1);	// ... object1
			lua_rawgeti(L, -1, 1);							// ... object1 class1
			lua_rawgeti(L, -1, LGOBJ_CC_COLLI);				// ... object1 class1 colli1
			lua_pushvalue(L, -3);							// ... object1 class1 colli1 object1
			lua_rawgeti(L, objects_index, object2->id + 1);	// ... object1 class1 colli1 object1 object2
			lua_call(L, 2, 0);								// ... object1 class1
			lua_pop(L, 2);									// ...
			CLR_fn->CallOnColli(object1->id, object2->id);
#ifdef USING_MULTI_GAME_WORLD
			m_pCurrentObject = nullptr;
#endif // USING_MULTI_GAME_WORLD
			m_LockObjectA = nullptr;
			m_LockObjectB = nullptr;
		}
	}

	int GameObjectPool::api_ObjFrame(lua_State* L) {
		lua::stack_t S(L);
		if (S.is_number(1)) {
			auto const version = S.get_value<int32_t>(1);
			if (version == 2) {
				g_GameObjectPool->GetObjectTable(L);
				auto const objects = S.index_of_top();
				g_GameObjectPool->updateMovements(objects.value, L);
				S.pop_value();
				return 0;
			}
		}
		// version 1
		g_GameObjectPool->GetObjectTable(L);
		auto const objects = S.index_of_top();
		g_GameObjectPool->updateMovementsLegacy(objects.value, L);
		S.pop_value();
		return 0;
	}
	int GameObjectPool::api_AfterFrame(lua_State* L) {
		lua::stack_t S(L);
		if (S.is_number(1)) {
			auto const version = S.get_value<int32_t>(1);
			if (version == 2) {
				g_GameObjectPool->GetObjectTable(L);
				auto const objects = S.index_of_top();
				g_GameObjectPool->updateNext(objects.value, L);
				S.pop_value();
				return 0;
			}
		}
		// version 1
		g_GameObjectPool->GetObjectTable(L);
		auto const objects = S.index_of_top();
		g_GameObjectPool->updateNextLegacy(objects.value, L);
		S.pop_value();
		return 0;
	}
	int GameObjectPool::api_BoundCheck(lua_State* L) {
		lua::stack_t S(L);
		if (S.is_number(1)) {
			auto const version = S.get_value<int32_t>(1);
			if (version == 2) {
				g_GameObjectPool->GetObjectTable(L);
				auto const objects = S.index_of_top();
				g_GameObjectPool->detectOutOfWorldBound(objects.value, L);
				S.pop_value();
				return 0;
			}
		}
		// version 1
		g_GameObjectPool->GetObjectTable(L);
		auto const objects = S.index_of_top();
		g_GameObjectPool->detectOutOfWorldBoundLegacy(objects.value, L);
		S.pop_value();
		return 0;
	}
	int GameObjectPool::api_CollisionCheck(lua_State* L) {
		lua::stack_t S(L);
		if (g_GameObjectPool->m_LockObjectA && g_GameObjectPool->m_LockObjectB) {
			return luaL_error(L, "invalid operation");
		}
		if (S.is_number(1) && S.is_number(2)) {
			auto const group1 = S.get_value<uint32_t>(1);
			auto const group2 = S.get_value<uint32_t>(2);
			if (group1 < 0 || group1 >= LOBJPOOL_GROUPN) {
				return luaL_error(L, "invalid collision group <%d>", group1);
			}
			if (group2 < 0 || group2 >= LOBJPOOL_GROUPN) {
				return luaL_error(L, "invalid collision group <%d>", group1);
			}
			g_GameObjectPool->GetObjectTable(L);
			auto const objects = S.index_of_top();
			g_GameObjectPool->detectIntersectionLegacy(group1, group2, objects.value, L);
			S.pop_value();
			return 0;
		}
		else if (S.is_table(1)) {
			// Stage 1
			auto const group_count = S.get_array_size(1);
			if (group_count == 0) {
				return 0; // early return
			}
			// Stage 2
			std::pmr::vector<IntersectionDetectionGroupPair> group_pairs{ &g_GameObjectPool->local_memory_resource };
			group_pairs.reserve(group_count);
			for (int32_t i = 1; i <= static_cast<int32_t>(group_count); i += 1) {
				auto const group_pair = S.get_array_value<lua::stack_index_t>(1, i);
				auto const group1 = S.get_array_value<uint32_t>(group_pair, 1);
				auto const group2 = S.get_array_value<uint32_t>(group_pair, 2);
				if (group1 < 0 || group1 >= LOBJPOOL_GROUPN) {
					return luaL_error(L, "invalid collision group <%d>", group1);
				}
				if (group2 < 0 || group2 >= LOBJPOOL_GROUPN) {
					return luaL_error(L, "invalid collision group <%d>", group1);
				}
				S.pop_value();
				group_pairs.emplace_back(group1, group2);
			}
			// Stage 3
			g_GameObjectPool->GetObjectTable(L);
			auto const objects = S.index_of_top();
			g_GameObjectPool->detectIntersection(group_pairs, objects.value, L);
			S.pop_value();
			return 0;
		}
		else {
			return luaL_error(L, "invalid parameters");
		}
	}

	int GameObjectPool::New(lua_State* L) noexcept
	{
		// 检查参数
		GameObjectFeatures features;
		features.read(L, 1);
		if (!features.is_class) {
			return luaL_error(L, "invalid argument #1, luastg object class required for 'New'.");
		}

		// 分配一个对象
		GameObject* p = _AllocObject();
		if (p == nullptr)
		{
			return luaL_error(L, "can't alloc object, object pool may be full.");
		}

		p->features = features;

		//											// class ...

		// 创建对象 table
		GetObjectTable(L);							// class ... ot
		lua_createtable(L, 3, 0);					// class ... ot object
		lua_pushvalue(L, 1);						// class ... ot object class
		lua_rawseti(L, -2, 1);						// class ... ot object
		lua_pushinteger(L, (lua_Integer)p->id);		// class ... ot object id
		lua_rawseti(L, -2, 2);						// class ... ot object
		lua_pushlightuserdata(L, p);				// class ... ot object pGameObject
		lua_rawseti(L, -2, 3);						// class ... ot object

		// 设置对象 metatable
		lua_rawgeti(L, -2, LOBJPOOL_METATABLE_IDX);	// class ... ot object mt
		lua_setmetatable(L, -2);					// class ... ot object

		// 设置到全局表 ot[n]
		lua_pushvalue(L, -1);						// class ... ot object object
		lua_rawseti(L, -3, (int)p->id + 1);			// class ... ot object

		if (p->features.has_callback_create) {
			// 调用 init
			lua_insert(L, 1);						// object class ... ot
			lua_pop(L, 1);							// object class ...
			lua_rawgeti(L, 2, LGOBJ_CC_INIT);		// object class ... init
			lua_insert(L, 3);						// object class init ...
			lua_pushvalue(L, 1);					// object class init ... object
			lua_insert(L, 4);						// object class init object ...
			lua_call(L, lua_gettop(L) - 3, 0);		// object class
			lua_pop(L, 1);							// object
		}

#if (defined(_DEBUG) && defined(LuaSTG_enable_GameObjectManager_Debug))
		static std::string _name("<null>");
		spdlog::debug("[object] new {}-{} (img = {})", p->id, p->uid, p->res ? p->res->GetResName() : _name);
#endif
		CLR_fn->CreateLuaGameObject((intptr_t)p);

		return 1;
	}
	void GameObjectPool::DirtResetObject(GameObject* p) noexcept
	{
		// 分配新的 UUID 并重新插入更新链表末尾
		_RemoveFromUpdateLinkList(p);
		_RemoveFromRenderList(p);
		_RemoveFromColliLinkList(p);
		p->uid = m_iUid;
		m_iUid += 1;
		_InsertToUpdateLinkList(p);
		_InsertToRenderList(p);
		_InsertToColliLinkList(p, (size_t)p->group);
	}
	void GameObjectPool::Del(GameObject* p, bool kill_mode) noexcept
	{
		if (p->status == GameObjectStatus::Active)
		{
			// 标记为即将回收的状态
			p->status = (!kill_mode) ? GameObjectStatus::Dead : GameObjectStatus::Killed;
			// 回调
			if ((!kill_mode && p->features.has_callback_destroy) || (kill_mode && p->features.has_callback_legacy_kill))
			{
				lua_rawgeti(G_L, 1, 1);												// object ... class
				lua_rawgeti(G_L, -1, (!kill_mode) ? LGOBJ_CC_DEL : LGOBJ_CC_KILL);	// object ... class callback
				lua_insert(G_L, 1);													// callback object ...
				lua_pop(G_L, 1);														// callback object ...
				lua_call(G_L, lua_gettop(G_L) - 1, 0);									// 
			}
		}
	}
	int GameObjectPool::IsValid(lua_State* L) noexcept
	{
		if (!lua_istable(L, 1))
		{
			lua_pushboolean(L, false);
			return 1;
		}
		lua_rawgeti(L, 1, 3);
		GameObject* p = (GameObject*)lua_touserdata(L, -1);
		lua_pop(L, 1);
		lua_pushboolean(L, p != nullptr);
		return 1;
	}

	bool GameObjectPool::SetImgState(GameObject* p, BlendMode m, core::Color4B c) noexcept
	{
		if (p->res)
		{
			switch (p->res->GetType())
			{
			case ResourceType::Sprite:
				static_cast<IResourceSprite*>(p->res)->SetBlendMode(m);
				static_cast<IResourceSprite*>(p->res)->GetSprite()->setColor(c);
				break;
			case ResourceType::Animation:
				do {
					auto* ani = static_cast<IResourceAnimation*>(p->res);
					ani->SetBlendMode(m);
					ani->SetVertexColor(c);
				} while (false);
				break;
			default:
				break;
			}
		}
		return true;
	}
	bool GameObjectPool::SetParState(GameObject* p, BlendMode m, core::Color4B c) noexcept
	{
		if (p->res)
		{
			switch (p->res->GetType())
			{
			case ResourceType::Particle:
				p->ps->SetBlendMode(m);
				p->ps->SetVertexColor(c);
				break;
			default:
				break;
			}
		}
		return true;
	}

	int GameObjectPool::FirstObject(int groupId) noexcept
	{
		if (groupId < 0 || groupId >= LOBJPOOL_GROUPN)
		{
			// 如果不是一个有效的分组，则在整个对象表中遍历
			GameObject* p = m_UpdateLinkList.first.pUpdateNext;
			if (p != &m_UpdateLinkList.second)
				return static_cast<int>(p->id);
			else
				return -1;
		}
		else
		{
			GameObject* p = m_ColliLinkList[groupId].first.pColliNext;
			if (p != &m_ColliLinkList[groupId].second)
				return static_cast<int>(p->id);
			else
				return -1;
		}
	}
	int GameObjectPool::NextObject(int groupId, int id) noexcept
	{
		if (id < 0)
			return -1;
		GameObject* p = m_ObjectPool.object(static_cast<size_t>(id));
		if (!p)
			return -1;
		if (groupId < 0 || groupId >= LOBJPOOL_GROUPN)
		{
			// 如果不是一个有效的分组，则在整个对象表中遍历
			if (p->pUpdateNext != &m_UpdateLinkList.second)
				return static_cast<int>(p->pUpdateNext->id);
			else
				return -1;
		}
		else
		{
			if (p->group != groupId)
				return -1;
			if (p->pColliNext != &m_ColliLinkList[groupId].second)
				return static_cast<int>(p->pColliNext->id);
			else
				return -1;
		}
	}

	void GameObjectPool::DrawCollider()
	{
#if (defined LDEVVERSION)
		struct ColliderDisplayConfig
		{
			int group;
			core::Color4B color;
			ColliderDisplayConfig() { group = 0; }
			ColliderDisplayConfig(int g, core::Color4B c) { group = g; color = c; }
		};
		static std::vector<ColliderDisplayConfig> m_collidercfg = {
			ColliderDisplayConfig(1, core::Color4B(163, 73, 164, 150)), // GROUP_ENEMY_
			ColliderDisplayConfig(2, core::Color4B(163, 73, 164, 150)), // GROUP_ENEMY
			ColliderDisplayConfig(5, core::Color4B(163, 73,  20, 150)), // GROUP_INDES
			ColliderDisplayConfig(4, core::Color4B(175, 15,  20, 150)), // GROUP_PLAYER
		};
		static bool f8 = false;
		static bool kf8 = false;

		if (!kf8 && LAPP.GetKeyState(/* VK_F8 */ 0x77)) { kf8 = true; f8 = !f8; }
		else if (kf8 && !LAPP.GetKeyState(/* VK_F8 */ 0x77)) { kf8 = false; }

		if (f8)
		{
			LAPP.DebugSetGeometryRenderState();
			for (ColliderDisplayConfig cfg : m_collidercfg)
			{
				DrawGroupCollider(cfg.group, cfg.color);
			}
		}
#endif
	}
	void GameObjectPool::DrawGroupCollider(int groupId, core::Color4B fillColor)
	{
#ifdef USING_MULTI_GAME_WORLD
		lua_Integer world = GetWorldFlag();
#endif // USING_MULTI_GAME_WORLD
		for (GameObject* p = m_ColliLinkList[groupId].first.pColliNext; p != &m_ColliLinkList[groupId].second; p = p->pColliNext)
		{
#ifdef USING_MULTI_GAME_WORLD
			if (p->colli && CheckWorld(p->world, world))
#else // !USING_MULTI_GAME_WORLD
			if (p->colli)
#endif // USING_MULTI_GAME_WORLD
			{
				if (p->rect)
				{
					LAPP.DebugDrawRect((float)p->x, (float)p->y, (float)p->a, (float)p->b, (float)p->rot, fillColor);
				}
				else if (!p->rect && p->a == p->b)
				{
					LAPP.DebugDrawCircle((float)p->x, (float)p->y, (float)p->a, fillColor);
				}
				else if (!p->rect && p->a != p->b)
				{
					LAPP.DebugDrawEllipse((float)p->x, (float)p->y, (float)p->a, (float)p->b, (float)p->rot, fillColor);
				}
				else {
					//备份，为以后做准备
					/*
					case _::Diamond:
					{
						core::Vector2F tHalfSize(cc.a, cc.b);
						// 计算出菱形的4个顶点
						f2dGraphics2DVertex tFinalPos[4] =
						{
							{  tHalfSize.x,         0.0f, 0.5f, fillColor.argb, 0.0f, 0.0f },
							{         0.0f, -tHalfSize.y, 0.5f, fillColor.argb, 0.0f, 1.0f },
							{ -tHalfSize.x,         0.0f, 0.5f, fillColor.argb, 1.0f, 1.0f },
							{         0.0f,  tHalfSize.y, 0.5f, fillColor.argb, 1.0f, 0.0f }
						};
						float tCos = std::cosf((float)p->rot);
						float tSin = std::sinf((float)p->rot);
						// 变换
						for (int i = 0; i < 4; i++)
						{
							float tx = tFinalPos[i].x * tCos - tFinalPos[i].y * tSin,
								ty = tFinalPos[i].x * tSin + tFinalPos[i].y * tCos;
							tFinalPos[i].x = tx + cc.absx;
							tFinalPos[i].y = ty + cc.absy;
						}
						graph->DrawQuad(nullptr, tFinalPos);
						break;
					}
					case _::Triangle:
					{
						core::Vector2F tHalfSize(cc.a, cc.b);
						// 计算出菱形的4个顶点
						f2dGraphics2DVertex tFinalPos[4] =
						{
							{  tHalfSize.x,         0.0f, 0.5f, fillColor.argb, 0.0f, 0.0f },
							{ -tHalfSize.x, -tHalfSize.y, 0.5f, fillColor.argb, 0.0f, 1.0f },
							{ -tHalfSize.x,  tHalfSize.y, 0.5f, fillColor.argb, 1.0f, 1.0f },
							{ -tHalfSize.x,  tHalfSize.y, 0.5f, fillColor.argb, 1.0f, 1.0f },//和第三个点相同
						};
						float tCos = std::cosf((float)p->rot);
						float tSin = std::sinf((float)p->rot);
						// 变换
						for (int i = 0; i < 4; i++)
						{
							float tx = tFinalPos[i].x * tCos - tFinalPos[i].y * tSin,
								ty = tFinalPos[i].x * tSin + tFinalPos[i].y * tCos;
							tFinalPos[i].x = tx + cc.absx;
							tFinalPos[i].y = ty + cc.absy;
						}
						graph->DrawQuad(nullptr, tFinalPos);
						break;
					}
					case _::Point:
					{
						//点使用直径1的圆来替代
						grender->____FillCircle(graph, core::Vector2F(cc.absx, cc.absy), 0.5f, fillColor, fillColor, 3);
						break;
					}
					//*/
				}
			}
		}
	}
	void GameObjectPool::DrawGroupCollider2(int groupId, core::Color4B fillColor)
	{
		LAPP.DebugSetGeometryRenderState();
		DrawGroupCollider(groupId, fillColor);
	}

	// --------------------------------------------------------------------------------

	int GameObjectPool::api_NextObject(lua_State* L) noexcept
	{
		lua_Integer g = luaL_checkinteger(L, 1);
		lua_Integer id = luaL_checkinteger(L, 2);
		if (id < 0)
			return 0;
		lua_pushinteger(L, g_GameObjectPool->NextObject(g, id));	// i(groupId) id(lastobj) id(next)
		g_GameObjectPool->GetObjectTable(L);						// i(groupId) id(lastobj) id(next) ot
		lua_rawgeti(L, -1, id + 1);									// i(groupId) id(lastobj) id(next) ot t(object)
		lua_remove(L, -2);											// i(groupId) id(lastobj) id(next) t(object)
		return 2;
	}
	int GameObjectPool::api_ObjList(lua_State* L) noexcept
	{
		lua_Integer g = luaL_checkinteger(L, 1);				// i(groupId)
		lua_pushcfunction(L, &api_NextObject);					// i(groupId) next(f)
		lua_insert(L, 1);										// next(f) i(groupId)
		lua_pushinteger(L, g_GameObjectPool->FirstObject(g));	// next(f) i(groupId) id(firstobj) 最后的两个参数作为迭代器参数传入
		return 3;
	}

	int GameObjectPool::api_New(lua_State* L) noexcept
	{
		return g_GameObjectPool->New(L);
	}
	int GameObjectPool::api_ResetObject(lua_State* L) noexcept
	{
		GameObject* p = g_GameObjectPool->_TableToGameObject(L, 1);
		g_GameObjectPool->DirtResetObject(p);
		return 0;
	}
	int GameObjectPool::api_Del(lua_State* L) noexcept
	{
		return g_GameObjectPool->Del(L);
	}
	int GameObjectPool::api_Kill(lua_State* L) noexcept
	{
		return g_GameObjectPool->Del(L, true);
	}
	int GameObjectPool::api_IsValid(lua_State* L) noexcept
	{
		return g_GameObjectPool->IsValid(L);
	}
	int GameObjectPool::api_BoxCheck(lua_State* L) noexcept
	{
		GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
		lua_Number const left = luaL_checknumber(L, 2);
		lua_Number const right = luaL_checknumber(L, 3);
		lua_Number const bottom = luaL_checknumber(L, 4);
		lua_Number const top = luaL_checknumber(L, 5);
		lua_pushboolean(L, p->IsInRect(left, right, bottom, top));
		return 1;
	}
	int GameObjectPool::api_ColliCheck(lua_State* L) noexcept
	{
		GameObject* p1 = g_GameObjectPool->_ToGameObject(L, 1);
		GameObject* p2 = g_GameObjectPool->_ToGameObject(L, 2);
#ifdef USING_MULTI_GAME_WORLD
		bool const ignore_world_mask = (lua_gettop(L) >= 3) ? lua_toboolean(L, 3) : false;
		if (ignore_world_mask)
		{
#endif // USING_MULTI_GAME_WORLD
			lua_pushboolean(L, CollisionCheck(p1, p2));
#ifdef USING_MULTI_GAME_WORLD
		}
		else
		{
			lua_pushboolean(L, g_GameObjectPool->CheckWorlds(p1->world, p2->world) && CollisionCheck(p1, p2));
		}
#endif // USING_MULTI_GAME_WORLD
		return 1;
	}
	int GameObjectPool::api_Angle(lua_State* L) noexcept
	{
		int const argc = lua_gettop(L);
		if (argc <= 2)
		{
			GameObject* p1 = g_GameObjectPool->_ToGameObject(L, 1);
			GameObject* p2 = g_GameObjectPool->_ToGameObject(L, 2);
			lua_pushnumber(L, std::atan2(p2->y - p1->y, p2->x - p1->x) * L_RAD_TO_DEG);
			return 1;
		}
		else if (argc == 3)
		{
			if (lua_istable(L, 1))
			{
				GameObject* p = g_GameObjectPool->_TableToGameObject(L, 1);
				lua_Number const x = luaL_checknumber(L, 2);
				lua_Number const y = luaL_checknumber(L, 3);
				lua_pushnumber(L, std::atan2(y - p->y, x - p->x) * L_RAD_TO_DEG);
				return 1;
			}
			else
			{
				lua_Number const x = luaL_checknumber(L, 1);
				lua_Number const y = luaL_checknumber(L, 2);
				GameObject* p = g_GameObjectPool->_ToGameObject(L, 3);
				lua_pushnumber(L, std::atan2(p->y - y, p->x - x) * L_RAD_TO_DEG);
				return 1;
			}
		}
		else
		{
			lua_Number const x1 = luaL_checknumber(L, 1);
			lua_Number const y1 = luaL_checknumber(L, 2);
			lua_Number const x2 = luaL_checknumber(L, 3);
			lua_Number const y2 = luaL_checknumber(L, 4);
			lua_pushnumber(L, std::atan2(y2 - y1, x2 - x1) * L_RAD_TO_DEG);
			return 1;
		}
	}
	int GameObjectPool::api_Dist(lua_State* L) noexcept
	{
		int const argc = lua_gettop(L);
		if (argc <= 2)
		{
			GameObject* p1 = g_GameObjectPool->_ToGameObject(L, 1);
			GameObject* p2 = g_GameObjectPool->_ToGameObject(L, 2);
			lua_Number const dx = p2->x - p1->x;
			lua_Number const dy = p2->y - p1->y;
			lua_pushnumber(L, std::sqrt(dx * dx + dy * dy));
			return 1;
		}
		else if (argc == 3)
		{
			if (lua_istable(L, 1))
			{
				GameObject* p = g_GameObjectPool->_TableToGameObject(L, 1);
				lua_Number const x = luaL_checknumber(L, 2);
				lua_Number const y = luaL_checknumber(L, 3);
				lua_Number const dx = x - p->x;
				lua_Number const dy = y - p->y;
				lua_pushnumber(L, std::sqrt(dx * dx + dy * dy));
				return 1;
			}
			else
			{
				lua_Number const x = luaL_checknumber(L, 1);
				lua_Number const y = luaL_checknumber(L, 2);
				GameObject* p = g_GameObjectPool->_ToGameObject(L, 3);
				lua_Number const dx = p->x - x;
				lua_Number const dy = p->y - y;
				lua_pushnumber(L, std::sqrt(dx * dx + dy * dy));
				return 1;
			}
		}
		else
		{
			lua_Number const x1 = luaL_checknumber(L, 1);
			lua_Number const y1 = luaL_checknumber(L, 2);
			lua_Number const x2 = luaL_checknumber(L, 3);
			lua_Number const y2 = luaL_checknumber(L, 4);
			lua_Number const dx = y2 - y1;
			lua_Number const dy = x2 - x1;
			lua_pushnumber(L, std::sqrt(dx * dx + dy * dy));
			return 1;
		}
	}
	int GameObjectPool::api_GetV(lua_State* L) noexcept
	{
		GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
		lua_pushnumber(L, std::sqrt(p->vx * p->vx + p->vy * p->vy));
		lua_pushnumber(L, std::atan2(p->vy, p->vx) * L_RAD_TO_DEG);
		return 2;
	}
	int GameObjectPool::api_SetV(lua_State* L) noexcept
	{
		GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
		lua_Number const v = luaL_checknumber(L, 2);
		lua_Number const a = luaL_checknumber(L, 3) * L_DEG_TO_RAD;
		bool const s = (lua_gettop(L) >= 4) ? lua_toboolean(L, 4) : false;
		p->vx = v * std::cos(a);
		p->vy = v * std::sin(a);
		if (s) p->rot = a;
		return 0;
	}

	int GameObjectPool::api_SetImgState(lua_State* L) noexcept
	{
		GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
		BlendMode m = TranslateBlendMode(L, 2);
		core::Color4B c = core::Color4B(
			(uint8_t)luaL_checkinteger(L, 4),
			(uint8_t)luaL_checkinteger(L, 5),
			(uint8_t)luaL_checkinteger(L, 6),
			(uint8_t)luaL_checkinteger(L, 3) // 这个才是 a 通道
		);
		g_GameObjectPool->SetImgState(p, m, c);
		return 0;
	}
	int GameObjectPool::api_SetParState(lua_State* L) noexcept
	{
		GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
		BlendMode m = TranslateBlendMode(L, 2);
		core::Color4B c = core::Color4B(
			(uint8_t)luaL_checkinteger(L, 4),
			(uint8_t)luaL_checkinteger(L, 5),
			(uint8_t)luaL_checkinteger(L, 6),
			(uint8_t)luaL_checkinteger(L, 3) // 这个才是 a 通道
		);
		g_GameObjectPool->SetParState(p, m, c);
		return 0;
	}
	int GameObjectPool::api_GetAttr(lua_State* L) noexcept
	{
		GameObject* p = g_GameObjectPool->_TableToGameObject(L, 1);
		p->GetAttr(L);
		return 1;
	}
	int GameObjectPool::api_SetAttr(lua_State* L) noexcept
	{
		GameObject* p = g_GameObjectPool->_TableToGameObject(L, 1);
		switch (p->SetAttr(L))
		{
		case 1: // group
			if (p == g_GameObjectPool->m_LockObjectA || p == g_GameObjectPool->m_LockObjectB)
				return luaL_error(L, "illegal operation, lstg object 'group' property should not be modified in 'lstg.CollisionCheck'");
			g_GameObjectPool->_MoveToColliLinkList(p, (size_t)p->group);
			break;
		case 2: // layer
			if (g_GameObjectPool->m_IsRendering)
				return luaL_error(L, "illegal operation, lstg object 'layer' property should not be modified in 'lstg.ObjRender'");
			g_GameObjectPool->_SetObjectLayer(p, p->nextlayer);
			break;
		}
		return 0;
	}

	int GameObjectPool::api_DefaultRenderFunc(lua_State* L) noexcept
	{
		GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
		p->Render();
		return 0;
	}

	int GameObjectPool::api_ParticleStop(lua_State* L) noexcept
	{
		GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
		if (!p->res || p->res->GetType() != ResourceType::Particle)
		{
#if !defined(NDEBUG)
			spdlog::warn("[luastg] ParticleStop: 试图停止一个不带有粒子发射器的对象的粒子发射过程 (uid={})", p->uid);
#endif
			return 0;
		}
		p->ps->SetActive(false);
		return 0;
	}
	int GameObjectPool::api_ParticleFire(lua_State* L) noexcept
	{
		GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
		if (!p->res || p->res->GetType() != ResourceType::Particle)
		{
#if !defined(NDEBUG)
			spdlog::warn("[luastg] ParticleFire: 试图启动一个不带有粒子发射器的对象的粒子发射过程 (uid={})", p->uid);
#endif
			return 0;
		}
		p->ps->SetActive(true);
		return 0;
	}
	int GameObjectPool::api_ParticleGetn(lua_State* L) noexcept
	{
		GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
		if (!p->res || p->res->GetType() != ResourceType::Particle)
		{
#if !defined(NDEBUG)
			spdlog::warn("[luastg] ParticleGetn: 试图获取一个不带有粒子发射器的对象的粒子数量 (uid={})", p->uid);
#endif
			lua_pushinteger(L, 0);
			return 1;
		}
		lua_pushinteger(L, (lua_Integer)p->ps->GetAliveCount());
		return 1;
	}
	int GameObjectPool::api_ParticleGetEmission(lua_State* L) noexcept
	{
		GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
		if (!p->res || p->res->GetType() != ResourceType::Particle)
		{
#if !defined(NDEBUG)
			spdlog::warn("[luastg] ParticleGetEmission: 试图获取一个不带有粒子发射器的对象的粒子发射密度 (uid={})", p->uid);
#endif
			lua_pushinteger(L, 0);
			return 1;
		}
		lua_pushinteger(L, p->ps->GetEmission());
		return 1;
	}
	int GameObjectPool::api_ParticleSetEmission(lua_State* L) noexcept
	{
		GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
		if (!p->res || p->res->GetType() != ResourceType::Particle)
		{
#if !defined(NDEBUG)
			spdlog::warn("[luastg] ParticleSetEmission: 试图设置一个不带有粒子发射器的对象的粒子发射密度 (uid={})", p->uid);
#endif
			return 0;
		}
		p->ps->SetEmission((int)std::max<lua_Integer>(0, luaL_checkinteger(L, 2)));
		return 0;
	}

	GameObject* GameObjectPool::CLR_New(uint32_t callbackMask) noexcept 
	{
		// 分配一个对象
		GameObject* p = _AllocObject();
		if (p == nullptr)
		{
			return nullptr;
		}

#if (defined(_DEBUG) && defined(LuaSTG_enable_GameObjectManager_Debug))
		static std::string _name("<null>");
		spdlog::debug("[object] new {}-{} (img = {})", p->id, p->uid, p->res ? p->res->GetResName() : _name);
#endif

#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		//TODO: create by arg
		//p->luaclass.CheckClassClass(G_L, 1);
#endif // USING_ADVANCE_GAMEOBJECT_CLASS

		lua_getglobal(G_L, "___clr_class");				// class ...

		// 创建对象 table
		GetObjectTable(G_L);							// class ... ot
		lua_createtable(G_L, 3, 0);					// class ... ot object
		lua_pushvalue(G_L, 1);						// class ... ot object class
		lua_rawseti(G_L, -2, 1);						// class ... ot object
		lua_pushinteger(G_L, (lua_Integer)p->id);		// class ... ot object id
		lua_rawseti(G_L, -2, 2);						// class ... ot object
		lua_pushlightuserdata(G_L, p);				// class ... ot object pGameObject
		lua_rawseti(G_L, -2, 3);						// class ... ot object

		// 设置对象 metatable
		lua_rawgeti(G_L, -2, LOBJPOOL_METATABLE_IDX);	// class ... ot object mt
		lua_setmetatable(G_L, -2);					// class ... ot object

		// 设置到全局表 ot[n]
		lua_pushvalue(G_L, -1);						// class ... ot object object
		lua_rawseti(G_L, -3, (int)p->id + 1);			// class ... ot object

		lua_settop(G_L, 0);

#if (defined(_DEBUG) && defined(LuaSTG_enable_GameObjectManager_Debug))
		static std::string _name("<null>");
		spdlog::debug("[object] new {}-{} (img = {})", p->id, p->uid, p->res ? p->res->GetResName() : _name);
#endif

		return p;
	}
	GameObjectPool* GameObjectPool::GetInstance()
	{
		return g_GameObjectPool;
	}
}
