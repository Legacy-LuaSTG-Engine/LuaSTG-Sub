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

	// 游戏对象
	struct GameObject {
		static constexpr uint64_t max_id = 0xffffull;
		static constexpr uint64_t max_uid = 0xffff'ffff'ffffull;

		static constexpr int unhandled_set_group = 1;
		static constexpr int unhandled_set_layer = 2;

		// 链表部分

		GameObject* update_list_previous;	// [P] [不可见] 更新链表上一个对象
		GameObject* update_list_next;		// [P] [不可见] 更新链表下一个对象
		GameObject* detect_list_previous;	// [P] [不可见] 相交检测链表上一个对象
		GameObject* detect_list_next;		// [P] [不可见] 相交检测链表下一个对象

		// 基本信息

		uint64_t id : 16;				// [8:16] [不可见] 对象在对象池中的索引
		uint64_t uid : 48;				// [8:48] [不可见] 对象全局唯一标识符

		// 分组

#ifdef USING_MULTI_GAME_WORLD
		lua_Integer world;				// [P] 世界标记位，用于对一个对象进行分组，影响更新、渲染、碰撞检测等
#endif // USING_MULTI_GAME_WORLD

		// 位置

		lua_Number lastx;				// [8] [不可见] 对象上一帧坐标 x
		lua_Number lasty;				// [8] [不可见] 对象上一帧坐标 y
		lua_Number x;					// [8] 对象坐标 x
		lua_Number y;					// [8] 对象坐标 y
		lua_Number dx;					// [8] [只读] 对象坐标增量 x
		lua_Number dy;					// [8] [只读] 对象坐标增量 y

		// 运动学

		lua_Number vx;					// [8] 对象速度 x 分量
		lua_Number vy;					// [8] 对象速度 y 分量
		lua_Number ax;					// [8] 对象加速度 x 分量
		lua_Number ay;					// [8] 对象加速度 x 分量
	#ifdef USER_SYSTEM_OPERATION
		lua_Number maxvx;				// [8] 对象速度 x 分量最大值
		lua_Number maxvy;				// [8] 对象速度 y 分量最大值
		lua_Number maxv;				// [8] 对象速度最大值
		lua_Number ag;					// [8] 重力加速度
	#endif
		//lua_Number va, speed; // 速度方向 速度值

		// 碰撞体

		lua_Integer group;				// [P] 对象所在的碰撞组
		lua_Number a;					// [8] 矩形模式下，为横向宽度一半；非矩形模式下，为圆半径或椭圆横向宽度一半
		lua_Number b;					// [8] 矩形模式下，为纵向宽度一半；非矩形模式下，为圆半径或椭圆纵向宽度一半
		lua_Number col_r;				// [8] [不可见] 碰撞体外接圆半径

		// 渲染

		lua_Number layer;				// [8] 图层
		lua_Number hscale;				// [8] 横向渲染缩放
		lua_Number vscale;				// [8] 纵向渲染缩放
		lua_Number rot;					// [8] 平面渲染旋转角
		lua_Number omega;				// [8] 平面渲染旋转角加速度
		lua_Integer ani_timer;			// [P] [只读] 动画自增计数器
		IResourceBase* res;				// [P] 渲染资源
		IParticlePool* ps;				// [P] 粒子系统

		// 更新控制

		lua_Integer timer;				// [P] 自增计数器
	#ifdef LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		lua_Integer pause;				// [P] 对象被暂停的时间(帧) 对象被暂停时，将跳过速度计算，但是timer会增加，frame仍会调用
	#endif // LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE

		// 小型属性
		// 小型属性 - 渲染
		uint32_t vertexcolor;			// [4] 顶点颜色
		BlendMode blendmode;			// [1] 混合模式
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
		uint8_t ignore_superpause : 1;	// [b] 是否无视超级暂停。 超级暂停时，timer不会增加，frame不会调用，但render会调用。
		uint8_t touch_lastx_lasty : 1;	// [b] 是否已经更新过 last_x 和 last_y 值，如果未更新过，表明对象刚生成，获取 dx 和 dy 时应当返回 0

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

		int GetAttr(lua_State* L) noexcept;
		int SetAttr(lua_State* L) noexcept;

		[[nodiscard]] bool isInRect(lua_Number const l, lua_Number const r, lua_Number const bb, lua_Number const t) const noexcept {
			return x >= l && x <= r && y >= bb && y <= t;
		}
		[[nodiscard]] bool isIntersect(GameObject const* other) const noexcept { return isIntersect(this, other); }

		[[nodiscard]] static bool isIntersect(GameObject const* p1, GameObject const* p2) noexcept;
	};

#pragma warning(pop)
}
