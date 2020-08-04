#include "GameObjectPool.h"
#include "LuaStringToEnum.hpp"
#include "AppFrame.h"
#include "CollisionDetect.h"
#include "LuaWrapper\LuaWrapper.hpp"

#define METATABLE_OBJ "mt"

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

GameObjectPool::GameObjectPool(lua_State* pL)
{
	// Lua_State
	L = pL;
	
	// 初始化对象链表
	m_UpdateList.clear();
	m_RenderList.clear();
	for (auto& i : m_ColliList) {
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
}

void GameObjectPool::_PrepareLuaObjectTable() {
	// 创建一个全局表用于存放所有对象
	lua_pushlightuserdata(L, (void*)& LAPP);	// ??? p   (使用APP实例指针作键用以防止用户访问)
	lua_createtable(L, LGOBJ_MAXCNT, 0);		// ??? p t (创建足够大的table用于存放所有的游戏对象在lua中的对应对象)

	struct Metatable {
		static int GetAttr(lua_State* L) {
			return LPOOL.GetAttr(L);
		}
		static int SetAttr(lua_State* L) {
			return LPOOL.SetAttr(L);
		}
	};

	lua_newtable(L);							// ??? p t mt 
	lua_pushcfunction(L, &Metatable::GetAttr);	// ??? p t mt index 
	lua_pushcfunction(L, &Metatable::SetAttr);	// ??? p t mt index newindex 
	lua_setfield(L, -3, "__newindex");			// ??? p t mt index 
	lua_setfield(L, -2, "__index");				// ??? p t mt

	// 保存元表到 register[app][mt]
	lua_setfield(L, -2, METATABLE_OBJ);			// ??? p t
	lua_settable(L, LUA_REGISTRYINDEX);			// ???
}

GameObject* GameObjectPool::_AllocObject() {
	size_t id = 0;
	if (!m_ObjectPool.Alloc(id)) {
		return nullptr;
	}
	GameObject* p = m_ObjectPool.Data(id);
	p->Reset();
	p->status = STATUS_DEFAULT;
	p->id = id;
	p->uid = m_iUid;
	m_iUid++;
#ifdef USING_MULTI_GAME_WORLD
	if (m_pCurrentObject) {
		p->world = m_pCurrentObject->world;
	}
#endif // USING_MULTI_GAME_WORLD
	m_UpdateList.insert(p);
	m_RenderList.insert(p);
	m_ColliList[p->group].insert(p);
	return p;
}

GameObject* GameObjectPool::_ReleaseObject(GameObject* object) {
	GameObject* ret = nullptr;
	{
		auto it = m_UpdateList.find(object);
		ret = *std::next(it);
	}
	m_UpdateList.erase(object);
	m_RenderList.erase(object);
	m_ColliList[object->group].erase(object);
	if (m_pCurrentObject == object) {
		m_pCurrentObject = nullptr;
	}
	object->status = STATUS_FREE;
	m_ObjectPool.Free(object->id);
	return ret;
}

void GameObjectPool::_SetObjectLayer(GameObject* object, lua_Number layer) {
	if (object->layer != layer) {
		m_RenderList.erase(object);
		object->layer = layer;
		m_RenderList.insert(object);
	}
}

void GameObjectPool::_SetObjectColliGroup(GameObject* object, lua_Integer group) {
	if (object->group != group) {
		m_ColliList[object->group].erase(object);
		object->group = group;
		m_ColliList[group].insert(object);
	}
}

GameObject* GameObjectPool::freeObject(GameObject* p)LNOEXCEPT
{
	// 删除lua对象表中元素
	GETOBJTABLE;					// ot
	lua_pushnil(L);					// ot nil
	lua_rawseti(L, -2, p->id + 1);	// ot
	lua_pop(L, 1);

	// 释放引用的资源
	p->ReleaseResource();

	GameObject* pRet = _ReleaseObject(p);

	return pRet;
}

int GameObjectPool::PushCurrentObject(lua_State* L) LNOEXCEPT
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

void GameObjectPool::DoFrame()LNOEXCEPT
{
	//处理超级暂停
	GETOBJTABLE;  // ot
	int superpause = UpdateSuperPause();

	lua_Number cache1, cache2;//速度限制计算时用到的中间变量
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

			if (p->pause <= 0) {
				if (p->resolve_move) {
					p->vx = p->x - p->lastx;
					p->vy = p->y - p->lasty;
				}
				else {
					// 更新对象状态
					p->vx += p->ax;
					p->vy += p->ay;
#ifdef USER_SYSTEM_OPERATION
					p->vy -= p->ag;//单独的重力更新
					//速度限制，来自lua层
					cache1 = sqrt(p->vx * p->vx + p->vy * p->vy);
					if (p->maxv == 0.) {
						p->vx = p->vy = 0.;
					}
					else if (p->maxv < cache1) { //防止maxv为最大值时相乘出现溢出的情况
						cache2 = p->maxv / cache1;
						p->vx = cache2 * p->vx;
						p->vy = cache2 * p->vy;
					}
					//针对x、y方向单独限制
					if (abs(p->vx) > p->maxvx) {
						p->vx = p->maxvx * ((p->vx > 0) ? 1 : -1);
					}
					if (abs(p->vy) > p->maxvy) {
						p->vy = p->maxvy * ((p->vy > 0) ? 1 : -1);
					}
#endif
					//坐标更新
					p->x += p->vx;
					p->y += p->vy;
				}
				p->rot += p->omiga;
				
				// 更新粒子系统（若有）
				if (p->res && p->res->GetType() == ResourceType::Particle)
				{
					float gscale = LRES.GetGlobalImageScaleFactor();
					p->ps->SetRotation((float)p->rot);
					if (p->ps->IsActived())  // 兼容性处理
					{
						p->ps->SetInactive();
						p->ps->SetCenter(fcyVec2((float)p->x, (float)p->y));
						p->ps->SetActive();
					}
					else
						p->ps->SetCenter(fcyVec2((float)p->x, (float)p->y));
					p->ps->Update(1.0f / 60.f);
				}

			}
			else {
				p->pause--;
			}
		}
		it++;
	}
	m_pCurrentObject = nullptr;

	lua_pop(L, 1);
}

