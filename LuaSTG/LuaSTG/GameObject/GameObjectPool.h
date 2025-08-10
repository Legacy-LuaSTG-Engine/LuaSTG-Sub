#pragma once
#include "GameObject/GameObject.hpp"
#include "core/FixedObjectPool.hpp"
#include <deque>
#include <list>
#include <memory_resource>
#include <ranges>
#include <algorithm>

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

	// 游戏对象管理器回调函数集
	struct CORE_NO_VIRTUAL_TABLE IGameObjectManagerCallbacks {
		// 获取当前回调函数集的名称
		[[nodiscard]] virtual std::string_view getCallbacksName() const noexcept = 0;
		// 对象管理器分配对象
		virtual void onCreate(GameObject* object) = 0;
		// 对象管理器回收对象
		virtual void onDestroy(GameObject* object) = 0;
		// 对象管理器批量回收对象之前
		virtual void onBeforeBatchDestroy() = 0;
		// 对象管理器批量回收对象之后
		virtual void onAfterBatchDestroy() = 0;
		// 对象管理器批量更新对象之前
		virtual void onBeforeBatchUpdate() = 0;
		// 对象管理器批量更新对象之后
		virtual void onAfterBatchUpdate() = 0;
		// 对象管理器批量渲染对象之前
		virtual void onBeforeBatchRender() = 0;
		// 对象管理器批量渲染对象之后
		virtual void onAfterBatchRender() = 0;
		// 对象管理器批量进行出界检查之前
		virtual void onBeforeBatchOutOfWorldBoundCheck() = 0;
		// 对象管理器批量进行出界检查之后
		virtual void onAfterBatchOutOfWorldBoundCheck() = 0;
		// 对象管理器批量进行相交检测之前
		virtual void onBeforeBatchIntersectDetect() = 0;
		// 对象管理器批量进行相交检测之后
		virtual void onAfterBatchIntersectDetect() = 0;
	};

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

		// GameObject lists
		std::pmr::unsynchronized_pool_resource m_memory_resource;
		GameObjectUpdateLinkedList m_update_list;
		std::pmr::set<GameObject*, GameObjectLayerComparer> m_render_list;
		std::array<GameObjectDetectLinkedList, LOBJPOOL_GROUPN> m_detect_lists;
		std::pmr::vector<IGameObjectManagerCallbacks*> m_callbacks;

		void resetGameObjectLists();

#ifdef USING_MULTI_GAME_WORLD
		GameObject* m_pCurrentObject{};
#endif // USING_MULTI_GAME_WORLD
		GameObject* m_LockObjectA{};
		GameObject* m_LockObjectB{};

		// 场景边界
		double m_BoundLeft = -100.f;
		double m_BoundRight = 100.f;
		double m_BoundTop = 100.f;
		double m_BoundBottom = -100.f;

		bool m_is_rendering{ false };
		bool m_is_detecting_intersect{ false };

		FrameStatistics m_statistics[2]{};
		size_t m_statistics_index{ 0 };

		struct IntersectionDetectionResult {
			uint64_t uid1{};
			uint64_t uid2{};
			GameObject* object1{};
			GameObject* object2{};
		};

	private:

		// 检查指定对象的坐标是否在场景边界内
		inline bool _ObjectBoundCheck(GameObject* object) const noexcept
		{
			if (!object->bound)
				return true;
			return object->isInRect(m_BoundLeft, m_BoundRight, m_BoundBottom, m_BoundTop);
		}

	public:
		void addCallbacks(IGameObjectManagerCallbacks* const callbacks) {
			for (auto const c : m_callbacks) {
				if (c == callbacks) {
					return;
				}
			}
			m_callbacks.push_back(callbacks);
		}
		void removeCallbacks(IGameObjectManagerCallbacks* const callbacks) {
			for (auto it = m_callbacks.begin(); it != m_callbacks.end();) {
				if (*it == callbacks) {
					it = m_callbacks.erase(it);
					continue;
				}
				++it;
			}
		}
		void dispatchOnCreate(GameObject* const object) {
			for (auto const c : m_callbacks) {
				c->onCreate(object);
			}
		}
		void dispatchOnDestroy(GameObject* const object) {
			for (auto const c : m_callbacks) {
				c->onDestroy(object);
			}
		}
		void dispatchOnBeforeBatchDestroy() {
			for (auto const c : m_callbacks) {
				c->onBeforeBatchDestroy();
			}
		}
		void dispatchOnAfterBatchDestroy() {
			for (auto const c : m_callbacks) {
				c->onAfterBatchDestroy();
			}
		}
		void dispatchOnBeforeBatchUpdate() {
			for (auto const c : m_callbacks) {
				c->onBeforeBatchUpdate();
			}
		}
		void dispatchOnAfterBatchUpdate() {
			for (auto const c : m_callbacks) {
				c->onAfterBatchUpdate();
			}
		}
		void dispatchOnBeforeBatchRender() {
			for (auto const c : m_callbacks) {
				c->onBeforeBatchRender();
			}
		}
		void dispatchOnAfterBatchRender() {
			for (auto const c : m_callbacks) {
				c->onAfterBatchRender();
			}
		}
		void dispatchOnBeforeBatchOutOfWorldBoundCheck() {
			for (auto const c : m_callbacks) {
				c->onBeforeBatchOutOfWorldBoundCheck();
			}
		}
		void dispatchOnAfterBatchOutOfWorldBoundCheck() {
			for (auto const c : m_callbacks) {
				c->onAfterBatchOutOfWorldBoundCheck();
			}
		}
		void dispatchOnBeforeBatchIntersectDetect() {
			for (auto const c : m_callbacks) {
				c->onBeforeBatchIntersectDetect();
			}
		}
		void dispatchOnAfterBatchIntersectDetect() {
			for (auto const c : m_callbacks) {
				c->onAfterBatchIntersectDetect();
			}
		}
		void DebugNextFrame();
		FrameStatistics DebugGetFrameStatistics();

	public:
