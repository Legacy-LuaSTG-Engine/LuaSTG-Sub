#include "GameObjectPool.h"
#include "AppFrame.h"
#include "LuaWrapper/LuaWrapper.hpp"
#include "LuaWrapper/lua_luastg_hash.hpp"
#include "LConfig.h"

#define NOMINMAX
#include <Windows.h>

//#define METATABLE_OBJ "mt"
#define LOBJPOOL_SIZE_INTERNAL (LOBJPOOL_SIZE + 1)
#define METATABLE_OBJ          LOBJPOOL_SIZE_INTERNAL

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#define GETOBJTABLE \
	do { \
		lua_pushlightuserdata(L, (void*)&LAPP); \
		lua_gettable(L, LUA_REGISTRYINDEX); \
	} while (false)

using namespace std;
using namespace LuaSTGPlus;

static GameObjectPool* g_GameObjectPool = nullptr;

GameObjectPool::GameObjectPool(lua_State* pL)
{
	assert(g_GameObjectPool == nullptr);
	g_GameObjectPool = this;
	// Lua_State
	L = pL;
	// 初始化对象链表
	m_UpdateList.clear();
	m_RenderList.clear();
	for (auto& i : m_ColliList)
	{
		i.clear();
	}
	// ex+
	m_pCurrentObject = nullptr;
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

void GameObjectPool::_PrepareLuaObjectTable()
{
	// 创建一个全局表用于存放所有对象
	lua_pushlightuserdata(L, (void*)& LAPP);		// ??? p   (使用APP实例指针作键用以防止用户访问)
	lua_createtable(L, LOBJPOOL_SIZE_INTERNAL, 0);	// ??? p t (创建足够大的table用于存放所有的游戏对象在lua中的对应对象)

	// 创建对象元表
	lua_createtable(L, 0, 2);						// ??? p t mt 
	lua_pushcfunction(L, &api_GetAttr);				// ??? p t mt index 
	lua_pushcfunction(L, &api_SetAttr);				// ??? p t mt index newindex 
	lua_setfield(L, -3, "__newindex");				// ??? p t mt index 
	lua_setfield(L, -2, "__index");					// ??? p t mt

	// 保存元表到 register[app][mt]
	lua_rawseti(L, -2, METATABLE_OBJ);				// ??? p t
	lua_settable(L, LUA_REGISTRYINDEX);				// ???
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
	m_UpdateList.insert(p);
	m_RenderList.insert(p);
	m_ColliList[p->group].insert(p);
	return p;
}

GameObject* GameObjectPool::_ReleaseObject(GameObject* object)
{
	GameObject* ret = nullptr;
	{
		auto it = std::next(m_UpdateList.find(object));
		if (it != m_UpdateList.end())
		{
			ret = *it;
		}
	}
	m_UpdateList.erase(object);
	m_RenderList.erase(object);
	m_ColliList[object->group].erase(object);
	if (m_pCurrentObject == object) {
		m_pCurrentObject = nullptr;
	}
	object->status = GameObjectStatus::Free;
	m_ObjectPool.free(object->id);
	return ret;
}

void GameObjectPool::_SetObjectLayer(GameObject* object, lua_Number layer)
{
	if (object->layer != layer)
	{
		m_RenderList.erase(object);
		object->layer = layer;
		m_RenderList.insert(object);
	}
}

void GameObjectPool::_SetObjectColliGroup(GameObject* object, lua_Integer group)
{
	if (object->group != group)
	{
		m_ColliList[object->group].erase(object);
		object->group = group;
		m_ColliList[group].insert(object);
	}
}

GameObject* GameObjectPool::_FreeObject(GameObject* p, int ot_at) noexcept
{
	int const index = (int)p->id + 1;
	int ot_stk = ot_at;

	// 删除lua对象表中元素
	if (ot_at <= 0)
	{
		GETOBJTABLE;					// ot
		ot_stk = lua_gettop(L);
	}
	lua_rawgeti(L, ot_stk, index);		// ot object
	lua_pushlightuserdata(L, nullptr);	// ot object nullptr
	lua_rawseti(L, -2, 3);				// ot object
	lua_pop(L, 1);						// ot
	lua_pushnil(L);						// ot nil
	lua_rawseti(L, ot_stk, index);		// ot
	if (ot_at <= 0)
	{
		lua_pop(L, 1);					// 
	}

	// 释放引用的资源
	p->ReleaseResource();

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
	lua_rawgeti(L, idx, 3);
	GameObject* p = (GameObject*)lua_touserdata(L, -1);
	lua_pop(L, 1);
	if (!p)
		luaL_error(L, "invalid lstg object");
	return p;
}

int GameObjectPool::PushCurrentObject(lua_State* L)  noexcept
{
	if (!m_pCurrentObject)
	{
		lua_pushnil(L);
		return 1;
	}
	GETOBJTABLE;
	lua_rawgeti(L, -1, m_pCurrentObject->id + 1);  // ot t(object)
	return 1;
}

void GameObjectPool::DoFrame() noexcept
{
	//处理超级暂停
	GETOBJTABLE;  // ot
	int superpause = UpdateSuperPause();

	
	for (auto it = m_UpdateList.begin(); it != m_UpdateList.end();) {
		auto p = *it;
		// 根据id获取对象的lua绑定table、拿到class再拿到framefunc
		if (superpause <= 0 || p->ignore_superpause) {
			m_pCurrentObject = p;
#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
			if (!p->luaclass.IsDefaultUpdate) {
#endif // USING_ADVANCE_GAMEOBJECT_CLASS
				lua_rawgeti(L, -1, p->id + 1);		// ot t(object)
				lua_rawgeti(L, -1, 1);				// ot t(object) t(class)
				lua_rawgeti(L, -1, LGOBJ_CC_FRAME);	// ot t(object) t(class) f(frame)
				lua_pushvalue(L, -3);				// ot t(object) t(class) f(frame) t(object)
				lua_call(L, 1, 0);					// ot t(object) t(class) 执行帧函数
				lua_pop(L, 2);						// ot
#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
			}
#endif // USING_ADVANCE_GAMEOBJECT_CLASS

			p->Update();
		}
		it++;
	}
	m_pCurrentObject = nullptr;

	lua_pop(L, 1);
}

void GameObjectPool::DoRender() noexcept
{
	GETOBJTABLE;  // ot

#ifdef USING_MULTI_GAME_WORLD
	lua_Integer world = GetWorldFlag();
#endif // USING_MULTI_GAME_WORLD
	for (auto& p : m_RenderList) {
#ifdef USING_MULTI_GAME_WORLD
		if (!p->hide && CheckWorld(p->world, world))  // 只渲染可见对象
#else // USING_MULTI_GAME_WORLD
		if (!p->hide)  // 只渲染可见对象
#endif // USING_MULTI_GAME_WORLD
		{
			m_pCurrentObject = p;
#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
			if (!p->luaclass.IsDefaultRender) {
#endif // USING_ADVANCE_GAMEOBJECT_CLASS
				// 根据id获取对象的lua绑定table、拿到class再拿到renderfunc
				lua_rawgeti(L, -1, p->id + 1);			// ot t(object)
				lua_rawgeti(L, -1, 1);					// ot t(object) t(class)
				lua_rawgeti(L, -1, LGOBJ_CC_RENDER);	// ot t(object) t(class) f(render)
				lua_pushvalue(L, -3);					// ot t(object) t(class) f(render) t(object)
				lua_call(L, 1, 0);						// ot t(object) t(class) 执行渲染函数
				lua_pop(L, 2);							// ot
#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
			}
			else {
				p->Render();;
			}
#endif // USING_ADVANCE_GAMEOBJECT_CLASS
		}
	}
	m_pCurrentObject = nullptr;

	lua_pop(L, 1);
}

void GameObjectPool::BoundCheck() noexcept
{
	GETOBJTABLE;  // ot

#ifdef USING_MULTI_GAME_WORLD
	lua_Integer world = GetWorldFlag();
#endif // USING_MULTI_GAME_WORLD
	for (auto it = m_UpdateList.begin(); it != m_UpdateList.end();) {
		auto p = *it;
		it++;
#ifdef USING_MULTI_GAME_WORLD
		if (CheckWorld(p->world, world)) {
#endif // USING_MULTI_GAME_WORLD
			if (!_ObjectBoundCheck(p))
			{
				m_pCurrentObject = p;
				// 越界设置为DEL状态
				p->status = GameObjectStatus::Dead;

				// 根据id获取对象的lua绑定table、拿到class再拿到delfunc
				lua_rawgeti(L, -1, p->id + 1);		// ot t(object)
				lua_rawgeti(L, -1, 1);				// ot t(object) t(class)
				lua_rawgeti(L, -1, LGOBJ_CC_DEL);	// ot t(object) t(class) f(del)
				lua_pushvalue(L, -3);				// ot t(object) t(class) f(del) t(object)
				lua_call(L, 1, 0);					// ot t(object) t(class)
				lua_pop(L, 2);						// ot
			}
#ifdef USING_MULTI_GAME_WORLD
		}
#endif // USING_MULTI_GAME_WORLD
	}
	m_pCurrentObject = nullptr;

	lua_pop(L, 1);
}

void GameObjectPool::CollisionCheck(size_t groupA, size_t groupB) noexcept
{
	if (groupA < 0 || groupA >= LOBJPOOL_SIZE || groupB < 0 || groupB >= LOBJPOOL_SIZE)
		luaL_error(L, "Invalid collision group.");

	GETOBJTABLE;  // ot

	for (auto itA = m_ColliList[groupA].begin(); itA != m_ColliList[groupA].end();) {
		auto pA = *itA;
		itA++;
		for (auto itB = m_ColliList[groupB].begin(); itB != m_ColliList[groupB].end();) {
			auto pB = *itB;
			itB++;
#ifdef USING_MULTI_GAME_WORLD
			if (CheckWorlds(pA->world, pB->world)) {
#endif // USING_MULTI_GAME_WORLD
				if (LuaSTGPlus::CollisionCheck(pA, pB))
				{
					m_pCurrentObject = pA;
					// 根据id获取对象的lua绑定table、拿到class再拿到collifunc
					lua_rawgeti(L, -1, pA->id + 1);		// ot t(object)
					lua_rawgeti(L, -1, 1);				// ot t(object) t(class)
					lua_rawgeti(L, -1, LGOBJ_CC_COLLI);	// ot t(object) t(class) f(colli)
					lua_pushvalue(L, -3);				// ot t(object) t(class) f(colli) t(object)
					lua_rawgeti(L, -5, pB->id + 1);		// ot t(object) t(class) f(colli) t(object) t(object)
					lua_call(L, 2, 0);					// ot t(object) t(class)
					lua_pop(L, 2);						// ot
				}
#ifdef USING_MULTI_GAME_WORLD
			}
#endif // USING_MULTI_GAME_WORLD
		}
	}
	m_pCurrentObject = nullptr;

	lua_pop(L, 1);
}

void GameObjectPool::UpdateXY() noexcept
{
	int superpause = GetSuperPauseTime();

	for (auto& p : m_UpdateList) {
		if (superpause <= 0 || p->ignore_superpause) {
			p->dx = p->x - p->lastx;
			p->dy = p->y - p->lasty;
			p->lastx = p->x;
			p->lasty = p->y;
			if (p->navi && (p->dx != 0 || p->dy != 0))
				p->rot = std::atan2(p->dy, p->dx);

		}
	}
}

void GameObjectPool::AfterFrame() noexcept
{
	int superpause = GetSuperPauseTime();
	GETOBJTABLE;
	int const ot_at = lua_gettop(L);
	for (auto it = m_UpdateList.begin(); it != m_UpdateList.end();) {
		auto p = *it;
		it++;
		if (superpause <= 0 || p->ignore_superpause) {
			p->timer++;
			p->ani_timer++;
			if (p->status != GameObjectStatus::Active) {
				_FreeObject(p, ot_at);
			}
		}
	}
	lua_pop(L, 1);
}

int GameObjectPool::New(lua_State* L) noexcept
{
	// 检查参数
	if (!GameObjectClass::CheckClassValid(L, 1))
	{
		return luaL_error(L, "invalid argument #1, luastg object class required for 'New'.");
	}

	// 分配一个对象
	GameObject* p = _AllocObject();
	if (p == nullptr)
	{
		return luaL_error(L, "can't alloc object, object pool may be full.");
	}

#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
	p->luaclass.CheckClassClass(L, 1);
#endif // USING_ADVANCE_GAMEOBJECT_CLASS

	//											// class ...

	// 创建对象 table
	GETOBJTABLE;								// class ... ot
	lua_createtable(L, 3, 0);					// class ... ot object
	lua_pushvalue(L, 1);						// class ... ot object class
	lua_rawseti(L, -2, 1);						// class ... ot object
	lua_pushinteger(L, (lua_Integer)p->id);		// class ... ot object id
	lua_rawseti(L, -2, 2);						// class ... ot object
	lua_pushlightuserdata(L, p);				// class ... ot object pGameObject
	lua_rawseti(L, -2, 3);						// class ... ot object

	// 设置对象 metatable
	lua_rawgeti(L, -2, METATABLE_OBJ);			// class ... ot object mt
	lua_setmetatable(L, -2);					// class ... ot object

	// 设置到全局表 ot[n]
	lua_pushvalue(L, -1);						// class ... ot object object
	lua_rawseti(L, -3, (int)p->id + 1);			// class ... ot object

	// 调用 init
	lua_insert(L, 1);							// object class ... ot
	lua_pop(L, 1);								// object class ...
	lua_rawgeti(L, 2, LGOBJ_CC_INIT);			// object class ... init
	lua_insert(L, 3);							// object class init ...
	lua_pushvalue(L, 1);						// object class init ... object
	lua_insert(L, 4);							// object class init object ...
	lua_call(L, lua_gettop(L) - 3, 0);			// object class
	lua_pop(L, 1);								// object
	
	// 更新初始状态
	p->lastx = p->x;
	p->lasty = p->y;

	return 1;
}

int GameObjectPool::Del(lua_State* L) noexcept
{
	GameObject* p = _ToGameObject(L, 1);
	if (p->status == GameObjectStatus::Active)
	{
		p->status = GameObjectStatus::Dead;

		// 调用类中的回调方法
		lua_rawgeti(L, 1, 1);				// t(object) ... class
		lua_rawgeti(L, -1, LGOBJ_CC_DEL);	// t(object) ... class f(del)
		lua_insert(L, 1);					// f(del) t(object) ... class
		lua_pop(L, 1);						// f(del) t(object) ...
		lua_call(L, lua_gettop(L) - 1, 0);
	}
	return 0;
}

int GameObjectPool::Kill(lua_State* L) noexcept
{
	GameObject* p = _ToGameObject(L, 1);
	if (p->status == GameObjectStatus::Active)
	{
		p->status = GameObjectStatus::Killed;

		// 调用类中的回调方法
		lua_rawgeti(L, 1, 1);				// t(object) ... class
		lua_rawgeti(L, -1, LGOBJ_CC_KILL);	// t(object) ... class f(kill)
		lua_insert(L, 1);					// f(kill) t(object) ... class
		lua_pop(L, 1);						// f(kill) t(object) ...
		lua_call(L, lua_gettop(L) - 1, 0);
	}
	return 0;
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

bool GameObjectPool::DirtResetObject(size_t id) noexcept {
	GameObject* p = m_ObjectPool.object(id);
	if (p)
	{
		_SetObjectLayer(p, 0.0);
		_SetObjectColliGroup(p, 0);
		p->DirtReset();
		p->uid = m_iUid;
		m_iUid += 1;
		return true;
	}
	else
	{
		return false;
	}
}

bool GameObjectPool::SetImgState(size_t id, BlendMode m, fcyColor c) noexcept
{
	GameObject* p = m_ObjectPool.object(id);
	if (!p)
		return false;
	if (p->res)
	{
		switch (p->res->GetType())
		{
		case ResourceType::Sprite:
			static_cast<ResSprite*>(p->res)->SetBlendMode(m);
			static_cast<ResSprite*>(p->res)->GetSprite()->SetColor(c);
			break;
		case ResourceType::Animation:
			do {
				ResAnimation* ani = static_cast<ResAnimation*>(p->res);
				ani->SetBlendMode(m);
				for (size_t i = 0; i < ani->GetCount(); ++i)
					ani->GetSprite(i)->SetColor(c);
			} while (false);
			break;
		default:
			break;
		}
	}
	return true;
}

bool GameObjectPool::SetParState(size_t id, BlendMode m, fcyColor c) noexcept
{
	GameObject* p = m_ObjectPool.object(id);
	if (!p)
		return false;
	if (p->res)
	{
		switch (p->res->GetType())
		{
		case ResourceType::Particle:
			p->ps->SetBlendMode(m);
			p->ps->SetMixColor(c);
			break;
		default:
			break;
		}
	}
	return true;
}

void GameObjectPool::ResetPool() noexcept
{
	GETOBJTABLE;
	int const ot_at = lua_gettop(L);
	for (auto it = m_UpdateList.begin(); it != m_UpdateList.end();) {
		auto p = *it;
		it++;
		_FreeObject(p, ot_at);
	}
	lua_pop(L, 1);

	m_UpdateList.clear();
	m_RenderList.clear();
	for (auto& i : m_ColliList) {
		i.clear();
	}
	// 重置一些东西，希望不会出现兼容性问题……
	m_iWorld = 15;
	m_Worlds = { 15, 0, 0, 0 };
	m_pCurrentObject = nullptr;
	m_superpause = 0;
	m_nextsuperpause = 0;
}

int GameObjectPool::NextObject(int groupId, int id) noexcept
{
	if (id < 0)
		return -1;

	GameObject* p = m_ObjectPool.object(static_cast<size_t>(id));
	if (!p)
		return -1;

	// 如果不是一个有效的分组，则在整个对象表中遍历
	if (groupId < 0 || groupId >= LOBJPOOL_GROUPN)
	{
		auto it = m_UpdateList.find(p);
		it++;
		if (it != m_UpdateList.end()) {
			return static_cast<int>((*it)->id);
		}
		else {
			return -1;
		}
	}
	else
	{
		if (p->group != groupId)
			return -1;

		auto it = m_ColliList[groupId].find(p);
		it++;
		if (it != m_ColliList[groupId].end()) {
			return static_cast<int>((*it)->id);
		}
		else {
			return -1;
		}
	}
}

int GameObjectPool::NextObject(lua_State* L) noexcept
{
	// i(groupId) id(lastobj)
	int g = luaL_checkinteger(L, 1);
	int id = luaL_checkinteger(L, 2);
	if (id < 0)
		return 0;

	lua_pushinteger(L, NextObject(g, id));	// ??? id(next)
	GETOBJTABLE;							// ??? id(next) ot
	lua_rawgeti(L, -1, id + 1);				// ??? id(next) ot t(object)
	lua_remove(L, -2);						// ??? id(next) t(object)
	return 2;
}

int GameObjectPool::FirstObject(int groupId) noexcept
{
	// 如果不是一个有效的分组，则在整个对象表中遍历
	if (groupId < 0 || groupId >= LOBJPOOL_GROUPN)
	{
		auto it = m_UpdateList.begin();
		if (it != m_UpdateList.end()) {
			return static_cast<int>((*it)->id);
		}
		else {
			return -1;
		}
	}
	else
	{
		auto it = m_ColliList[groupId].begin();
		if (it != m_ColliList[groupId].end()) {
			return static_cast<int>((*it)->id);
		}
		else {
			return -1;
		}
	}
}

int GameObjectPool::GetObjectTable(lua_State* L) noexcept
{
	GETOBJTABLE;
	return 1;
}

void GameObjectPool::DrawCollider()
{
#if (defined LDEVVERSION) || (defined LDEBUG)
	struct ColliderDisplayConfig {
		int group;
		fcyColor color;
		ColliderDisplayConfig() {
			group = 0;
			color.argb = 0x00000000;
		}
		ColliderDisplayConfig(int g, fcyColor c) {
			group = g;
			color.argb = c.argb;
		}
	};
	static std::vector<ColliderDisplayConfig> m_collidercfg = {
		ColliderDisplayConfig(1, fcyColor(150, 163, 73, 164)), // GROUP_ENEMY_
		ColliderDisplayConfig(2, fcyColor(150, 163, 73, 164)), // GROUP_ENEMY
		ColliderDisplayConfig(5, fcyColor(150, 163, 73,  20)), // GROUP_INDES
		ColliderDisplayConfig(4, fcyColor(100, 175, 15,  20)), // GROUP_PLAYER
	};
	static bool f8 = false;
	static bool kf8 = false;
	
	if (!kf8 && LAPP.GetKeyState(VK_F8)) { kf8 = true; f8 = !f8; }
	else if (kf8 && !LAPP.GetKeyState(VK_F8)) { kf8 = false; }
	
	if (f8)
	{
		LAPP.GetRenderDev()->ClearZBuffer();
		
		fcyRefPointer<f2dGeometryRenderer> grender = LAPP.GetGeometryRenderer();
		fcyRefPointer<f2dGraphics2D> graph = LAPP.GetGraphics2D();
		
		f2dBlendState stState = graph->GetBlendState();
		f2dBlendState stStateClone = stState;
		stStateClone.SrcBlend = F2DBLENDFACTOR_SRCALPHA;
		stStateClone.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
		stStateClone.BlendOp = F2DBLENDOPERATOR_ADD;
		graph->SetBlendState(stStateClone);
		F2DGRAPH2DBLENDTYPE txState = graph->GetColorBlendType();
		graph->SetColorBlendType(F2DGRAPH2DBLENDTYPE_ADD);//修复反色混合模式的时候会出现颜色异常的问题
		
		for (ColliderDisplayConfig cfg : m_collidercfg) {
			DrawGroupCollider(*graph, *grender, cfg.group, fcyColor(cfg.color.argb));
		}
		
		graph->SetBlendState(stState);
		graph->SetColorBlendType(txState);
	}
#endif
}
void GameObjectPool::DrawGroupCollider(f2dGraphics2D* graph, f2dGeometryRenderer* grender, int groupId, fcyColor fillColor)
{
#ifdef USING_MULTI_GAME_WORLD
	lua_Integer world = GetWorldFlag();
#endif // USING_MULTI_GAME_WORLD
	for (auto& p : m_ColliList[groupId]) {
#ifdef USING_MULTI_GAME_WORLD
		if (p->colli && CheckWorld(p->world, world))
#else // USING_MULTI_GAME_WORLD
		if (p->colli)
#endif // USING_MULTI_GAME_WORLD
		{
			if (p->rect)
			{
				// 计算出矩形的4个顶点
				fcyVec2 tHalfSize((float)p->a, (float)p->b);
				f2dGraphics2DVertex tFinalPos[4] =
				{
					{ -tHalfSize.x, -tHalfSize.y, 0.5f, fillColor.argb, 0.0f, 0.0f },
					{ tHalfSize.x, -tHalfSize.y, 0.5f, fillColor.argb, 0.0f, 1.0f },
					{ tHalfSize.x, tHalfSize.y, 0.5f, fillColor.argb, 1.0f, 1.0f },
					{ -tHalfSize.x, tHalfSize.y, 0.5f, fillColor.argb, 1.0f, 0.0f }
				};
				// 变换
				float tCos = std::cosf((float)p->rot);
				float tSin = std::sinf((float)p->rot);
				for (int i = 0; i < 4; i++)
				{
					fFloat tx = tFinalPos[i].x * tCos - tFinalPos[i].y * tSin,
						ty = tFinalPos[i].x * tSin + tFinalPos[i].y * tCos;
					tFinalPos[i].x = tx + (float)p->x; tFinalPos[i].y = ty + (float)p->y;
				}
				//绘制
				graph->DrawQuad(nullptr, tFinalPos);
			}
			else if (!p->rect && p->a == p->b)
			{
				//绘制
				grender->FillCircle(graph, fcyVec2((float)p->x, (float)p->y), (float)p->a, fillColor, fillColor,
					p->a < 10 ? 4 : (p->a < 20 ? 8 : 16));
			}
			else if (!p->rect && p->a != p->b)
			{
				const int vertcount = 37;//分割36份，还要中心一个点
				const int indexcount = 111;//37*3加一个组成封闭图形
				//准备顶点索引
				fuShort index[indexcount];
				{
					for (int i = 0; i < (vertcount - 1); i++) {
						index[i * 3] = 0;//中心点
						index[i * 3 + 1] = i;//1
						index[i * 3 + 2] = i + 1;//2
						//index[i * 3 + 3] = i + 1;//2 //fancy2d貌似不是以三角形为单位……
					}
					index[108] = 0;//中心点
					index[109] = 36;//1
					index[110] = 1;//2
				}
				//准备顶点
				f2dGraphics2DVertex vert[vertcount];
				{
					vert[0].x = 0.0f;
					vert[0].y = 0.0f;
					vert[0].z = 0.5f;//2D下固定z0.5
					vert[0].color = fillColor.argb;
					vert[0].u = 0.0f; vert[0].v = 0.0f;//没有使用到贴图，uv是多少无所谓
					float angle;
					for (int i = 1; i < vertcount; i++) {
						//椭圆参方
						angle = 10.0f * (i - 1) * LDEGREE2RAD;
						vert[i].x = p->a * std::cosf(angle);
						vert[i].y = p->b * std::sinf(angle);
						vert[i].z = 0.5f;//2D下固定z0.5
						vert[i].color = fillColor.argb;
						vert[i].u = 0.0f; vert[i].v = 0.0f;//没有使用到贴图，uv是多少无所谓
					}
				}
				// 变换
				{
					float tCos = std::cosf((float)p->rot);
					float tSin = std::sinf((float)p->rot);
					for (int i = 0; i < vertcount; i++)
					{
						fFloat tx = vert[i].x * tCos - vert[i].y * tSin,
							ty = vert[i].x * tSin + vert[i].y * tCos;
						vert[i].x = tx + p->x;
						vert[i].y = ty + p->y;
					}
				}
				//绘制
				graph->DrawRaw(nullptr, vertcount, indexcount, vert, index, false);
			}
			else {
				//备份，为以后做准备
				/*
				case _::Diamond:
				{
					fcyVec2 tHalfSize(cc.a, cc.b);
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
						fFloat tx = tFinalPos[i].x * tCos - tFinalPos[i].y * tSin,
							ty = tFinalPos[i].x * tSin + tFinalPos[i].y * tCos;
						tFinalPos[i].x = tx + cc.absx;
						tFinalPos[i].y = ty + cc.absy;
					}
					graph->DrawQuad(nullptr, tFinalPos);
					break;
				}
				case _::Triangle:
				{
					fcyVec2 tHalfSize(cc.a, cc.b);
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
						fFloat tx = tFinalPos[i].x * tCos - tFinalPos[i].y * tSin,
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
					grender->FillCircle(graph, fcyVec2(cc.absx, cc.absy), 0.5f, fillColor, fillColor, 3);
					break;
				}
				//*/
			}
		}
	}
}
void GameObjectPool::DrawGroupCollider2(int groupId, fcyColor fillColor)
{
	LAPP.GetRenderDev()->ClearZBuffer();

	fcyRefPointer<f2dGeometryRenderer> grender = LAPP.GetGeometryRenderer();
	fcyRefPointer<f2dGraphics2D> graph = LAPP.GetGraphics2D();
	
	f2dBlendState stState = graph->GetBlendState();
	f2dBlendState stStateClone = stState;
	stStateClone.SrcBlend = F2DBLENDFACTOR_SRCALPHA;
	stStateClone.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
	stStateClone.BlendOp = F2DBLENDOPERATOR_ADD;
	graph->SetBlendState(stStateClone);
	F2DGRAPH2DBLENDTYPE txState = graph->GetColorBlendType();
	graph->SetColorBlendType(F2DGRAPH2DBLENDTYPE_ADD);//修复反色混合模式的时候会出现颜色异常的问题

	DrawGroupCollider(*graph, *grender, groupId, fillColor);

	graph->SetBlendState(stState);
	graph->SetColorBlendType(txState);
}

// --------------------------------------------------------------------------------

int GameObjectPool::api_NextObject(lua_State* L) noexcept
{
	return g_GameObjectPool->NextObject(L);
}
int GameObjectPool::api_ObjList(lua_State* L) noexcept
{
	/*
	int g = luaL_checkinteger(L, 1);					// i(groupId)
	lua_pushcfunction(L, &api_NextObject);				// i(groupId) next(f)
	lua_pushinteger(L, g);								// i(groupId) next(f) i(groupId)
	lua_pushinteger(L, LPOOL.FirstObject(g));			// i(groupId) next(f) i(groupId) id(firstobj) 最后的两个参数作为迭代器参数传入
	//*/
	int g = luaL_checkinteger(L, 1);						// i(groupId)
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
	g_GameObjectPool->DirtResetObject(p->id);
	return 0;
}
int GameObjectPool::api_Del(lua_State* L) noexcept
{
	return g_GameObjectPool->Del(L);
}
int GameObjectPool::api_Kill(lua_State* L) noexcept
{
	return g_GameObjectPool->Kill(L);
}
int GameObjectPool::api_IsValid(lua_State* L) noexcept
{
	return g_GameObjectPool->IsValid(L);
}
int GameObjectPool::api_BoxCheck(lua_State* L) noexcept
{
	GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
	if (lua_gettop(L) <= 1)
	{
		lua_Number const left = luaL_checknumber(L, 2);
		lua_Number const right = luaL_checknumber(L, 3);
		lua_Number const bottom = luaL_checknumber(L, 4);
		lua_Number const top = luaL_checknumber(L, 5);
		lua_pushboolean(L, (p->x > left) && (p->x < right) && (p->y > bottom) && (p->y < top));
	}
	else
	{
		lua_pushboolean(L, g_GameObjectPool->_ObjectBoundCheck(p));
	}
	return 1;
}
int GameObjectPool::api_ColliCheck(lua_State* L) noexcept
{
	GameObject* p1 = g_GameObjectPool->_ToGameObject(L, 1);
	GameObject* p2 = g_GameObjectPool->_ToGameObject(L, 2);
	bool const ignoreWorldMask = (lua_gettop(L) >= 3) ? lua_toboolean(L, 3) : false;
#ifdef USING_MULTI_GAME_WORLD
	if (ignoreWorldMask)
	{
#endif // USING_MULTI_GAME_WORLD
		lua_pushboolean(L, LuaSTGPlus::CollisionCheck(p1, p2));
#ifdef USING_MULTI_GAME_WORLD
	}
	else
	{
		lua_pushboolean(L, g_GameObjectPool->CheckWorlds(p1->world, p2->world) && LuaSTGPlus::CollisionCheck(p1, p2));
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
		lua_pushnumber(L, std::atan2(p2->y - p1->y, p2->x - p1->x) * LRAD2DEGREE);
		return 1;
	}
	else if (argc == 3)
	{
		if (lua_istable(L, 1))
		{
			GameObject* p = g_GameObjectPool->_TableToGameObject(L, 1);
			lua_Number const x = luaL_checknumber(L, 2);
			lua_Number const y = luaL_checknumber(L, 3);
			lua_pushnumber(L, std::atan2(y - p->y, x - p->x) * LRAD2DEGREE);
			return 1;
		}
		else
		{
			lua_Number const x = luaL_checknumber(L, 1);
			lua_Number const y = luaL_checknumber(L, 2);
			GameObject* p = g_GameObjectPool->_ToGameObject(L, 3);
			lua_pushnumber(L, std::atan2(p->y - y, p->x - x) * LRAD2DEGREE);
			return 1;
		}
	}
	else
	{
		lua_Number const x1 = luaL_checknumber(L, 1);
		lua_Number const y1 = luaL_checknumber(L, 2);
		lua_Number const x2 = luaL_checknumber(L, 3);
		lua_Number const y2 = luaL_checknumber(L, 4);
		lua_pushnumber(L, std::atan2(y2 - y1, x2 - x1) * LRAD2DEGREE);
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
	lua_pushnumber(L, std::atan2(p->vy, p->vx) * LRAD2DEGREE);
	return 2;
}
int GameObjectPool::api_SetV(lua_State* L) noexcept
{
	GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
	lua_Number const v = luaL_checknumber(L, 2);
	lua_Number const a = luaL_checknumber(L, 3) * LDEGREE2RAD;
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
	fcyColor c(
		(fInt)luaL_checkinteger(L, 3),
		(fInt)luaL_checkinteger(L, 4),
		(fInt)luaL_checkinteger(L, 5),
		(fInt)luaL_checkinteger(L, 6)
	);
	g_GameObjectPool->SetImgState(p->id, m, c);
	return 0;
}
int GameObjectPool::api_SetParState(lua_State* L) noexcept
{
	GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
	BlendMode m = TranslateBlendMode(L, 2);
	fcyColor c(
		(fInt)luaL_checkinteger(L, 3),
		(fInt)luaL_checkinteger(L, 4),
		(fInt)luaL_checkinteger(L, 5),
		(fInt)luaL_checkinteger(L, 6)
	);
	g_GameObjectPool->SetParState(p->id, m, c);
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
		g_GameObjectPool->_SetObjectColliGroup(p, p->nextgroup);
		break;
	case 2: // layer
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
		spdlog::warn("[luastg] ParticleStop: 试图停止一个不带有粒子发射器的对象的粒子发射过程 (uid={})", p->uid);
		return 0;
	}
	p->ps->SetInactive();
	return 0;
}
int GameObjectPool::api_ParticleFire(lua_State* L) noexcept
{
	GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
	if (!p->res || p->res->GetType() != ResourceType::Particle)
	{
		spdlog::warn("[luastg] ParticleFire: 试图启动一个不带有粒子发射器的对象的粒子发射过程 (uid={})", p->uid);
		return 0;
	}
	p->ps->SetActive();
	return 0;
}
int GameObjectPool::api_ParticleGetn(lua_State* L) noexcept
{
	GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
	if (!p->res || p->res->GetType() != ResourceType::Particle)
	{
		spdlog::warn("[luastg] ParticleGetn: 试图获取一个不带有粒子发射器的对象的粒子数量 (uid={})", p->uid);
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
		spdlog::warn("[luastg] ParticleGetEmission: 试图获取一个不带有粒子发射器的对象的粒子发射密度 (uid={})", p->uid);
		lua_pushinteger(L, 0);
		return 1;
	}
	lua_pushnumber(L, p->ps->GetEmission());
	return 1;
}
int GameObjectPool::api_ParticleSetEmission(lua_State* L) noexcept
{
	GameObject* p = g_GameObjectPool->_ToGameObject(L, 1);
	if (!p->res || p->res->GetType() != ResourceType::Particle)
	{
		spdlog::warn("[luastg] ParticleSetEmission: 试图设置一个不带有粒子发射器的对象的粒子发射密度 (uid={})", p->uid);
		return 0;
	}
	p->ps->SetEmission((float)std::max(0.0, luaL_checknumber(L, 2)));
	return 0;
}
