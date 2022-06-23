#pragma once
#include "GameResource/ResourceBase.hpp"
#include "GameResource/ResourceParticle.hpp"
#include "GameObject/GameObjectClass.hpp"
#include "lua.hpp"

namespace LuaSTGPlus
{
	// 游戏对象状态
	enum class GameObjectStatus : uint32_t
	{
		Free   = 0, // 空闲可用状态
		Active = 1, // 正常活跃状态
		Dead   = 2, // 生命周期结束
		Killed = 4, // 生命周期结束
	};
	
#pragma warning(push)
#pragma warning(disable:26495)

	// 游戏对象
	struct GameObject
	{

		// 链表部分
		GameObject* pUpdatePrev;		// [P] [不可见]
		GameObject* pUpdateNext;		// [P] [不可见]
		GameObject* pColliPrev;			// [P] [不可见]
		GameObject* pColliNext;			// [P] [不可见]

		// 基本信息

		GameObjectStatus status;		// [4] 对象状态
	#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		GameObjectClass luaclass;		// [4] [不可见] 对象类的一些特性
	#endif // USING_ADVANCE_GAMEOBJECT_CLASS
		uint64_t uid;					// [8] [不可见] 对象全局唯一标识符
		size_t id;						// [P] [不可见] 对象在对象池中的索引

		// 分组

		lua_Integer world;				// [P] 世界标记位，用于对一个对象进行分组，影响更新、渲染、碰撞检测等

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
		uint8_t bound;					// [1] 是否离开边界自动回收
		uint8_t colli;					// [1] 是否参与碰撞
		uint8_t rect;					// [1] 是否为矩形碰撞盒
		lua_Number a;					// [8] 矩形模式下，为横向宽度一半；非矩形模式下，为圆半径或椭圆横向宽度一半
		lua_Number b;					// [8] 矩形模式下，为纵向宽度一半；非矩形模式下，为圆半径或椭圆纵向宽度一半
		lua_Number col_r;				// [8] [不可见] 碰撞体外接圆半径

		// 渲染

		lua_Number layer;				// [8] 图层
		lua_Number nextlayer;			// [8] [不可见] 对象要切换到的图层
		lua_Number hscale;				// [8] 横向渲染缩放
		lua_Number vscale;				// [8] 纵向渲染缩放
		lua_Number rot;					// [8] 平面渲染旋转角
		lua_Number omega;				// [8] 平面渲染旋转角加速度
	#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		BlendMode blendmode;			// [4] 混合模式
		uint32_t vertexcolor;			// [4] 顶点颜色
	#endif // USING_ADVANCE_GAMEOBJECT_CLASS
		lua_Integer ani_timer;			// [P] [只读] 动画自增计数器
		uint8_t hide;					// [1] 不渲染
		uint8_t navi;					// [1] 根据坐标增量自动设置渲染旋转角
		Resource* res;					// [P] 渲染资源
		ResParticle::ParticlePool* ps;	// [P] 粒子系统

		// 更新控制

		lua_Integer timer;				// [P] 自增计数器
		lua_Integer pause;				// [P] 对象被暂停的时间(帧) 对象被暂停时，将跳过速度计算，但是timer会增加，frame仍会调用
		uint8_t resolve_move;			// [1] 是否为计算速度而非计算位置
		uint8_t ignore_superpause;		// [1] 是否无视超级暂停。 超级暂停时，timer不会增加，frame不会调用，但render会调用。
		
	

		// 成员方法

		void Reset();
		void DirtReset();
		void UpdateCollisionCirclrRadius();
		bool ChangeResource(std::string_view const& res_name);
		void ChangeLuaRC(lua_State* L, int idx);
		void ReleaseResource();
		void ReleaseLuaRC(lua_State* L, int idx);

		void Update();
		void UpdateLast();
		void UpdateTimer();
		void Render();

		int GetAttr(lua_State* L) noexcept;
		int SetAttr(lua_State* L) noexcept;
	};

#pragma warning(pop)
	
	// 对两个游戏对象进行碰撞检测
	bool CollisionCheck(GameObject* p1, GameObject* p2) noexcept;
}
