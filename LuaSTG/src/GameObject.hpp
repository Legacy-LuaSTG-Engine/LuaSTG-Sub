#pragma once
#include "Global.h"
#include "XCollision.h"
#include "ResourceBase.hpp"
#include "ResourceParticle.hpp"
#include "GameObjectClass.hpp"

namespace LuaSTGPlus
{
	// 游戏对象状态
	enum GAMEOBJECTSTATUS
	{
		STATUS_FREE = 0,//空闲状态、用于标识链表伪头部
		STATUS_DEFAULT = 1,//正常状态
		STATUS_KILL = 2,//被kill事件触发
		STATUS_DEL = 3,//被del事件触发
	};

	//游戏碰撞体类型
	enum class GameObjectColliderType {
		None = -1, //关闭

		Circle = 0,  //严格圆
		OBB = 1,  //矩形
		Ellipse = 2,  //椭圆
		Diamond = 3,  //菱形
		Triangle = 4,  //三角
		Point = 5,  //点

		BentLazer = 100,//曲线激光
	};

	//游戏碰撞体
	struct GameObjectCollider {
		GameObjectColliderType type;  //碰撞体类型
		float a;                      //椭圆半长轴、矩形半宽
		float b;                      //椭圆半短轴、矩形半高
		float rot;                    //相对旋转
		float dx;                     //相对偏移x
		float dy;                     //相对偏移y

		float circum_r;               //外接圆

		float absx;                   //计算后的绝对坐标x
		float absy;                   //计算后的绝对坐标y
		float absrot;                 //计算后的绝对旋转方向
		XColliderType xtype;          //转换后的碰撞体类型

		//重置数值
		void reset() {
			type = GameObjectColliderType::None;
			a = 0.0f; b = 0.0f; rot = 0.0f;
			dx = 0.0f; dy = 0.0f;
			circum_r = 0.0f;
			absx = 0.0f; absy = 0.0f; absrot = 0.0f;
			xtype = XColliderType::Ellipse;
		}
		//计算外接圆和对应的XMath库碰撞体类型
		void calcircum() {
			switch (type)
			{
			case GameObjectColliderType::Circle:
				circum_r = a > b ? a : b;
				xtype = XColliderType::Circle;
				break;
			case GameObjectColliderType::OBB:
				circum_r = std::sqrtf(std::powf(a, 2.0f) + std::powf(b, 2.0f));
				xtype = XColliderType::OBB;
				break;
			case GameObjectColliderType::Ellipse:
				circum_r = a > b ? a : b;
				xtype = XColliderType::Ellipse;
				break;
			case GameObjectColliderType::Diamond:
				circum_r = a > b ? a : b;
				xtype = XColliderType::Diamond;
				break;
			case GameObjectColliderType::Triangle:
				circum_r = std::sqrtf(std::powf(a, 2.0f) + std::powf(b, 2.0f));
				xtype = XColliderType::Triangle;
				break;
			case GameObjectColliderType::Point:
				circum_r = 0.0f;
				xtype = XColliderType::Point;
				break;
			}
		}
		//根据偏移计算绝对坐标和旋转
		void caloffset(float x, float y, float _rot) {
			float tCos = std::cosf(-_rot);
			float tSin = std::sinf(-_rot);
			absx = x + dx * tCos + dy * tSin;
			absy = y + dy * tCos - dx * tSin;
			absrot = _rot + rot;
		}
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

		inline void UpdateCollisionCirclrRadius() {
			if (rect) {
				//矩形
				col_r = ::sqrt(a * a + b * b);
			}
			else if (!rect && (a != b)) {
				//椭圆
				col_r = a > b ? a : b;
			}
			else {
				//严格的正圆
				col_r = (a + b) / 2;
			}
		}

		void ReleaseResource();

		bool ChangeResource(const char* res_name);
	};

	//对两个游戏对象进行碰撞检测
	bool CollisionCheck(GameObject* p1, GameObject* p2)LNOEXCEPT;
}
