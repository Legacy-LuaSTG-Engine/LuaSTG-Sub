#include "GameObject.hpp"
#include "AppFrame.h"
#include "ResourceSprite.hpp"
#include "ResourceAnimation.hpp"
#include "XCollision.h"

namespace LuaSTGPlus
{
	//【弃用】游戏碰撞体类型
	enum class GameObjectColliderType {
		None      = -1, // 关闭
		
		Circle    = 0,  // 严格圆
		OBB       = 1,  // 矩形
		Ellipse   = 2,  // 椭圆
		Diamond   = 3,  // 菱形
		Triangle  = 4,  // 三角
		Point     = 5,  // 点
		
		BentLazer = 100, //曲线激光
	};
	
	//【弃用】游戏碰撞体
	struct GameObjectCollider
	{
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
	
	void GameObject::Reset() {
		status = (GAMEOBJECTSTATUS)STATUS_FREE;
		id = (size_t)-1;
		uid = 0;
#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		luaclass.Reset();
#endif // USING_ADVANCE_GAMEOBJECT_CLASS

		x = y = 0.;
		lastx = lasty = 0.;
		dx = dy = 0.;
		rot = omiga = 0.;
		vx = vy = 0.;
		ax = ay = 0.;
		layer = 0.;
		hscale = vscale = 1.;
#ifdef USER_SYSTEM_OPERATION
		maxv = maxvx = maxvy = DBL_HALF_MAX; // 平时应该不会有人弄那么大的速度吧，希望计算时不会溢出（
		ag = 0.;
#endif

		colli = bound = true;
		hide = navi = false;

		group = 0;
		timer = ani_timer = 0;

		res = nullptr;
		ps = nullptr;

		resolve_move = false;
		pause = 0;
		ignore_superpause = false;

		world = 15;

		rect = false;
		a = b = 0.;
		col_r = 0.;

#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		blendmode = BlendMode::MulAlpha;
		vertexcolor.argb = 0xFFFFFFFF;
#endif // USING_ADVANCE_GAMEOBJECT_CLASS
	}
	
	void GameObject::DirtReset()
	{
		status = (GAMEOBJECTSTATUS)STATUS_DEFAULT;

		x = y = 0.;
		lastx = lasty = 0.;
		dx = dy = 0.;
		rot = omiga = 0.;
		vx = vy = 0.;
		ax = ay = 0.;
		layer = 0.;
		hscale = vscale = 1.;
#ifdef USER_SYSTEM_OPERATION
		maxv = maxvx = maxvy = DBL_HALF_MAX; // 平时应该不会有人弄那么大的速度吧，希望计算时不会溢出（
		ag = 0.;
#endif

		colli = bound = true;
		hide = navi = false;

		group = 0;
		timer = ani_timer = 0;

		ReleaseResource();

		resolve_move = false;
		pause = 0;
		ignore_superpause = false;

		world = 15;

		rect = false;
		a = b = 0.;
		col_r = 0.;

#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		blendmode = BlendMode::MulAlpha;
		vertexcolor.argb = 0xFFFFFFFF;
#endif // USING_ADVANCE_GAMEOBJECT_CLASS
	}
	
