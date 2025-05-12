#pragma once
#include "GameResource/ResourceBase.hpp"
#include "GameResource/ResourceParticle.hpp"
#include "lua.hpp"

#define LGOBJ_CC_INIT 1
#define LGOBJ_CC_DEL 2
#define LGOBJ_CC_FRAME 3
#define LGOBJ_CC_RENDER 4
#define LGOBJ_CC_COLLI 5
#define LGOBJ_CC_KILL 6

namespace luastg
{
	// 游戏对象状态
	enum class GameObjectStatus : uint8_t
	{
		Free   = 0, // 空闲可用状态
		Active = 1, // 正常活跃状态
		Dead   = 2, // 生命周期结束
		Killed = 4, // 生命周期结束
	};

	struct GameObjectFeatures {
		uint8_t is_class : 1;
		uint8_t is_render_class : 1;
		uint8_t has_callback_create : 1;
		uint8_t has_callback_destroy : 1;
		uint8_t has_callback_update : 1;
		uint8_t has_callback_render : 1;
		uint8_t has_callback_trigger : 1;
		uint8_t has_callback_legacy_kill : 1;

		void reset() { static_assert(sizeof(GameObjectFeatures) == sizeof(uint8_t)); *reinterpret_cast<uint8_t*>(this) = 0u; }
		void read(lua_State* vm, int index);
	};

#pragma warning(push)
#pragma warning(disable:26495)

	// 游戏对象前向定义
	struct GameObject;

	// 游戏对象回调函数集和调用链
	struct CORE_NO_VIRTUAL_TABLE IGameObjectCallbacks {
		// 获取当前回调函数集的名称
		virtual std::string_view getCallbacksName(GameObject* self) const noexcept = 0;
		// 获取调用链的下一个回调函数集，返回 nullptr 表示已无后续的回调函数集
		virtual IGameObjectCallbacks* getNextCallbacks(GameObject* self) const noexcept = 0;
		// 设置调用链的下一个回调函数集
		virtual void setNextCallbacks(GameObject* self, IGameObjectCallbacks* callbacks) = 0;
		// 被分配后调用
		virtual void onCreate(GameObject* self) = 0;
		// 被回收前调用
		virtual void onDestroy(GameObject* self) = 0;
		// 被标记为删除状态时调用
		virtual void onQueueToDestroy(GameObject* self, std::string_view reason) = 0;
		// 每帧的运行更新前调用
		virtual void onUpdate(GameObject* self) = 0;
		// 每帧的运动更新后调用
		virtual void onLateUpdate(GameObject* self) = 0;
		// 每次渲染到该游戏对象时调用, TODO: 考虑将该能力废弃掉，改为使用 Renderer 组件实现
		virtual void onRender(GameObject* self) = 0;
		// 每次与别的游戏对象相交时调用
		virtual void onTrigger(GameObject* self, GameObject* other) = 0;
	};

	// 游戏对象
	struct GameObject {
		static constexpr uint64_t max_id = 0xffffull;
		static constexpr uint64_t max_unique_id = 0xffff'ffff'ffffull;

		static constexpr int unhandled_set_group = 1;
		static constexpr int unhandled_set_layer = 2;

		// 回调函数
		IGameObjectCallbacks* callbacks;	// [P] [不可见] 回调函数集和调用链

		// 链表部分

		GameObject* update_list_previous;	// [P] [不可见] 更新链表上一个对象
		GameObject* update_list_next;		// [P] [不可见] 更新链表下一个对象
		GameObject* detect_list_previous;	// [P] [不可见] 相交检测链表上一个对象
		GameObject* detect_list_next;		// [P] [不可见] 相交检测链表下一个对象

		// 基本信息

		uint64_t id : 16;				// [8:16] [不可见] 对象在对象池中的索引
		uint64_t unique_id : 48;		// [8:48] [不可见] 对象全局唯一标识符

		// 分组

#ifdef USING_MULTI_GAME_WORLD
		Integer world;					// [8] 世界标记位，用于对一个对象进行分组，影响更新、渲染、碰撞检测等
#endif // USING_MULTI_GAME_WORLD

		// 位置

		double last_x;					// [8] [不可见] 对象上一帧坐标 x
		double last_y;					// [8] [不可见] 对象上一帧坐标 y
		double x;						// [8] 对象坐标 x
		double y;						// [8] 对象坐标 y
		double dx;						// [8] [只读] 对象坐标增量 x
		double dy;						// [8] [只读] 对象坐标增量 y

		// 运动学