void GameObjectPool::DoRender()LNOEXCEPT
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
				DoDefaultRender(p);
			}
#endif // USING_ADVANCE_GAMEOBJECT_CLASS
		}
	}
	m_pCurrentObject = nullptr;

	lua_pop(L, 1);
}

void GameObjectPool::BoundCheck()LNOEXCEPT
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
				p->status = STATUS_DEL;

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

void GameObjectPool::CollisionCheck(size_t groupA, size_t groupB)LNOEXCEPT
{
	if (groupA < 0 || groupA >= LGOBJ_MAXCNT || groupB < 0 || groupB >= LGOBJ_MAXCNT)
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

void GameObjectPool::UpdateXY()LNOEXCEPT
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

void GameObjectPool::AfterFrame()LNOEXCEPT
{
	int superpause = GetSuperPauseTime();

	for (auto it = m_UpdateList.begin(); it != m_UpdateList.end();) {
		auto p = *it;
		it++;
		if (superpause <= 0 || p->ignore_superpause) {
			p->timer++;
			p->ani_timer++;
			if (p->status != STATUS_DEFAULT) {
				freeObject(p);
			}
		}
	}
}

int GameObjectPool::New(lua_State* L)LNOEXCEPT
{
	// 检查参数
	if (!GameObjectClass::CheckClassValid(L, 1)) {
		return luaL_error(L, "invalid argument #1, luastg object class required for 'New'.");
	}

	// 分配一个对象
	GameObject* p = _AllocObject();
	if (p == nullptr) {
		return luaL_error(L, "can't alloc object, object pool may be full.");
	}

	//  t(class) 
#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
	p->luaclass.CheckClassClass(L, 1);
#endif // USING_ADVANCE_GAMEOBJECT_CLASS
	GETOBJTABLE;								// t(class) ... ot
	lua_createtable(L, 2, 0);					// t(class) ... ot t(object)
	lua_pushvalue(L, 1);						// t(class) ... ot t(object) class
	lua_rawseti(L, -2, 1);						// t(class) ... ot t(object)  设置class
	lua_pushinteger(L, (lua_Integer)(p->id));	// t(class) ... ot t(object) id
	lua_rawseti(L, -2, 2);						// t(class) ... ot t(object)  设置id
	lua_getfield(L, -2, METATABLE_OBJ);			// t(class) ... ot t(object) mt
	lua_setmetatable(L, -2);					// t(class) ... ot t(object)  设置元表
	lua_pushvalue(L, -1);						// t(class) ... ot t(object) t(object)
	lua_rawseti(L, -3, p->id + 1);				// t(class) ... ot t(object)  设置到全局表
	lua_insert(L, 1);							// t(object) t(class) ... ot
	lua_pop(L, 1);								// t(object) t(class) ...
	lua_rawgeti(L, 2, LGOBJ_CC_INIT);			// t(object) t(class) ... f(init)
	lua_insert(L, 3);							// t(object) t(class) f(init) ...
	lua_pushvalue(L, 1);						// t(object) t(class) f(init) ... t(object)
	lua_insert(L, 4);							// t(object) t(class) f(init) t(object) ...
	lua_call(L, lua_gettop(L) - 3, 0);			// t(object) t(class)  执行构造函数
	lua_pop(L, 1);								// t(object)

	p->lastx = p->x;
	p->lasty = p->y;
	return 1;
}

int GameObjectPool::Del(lua_State* L)LNOEXCEPT
{
	if (!lua_istable(L, 1))
		return luaL_error(L, "invalid argument #1, luastg object required for 'Del'.");
	lua_rawgeti(L, 1, 2);  // t(object) ... id
	GameObject* p = m_ObjectPool.Data((size_t)luaL_checknumber(L, -1));
	lua_pop(L, 1);  // t(object) ...
	if (!p)
		return luaL_error(L, "invalid argument #1, invalid luastg object.");
	
	if (p->status == STATUS_DEFAULT)
	{
		p->status = STATUS_DEL;

		// 调用类中的回调方法
		lua_rawgeti(L, 1, 1);				// t(object) ... class
		lua_rawgeti(L, -1, LGOBJ_CC_DEL);	// t(object) ... class f(del)
		lua_insert(L, 1);					// f(del) t(object) ... class
		lua_pop(L, 1);						// f(del) t(object) ...
		lua_call(L, lua_gettop(L) - 1, 0);
	}
	return 0;
}

int GameObjectPool::Kill(lua_State* L)LNOEXCEPT
{
	if (!lua_istable(L, 1))
		return luaL_error(L, "invalid argument #1, luastg object required for 'Kill'.");
	lua_rawgeti(L, 1, 2);  // t(object) ... id
	GameObject* p = m_ObjectPool.Data((size_t)luaL_checknumber(L, -1));
	lua_pop(L, 1);  // t(object) ...
	if (!p)
		return luaL_error(L, "invalid argument #1, invalid luastg object.");

	if (p->status == STATUS_DEFAULT)
	{
		p->status = STATUS_KILL;

		// 调用类中的回调方法
		lua_rawgeti(L, 1, 1);				// t(object) ... class
		lua_rawgeti(L, -1, LGOBJ_CC_KILL);	// t(object) ... class f(kill)
		lua_insert(L, 1);					// f(kill) t(object) ... class
		lua_pop(L, 1);						// f(kill) t(object) ...
		lua_call(L, lua_gettop(L) - 1, 0);
	}
	return 0;
}

int GameObjectPool::IsValid(lua_State* L)LNOEXCEPT
{
	if (lua_gettop(L) != 1)
		return luaL_error(L, "invalid argument count, 1 argument required for 'IsValid'.");
	if (!lua_istable(L, -1))
	{
		lua_pushboolean(L, 0);
		return 1;
	}
	lua_rawgeti(L, -1, 2);  // t(object) id
	if (!lua_isnumber(L, -1))
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	// 在对象池中检查
	size_t id = (size_t)lua_tonumber(L, -1);
	lua_pop(L, 1);  // t(object)
	if (!m_ObjectPool.Data(id))
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	GETOBJTABLE;  // t(object) ot
	lua_rawgeti(L, -1, (lua_Integer)(id + 1));  // t(object) ot t(object)
	if (lua_rawequal(L, -1, -3))
		lua_pushboolean(L, 1);
	else
		lua_pushboolean(L, 0);
	return 1;
}

bool GameObjectPool::DirtResetObject(size_t id)LNOEXCEPT {
	GameObject* p = m_ObjectPool.Data(id);
	if (p) {
		_SetObjectLayer(p, 0.0);
		_SetObjectColliGroup(p, LGOBJ_DEFAULTGROUP);
		p->DirtReset();
		return true;
	}
	else {
		return false;
	}
}

bool GameObjectPool::Angle(size_t idA, size_t idB, double& out)LNOEXCEPT
{
	GameObject* pA = m_ObjectPool.Data(idA);
	GameObject* pB = m_ObjectPool.Data(idB);
	if (!pA || !pB)
		return false;
	out = LRAD2DEGREE * atan2(pB->y - pA->y, pB->x - pA->x);
	return true;
}

bool GameObjectPool::Dist(size_t idA, size_t idB, double& out)LNOEXCEPT
{
	GameObject* pA = m_ObjectPool.Data(idA);
	GameObject* pB = m_ObjectPool.Data(idB);
	if (!pA || !pB)
		return false;
	lua_Number dx = pB->x - pA->x;
	lua_Number dy = pB->y - pA->y;
	out = sqrt(dx*dx + dy*dy);
	return true;
}

bool GameObjectPool::ColliCheck(size_t idA, size_t idB, bool ignoreWorldMask, bool& out)LNOEXCEPT {
	GameObject* pA = m_ObjectPool.Data(idA);
	GameObject* pB = m_ObjectPool.Data(idB);
	if (!pA || !pB) {
		return false;//找不到对象，GG
	}
#ifdef USING_MULTI_GAME_WORLD
	if (ignoreWorldMask) {
#endif // USING_MULTI_GAME_WORLD
		out = LuaSTGPlus::CollisionCheck(pA, pB);
#ifdef USING_MULTI_GAME_WORLD
	}
	else{
		if (CheckWorlds(pA->world, pB->world)) {
			out = LuaSTGPlus::CollisionCheck(pA, pB);
		}
		else {
			out = false;//不在同一个world
		}
	}
#endif // USING_MULTI_GAME_WORLD
	return true;
}

bool GameObjectPool::GetV(size_t id, double& v, double& a)LNOEXCEPT
{
	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return false;
	v = sqrt(p->vx * p->vx + p->vy * p->vy);
	a = atan2(p->vy, p->vx) * LRAD2DEGREE;
	return true;
}

bool GameObjectPool::SetV(size_t id, double v, double a, bool updateRot)LNOEXCEPT
{
	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return false;
	a *= LDEGREE2RAD;
	p->vx = v*cos(a);
	p->vy = v*sin(a);
	if (updateRot)
		p->rot = a;
	return true;
}

bool GameObjectPool::SetImgState(size_t id, BlendMode m, fcyColor c)LNOEXCEPT
{
	GameObject* p = m_ObjectPool.Data(id);
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

bool GameObjectPool::SetParState(size_t id, BlendMode m, fcyColor c)LNOEXCEPT
{
	GameObject* p = m_ObjectPool.Data(id);
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

bool GameObjectPool::BoxCheck(size_t id, double left, double right, double top, double bottom, bool& ret)LNOEXCEPT
{
	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return false;
	ret = (p->x > left) && (p->x < right) && (p->y > top) && (p->y < bottom);
	return true;
}

void GameObjectPool::ResetPool()LNOEXCEPT
{
	for (auto it = m_UpdateList.begin(); it != m_UpdateList.end();) {
		auto p = *it;
		it++;
		freeObject(p);
	}
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

bool GameObjectPool::DoDefaultRender(GameObject* p)LNOEXCEPT
{
	if (!p) {
		return false;
	}
	if (p->res)
	{
#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		if (!p->luaclass.IsRenderClass) {
#endif // USING_ADVANCE_GAMEOBJECT_CLASS
			switch (p->res->GetType())
			{
			case ResourceType::Sprite:
				LAPP.Render(
					static_cast<ResSprite*>(p->res),
					static_cast<float>(p->x),
					static_cast<float>(p->y),
					static_cast<float>(p->rot),
					static_cast<float>(p->hscale * LRES.GetGlobalImageScaleFactor()),
					static_cast<float>(p->vscale * LRES.GetGlobalImageScaleFactor())
				);
				break;
			case ResourceType::Animation:
				LAPP.Render(
					static_cast<ResAnimation*>(p->res),
					p->ani_timer,
					static_cast<float>(p->x),
					static_cast<float>(p->y),
					static_cast<float>(p->rot),
					static_cast<float>(p->hscale * LRES.GetGlobalImageScaleFactor()),
					static_cast<float>(p->vscale * LRES.GetGlobalImageScaleFactor())
				);
				break;
			case ResourceType::Particle:
				LAPP.Render(
					p->ps,
					static_cast<float>(p->hscale * LRES.GetGlobalImageScaleFactor()),
					static_cast<float>(p->vscale * LRES.GetGlobalImageScaleFactor())
				);
				break;
			default:
				break;
			}
#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		}
		else {
			switch (p->res->GetType())
			{
			case ResourceType::Sprite:
			{
				fcyColor color = static_cast<ResSprite*>(p->res)->GetSprite()->GetColor((fuInt)0);
				BlendMode blend = static_cast<ResSprite*>(p->res)->GetBlendMode();
				static_cast<ResSprite*>(p->res)->GetSprite()->SetColor(fcyColor(p->vertexcolor.argb));
				static_cast<ResSprite*>(p->res)->SetBlendMode(p->blendmode);
				LAPP.Render(
					static_cast<ResSprite*>(p->res),
					static_cast<float>(p->x),
					static_cast<float>(p->y),
					static_cast<float>(p->rot),
					static_cast<float>(p->hscale * LRES.GetGlobalImageScaleFactor()),
					static_cast<float>(p->vscale * LRES.GetGlobalImageScaleFactor())
				);
				static_cast<ResSprite*>(p->res)->GetSprite()->SetColor(&color);
				static_cast<ResSprite*>(p->res)->SetBlendMode(blend);
				break;
			}
			case ResourceType::Animation:
			{
				fcyColor color = static_cast<ResAnimation*>(p->res)->GetSprite(0u)->GetColor(0u);
				BlendMode blend = static_cast<ResAnimation*>(p->res)->GetBlendMode();
				fcyColor newcolor(p->vertexcolor.argb);
				for (size_t i = 0; i < static_cast<ResAnimation*>(p->res)->GetCount(); ++i) {
					static_cast<ResAnimation*>(p->res)->GetSprite(i)->SetColor(newcolor);
				}
				LAPP.Render(
					static_cast<ResAnimation*>(p->res),
					p->ani_timer,
					static_cast<float>(p->x),
					static_cast<float>(p->y),
					static_cast<float>(p->rot),
					static_cast<float>(p->hscale * LRES.GetGlobalImageScaleFactor()),
					static_cast<float>(p->vscale * LRES.GetGlobalImageScaleFactor())
				);
				for (size_t i = 0; i < static_cast<ResAnimation*>(p->res)->GetCount(); ++i) {
					static_cast<ResAnimation*>(p->res)->GetSprite(i)->SetColor(color);
				}
				static_cast<ResAnimation*>(p->res)->SetBlendMode(blend);
				break;
			}
			case ResourceType::Particle:
			{
				p->ps->SetBlendMode(p->blendmode);
				p->ps->SetMixColor(fcyColor(p->vertexcolor.argb));
				LAPP.Render(
					p->ps,
					static_cast<float>(p->hscale * LRES.GetGlobalImageScaleFactor()),
					static_cast<float>(p->vscale * LRES.GetGlobalImageScaleFactor())
				);
				break;
			}
			default:
				break;
			}
		}
#endif // USING_ADVANCE_GAMEOBJECT_CLASS
	}
	return true;
}

int GameObjectPool::NextObject(int groupId, int id)LNOEXCEPT
{
	if (id < 0)
		return -1;

	GameObject* p = m_ObjectPool.Data(static_cast<size_t>(id));
	if (!p)
		return -1;

	// 如果不是一个有效的分组，则在整个对象表中遍历
	if (groupId < 0 || groupId >= LGOBJ_GROUPCNT)
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

int GameObjectPool::NextObject(lua_State* L)LNOEXCEPT
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

int GameObjectPool::FirstObject(int groupId)LNOEXCEPT
{
	// 如果不是一个有效的分组，则在整个对象表中遍历
	if (groupId < 0 || groupId >= LGOBJ_GROUPCNT)
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

int GameObjectPool::GetAttr(lua_State* L)LNOEXCEPT
{
	using namespace Xrysnow;
	
	lua_rawgeti(L, 1, 2);  // t(object) s(key) ??? i(id)
	size_t id = static_cast<size_t>(lua_tonumber(L, -1));
	lua_pop(L, 1);  // t(object) s(key)

	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return luaL_error(L, "invalid lstg object for '__index' meta operation.");
	
	// 查询属性
	const char* key = luaL_checkstring(L, 2);
	
	// 对x,y作特化处理
	if (key[1] == '\0') {
		switch (key[0])
		{
		case 'x':
			lua_pushnumber(L, p->x);
			return 1;
		case 'y':
			lua_pushnumber(L, p->y);
			return 1;
		}
	}

	// 一般属性
	switch (GameObjectPropertyHash(L, 2))
	//switch (GameObjectPropertyHash(key))
	{
	case GameObjectProperty::DX:
		lua_pushnumber(L, p->dx);
		break;
	case GameObjectProperty::DY:
		lua_pushnumber(L, p->dy);
		break;
	case GameObjectProperty::ROT:
		lua_pushnumber(L, p->rot * LRAD2DEGREE);
		break;
	case GameObjectProperty::OMEGA:
		lua_pushnumber(L, p->omiga * LRAD2DEGREE);
		break;
	case GameObjectProperty::TIMER:
		lua_pushinteger(L, p->timer);
		break;
	case GameObjectProperty::VX:
		lua_pushnumber(L, p->vx);
		break;
	case GameObjectProperty::VY:
		lua_pushnumber(L, p->vy);
		break;
	case GameObjectProperty::AX:
		lua_pushnumber(L, p->ax);
		break;
	case GameObjectProperty::AY:
		lua_pushnumber(L, p->ay);
		break;
#ifdef USER_SYSTEM_OPERATION
	case GameObjectProperty::MAXV:
		lua_pushnumber(L, p->maxv);
		break;
	case GameObjectProperty::MAXVX:
		lua_pushnumber(L, p->maxvx);
		break;
	case GameObjectProperty::MAXVY:
		lua_pushnumber(L, p->maxvy);
		break;
	case GameObjectProperty::AG:
		lua_pushnumber(L, p->ag);
		break;
#endif
	case GameObjectProperty::LAYER:
		lua_pushnumber(L, p->layer);
		break;
	case GameObjectProperty::GROUP:
		lua_pushinteger(L, p->group);
		break;
	case GameObjectProperty::HIDE:
		lua_pushboolean(L, p->hide);
		break;
	case GameObjectProperty::BOUND:
		lua_pushboolean(L, p->bound);
		break;
	case GameObjectProperty::NAVI:
		lua_pushboolean(L, p->navi);
		break;
	case GameObjectProperty::COLLI:
		lua_pushboolean(L, p->colli);
		break;
	case GameObjectProperty::STATUS:
		switch (p->status)
		{
		case STATUS_DEFAULT:
			lua_pushstring(L, "normal");
			break;
		case STATUS_KILL:
			lua_pushstring(L, "kill");
			break;
		case STATUS_DEL:
			lua_pushstring(L, "del");
			break;
		default:
			LASSERT(false);
			break;
		}
		break;
	case GameObjectProperty::HSCALE:
		lua_pushnumber(L, p->hscale);
		break;
	case GameObjectProperty::VSCALE:
		lua_pushnumber(L, p->vscale);
		break;
	case GameObjectProperty::CLASS:
		lua_rawgeti(L, 1, 1);
		break;
	case GameObjectProperty::A:
#ifdef GLOBAL_SCALE_COLLI_SHAPE
		lua_pushnumber(L, p->a / LRES.GetGlobalImageScaleFactor());
#else
		lua_pushnumber(L, p->a);
#endif // GLOBAL_SCALE_COLLI_SHAPE
		break;
	case GameObjectProperty::B:
#ifdef GLOBAL_SCALE_COLLI_SHAPE
		lua_pushnumber(L, p->b / LRES.GetGlobalImageScaleFactor());
#else
		lua_pushnumber(L, p->b);
#endif // GLOBAL_SCALE_COLLI_SHAPE
		break;
	case GameObjectProperty::RECT:
		lua_pushboolean(L, p->rect);
		break;
	case GameObjectProperty::IMG:
		if (p->res)
			lua_pushstring(L, p->res->GetResName().c_str());
		else
			lua_pushnil(L);
		break;
	case GameObjectProperty::ANI:
		lua_pushinteger(L, p->ani_timer);
		break;
	case GameObjectProperty::RESOLVEMOVE:
		lua_pushboolean(L, p->resolve_move);
		break;
	case GameObjectProperty::VSPEED:
		lua_pushnumber(L, sqrt(p->vx*p->vx + p->vy*p->vy));
		break;
	case GameObjectProperty::VANGLE:
		if (p->vx || p->vy){
			lua_pushnumber(L, atan2(p->vy, p->vx)*LRAD2DEGREE);
		}
		else{
			lua_pushnumber(L, p->rot*LRAD2DEGREE);
		}
		break;
	case GameObjectProperty::IGNORESUPERPAUSE:
		lua_pushboolean(L, p->ignore_superpause);
		break;
	case GameObjectProperty::PAUSE:
		lua_pushinteger(L, p->pause);
		break;
	case GameObjectProperty::WORLD:
		lua_pushinteger(L, p->world);
		break;
#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
	case GameObjectProperty::_BLEND:
		if (p->luaclass.IsRenderClass) {
			TranslateBlendModeToString(L, p->blendmode);
		}
		else {
			lua_pushnil(L);
		}
		break;
	case GameObjectProperty::_COLOR:
		if (p->luaclass.IsRenderClass) {
			LuaWrapper::ColorWrapper::CreateAndPush(L, fcyColor(p->vertexcolor.argb));
		}
		else {
			lua_pushnil(L);
		}
		break;
	case GameObjectProperty::_A:
		if (p->luaclass.IsRenderClass) {
			lua_pushinteger(L, p->vertexcolor.a);
		}
		else {
			lua_pushnil(L);
		}
		break;
	case GameObjectProperty::_R:
		if (p->luaclass.IsRenderClass) {
			lua_pushinteger(L, p->vertexcolor.r);
		}
		else {
			lua_pushnil(L);
		}
		break;
	case GameObjectProperty::_G:
		if (p->luaclass.IsRenderClass) {
			lua_pushinteger(L, p->vertexcolor.g);
		}
		else {
			lua_pushnil(L);
		}
		break;
	case GameObjectProperty::_B:
		if (p->luaclass.IsRenderClass) {
			lua_pushinteger(L, p->vertexcolor.b);
		}
		else {
			lua_pushnil(L);
		}
		break;
#endif // USING_ADVANCE_GAMEOBJECT_CLASS
	case GameObjectProperty::X:
	case GameObjectProperty::Y:
		break;
	default:
		lua_pushnil(L);
		break;
	}

	return 1;
}

int GameObjectPool::SetAttr(lua_State* L)LNOEXCEPT
{
	using namespace Xrysnow;
	
	lua_rawgeti(L, 1, 2);  // t(object) s(key) any(v) i(id)
	size_t id = static_cast<size_t>(lua_tonumber(L, -1));
	lua_pop(L, 1);  // t(object) s(key) any(v)

	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return luaL_error(L, "invalid lstg object for '__newindex' meta operation.");

	// 查询属性
	const char* key = luaL_checkstring(L, 2);
	std::string keypp = key;

	// 对x,y作特化处理
	if (key[1] == '\0') {
		switch (key[0])
		{
		case 'x':
			p->x = luaL_checknumber(L, 3);
			return 0;
		case 'y':
			p->y = luaL_checknumber(L, 3);
			return 0;
		}
	}

	// 一般属性
	switch (GameObjectPropertyHash(L, 2))
	{
	case GameObjectProperty::DX:
		return luaL_error(L, "property 'dx' is readonly.");
	case GameObjectProperty::DY:
		return luaL_error(L, "property 'dy' is readonly.");
	case GameObjectProperty::ROT:
		p->rot = luaL_checknumber(L, 3) * LDEGREE2RAD;
		break;
	case GameObjectProperty::OMEGA:
		p->omiga = luaL_checknumber(L, 3) * LDEGREE2RAD;
		break;
	case GameObjectProperty::TIMER:
		p->timer = luaL_checkinteger(L, 3);
		break;
	case GameObjectProperty::VX:
		p->vx = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::VY:
		p->vy = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::AX:
		p->ax = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::AY:
		p->ay = luaL_checknumber(L, 3);
		break;
#ifdef USER_SYSTEM_OPERATION
	case GameObjectProperty::MAXV:
		p->maxv = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::MAXVX:
		p->maxvx = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::MAXVY:
		p->maxvy = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::AG:
		p->ag = luaL_checknumber(L, 3);
		break;
#endif
	case GameObjectProperty::LAYER:
		_SetObjectLayer(p, luaL_checknumber(L, 3));
		break;
	case GameObjectProperty::GROUP:
	{
		int group = luaL_checkinteger(L, 3);
		if (0 <= group && group < LGOBJ_GROUPCNT)
		{
			_SetObjectColliGroup(p, group);
		}
		break;
	}
	case GameObjectProperty::HIDE:
		p->hide = lua_toboolean(L, 3) == 0 ? false : true;
		break;
	case GameObjectProperty::BOUND:
		p->bound = lua_toboolean(L, 3) == 0 ? false : true;
		break;
	case GameObjectProperty::NAVI:
		p->navi = lua_toboolean(L, 3) == 0 ? false : true;
		break;
	case GameObjectProperty::COLLI:
		p->colli = lua_toboolean(L, 3) == 0 ? false : true;
		break;

	case GameObjectProperty::RESOLVEMOVE:
		p->resolve_move = lua_toboolean(L, 3) == 0 ? false : true;
		break;
	case GameObjectProperty::IGNORESUPERPAUSE:
		p->ignore_superpause = lua_toboolean(L, 3) == 0 ? false : true;
		break;
	case GameObjectProperty::PAUSE:
		p->pause = luaL_checkinteger(L, 3);
		break;
	case GameObjectProperty::STATUS:
		do {
			const char* val = luaL_checkstring(L, 3);
			if (strcmp(val, "normal") == 0)
				p->status = STATUS_DEFAULT;
			else if (strcmp(val, "del") == 0)
				p->status = STATUS_DEL;
			else if (strcmp(val, "kill") == 0)
				p->status = STATUS_KILL;
			else
				return luaL_error(L, "invalid argument for property 'status'.");
		} while (false);
		break;
	case GameObjectProperty::HSCALE:
		p->hscale = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::VSCALE:
		p->vscale = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::CLASS:
	{
#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		if (!GameObjectClass::CheckClassValid(L, 3))
			return luaL_error(L, "invalid argument, require luastg object class.");
		p->luaclass.CheckClassClass(L, 3); // 刷新对象的class
#endif // USING_ADVANCE_GAMEOBJECT_CLASS
		lua_rawseti(L, 1, 1);
		break;
	}
	case GameObjectProperty::A:
#ifdef GLOBAL_SCALE_COLLI_SHAPE
		p->a = luaL_checknumber(L, 3) * LRES.GetGlobalImageScaleFactor();
#else
		p->a = luaL_checknumber(L, 3);
#endif // GLOBAL_SCALE_COLLI_SHAPE
		p->UpdateCollisionCirclrRadius();
		break;
	case GameObjectProperty::B:
#ifdef GLOBAL_SCALE_COLLI_SHAPE
		p->b = luaL_checknumber(L, 3) * LRES.GetGlobalImageScaleFactor();
#else
		p->b = luaL_checknumber(L, 3);
#endif // GLOBAL_SCALE_COLLI_SHAPE
		p->UpdateCollisionCirclrRadius();
		break;
	case GameObjectProperty::RECT:
		p->rect = lua_toboolean(L, 3) == 0 ? false : true;
		p->UpdateCollisionCirclrRadius();
		break;
	case GameObjectProperty::IMG:
	{
		if (lua_isstring(L, 3)) {
			const char* name = lua_tostring(L, 3);
			if (!p->res || strcmp(name, p->res->GetResName().c_str()) != 0)
			{
				p->ReleaseResource();
				if (!p->ChangeResource(name))
					return luaL_error(L, "can't find resource '%s' in image/animation/particle pool.", luaL_checkstring(L, 3));
			}
		}
		else {
			p->ReleaseResource();
		}
		break;
	}
	case GameObjectProperty::VSPEED:
	{
		float a1 = sqrt(p->vx*p->vx + p->vy*p->vy);
		float a2 = luaL_checknumber(L, 3);
		if (!a1){	
			p->vx = cos(p->rot)*a2;
			p->vy = sin(p->rot)*a2;
			break;
		}
		a2 = a2 / a1;
		p->vx *= a2;
		p->vy *= a2;
		break;
	}
	case GameObjectProperty::VANGLE:
	{
		float a1 = sqrt(p->vx * p->vx + p->vy * p->vy);
		float a2 = luaL_checknumber(L, 3) * LDEGREE2RAD;
		if (!a1){
			p->rot = a2;
			break;
		}
		p->vx = a1 * cos(a2);
		p->vy = a1 * sin(a2);
		break;
	}
	case GameObjectProperty::WORLD:
		p->world = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::ANI:
		return luaL_error(L, "property 'ani' is readonly.");
#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
	case GameObjectProperty::_BLEND:
		if (p->luaclass.IsRenderClass) {
			p->blendmode = TranslateBlendMode(L, 3);
		}
		else {
			lua_rawset(L, 1);
		}
		break;
	case GameObjectProperty::_COLOR:
		if (p->luaclass.IsRenderClass) {
			p->vertexcolor.argb = static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR))->argb;
		}
		else {
			lua_rawset(L, 1);
		}
		break;
	case GameObjectProperty::_A:
		if (p->luaclass.IsRenderClass) {
			p->vertexcolor.a = (uint8_t)luaL_checknumber(L, 3);
		}
		else {
			lua_rawset(L, 1);
		}
		break;
	case GameObjectProperty::_R:
		if (p->luaclass.IsRenderClass) {
			p->vertexcolor.r = (uint8_t)luaL_checknumber(L, 3);
		}
		else {
			lua_rawset(L, 1);
		}
		break;
	case GameObjectProperty::_G:
		if (p->luaclass.IsRenderClass) {
			p->vertexcolor.g = (uint8_t)luaL_checknumber(L, 3);
		}
		else {
			lua_rawset(L, 1);
		}
		break;
	case GameObjectProperty::_B:
		if (p->luaclass.IsRenderClass) {
			p->vertexcolor.b = (uint8_t)luaL_checknumber(L, 3);
		}
		else {
			lua_rawset(L, 1);
		}
		break;
#endif // USING_ADVANCE_GAMEOBJECT_CLASS
	case GameObjectProperty::X:
	case GameObjectProperty::Y:
		break;
	default:
		lua_rawset(L, 1);
		break;
	}

	return 0;
}

int GameObjectPool::GetObjectTable(lua_State* L)LNOEXCEPT
{
	GETOBJTABLE;
	return 1;
}

int GameObjectPool::ParticleStop(lua_State* L)LNOEXCEPT
{
	if (!lua_istable(L, 1))
		return luaL_error(L, "invalid lstg object for 'ParticleStop'.");
	lua_rawgeti(L, 1, 2);  // t(object) ??? id
	size_t id = (size_t)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return luaL_error(L, "invalid lstg object for 'ParticleStop'.");
	if (!p->res || p->res->GetType() != ResourceType::Particle)
	{
		LWARNING("ParticleStop: 试图停止一个不带有粒子发射器的对象的粒子发射过程(uid=%d)", m_iUid);
		return 0;
	}	
	p->ps->SetInactive();
	return 0;
}

int GameObjectPool::ParticleFire(lua_State* L)LNOEXCEPT
{
	if (!lua_istable(L, 1))
	return luaL_error(L, "invalid lstg object for 'ParticleFire'.");
	lua_rawgeti(L, 1, 2);  // t(object) ??? id
	size_t id = (size_t)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return luaL_error(L, "invalid lstg object for 'ParticleFire'.");
	if (!p->res || p->res->GetType() != ResourceType::Particle)
	{
		LWARNING("ParticleFire: 试图启动一个不带有粒子发射器的对象的粒子发射过程(uid=%d)", m_iUid);
		return 0;
	}	
	p->ps->SetActive();
	return 0;
}

int GameObjectPool::ParticleGetn(lua_State* L)LNOEXCEPT
{
	if (!lua_istable(L, 1))
	return luaL_error(L, "invalid lstg object for 'ParticleFire'.");
	lua_rawgeti(L, 1, 2);  // t(object) ??? id
	size_t id = (size_t)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return luaL_error(L, "invalid lstg object for 'ParticleFire'.");
	if (!p->res || p->res->GetType() != ResourceType::Particle)
	{
		lua_pushinteger(L, 0);
		return 1;
	}
	lua_pushinteger(L, (int)p->ps->GetAliveCount());
	return 1;
}

int GameObjectPool::ParticleGetEmission(lua_State* L)LNOEXCEPT
{
	if (!lua_istable(L, 1))
	return luaL_error(L, "invalid lstg object for 'ParticleGetEmission'.");
	lua_rawgeti(L, 1, 2);  // t(object) ??? id
	size_t id = (size_t)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return luaL_error(L, "invalid lstg object for 'ParticleGetEmission'.");
	if (!p->res || p->res->GetType() != ResourceType::Particle)
	{
		LWARNING("ParticleGetEmission: 试图获取一个不带有粒子发射器的对象的粒子发射密度(uid=%d)", m_iUid);
		lua_pushinteger(L, 0);
		return 1;
	}
	lua_pushnumber(L, p->ps->GetEmission());
	return 1;
}

int GameObjectPool::ParticleSetEmission(lua_State* L)LNOEXCEPT
{
	if (!lua_istable(L, 1))
	return luaL_error(L, "invalid lstg object for 'ParticleGetEmission'.");
	lua_rawgeti(L, 1, 2);  // t(object) ??? id
	size_t id = (size_t)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return luaL_error(L, "invalid lstg object for 'ParticleGetEmission'.");
	if (!p->res || p->res->GetType() != ResourceType::Particle)
	{
		LWARNING("ParticleSetEmission: 试图设置一个不带有粒子发射器的对象的粒子发射密度(uid=%d)", m_iUid);
		return 0;
	}
	p->ps->SetEmission((float)::max(0., luaL_checknumber(L, 2)));
	return 0;
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
				float tSin, tCos;
				SinCos((float)p->rot, tSin, tCos);
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
					float tSin, tCos;
					SinCos(p->rot, tSin, tCos);
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
				case GameObjectColliderType::Diamond:
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
					float tSin, tCos;
					SinCos(cc.absrot, tSin, tCos);
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
				case GameObjectColliderType::Triangle:
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
					float tSin, tCos;
					SinCos(cc.absrot, tSin, tCos);
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
				case GameObjectColliderType::Point:
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
