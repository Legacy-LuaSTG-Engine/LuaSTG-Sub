#pragma once
#include "GameObject/GameObject.hpp"
#include "core/FixedObjectPool.hpp"
#include <deque>
#include <list>
#include <memory_resource>

// 对象池信息
#define LOBJPOOL_SIZE   32768 // 最大对象数 //32768(full) //16384(half)
#define LOBJPOOL_GROUPN 16    // 碰撞组数

namespace luastg
{
	struct GameObjectLayerComparer {
		bool operator()(GameObject const* const left, GameObject const* const right) const noexcept {
			if (left->layer != right->layer) {
				return left->layer < right->layer;
			}
			return left->unique_id < right->unique_id;
		}
	};

	struct GameObjectUpdateLinkedListFieldAssessor {
		static GameObject* getPrevious(GameObject const* const object) noexcept {
			return object->update_list_previous;
		}
		static void setPrevious(GameObject* const object, GameObject* value) noexcept {
			object->update_list_previous = value;
		}
		static GameObject* getNext(GameObject const* const object) noexcept {
			return object->update_list_next;
		}
		static void setNext(GameObject* const object, GameObject* value) noexcept {
			object->update_list_next = value;
		}
	};

	struct GameObjectDetectLinkedListFieldAssessor {
		static GameObject* getPrevious(GameObject const* const object) noexcept {
			return object->detect_list_previous;
		}
		static void setPrevious(GameObject* const object, GameObject* value) noexcept {
			object->detect_list_previous = value;
		}
		static GameObject* getNext(GameObject const* const object) noexcept {
			return object->detect_list_next;
		}
		static void setNext(GameObject* const object, GameObject* value) noexcept {
			object->detect_list_next = value;
		}
	};

	template<typename FieldAssessor>
	class GameObjectLinkedList {
	public:
		[[nodiscard]] bool empty() const noexcept {
			return m_first == nullptr && m_last == nullptr;
		}
		void add(GameObject* object) noexcept {
			assert(object != nullptr);
			if (empty()) {
				m_first = object;
				m_last = object;
				FieldAssessor::setPrevious(object, nullptr);
				FieldAssessor::setNext(object, nullptr);
			}
			else {
				auto const last = m_last;
				FieldAssessor::setNext(last, object);
				FieldAssessor::setPrevious(object, last);
				m_last = object;
			}
		}
		GameObject* remove(GameObject* object) noexcept {
			assert(object != nullptr);
			assert(!empty());
			if (m_first == object && m_last == object) {
				m_first = nullptr;
				m_last = nullptr;
				assert(FieldAssessor::getPrevious(object) == nullptr);
				assert(FieldAssessor::getNext(object) == nullptr);
				return nullptr;
			}
			auto const previous = FieldAssessor::getPrevious(object);
			auto const next = FieldAssessor::getNext(object);
			if (previous != nullptr) {
				FieldAssessor::setNext(previous, next);
			}
			if (next != nullptr) {
				FieldAssessor::setPrevious(next, previous);
			}
			FieldAssessor::setPrevious(object, nullptr);
			FieldAssessor::setNext(object, nullptr);
			if (m_first == object) {
				m_first = next;
			}
			if (m_last == object) {
				m_last = previous;
			}
			return next;
		}
		[[nodiscard]] GameObject* first() const noexcept { return m_first; }
		void clear() {
			GameObject* object = m_first;
			while (object) {
				GameObject* const current = object;
				object = FieldAssessor::getNext(object);
				FieldAssessor::setPrevious(current, nullptr);
				FieldAssessor::setNext(current, nullptr);
			}
		}
	private:
		GameObject* m_first{};
		GameObject* m_last{};
	};

	using GameObjectUpdateLinkedList = GameObjectLinkedList<GameObjectUpdateLinkedListFieldAssessor>;
	using GameObjectDetectLinkedList = GameObjectLinkedList<GameObjectDetectLinkedListFieldAssessor>;

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

		struct IntersectionDetectionGroupPair {
			uint32_t group1{};
			uint32_t group2{};
		};

	private:
		core::FixedObjectPool<GameObject, LOBJPOOL_SIZE> m_ObjectPool;
		uint64_t m_iUid = 0;
		lua_State* G_L = nullptr;

		// GameObject lists
		std::pmr::unsynchronized_pool_resource m_memory_resource;
		GameObjectUpdateLinkedList m_update_list;
		std::pmr::set<GameObject*, GameObjectLayerComparer> m_render_list;
		std::array<GameObjectDetectLinkedList, LOBJPOOL_GROUPN> m_detect_lists;

		void resetGameObjectLists();

#ifdef USING_MULTI_GAME_WORLD
		GameObject* m_pCurrentObject{};
#endif // USING_MULTI_GAME_WORLD
		GameObject* m_LockObjectA{};
		GameObject* m_LockObjectB{};

		// 场景边界
		lua_Number m_BoundLeft = -100.f;
		lua_Number m_BoundRight = 100.f;
		lua_Number m_BoundTop = 100.f;
		lua_Number m_BoundBottom = -100.f;

		bool m_IsRendering{ false };

		FrameStatistics m_DbgData[2]{};
		size_t m_DbgIdx{ 0 };

		struct IntersectionDetectionResult {
			uint64_t uid1{};
			uint64_t uid2{};
			GameObject* object1{};
			GameObject* object2{};
		};

	private:

		// 申请一个对象，重置对象并将对象插入到各个链表，不处理lua部分，返回申请的对象
		GameObject* _AllocObject();

		// 释放一个对象，将对象从渲染和检测链表中移除，并回收，不处理lua部分和对象资源
		GameObject* _ReleaseObject(GameObject* object);