	void GameObject::UpdateCollisionCirclrRadius()
	{
		if (rect) {
			//矩形
			col_r = std::sqrt(a * a + b * b);
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
	
	bool GameObject::ChangeResource(const char* res_name)
	{
		fcyRefPointer<ResSprite> tSprite = LRES.FindSprite(res_name);
		if (tSprite)
		{
			res = tSprite;
			res->AddRef();
#ifdef GLOBAL_SCALE_COLLI_SHAPE
			a = tSprite->GetHalfSizeX() * LRES.GetGlobalImageScaleFactor();
			b = tSprite->GetHalfSizeY() * LRES.GetGlobalImageScaleFactor();
#else
			a = tSprite->GetHalfSizeX();
			b = tSprite->GetHalfSizeY();
#endif // GLOBAL_SCALE_COLLI_SHAPE
			rect = tSprite->IsRectangle();
			UpdateCollisionCirclrRadius();
			return true;
		}

		fcyRefPointer<ResAnimation> tAnimation = LRES.FindAnimation(res_name);
		if (tAnimation)
		{
			res = tAnimation;
			res->AddRef();
#ifdef GLOBAL_SCALE_COLLI_SHAPE
			a = tAnimation->GetHalfSizeX() * LRES.GetGlobalImageScaleFactor();
			b = tAnimation->GetHalfSizeY() * LRES.GetGlobalImageScaleFactor();
#else
			a = tAnimation->GetHalfSizeX();
			b = tAnimation->GetHalfSizeY();
#endif // GLOBAL_SCALE_COLLI_SHAPE
			rect = tAnimation->IsRectangle();
			UpdateCollisionCirclrRadius();
			return true;
		}

		fcyRefPointer<ResParticle> tParticle = LRES.FindParticle(res_name);
		if (tParticle)
		{
			res = tParticle;
			if (!(ps = tParticle->AllocInstance()))
			{
				res = nullptr;
				spdlog::error("[luastg] ResParticle: 无法构造粒子池，内存不足");
				return false;
			}
			ps->SetInactive();
			ps->SetCenter(fcyVec2((float)x, (float)y));
			ps->SetRotation((float)rot);
			ps->SetActive();

			res->AddRef();
#ifdef GLOBAL_SCALE_COLLI_SHAPE
			a = tParticle->GetHalfSizeX() * LRES.GetGlobalImageScaleFactor();
			b = tParticle->GetHalfSizeY() * LRES.GetGlobalImageScaleFactor();
#else
			a = tParticle->GetHalfSizeX();
			b = tParticle->GetHalfSizeY();
#endif // GLOBAL_SCALE_COLLI_SHAPE
			rect = tParticle->IsRectangle();
			UpdateCollisionCirclrRadius();
			return true;
		}

		return false;
	}
	
	void GameObject::ReleaseResource()
	{
		if (res)
		{
			if (res->GetType() == ResourceType::Particle)
			{
				assert(ps);
				static_cast<ResParticle*>(res)->FreeInstance(ps);
				ps = nullptr;
			}
			res->Release();
			res = nullptr;
		}
	}
	
	bool CollisionCheck(GameObject* p1, GameObject* p2) noexcept {
		//忽略不碰撞对象
		if (!p1->colli || !p2->colli)
			return false;//返回点0
		
		//快速AABB检测
		if ((p1->x - p1->col_r >= p2->x + p2->col_r) ||
			(p1->x + p1->col_r <= p2->x - p2->col_r) ||
			(p1->y - p1->col_r >= p2->y + p2->col_r) ||
			(p1->y + p1->col_r <= p2->y - p2->col_r))
		{
			return false;
		}
		
		float x1 = (float)p1->x;
		float x2 = (float)p2->x;
		float y1 = (float)p1->y;
		float y2 = (float)p2->y;
		float a1 = (float)p1->a;
		float a2 = (float)p2->a;
		float b1 = (float)p1->b;
		float b2 = (float)p2->b;
		float rot1 = (float)p1->rot;
		float rot2 = (float)p2->rot;
		float cr1 = (float)p1->col_r;
		float cr2 = (float)p2->col_r;
		
		using XVec2 = cocos2d::Vec2;
		
		//外接圆碰撞检测，没发生碰撞则直接PASS
		
		if (!xmath::collision::check(XVec2(x1, y1), cr1, cr1, rot1, XColliderType::Circle,
			XVec2(x2, y2), cr2, cr2, rot2, XColliderType::Circle)) {
			return false;
		}
		
		//精确碰撞检测
		if (!p1->rect && !p2->rect) {
			//椭圆、椭圆碰撞检测
			return xmath::collision::check(XVec2(x1, y1), a1, b1, rot1, XColliderType::Ellipse,
				XVec2(x2, y2), a2, b2, rot2, XColliderType::Ellipse);
		}
		else if (p1->rect && p2->rect) {
			//矩形、矩形碰撞检测
			return xmath::collision::check(XVec2(x1, y1), a1, b1, rot1, XColliderType::OBB,
				XVec2(x2, y2), a2, b2, rot2, XColliderType::OBB);
		}
		else
		{
			//矩形、椭圆碰撞检测
			if (p1->rect && (!p2->rect))
			{
				return xmath::collision::check(XVec2(x1, y1), a1, b1, rot1, XColliderType::OBB,
					XVec2(x2, y2), a2, b2, rot2, XColliderType::Ellipse);
			}
			else if ((!p1->rect) && p2->rect)
			{
				return xmath::collision::check(XVec2(x1, y1), a1, b1, rot1, XColliderType::Ellipse,
					XVec2(x2, y2), a2, b2, rot2, XColliderType::OBB);
			}
		}
		return false;
	}
}
