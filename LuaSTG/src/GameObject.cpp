#include "GameObject.hpp"
#include "AppFrame.h"
#include "ResourceSprite.hpp"
#include "ResourceAnimation.hpp"

namespace LuaSTGPlus {
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

		group = LGOBJ_DEFAULTGROUP;
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

		group = LGOBJ_DEFAULTGROUP;
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

	void GameObject::ReleaseResource()
	{
		if (res)
		{
			if (res->GetType() == ResourceType::Particle)
			{
				LASSERT(ps);
				static_cast<ResParticle*>(res)->FreeInstance(ps);
				ps = nullptr;
			}
			res->Release();
			res = nullptr;
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
				LERROR("无法构造粒子池，内存不足");
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

	bool CollisionCheck(GameObject* p1, GameObject* p2)LNOEXCEPT {
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

		//外接圆碰撞检测，没发生碰撞则直接PASS
		if (!xmath::collision::check(xmath::Vec2(x1, y1), cr1, cr1, rot1, XColliderType::Circle,
			xmath::Vec2(x2, y2), cr2, cr2, rot2, XColliderType::Circle)) {
			return false;
		}

		//精确碰撞检测
		if (!p1->rect && !p2->rect) {
			//椭圆、椭圆碰撞检测
			return xmath::collision::check(xmath::Vec2(x1, y1), a1, b1, rot1, XColliderType::Ellipse,
				xmath::Vec2(x2, y2), a2, b2, rot2, XColliderType::Ellipse);
		}
		else if (p1->rect && p2->rect) {
			//矩形、矩形碰撞检测
			return xmath::collision::check(xmath::Vec2(x1, y1), a1, b1, rot1, XColliderType::OBB,
				xmath::Vec2(x2, y2), a2, b2, rot2, XColliderType::OBB);
		}
		else
		{
			//矩形、椭圆碰撞检测
			if (p1->rect && (!p2->rect))
			{
				return xmath::collision::check(xmath::Vec2(x1, y1), a1, b1, rot1, XColliderType::OBB,
					xmath::Vec2(x2, y2), a2, b2, rot2, XColliderType::Ellipse);
			}
			else if ((!p1->rect) && p2->rect)
			{
				return xmath::collision::check(xmath::Vec2(x1, y1), a1, b1, rot1, XColliderType::Ellipse,
					xmath::Vec2(x2, y2), a2, b2, rot2, XColliderType::OBB);
			}
		}
		return false;
	}
}
