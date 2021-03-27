#pragma once
#include "cpp/fixed_object_pool.hpp"
#include "GameObject.hpp"
#include <array>
#include <set>
#include "Config.h"

// 对象池信息
#define LOBJPOOL_SIZE   32768 // 最大对象数 //32768(full) //16384(half)
#define LOBJPOOL_GROUPN 16    // 碰撞组数

namespace LuaSTGPlus
{
	//游戏对象池
	class GameObjectPool
	{
	private:
		lua_State* L = nullptr;
		uint64_t m_iUid = 0;
		cpp::fixed_object_pool<GameObject, LOBJPOOL_SIZE> m_ObjectPool;
		GameObject* m_pCurrentObject = nullptr;
		
		// Comparer
		struct _less_object {
			bool operator()(const GameObject* x, const GameObject* y) const {
				return x->uid < y->uid;
			}
		};
		struct _less_render {
			bool operator()(const GameObject* x, const GameObject* y) const {
				if (x->layer != y->layer) {
					return x->layer < y->layer;
				}
				else {
					return x->uid < y->uid;
				}
			}
		};
		// GameObject List
		std::set<GameObject*, _less_object> m_UpdateList;
		std::set<GameObject*, _less_render> m_RenderList;
		std::array<std::set<GameObject*, _less_object>, LOBJPOOL_GROUPN> m_ColliList;
		
		// 场景边界
		lua_Number m_BoundLeft = -100.f;
		lua_Number m_BoundRight = 100.f;
		lua_Number m_BoundTop = 100.f;
		lua_Number m_BoundBottom = -100.f;
	private:
		//准备lua表用于存放对象
		void _PrepareLuaObjectTable();
		
		// 申请一个对象，重置对象并将对象插入到各个链表，不处理lua部分，返回申请的对象
		GameObject* _AllocObject();
		
		// 释放一个对象，将对象从各个链表中移除，并回收，不处理lua部分和对象资源，返回下一个可用的对象（可能为nullptr）
		GameObject* _ReleaseObject(GameObject* object);
		
		// 更改指定对象的图层，该操作会刷新对象在渲染链表中的位置
		void _SetObjectLayer(GameObject* object, lua_Number layer);
		
		// 更改指定对象的碰撞组，该操作会移动对象在碰撞组中的位置
		void _SetObjectColliGroup(GameObject* object, lua_Integer group);
		
		// 检查指定对象的坐标是否在场景边界内
		inline bool _ObjectBoundCheck(GameObject* object) {
			if (!object->bound)
				return true;
			if (object->x < m_BoundLeft ||
				object->x > m_BoundRight ||
				object->y < m_BoundBottom ||
				object->y > m_BoundTop)
				return false;
			return true;
		}
		
		// 释放一个对象，完全释放，返回下一个可用的对象（可能为nullptr）
		GameObject* freeObject(GameObject* p) noexcept;
	public:
		int PushCurrentObject(lua_State* L) noexcept;
		
		/// @brief 检查是否为主线程
		bool CheckIsMainThread(lua_State* pL) noexcept { return pL == L; }
		
		/// @brief 获取已分配对象数量
		size_t GetObjectCount() noexcept { return m_ObjectPool.size(); }
		
		/// @brief 获取对象
		GameObject* GetPooledObject(size_t i) noexcept { return m_ObjectPool.object(i); }
		
		/// @brief 执行对象的Frame函数
		void DoFrame() noexcept;
		
		/// @brief 执行对象的Render函数
		void DoRender() noexcept;
		
		/// @brief 获取舞台边界
		fcyRect GetBound() noexcept
		{
			return fcyRect((float)m_BoundLeft, (float)m_BoundTop, (float)m_BoundRight, (float)m_BoundBottom);
		}
		
		/// @brief 设置舞台边界
		void SetBound(lua_Number l, lua_Number r, lua_Number b, lua_Number t) noexcept
		{
			m_BoundLeft = l;
			m_BoundRight = r;
			m_BoundTop = t;
			m_BoundBottom = b;
		}
		
		/// @brief 执行边界检查
		void BoundCheck() noexcept;
		
		/// @brief 碰撞检查
		/// @param[in] groupA 对象组A
		/// @param[in] groupB 对象组B
		void CollisionCheck(size_t groupA, size_t groupB) noexcept;
		
		/// @brief 更新对象的XY坐标偏移量
		void UpdateXY() noexcept;
		
		/// @brief 帧末更新函数
		void AfterFrame() noexcept;
		
		/// @brief 创建新对象
		int New(lua_State* L) noexcept;
		
		/// @brief 通知对象删除
		int Del(lua_State* L) noexcept;
		
		/// @brief 通知对象消亡
		int Kill(lua_State* L) noexcept;
		
		/// @brief 检查对象是否有效
		int IsValid(lua_State* L) noexcept;
		
		//重置对象的各项属性，并释放资源，保留uid和id
		bool DirtResetObject(size_t id) noexcept;
		
		/// @brief 求夹角
		bool Angle(size_t idA, size_t idB, double& out) noexcept;
		
		/// @brief 求距离
		bool Dist(size_t idA, size_t idB, double& out) noexcept;
		
		/// @brief 求两个对象是否发生碰撞
		bool ColliCheck(size_t idA, size_t idB, bool ignoreWorldMask, bool& out) noexcept;
		
