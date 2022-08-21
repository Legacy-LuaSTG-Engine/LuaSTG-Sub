#pragma once
#include "GameObject/GameObject.hpp"
#include "Utility/fixed_object_pool.hpp"

// 对象池信息
#define LOBJPOOL_SIZE   32768 // 最大对象数 //32768(full) //16384(half)
#define LOBJPOOL_GROUPN 16    // 碰撞组数

namespace LuaSTGPlus
{
	//游戏对象池
	class GameObjectPool
	{
	public:
		struct FrameStatistics
		{
			uint64_t object_alloc{ 0 };
			uint64_t object_free{ 0 };
			uint64_t object_alive{ 0 };
			uint64_t object_colli_check{ 0 };
			uint64_t object_colli_callback{ 0 };
		};

	private:
		cpp::fixed_object_pool<GameObject, LOBJPOOL_SIZE> m_ObjectPool;
		uint64_t m_iUid = 0;
		lua_State* G_L = nullptr;
		GameObject* m_pCurrentObject = nullptr;
		
		// GameObject List
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
		std::set<GameObject*, _less_render> m_RenderList;
		std::pair<GameObject, GameObject> m_UpdateLinkList;
		std::array<std::pair<GameObject, GameObject>, LOBJPOOL_GROUPN> m_ColliLinkList = {};

		// 场景边界
		lua_Number m_BoundLeft = -100.f;
		lua_Number m_BoundRight = 100.f;
		lua_Number m_BoundTop = 100.f;
		lua_Number m_BoundBottom = -100.f;

		bool m_IsRendering = false;

		FrameStatistics m_DbgData[2]{};
		size_t m_DbgIdx{ 0 };

	private:
		GameObject* m_LockObjectA{};
		GameObject* m_LockObjectB{};

		void _ClearLinkList();
		void _InsertToUpdateLinkList(GameObject* p);
		void _RemoveFromUpdateLinkList(GameObject* p);
		void _InsertToColliLinkList(GameObject* p, size_t group);
		void _RemoveFromColliLinkList(GameObject* p);
		void _MoveToColliLinkList(GameObject* p, size_t group);

		void _InsertToRenderList(GameObject* p);
		void _RemoveFromRenderList(GameObject* p);
		void _SetObjectLayer(GameObject* object, lua_Number layer);

		//准备lua表用于存放对象
		void _PrepareLuaObjectTable();
		
		// 申请一个对象，重置对象并将对象插入到各个链表，不处理lua部分，返回申请的对象
		GameObject* _AllocObject();
		
		// 释放一个对象，将对象从各个链表中移除，并回收，不处理lua部分和对象资源，返回下一个可用的对象（可能为nullptr）
		GameObject* _ReleaseObject(GameObject* object);
		
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
		GameObject* _FreeObject(GameObject* p, int ot_at = 0) noexcept;

		GameObject* _ToGameObject(lua_State* L, int idx);
		GameObject* _TableToGameObject(lua_State* L, int idx);

		void _GameObjectCallback(lua_State* L, int otidx, GameObject* p, int cbidx);

	public:
		void DebugNextFrame();
		FrameStatistics DebugGetFrameStatistics();

	public:
		int PushCurrentObject(lua_State* L) noexcept;
		
		/// @brief 检查是否为主线程
		bool CheckIsMainThread(lua_State* pL) noexcept { return pL == G_L; }
		
		/// @brief 获取已分配对象数量
		size_t GetObjectCount() noexcept { return m_ObjectPool.size(); }
		
		/// @brief 获取对象
		GameObject* GetPooledObject(size_t i) noexcept { return m_ObjectPool.object(i); }
		
		/// @brief 执行对象的Frame函数
		void DoFrame() noexcept;
		
		/// @brief 执行对象的Render函数
		void DoRender() noexcept;
		