#ifdef USING_MULTI_GAME_WORLD
		GameObject* getCurrentGameObject() const noexcept { return m_pCurrentObject; }
#endif // USING_MULTI_GAME_WORLD

		/// @brief 获取已分配对象数量
		size_t GetObjectCount() noexcept { return m_ObjectPool.size(); }

		/// @brief 获取对象
		GameObject* GetPooledObject(size_t i) noexcept { return m_ObjectPool.object(i); }

		// 对象更新：传统模式
		// 回调 -> 运动更新 -> 回调 -> 运动更新 -> ...
		void updateMovementsLegacy();

		// 对象更新：批量模式
		// 回调所有 -> 更新所有运动
		void updateMovements();

		// 对象更新：传统模式新旧帧衔接
		void updateNextLegacy();

		// 对象更新：新旧帧衔接
		void updateNext();

		// 渲染所有游戏对象
		void render();

		/// @brief 设置舞台边界
		inline void SetBound(double const l, double const r, double const b, double const t) noexcept {
			assert(r >= l && t >= b);
			m_BoundLeft = l;
			m_BoundRight = r;
			m_BoundTop = t;
			m_BoundBottom = b;
		}

		inline bool isPointInBound(double const x, double const y) const noexcept {
			return x >= m_BoundLeft
				&& x <= m_BoundRight
				&& y >= m_BoundBottom
				&& y <= m_BoundTop;
		}

		// 脱离世界边界检测：传统模式
		void detectOutOfWorldBoundLegacy();

		// 脱离世界边界检测：延迟模式
		void detectOutOfWorldBound();

		// 相交检测：传统模式
		// 检测 -> 回调（如果相交） -> 检测 -> 回调（如果相交） -> ...
		void detectIntersectionLegacy(uint32_t group1, uint32_t group2);

		// 相交检测：批量模式
		// 检测所有 -> 回调所有
		void detectIntersection(std::pmr::vector<IntersectionDetectionGroupPair> const& group_pairs);

		/// @brief 更新对象的XY坐标偏移量
		void UpdateXY() noexcept;
	
		//重置对象的各项属性，并释放资源，保留uid和id
		void DirtResetObject(GameObject* p) noexcept;

		// 修改游戏对象所在的碰撞组：从原碰撞组链表移除，插入到新碰撞组链表，并更新 group 属性
		void setGroup(GameObject* object, size_t group);

		// 修改游戏对象渲染图层：从有序渲染链表移除，更新 layer 属性，重新插入有序渲染链表
		void setLayer(GameObject* object, double layer);

		/// @brief 清空对象池
		void ResetPool() noexcept;

		[[nodiscard]] GameObject* allocate() { return allocateWithCallbacks(nullptr); }
		[[nodiscard]] GameObject* allocateWithCallbacks(IGameObjectCallbacks* callbacks);
		GameObject* freeWithCallbacks(GameObject* object);
		bool queueToFree(GameObject* object, bool legacy_kill_mode = false);
		[[nodiscard]] bool isLockedByDetectIntersection(GameObject const* const object) const noexcept { return object == m_LockObjectA || object == m_LockObjectB; }
		[[nodiscard]] bool isRendering() const noexcept { return m_is_rendering; }
		[[nodiscard]] bool isDetectingIntersect() const noexcept { return m_is_detecting_intersect; }

		GameObject* getUpdateListFirst() { return m_update_list.first(); }
		GameObject* getUpdateListNext(size_t const id) { return getUpdateListNext(m_ObjectPool.object(id)); }
		GameObject* getUpdateListNext(GameObject const* object) {
			if (object == nullptr) {
				return nullptr;
			}
			return object->update_list_next;
		}
		GameObject* getDetectListFirst(size_t const group) { return m_detect_lists[group].first(); }
		GameObject* getDetectListNext(size_t const group, size_t const id) { return getDetectListNext(group, m_ObjectPool.object(id)); }
		GameObject* getDetectListNext(size_t const group, GameObject const* const object) {
			if (object == nullptr) {
				return nullptr;
			}
			if (static_cast<size_t>(object->group) != group) {
				return nullptr;
			}
			return object->detect_list_next;
		}