		double vx;						// [8] 对象速度 x 分量
		double vy;						// [8] 对象速度 y 分量
		double ax;						// [8] 对象加速度 x 分量
		double ay;						// [8] 对象加速度 x 分量
	#ifdef USER_SYSTEM_OPERATION
		double max_vx;					// [8] 对象速度 x 分量最大值
		double max_vy;					// [8] 对象速度 y 分量最大值
		double max_v;					// [8] 对象速度最大值
		double ag;						// [8] 重力加速度
	#endif
		//lua_Number va, speed; // 速度方向 速度值

		// 碰撞体

		int64_t group;					// [8] 对象所在的碰撞组
		double a;						// [8] 矩形模式下，为横向宽度一半；非矩形模式下，为圆半径或椭圆横向宽度一半
		double b;						// [8] 矩形模式下，为纵向宽度一半；非矩形模式下，为圆半径或椭圆纵向宽度一半
		double col_r;					// [8] [不可见] 碰撞体外接圆半径

		// 渲染

		double layer;					// [8] 图层
		double hscale;					// [8] 横向渲染缩放
		double vscale;					// [8] 纵向渲染缩放
		double rot;						// [8] 平面渲染旋转角
		double omega;					// [8] 平面渲染旋转角加速度
		int64_t ani_timer;				// [P] [只读] 动画自增计数器
		IResourceBase* res;				// [P] 渲染资源
		IParticlePool* ps;				// [P] 粒子系统

		// 更新控制

		int64_t timer;					// [P] 自增计数器
	#ifdef LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		int64_t pause;					// [P] 对象被暂停的时间(帧) 对象被暂停时，将跳过速度计算，但是timer会增加，frame仍会调用
	#endif // LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE

		// 小型属性
		// 小型属性 - 渲染
		core::Color4B vertex_color;		// [4] 顶点颜色
		BlendMode blend_mode;			// [1] 混合模式
		// 小型属性 - 基本信息
		GameObjectFeatures features;	// [1] [不可见] 对象类的一些特性
		GameObjectStatus status;		// [1] 对象状态

		// 布尔属性
		// 布尔属性 - 碰撞体
		uint8_t bound : 1;				// [1] 是否离开边界自动回收
		uint8_t colli : 1;				// [1] 是否参与碰撞
		uint8_t rect : 1;				// [1] 是否为矩形碰撞盒
		// 布尔属性 - 渲染
		uint8_t hide : 1;				// [b] 不渲染
		uint8_t navi : 1;				// [b] 根据坐标增量自动设置渲染旋转角
		// 布尔属性 - 更新控制
	#ifdef LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		uint8_t resolve_move : 1;		// [b] 是否为计算速度而非计算位置
	#endif // LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		uint8_t ignore_super_pause : 1;	// [b] 是否无视超级暂停。 超级暂停时，timer不会增加，frame不会调用，但render会调用。
		uint8_t last_xy_touched : 1;	// [b] 是否已经更新过 last_x 和 last_y 值，如果未更新过，表明对象刚生成，获取 dx 和 dy 时应当返回 0

		// 成员方法

		void Reset();
		void DirtReset();
		void UpdateCollisionCircleRadius();
		bool ChangeResource(std::string_view const& res_name);
		void ReleaseResource();
#ifdef LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
		void ChangeLuaRC(lua_State* L, int idx);
		void ReleaseLuaRC(lua_State* L, int idx);
#endif // LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT

		void Update();
		void UpdateLast();
		void UpdateTimer();
		void Render();

		void UpdateV2();
		void UpdateLastV2();