		/// @brief 计算速度方向和大小
		bool GetV(size_t id, double& v, double& a) noexcept;
		
		/// @brief 设置速度方向和大小
		bool SetV(size_t id, double v, double a, bool updateRot) noexcept;
		
		/// @brief 设置元素的图像状态
		bool SetImgState(size_t id, BlendMode m, fcyColor c) noexcept;
		
		/// @brief 特化设置HGE粒子的渲染状态
		bool SetParState(size_t id, BlendMode m, fcyColor c) noexcept;
		
		/// @brief 范围检查
		bool BoxCheck(size_t id, double left, double right, double top, double bottom, bool& ret) noexcept;
		
		/// @brief 清空对象池
		void ResetPool() noexcept;
		
		/// @brief 执行默认渲染
		bool DoDefaultRender(GameObject* p) noexcept;
		
		/// @brief 执行默认渲染
		bool DoDefaultRender(size_t id) noexcept {
			return DoDefaultRender(m_ObjectPool.object(id));
		}
		
		/// @brief 获取下一个元素的ID
		/// @return 返回-1表示无元素
		int NextObject(int groupId, int id) noexcept;
		
		//返回一个碰撞组迭代器
		int NextObject(lua_State* L) noexcept;
		
		/// @brief 获取列表中的第一个元素ID
		/// @note 为迭代器使用
		/// @return 返回-1表示无元素
		int FirstObject(int groupId) noexcept;
		
		/// @brief 属性读方法
		int GetAttr(lua_State* L) noexcept;
		
		/// @brief 属性写方法
		int SetAttr(lua_State* L) noexcept;
		
		/// @brief 调试目的，获取对象列表
		int GetObjectTable(lua_State* L) noexcept;
		
		/// @brief 对象粒子池相关操作
		int ParticleStop(lua_State* L) noexcept;
		int ParticleFire(lua_State* L) noexcept;
		int ParticleGetn(lua_State* L) noexcept;
		int ParticleGetEmission(lua_State* L) noexcept;
		int ParticleSetEmission(lua_State* L) noexcept;
		
		/// @brief 对象资源相关操作
	private:
		///用于多world
		
		lua_Integer m_iWorld = 15;//当前的world mask
		std::array<lua_Integer, 4> m_Worlds = { 15, 0, 0, 0 };//预置的world mask
	public:
		///用于多world
		
		//设置当前的world mask
		inline void SetWorldFlag(lua_Integer world) noexcept {
			m_iWorld = world;
		}
		//获取当前的world mask
		inline lua_Integer GetWorldFlag() noexcept {
			return m_iWorld;
		}
		//设置预置的world mask
		inline void ActiveWorlds(lua_Integer a, lua_Integer b, lua_Integer c, lua_Integer d) noexcept {
			m_Worlds[0] = a;
			m_Worlds[1] = b;
			m_Worlds[2] = c;
			m_Worlds[3] = d;
		}
		//检查两个world mask位与或的结果 //静态函数，不应该只用于类内
		static inline bool CheckWorld(lua_Integer gameworld, lua_Integer objworld) {
			return (gameworld == objworld) || (gameworld & objworld);
		}
		//对两个world mask，分别与预置的world mask位与或，用于检查是否在同一个world内
		bool CheckWorlds(int a, int b) noexcept {
			if (CheckWorld(a, m_Worlds[0]) && CheckWorld(b, m_Worlds[0]))return true;
			if (CheckWorld(a, m_Worlds[1]) && CheckWorld(b, m_Worlds[1]))return true;
			if (CheckWorld(a, m_Worlds[2]) && CheckWorld(b, m_Worlds[2]))return true;
			if (CheckWorld(a, m_Worlds[3]) && CheckWorld(b, m_Worlds[3]))return true;
			return false;
		}
	private:
		///用于超级暂停
		
		lua_Integer m_superpause = 0;
		lua_Integer m_nextsuperpause = 0;
	public:
		///用于超级暂停
		
		//获取可信的超级暂停时间
		inline lua_Integer GetSuperPauseTime() noexcept {
			return m_superpause;
		}
		//获取超级暂停剩余时间
		inline lua_Integer GetNextFrameSuperPauseTime() noexcept {
			return m_nextsuperpause;
		}
		//设置超级暂停剩余时间
		inline void SetNextFrameSuperPauseTime(lua_Integer time) noexcept {
			m_nextsuperpause = time;
		}
		//更新超级暂停的剩余时间并返回当前的可信值
		inline lua_Integer UpdateSuperPause() {
			m_superpause = m_nextsuperpause;
			if (m_nextsuperpause > 0)
				m_nextsuperpause = m_nextsuperpause - 1;
			return m_superpause;
		}
	public:  // 内部使用
		void DrawCollider();
		void DrawGroupCollider(f2dGraphics2D* graph, f2dGeometryRenderer* grender, int groupId, fcyColor fillColor);
		void DrawGroupCollider2(int groupId, fcyColor fillColor);
	private:
		GameObjectPool& operator=(const GameObjectPool&);
		GameObjectPool(const GameObjectPool&);
	public:
		GameObjectPool(lua_State* pL);
		~GameObjectPool();
	};
}