		// 检查指定对象的坐标是否在场景边界内
		inline bool _ObjectBoundCheck(GameObject* object) const noexcept
		{
			if (!object->bound)
				return true;
			return object->isInRect(m_BoundLeft, m_BoundRight, m_BoundBottom, m_BoundTop);
		}

		// 释放一个对象，完全释放
		GameObject* _FreeObject(GameObject* p, int ot_at = 0) noexcept;

		GameObject* _ToGameObject(lua_State* L, int idx);
		GameObject* _TableToGameObject(lua_State* L, int idx);

		void _GameObjectCallback(lua_State* L, int otidx, GameObject* p, int cbidx);

	public:
		void DebugNextFrame();
		FrameStatistics DebugGetFrameStatistics();

	public:
#ifdef USING_MULTI_GAME_WORLD
		int PushCurrentObject(lua_State* L) noexcept;
#endif // USING_MULTI_GAME_WORLD

		GameObject* CastGameObject(lua_State* L, int idx);

		/// @brief 检查是否为主线程
		bool CheckIsMainThread(lua_State* pL) noexcept { return pL == G_L; }

		/// @brief 获取已分配对象数量
		size_t GetObjectCount() noexcept { return m_ObjectPool.size(); }

		/// @brief 获取对象
		GameObject* GetPooledObject(size_t i) noexcept { return m_ObjectPool.object(i); }

		// 对象更新：传统模式
		// 回调 -> 运动更新 -> 回调 -> 运动更新 -> ...
		void updateMovementsLegacy(int32_t objects_index = 0, lua_State* L = nullptr);

		// 对象更新：批量模式
		// 回调所有 -> 更新所有运动
		void updateMovements(int32_t objects_index = 0, lua_State* L = nullptr);

		// 对象更新：传统模式新旧帧衔接
		void updateNextLegacy(int32_t objects_index = 0, lua_State* L = nullptr);

		// 对象更新：新旧帧衔接
		void updateNext(int32_t objects_index = 0, lua_State* L = nullptr);

		/// @brief 执行对象的Render函数
		void DoRender() noexcept;

		/// @brief 设置舞台边界
		inline void SetBound(lua_Number l, lua_Number r, lua_Number b, lua_Number t) noexcept {
			assert(r >= l && t >= b);
			m_BoundLeft = l;
			m_BoundRight = r;
			m_BoundTop = t;
			m_BoundBottom = b;
		}

		inline bool isPointInBound(lua_Number x, lua_Number y) noexcept {
			return x >= m_BoundLeft
				&& x <= m_BoundRight
				&& y >= m_BoundBottom
				&& y <= m_BoundTop;
		}

		// 脱离世界边界检测：传统模式
		void detectOutOfWorldBoundLegacy(int32_t objects_index = 0, lua_State* L = nullptr);

		// 脱离世界边界检测：延迟模式
		void detectOutOfWorldBound(int32_t objects_index = 0, lua_State* L = nullptr);

		// 相交检测：传统模式
		// 检测 -> 回调（如果相交） -> 检测 -> 回调（如果相交） -> ...
		void detectIntersectionLegacy(uint32_t group1, uint32_t group2, int32_t objects_index = 0, lua_State* L = nullptr);

		// 相交检测：批量模式
		// 检测所有 -> 回调所有
		void detectIntersection(std::pmr::vector<IntersectionDetectionGroupPair> const& group_pairs, int32_t objects_index = 0, lua_State* L = nullptr);

		/// @brief 更新对象的XY坐标偏移量
		void UpdateXY() noexcept;
		
		/// @brief 通知对象删除
		int Del(lua_State* L, bool kill_mode = false) noexcept;

		/// @brief 检查对象是否有效
		int IsValid(lua_State* L) noexcept;

		//重置对象的各项属性，并释放资源，保留uid和id
		void DirtResetObject(GameObject* p) noexcept;

		/// @brief 设置元素的图像状态
		bool SetImgState(GameObject* p, BlendMode m, core::Color4B c) noexcept;

		/// @brief 特化设置HGE粒子的渲染状态
		bool SetParState(GameObject* p, BlendMode m, core::Color4B c) noexcept;

		// 修改游戏对象所在的碰撞组：从原碰撞组链表移除，插入到新碰撞组链表，并更新 group 属性
		void setGroup(GameObject* object, size_t group);

		// 修改游戏对象渲染图层：从有序渲染链表移除，更新 layer 属性，重新插入有序渲染链表
		void setLayer(GameObject* object, double layer);

		/// @brief 清空对象池
		void ResetPool() noexcept;

		/// @brief 获取下一个元素的ID
		/// @return 返回-1表示无元素
		int NextObject(int group, int id) noexcept;

		/// @brief 获取列表中的第一个元素ID
		/// @note 为迭代器使用
		/// @return 返回-1表示无元素
		int FirstObject(int group) noexcept;

		[[nodiscard]] GameObject* allocate() { return _AllocObject(); }
		[[nodiscard]] bool isLockedByDetectIntersection(GameObject const* const object) const noexcept { return object == m_LockObjectA || object == m_LockObjectB; }
		[[nodiscard]] bool isRendering() const noexcept { return m_IsRendering; }

#ifdef USING_MULTI_GAME_WORLD
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
#endif // USING_MULTI_GAME_WORLD

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
		void DrawGroupCollider(int groupId, core::Color4B fillColor);
		void DrawGroupCollider2(int groupId, core::Color4B fillColor);
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
		
		static int api_ObjFrame(lua_State* L);
		static int api_AfterFrame(lua_State* L);
		static int api_BoundCheck(lua_State* L);
		static int api_CollisionCheck(lua_State* L);

		static int api_SetImgState(lua_State* L) noexcept;
		static int api_SetParState(lua_State* L) noexcept;
	
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