		// TODO: double -> float ???
		/// @brief 获取舞台边界
		Core::RectF GetBound() noexcept
		{
			return Core::RectF((float)m_BoundLeft, (float)m_BoundTop, (float)m_BoundRight, (float)m_BoundBottom);
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
		int Del(lua_State* L, bool kill_mode = false) noexcept;
		
		/// @brief 检查对象是否有效
		int IsValid(lua_State* L) noexcept;
		
		//重置对象的各项属性，并释放资源，保留uid和id
		void DirtResetObject(GameObject* p) noexcept;
		
		/// @brief 设置元素的图像状态
		bool SetImgState(GameObject* p, BlendMode m, Core::Color4B c) noexcept;
		
		/// @brief 特化设置HGE粒子的渲染状态
		bool SetParState(GameObject* p, BlendMode m, Core::Color4B c) noexcept;
		
		/// @brief 清空对象池
		void ResetPool() noexcept;
		
		/// @brief 获取下一个元素的ID
		/// @return 返回-1表示无元素
		int NextObject(int groupId, int id) noexcept;
		
		/// @brief 获取列表中的第一个元素ID
		/// @note 为迭代器使用
		/// @return 返回-1表示无元素
		int FirstObject(int groupId) noexcept;
		
		/// @brief 调试目的，获取对象列表
		int GetObjectTable(lua_State* L) noexcept;
	private:
		// 用于多world
		
		lua_Integer m_iWorld = 15; // 当前的 world mask
		std::array<lua_Integer, 4> m_Worlds = { 15, 0, 0, 0 }; // 预置的 world mask
	public:
		// 用于多world
		
		// 设置当前的world mask
		inline void SetWorldFlag(lua_Integer world) noexcept {
			m_iWorld = world;
		}
		// 获取当前的world mask
		inline lua_Integer GetWorldFlag() noexcept {
			return m_iWorld;
		}
		// 设置预置的world mask
		inline void ActiveWorlds(lua_Integer a, lua_Integer b, lua_Integer c, lua_Integer d) noexcept {
			m_Worlds[0] = a;
			m_Worlds[1] = b;
			m_Worlds[2] = c;
			m_Worlds[3] = d;
		}
		// 检查两个world mask位与或的结果 //静态函数，不应该只用于类内
		static inline bool CheckWorld(lua_Integer gameworld, lua_Integer objworld) {
			return (gameworld == objworld) || (gameworld & objworld);
		}
		// 对两个world mask，分别与预置的world mask位与或，用于检查是否在同一个world内
		bool CheckWorlds(int a, int b) noexcept {
			if (CheckWorld(a, m_Worlds[0]) && CheckWorld(b, m_Worlds[0]))return true;
			if (CheckWorld(a, m_Worlds[1]) && CheckWorld(b, m_Worlds[1]))return true;
			if (CheckWorld(a, m_Worlds[2]) && CheckWorld(b, m_Worlds[2]))return true;
			if (CheckWorld(a, m_Worlds[3]) && CheckWorld(b, m_Worlds[3]))return true;
			return false;
		}
	private:
		// 用于超级暂停
		
		lua_Integer m_superpause = 0;
		lua_Integer m_nextsuperpause = 0;
	public:
		// 用于超级暂停
		
		// 获取可信的超级暂停时间
		inline lua_Integer GetSuperPauseTime() noexcept {
			return m_superpause;
		}
		// 获取超级暂停剩余时间
		inline lua_Integer GetNextFrameSuperPauseTime() noexcept {
			return m_nextsuperpause;
		}
		// 设置超级暂停剩余时间
		inline void SetNextFrameSuperPauseTime(lua_Integer time) noexcept {
			m_nextsuperpause = time;
		}
		// 更新超级暂停的剩余时间并返回当前的可信值
		inline lua_Integer UpdateSuperPause() {
			m_superpause = m_nextsuperpause;
			if (m_nextsuperpause > 0)
				m_nextsuperpause = m_nextsuperpause - 1;
			return m_superpause;
		}
	public:
		// 内部使用
		
		void DrawCollider();
		void DrawGroupCollider(int groupId, Core::Color4B fillColor);
		void DrawGroupCollider2(int groupId, Core::Color4B fillColor);
	public:
		// lua api

		static int api_NextObject(lua_State* L) noexcept;
		static int api_ObjList(lua_State* L) noexcept;

		static int api_New(lua_State* L) noexcept;
		static int api_ResetObject(lua_State* L) noexcept;
		static int api_Del(lua_State* L) noexcept;
		static int api_Kill(lua_State* L) noexcept;
		static int api_IsValid(lua_State* L) noexcept;
		static int api_BoxCheck(lua_State* L) noexcept;
		static int api_ColliCheck(lua_State* L) noexcept;
		static int api_Angle(lua_State* L) noexcept;
		static int api_Dist(lua_State* L) noexcept;
		static int api_GetV(lua_State* L) noexcept;
		static int api_SetV(lua_State* L) noexcept;

		static int api_SetImgState(lua_State* L) noexcept;
		static int api_SetParState(lua_State* L) noexcept;
		static int api_GetAttr(lua_State* L) noexcept;
		static int api_SetAttr(lua_State* L) noexcept;

		static int api_DefaultRenderFunc(lua_State* L) noexcept;

		static int api_ParticleStop(lua_State* L) noexcept;
		static int api_ParticleFire(lua_State* L) noexcept;
		static int api_ParticleGetn(lua_State* L) noexcept;
		static int api_ParticleGetEmission(lua_State* L) noexcept;
		static int api_ParticleSetEmission(lua_State* L) noexcept;

	public:
		GameObjectPool(lua_State* pL);
		GameObjectPool& operator=(const GameObjectPool&) = delete;
		GameObjectPool(const GameObjectPool&) = delete;
		~GameObjectPool();
	};
}