#ifdef USING_MULTI_GAME_WORLD
	private:
		// 用于多world

		int32_t m_iWorld = 15; // 当前的 world mask
		std::array<int32_t, 4> m_Worlds = { 15, 0, 0, 0 }; // 预置的 world mask
	public:
		// 用于多world

		// 设置当前的world mask
		void SetWorldFlag(int32_t const world) noexcept {
			m_iWorld = world;
		}
		// 获取当前的world mask
		int32_t GetWorldFlag() const noexcept {
			return m_iWorld;
		}
		// 设置预置的world mask
		void ActiveWorlds(int32_t const a, int32_t const b, int32_t const c, int32_t const d) noexcept {
			m_Worlds[0] = a;
			m_Worlds[1] = b;
			m_Worlds[2] = c;
			m_Worlds[3] = d;
		}
		// 检查两个world mask位与或的结果 //静态函数，不应该只用于类内
		static inline bool CheckWorld(int32_t const gameworld, int32_t const objworld) {
			return (gameworld == objworld) || (gameworld & objworld);
		}
		// 对两个world mask，分别与预置的world mask位与或，用于检查是否在同一个world内
		bool CheckWorlds(int32_t const a, int32_t const b) const noexcept {
			if (CheckWorld(a, m_Worlds[0]) && CheckWorld(b, m_Worlds[0])) return true;
			if (CheckWorld(a, m_Worlds[1]) && CheckWorld(b, m_Worlds[1])) return true;
			if (CheckWorld(a, m_Worlds[2]) && CheckWorld(b, m_Worlds[2])) return true;
			if (CheckWorld(a, m_Worlds[3]) && CheckWorld(b, m_Worlds[3])) return true;
			return false;
		}
#endif // USING_MULTI_GAME_WORLD

	private:
		// 用于超级暂停

		int64_t m_superpause = 0;
		int64_t m_nextsuperpause = 0;
	public:
		// 用于超级暂停

		// 获取可信的超级暂停时间
		inline int64_t GetSuperPauseTime() const noexcept {
			return m_superpause;
		}
		// 获取超级暂停剩余时间
		inline int64_t GetNextFrameSuperPauseTime() const noexcept {
			return m_nextsuperpause;
		}
		// 设置超级暂停剩余时间
		inline void SetNextFrameSuperPauseTime(int64_t const time) noexcept {
			m_nextsuperpause = time;
		}
		// 更新超级暂停的剩余时间并返回当前的可信值
		inline int64_t UpdateSuperPause() noexcept {
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
		GameObjectPool();
		GameObjectPool& operator=(const GameObjectPool&) = delete;
		GameObjectPool(const GameObjectPool&) = delete;
		~GameObjectPool();
	};
}