		void addCallbacks(IGameObjectCallbacks* const c) {
			if (c == nullptr) {
				assert(false); return;
			}
			assert(c->getNextCallbacks(this) == nullptr);
			if (callbacks == nullptr) {
				callbacks = c;
				c->setNextCallbacks(this, nullptr);
				return;
			}
			for (auto cc = callbacks; cc != nullptr; cc = cc->getNextCallbacks(this)) {
				if (cc == c) {
					return;
				}
				if (cc->getNextCallbacks(this) == nullptr) {
					cc->setNextCallbacks(this, c);
					c->setNextCallbacks(this, nullptr);
					return;
				}
			}
		}
		void removeCallbacks(IGameObjectCallbacks* const c) {
			if (c == nullptr) {
				assert(false); return;
			}
			if (callbacks == c) {
				callbacks = callbacks->getNextCallbacks(this);
				c->setNextCallbacks(this, nullptr);
				return;
			}
			for (auto cc = callbacks; cc != nullptr; cc = cc->getNextCallbacks(this)) {
				if (auto const next = cc->getNextCallbacks(this); next == c) {
					cc->setNextCallbacks(this, next->getNextCallbacks(this));
					c->setNextCallbacks(this, nullptr);
					return;
				}
			}
		}
		void removeAllCallbacks() {
			for (auto c = callbacks; c != nullptr;) {
				auto const next = c->getNextCallbacks(this);
				c->setNextCallbacks(this, nullptr);
				c = next;
			}
			callbacks = nullptr;
		}
		void dispatchOnCreate() {
			for (auto c = callbacks; c != nullptr; c = c->getNextCallbacks(this)) {
				c->onCreate(this);
			}
		}
		void dispatchOnDestroy() {
			for (auto c = callbacks; c != nullptr; c = c->getNextCallbacks(this)) {
				c->onDestroy(this);
			}
		}
		void dispatchOnQueueToDestroy(std::string_view const reason) {
			for (auto c = callbacks; c != nullptr; c = c->getNextCallbacks(this)) {
				c->onQueueToDestroy(this, reason);
			}
		}
		void dispatchOnUpdate() {
			for (auto c = callbacks; c != nullptr; c = c->getNextCallbacks(this)) {
				c->onUpdate(this);
			}
		}
		void dispatchOnLateUpdate() {
			for (auto c = callbacks; c != nullptr; c = c->getNextCallbacks(this)) {
				c->onLateUpdate(this);
			}
		}
		void dispatchOnRender() {
			for (auto c = callbacks; c != nullptr; c = c->getNextCallbacks(this)) {
				c->onRender(this);
			}
		}
		void dispatchOnTrigger(GameObject* other) {
			for (auto c = callbacks; c != nullptr; c = c->getNextCallbacks(this)) {
				c->onTrigger(this, other);
			}
		}
		[[nodiscard]] bool hasRenderResource() const noexcept { return res != nullptr; }
		[[nodiscard]] bool hasParticlePool() const noexcept { return res != nullptr && res->GetType() == ResourceType::Particle && ps != nullptr; }
		[[nodiscard]] std::string_view getRenderResourceName() const noexcept {
			if (res != nullptr) {
				return res->GetResName();
			}
			return {""};
		}
		[[nodiscard]] double calculateSpeed() const noexcept { return std::hypot(vx, vy); }
		[[nodiscard]] double calculateSpeedDirection() const noexcept {
			if (std::abs(vx) > std::numeric_limits<double>::min() && std::abs(vy) > std::numeric_limits<double>::min()) {
				return std::atan2(vy, vx);
			}
			return rot;
		}
		void setSpeed(double const speed) noexcept {
			if (auto const current = calculateSpeed(); current > std::numeric_limits<double>::min()) {
				auto const a3 = speed / current;
				vx *= a3;
				vy *= a3;
			}
			else {
				vx = std::cos(rot) * speed;
				vy = std::sin(rot) * speed;
			}
		}
		void setSpeedDirection(double const direction) noexcept {
			if (auto const speed = calculateSpeed(); speed > std::numeric_limits<double>::min()) {
				vx = speed * std::cos(direction);
				vy = speed * std::sin(direction);
			}
			else {
				rot = direction;
			}
		}
		void setGroup(int64_t new_group);
		void setLayer(double new_layer);
		[[nodiscard]] bool isInRect(lua_Number const l, lua_Number const r, lua_Number const bb, lua_Number const t) const noexcept {
			return x >= l && x <= r && y >= bb && y <= t;
		}
		[[nodiscard]] bool isIntersect(GameObject const* other) const noexcept { return isIntersect(this, other); }

		void setResourceRenderState(BlendMode blend, core::Color4B color);
		void setParticleRenderState(BlendMode blend, core::Color4B color);
		void stopParticle() {
			if (!hasParticlePool()) {
				return;
			}
			ps->SetActive(false);
		}
		void startParticle() {
			if (!hasParticlePool()) {
				return;
			}
			ps->SetActive(true);
		}
		[[nodiscard]] size_t getParticleCount() const {
			if (!hasParticlePool()) {
				return 0;
			}
			return ps->GetAliveCount();
		}
		[[nodiscard]] int32_t getParticleEmission() const {
			if (!hasParticlePool()) {
				return 0;
			}
			return ps->GetEmission();
		}
		void setParticleEmission(int32_t const value) {
			if (!hasParticlePool()) {
				return;
			}
			ps->SetEmission(value);
		}
		
		[[nodiscard]] static bool isIntersect(GameObject const* p1, GameObject const* p2) noexcept;
	};

#pragma warning(pop)
}
