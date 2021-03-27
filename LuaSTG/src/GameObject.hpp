#pragma once
#include <cstdint>
#include "ResourceBase.hpp"
#include "ResourceParticle.hpp"
#include "lua.hpp"
#include "GameObjectClass.hpp"
#include "Config.h"

namespace LuaSTGPlus
{
	// 游戏对象状态
	enum GAMEOBJECTSTATUS
	{
		STATUS_FREE    = 0, //空闲状态、用于标识链表伪头部
		STATUS_DEFAULT = 1, //正常状态
		STATUS_KILL    = 2, //被kill事件触发
		STATUS_DEL     = 3, //被del事件触发
	};
	
	// 游戏对象
	struct GameObject
	{
		GAMEOBJECTSTATUS status;  // (不可见)对象状态
		size_t id;  // (不可见)对象在对象池中的id
		int64_t uid;  // (不可见)对象唯一id
#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		GameObjectClass luaclass;// lua class，不可见
#endif // USING_ADVANCE_GAMEOBJECT_CLASS

		lua_Number x, y;  // 中心坐标
		lua_Number lastx, lasty;  // (不可见)上一帧中心坐标
		lua_Number dx, dy;  // (只读)上一帧中心坐标相对中心坐标的偏移量
		lua_Number rot, omiga;  // 旋转角度与角度增量
		lua_Number vx, vy;  // 速度
#ifdef USER_SYSTEM_OPERATION
		lua_Number maxv, maxvx, maxvy; // 速度限制
		lua_Number ag;  // 重力加速度
#endif
		lua_Number ax, ay;  // 加速度
		//lua_Number va, speed; // 速度方向 速度值
		lua_Number layer;  // 图层
		lua_Number hscale, vscale;  // 横向、纵向拉伸率，仅影响渲染

		bool colli;  // 是否参与碰撞
		bool bound;  // 是否越界清除
		bool hide;  // 是否隐藏
		bool navi;  // 是否自动转向

		//EX+
		bool resolve_move; //是否为计算速度而非计算位置
		lua_Integer pause; //对象被暂停的时间(帧) 对象被暂停时，将跳过速度计算，但是timer会增加，frame仍会调用
		bool ignore_superpause; //是否无视超级暂停。 超级暂停时，timer不会增加，frame不会调用，但render会调用。
		lua_Integer world; //世界标记位

		lua_Integer group;  // 对象所在的碰撞组
		lua_Integer timer, ani_timer;  // 计数器

		Resource* res;  // 渲染资源
		ResParticle::ParticlePool* ps;  // 粒子系统

		bool rect; //是否为矩形碰撞盒
		lua_Number a, b; //单位的横向、纵向碰撞大小的一半
		lua_Number col_r; //受colli,a,b,rect参数影响的碰撞盒外圆半径

#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		BlendMode blendmode;
		struct {
			union {
				uint32_t argb;
				struct
				{
					uint8_t b;
					uint8_t g;
					uint8_t r;
					uint8_t a;
				};
			};
		} vertexcolor;
#endif // USING_ADVANCE_GAMEOBJECT_CLASS

		void Reset();
		void DirtReset();
		void UpdateCollisionCirclrRadius();
		bool ChangeResource(const char* res_name);
		void ReleaseResource();
	};
	
	//对两个游戏对象进行碰撞检测
	bool CollisionCheck(GameObject* p1, GameObject* p2) noexcept;
}
