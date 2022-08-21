#include "GameObject/GameObject.hpp"
#include "GameResource/ResourceSprite.hpp"
#include "GameResource/ResourceAnimation.hpp"
#include "LuaBinding/lua_luastg_hash.hpp"
#include "LuaBinding/lua_utility.hpp"
#include "LuaBinding/LuaWrapperMisc.hpp"
#include "LuaBinding/LuaWrapper.hpp"
#include "XCollision.h"
#include "AppFrame.h"

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
	
	void GameObject::Reset()
	{
		pUpdatePrev = pUpdateNext = nullptr;
		pColliPrev = pColliNext = nullptr;

		status = GameObjectStatus::Free;
		id = (size_t)-1;
		uid = 0;
#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		luaclass.Reset();
#endif // USING_ADVANCE_GAMEOBJECT_CLASS

		x = y = 0.;
		lastx = lasty = 0.;
		dx = dy = 0.;
		rot = omega = 0.;
		vx = vy = 0.;
		ax = ay = 0.;
		layer = 0.;
		hscale = vscale = 1.;
#ifdef USER_SYSTEM_OPERATION
		maxv = DBL_MAX * 0.5; // 平时应该不会有人弄那么大的速度吧，希望计算时不会溢出（
		maxvx = maxvy = DBL_MAX;
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
		vertexcolor = 0xFFFFFFFF;
#endif // USING_ADVANCE_GAMEOBJECT_CLASS
	}
	void GameObject::DirtReset()
	{
		status = GameObjectStatus::Active;

		x = y = 0.;
		lastx = lasty = 0.;
		dx = dy = 0.;
		rot = omega = 0.;
		vx = vy = 0.;
		ax = ay = 0.;
		layer = 0.;
		hscale = vscale = 1.;
#ifdef USER_SYSTEM_OPERATION
		maxv = DBL_MAX * 0.5; // 平时应该不会有人弄那么大的速度吧，希望计算时不会溢出（
		maxvx = maxvy = DBL_MAX;
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
		vertexcolor = 0xFFFFFFFF;
#endif // USING_ADVANCE_GAMEOBJECT_CLASS
	}
	
	void GameObject::UpdateCollisionCircleRadius()
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
	
	bool GameObject::ChangeResource(std::string_view const& res_name)
	{
		fcyRefPointer<ResSprite> tSprite = LRES.FindSprite(res_name.data());
		if (tSprite)
		{
			res = *tSprite;
			res->AddRef();
#ifdef GLOBAL_SCALE_COLLI_SHAPE
			a = tSprite->GetHalfSizeX() * LRES.GetGlobalImageScaleFactor();
			b = tSprite->GetHalfSizeY() * LRES.GetGlobalImageScaleFactor();
#else
			a = tSprite->GetHalfSizeX();
			b = tSprite->GetHalfSizeY();
#endif // GLOBAL_SCALE_COLLI_SHAPE
			rect = tSprite->IsRectangle();
			UpdateCollisionCircleRadius();
			return true;
		}

		fcyRefPointer<ResAnimation> tAnimation = LRES.FindAnimation(res_name.data());
		if (tAnimation)
		{
			res = *tAnimation;
			res->AddRef();
#ifdef GLOBAL_SCALE_COLLI_SHAPE
			a = tAnimation->GetHalfSizeX() * LRES.GetGlobalImageScaleFactor();
			b = tAnimation->GetHalfSizeY() * LRES.GetGlobalImageScaleFactor();
#else
			a = tAnimation->GetHalfSizeX();
			b = tAnimation->GetHalfSizeY();
#endif // GLOBAL_SCALE_COLLI_SHAPE
			rect = tAnimation->IsRectangle();
			UpdateCollisionCircleRadius();
			return true;
		}

		fcyRefPointer<ResParticle> tParticle = LRES.FindParticle(res_name.data());
		if (tParticle)
		{
			// 分配粒子池
			ps = tParticle->AllocInstance();
			if (!ps)
			{
				res = nullptr;
				spdlog::error("[luastg] ResParticle: 无法分配粒子池，内存不足");
				return false;
			}
			ps->SetActive(false);
			ps->SetCenter(Core::Vector2F((float)x, (float)y));
			ps->SetRotation((float)rot);
			ps->SetActive(true);
			// 设置资源
			res = *tParticle;
			res->AddRef();
#ifdef GLOBAL_SCALE_COLLI_SHAPE
			a = tParticle->GetHalfSizeX() * LRES.GetGlobalImageScaleFactor();
			b = tParticle->GetHalfSizeY() * LRES.GetGlobalImageScaleFactor();
#else
			a = tParticle->GetHalfSizeX();
			b = tParticle->GetHalfSizeY();
#endif // GLOBAL_SCALE_COLLI_SHAPE
			rect = tParticle->IsRectangle();
			UpdateCollisionCircleRadius();
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
	void GameObject::ChangeLuaRC(lua_State* L, int idx)
	{
		if (luaclass.IsRenderClass && res && ps)
		{
			auto p = LuaWrapper::ParticleSystemWrapper::Create(L);
			p->res = dynamic_cast<ResParticle*>(res); res->AddRef();
			p->ptr = ps;
			lua_rawseti(L, idx, 4);
		}
	}
	void GameObject::ReleaseLuaRC(lua_State* L, int idx)
	{
		// release
		lua_rawgeti(L, idx, 4);
		if (lua_isuserdata(L, -1))
		{
			if (auto p = LuaWrapper::ParticleSystemWrapper::Cast(L, -1))
			{
				if (p->res) p->res->Release();
				p->ptr = nullptr; // 不要释放 ps，因为已经在 ReleaseResource 做过了
			}
		}
		lua_pop(L, 1);
		// set nil
		lua_pushnil(L);
		lua_rawseti(L, idx, 4);
	}
	
	void GameObject::Update()
	{
		if (pause > 0)
		{
			pause -= 1;
		}
		else
		{
			if (resolve_move)
			{
				vx = x - lastx;
				vy = y - lasty;
			}
			else
			{
				// 更新速度
				vx += ax;
				vy += ay;
			#ifdef USER_SYSTEM_OPERATION
				// 单独应用重力加速度
				vy -= ag;
				// 速度限制，来自lua层
				if (maxv <= DBL_MIN)
				{
					vx = 0.0;
					vy = 0.0;
				}
				else
				{
					lua_Number const speed_ = std::sqrt(vx * vx + vy * vy);
					if (maxv < speed_ && speed_ > DBL_MIN)
					{
						lua_Number const scale_ = maxv / speed_;
						vx = scale_ * vx;
						vy = scale_ * vy;
					}
				}
				//针对x、y方向单独限制
				vx = std::clamp(vx, -maxvx, maxvx);
				vy = std::clamp(vy, -maxvy, maxvy);
			#endif
				x += vx;
				y += vy;
			}
			
			rot += omega;

			// 更新粒子系统（若有）
			if (res && res->GetType() == ResourceType::Particle)
			{
				ps->SetRotation((float)rot);
				if (ps->IsActived()) // 兼容性处理
				{
					ps->SetActive(false);
					ps->SetCenter(Core::Vector2F((float)x, (float)y));
					ps->SetActive(true);
				}
				else
				{
					ps->SetCenter(Core::Vector2F((float)x, (float)y));
				}
				ps->Update(1.0f / 60.f);
			}
		}
	}
	void GameObject::UpdateLast()
	{
		dx = x - lastx;
		dy = y - lasty;
		lastx = x;
		lasty = y;
		if (navi && (std::abs(dx) > DBL_MIN || std::abs(dy) > DBL_MIN))
		{
			rot = std::atan2(dy, dx);
		}
	}
	void GameObject::UpdateTimer()
	{
		timer += 1;
		ani_timer += 1;
	}

	void GameObject::Render()
	{
		if (res)
		{
			float const gscale = LRES.GetGlobalImageScaleFactor();
		#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
			if (!luaclass.IsRenderClass)
			{
		#endif // USING_ADVANCE_GAMEOBJECT_CLASS
				switch (res->GetType())
				{
				case ResourceType::Sprite:
					LAPP.Render(
						static_cast<ResSprite*>(res),
						static_cast<float>(x),
						static_cast<float>(y),
						static_cast<float>(rot),
						static_cast<float>(hscale) * gscale,
						static_cast<float>(vscale) * gscale
					);
					break;
				case ResourceType::Animation:
					LAPP.Render(
						static_cast<ResAnimation*>(res),
						static_cast<int>(ani_timer),
						static_cast<float>(x),
						static_cast<float>(y),
						static_cast<float>(rot),
						static_cast<float>(hscale) * gscale,
						static_cast<float>(vscale) * gscale
					);
					break;
				case ResourceType::Particle:
					if (ps)
					{
						LAPP.Render(
							ps,
							static_cast<float>(hscale) * gscale,
							static_cast<float>(vscale) * gscale
						);
					}
					break;
				}
		#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
			}
			else
			{
				switch (res->GetType())
				{
				case ResourceType::Sprite:
					do {
						ResSprite* img = static_cast<ResSprite*>(res);
						// backup
						Core::Color4B color[4] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
						img->GetSprite()->getColor(color);
						BlendMode blend = img->GetBlendMode();
						// setup
						img->GetSprite()->setColor(Core::Color4B(vertexcolor));
						img->SetBlendMode(blendmode);
						LAPP.Render(
							img,
							static_cast<float>(x),
							static_cast<float>(y),
							static_cast<float>(rot),
							static_cast<float>(hscale) * gscale,
							static_cast<float>(vscale) * gscale
						);
						// restore
						img->GetSprite()->setColor(color);
						img->SetBlendMode(blend);
					} while (false);
					break;
				case ResourceType::Animation:
					do {
						ResAnimation* ani = static_cast<ResAnimation*>(res);
						uint32_t const idx = ani->GetSpriteIndexByTimer(ani_timer);
						// backup
						Core::Color4B color[4] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
						ani->GetSprite(idx)->getColor(color);
						BlendMode blend = ani->GetBlendMode();
						// setup
						ani->GetSprite(idx)->setColor(Core::Color4B(vertexcolor));
						ani->SetBlendMode(blend);
						LAPP.Render(
							ani,
							static_cast<int>(ani_timer),
							static_cast<float>(x),
							static_cast<float>(y),
							static_cast<float>(rot),
							static_cast<float>(hscale) * gscale,
							static_cast<float>(vscale) * gscale
						);
						// restore
						ani->GetSprite(idx)->setColor(color);
						ani->SetBlendMode(blend);
					} while (false);
					break;
				case ResourceType::Particle:
					if (ps)
					{
						ps->SetBlendMode(blendmode);
						ps->SetVertexColor(vertexcolor);
						LAPP.Render(
							ps,
							static_cast<float>(hscale) * gscale,
							static_cast<float>(vscale) * gscale
						);
					}
					break;
				}
			}
		#endif // USING_ADVANCE_GAMEOBJECT_CLASS
		}
	}
	
	int GameObject::GetAttr(lua_State* L) noexcept
	{
		// self k
		std::string_view const key = luaL_check_string_view(L, 2);
		switch (LuaSTG::MapGameObjectMember(key.data()))
		{
			// 基本信息

		case LuaSTG::GameObjectMember::STATUS:
			switch (status)
			{
			default:
				return luaL_error(L, "unknown lstg object status.");
			case GameObjectStatus::Active:
				lua_push_string_view(L, "normal");
				break;
			case GameObjectStatus::Dead:
				lua_push_string_view(L, "del");
				break;
			case GameObjectStatus::Killed:
				lua_push_string_view(L, "kill");
				break;
			}
			return 1;
		case LuaSTG::GameObjectMember::CLASS:
			lua_rawgeti(L, 1, 1);
			return 1;

			// 分组

		case LuaSTG::GameObjectMember::WORLD:
			lua_pushinteger(L, world);
			return 1;

			// 位置

		case LuaSTG::GameObjectMember::X:
			lua_pushnumber(L, x);
			return 1;
		case LuaSTG::GameObjectMember::Y:
			lua_pushnumber(L, y);
			return 1;
		case LuaSTG::GameObjectMember::DX:
			lua_pushnumber(L, dx);
			return 1;
		case LuaSTG::GameObjectMember::DY:
			lua_pushnumber(L, dy);
			return 1;

			// 运动学

		case LuaSTG::GameObjectMember::VX:
			lua_pushnumber(L, vx);
			return 1;
		case LuaSTG::GameObjectMember::VY:
			lua_pushnumber(L, vy);
			return 1;
		case LuaSTG::GameObjectMember::AX:
			lua_pushnumber(L, ax);
			return 1;
		case LuaSTG::GameObjectMember::AY:
			lua_pushnumber(L, ay);
			return 1;
		#ifdef USER_SYSTEM_OPERATION
		case LuaSTG::GameObjectMember::MAXVX:
			lua_pushnumber(L, maxvx);
			return 1;
		case LuaSTG::GameObjectMember::MAXVY:
			lua_pushnumber(L, maxvy);
			return 1;
		case LuaSTG::GameObjectMember::MAXV:
			lua_pushnumber(L, maxv);
			return 1;
		case LuaSTG::GameObjectMember::AG:
			lua_pushnumber(L, ag);
			return 1;
		#endif
		case LuaSTG::GameObjectMember::VSPEED:
			lua_pushnumber(L, std::sqrt(vx * vx + vy * vy));
			return 1;
		case LuaSTG::GameObjectMember::VANGLE:
			if (std::abs(vx) > DBL_MIN || std::abs(vy) > DBL_MIN)
				lua_pushnumber(L, std::atan2(vy, vx) * L_RAD_TO_DEG);
			else
				lua_pushnumber(L, rot * L_RAD_TO_DEG);
			return 1;

			// 碰撞体

		case LuaSTG::GameObjectMember::GROUP:
			lua_pushinteger(L, group);
			return 1;
		case LuaSTG::GameObjectMember::BOUND:
			lua_pushboolean(L, bound);
			return 1;
		case LuaSTG::GameObjectMember::COLLI:
			lua_pushboolean(L, colli);
			return 1;
		case LuaSTG::GameObjectMember::RECT:
			lua_pushboolean(L, rect);
			return 1;
		case LuaSTG::GameObjectMember::A:
		#ifdef GLOBAL_SCALE_COLLI_SHAPE
			lua_pushnumber(L, a / LRES.GetGlobalImageScaleFactor());
		#else
			lua_pushnumber(L, a);
		#endif // GLOBAL_SCALE_COLLI_SHAPE
			return 1;
		case LuaSTG::GameObjectMember::B:
		#ifdef GLOBAL_SCALE_COLLI_SHAPE
			lua_pushnumber(L, b / LRES.GetGlobalImageScaleFactor());
		#else
			lua_pushnumber(L, b);
		#endif // GLOBAL_SCALE_COLLI_SHAPE
			return 1;

			// 渲染

		case LuaSTG::GameObjectMember::LAYER:
			lua_pushnumber(L, layer);
			return 1;
		case LuaSTG::GameObjectMember::HSCALE:
			lua_pushnumber(L, hscale);
			return 1;
		case LuaSTG::GameObjectMember::VSCALE:
			lua_pushnumber(L, vscale);
			return 1;
		case LuaSTG::GameObjectMember::ROT:
			lua_pushnumber(L, rot * L_RAD_TO_DEG);
			return 1;
		case LuaSTG::GameObjectMember::OMEGA:
			lua_pushnumber(L, omega * L_RAD_TO_DEG);
			return 1;
		#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		case LuaSTG::GameObjectMember::_BLEND:
			if (luaclass.IsRenderClass)
				TranslateBlendModeToString(L, blendmode);
			else
				lua_pushnil(L);
			return 1;
		case LuaSTG::GameObjectMember::_COLOR:
			if (luaclass.IsRenderClass)
				LuaWrapper::ColorWrapper::CreateAndPush(L, Core::Color4B(vertexcolor));
			else
				lua_pushnil(L);
			return 1;
		case LuaSTG::GameObjectMember::_A:
			if (luaclass.IsRenderClass)
				lua_pushinteger(L, (lua_Integer)((uint8_t*)&vertexcolor)[3]);
			else
				lua_pushnil(L);
			return 1;
		case LuaSTG::GameObjectMember::_R:
			if (luaclass.IsRenderClass)
				lua_pushinteger(L, (lua_Integer)((uint8_t*)&vertexcolor)[2]);
			else
				lua_pushnil(L);
			return 1;
		case LuaSTG::GameObjectMember::_G:
			if (luaclass.IsRenderClass)
				lua_pushinteger(L, (lua_Integer)((uint8_t*)&vertexcolor)[1]);
			else
				lua_pushnil(L);
			return 1;
		case LuaSTG::GameObjectMember::_B:
			if (luaclass.IsRenderClass)
				lua_pushinteger(L, (lua_Integer)((uint8_t*)&vertexcolor)[0]);
			else
				lua_pushnil(L);
			return 1;
		#endif // USING_ADVANCE_GAMEOBJECT_CLASS
		case LuaSTG::GameObjectMember::ANI:
			lua_pushinteger(L, ani_timer);
			return 1;
		case LuaSTG::GameObjectMember::HIDE:
			lua_pushboolean(L, hide);
			return 1;
		case LuaSTG::GameObjectMember::NAVI:
			lua_pushboolean(L, navi);
			return 1;
		case LuaSTG::GameObjectMember::IMG:
			if (res)
				lua_push_string_view(L, res->GetResName());
			else
				lua_pushnil(L);
			return 1;
		#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		case LuaSTG::GameObjectMember::RES_RC:
			if (luaclass.IsRenderClass)
				lua_rawgeti(L, 1, 4);
			else
				lua_pushnil(L);
			return 1;
		#endif // USING_ADVANCE_GAMEOBJECT_CLASS

			// 更新控制

		case LuaSTG::GameObjectMember::TIMER:
			lua_pushinteger(L, timer);
			return 1;
		case LuaSTG::GameObjectMember::PAUSE:
			lua_pushinteger(L, pause);
			return 1;
		case LuaSTG::GameObjectMember::RESOLVEMOVE:
			lua_pushboolean(L, resolve_move);
			return 1;
		case LuaSTG::GameObjectMember::IGNORESUPERPAUSE:
			lua_pushboolean(L, ignore_superpause);
			return 1;
		
		default:
			lua_pushnil(L);
			return 1;
		}
	}
	int GameObject::SetAttr(lua_State* L) noexcept
	{
		// self k v
		std::string_view const key = luaL_check_string_view(L, 2);
		switch (LuaSTG::MapGameObjectMember(key.data()))
		{
			// 基本信息

		case LuaSTG::GameObjectMember::STATUS:
			do {
				std::string_view const value = luaL_check_string_view(L, 3);
				if (value == "normal")
					status = GameObjectStatus::Active;
				else if (value == "del")
					status = GameObjectStatus::Dead;
				else if (value == "kill")
					status = GameObjectStatus::Killed;
				else
					return luaL_error(L, "invalid argument for property 'status', must be 'normal', 'del' or 'kill'");
			} while (false);
			return 0;
		case LuaSTG::GameObjectMember::CLASS:
			do {
			#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
				if (!GameObjectClass::CheckClassValid(L, 3))
					return luaL_error(L, "invalid argument for property 'class', required luastg object class.");
				luaclass.CheckClassClass(L, 3); // 刷新对象的class
				if (!luaclass.IsRenderClass) ReleaseLuaRC(L, 1); // 你怎么回事，还给变回去了，那就释放资源
			#endif // USING_ADVANCE_GAMEOBJECT_CLASS
				lua_rawseti(L, 1, 1);
			} while (false);
			return 0;
			
			// 分组

		case LuaSTG::GameObjectMember::WORLD:
			world = luaL_checkinteger(L, 3);
			return 0;

			// 位置

		case LuaSTG::GameObjectMember::X:
			x = luaL_checknumber(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::Y:
			y = luaL_checknumber(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::DX:
			return luaL_error(L, "property 'dx' is readonly.");
		case LuaSTG::GameObjectMember::DY:
			return luaL_error(L, "property 'dy' is readonly.");

			// 运动学

		case LuaSTG::GameObjectMember::VX:
			vx = luaL_checknumber(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::VY:
			vy = luaL_checknumber(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::AX:
			ax = luaL_checknumber(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::AY:
			ay = luaL_checknumber(L, 3);
			return 0;
		#ifdef USER_SYSTEM_OPERATION
		case LuaSTG::GameObjectMember::MAXVX:
			maxvx = std::abs(luaL_checknumber(L, 3));
			return 0;
		case LuaSTG::GameObjectMember::MAXVY:
			maxvy = std::abs(luaL_checknumber(L, 3));
			return 0;
		case LuaSTG::GameObjectMember::MAXV:
			maxv = luaL_checknumber(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::AG:
			ag = luaL_checknumber(L, 3);
			return 0;
		#endif
		case LuaSTG::GameObjectMember::VSPEED:
			do {
				lua_Number const cur_speed_ = std::sqrt(vx * vx + vy * vy);
				lua_Number const new_speed_ = luaL_checknumber(L, 3);
				if (cur_speed_ <= DBL_MIN)
				{
					vx = std::cos(rot) * new_speed_;
					vy = std::sin(rot) * new_speed_;
				}
				else
				{
					lua_Number const a3 = new_speed_ / cur_speed_;
					vx *= a3;
					vy *= a3;
				}
			} while (false);
			return 0;
		case LuaSTG::GameObjectMember::VANGLE:
			do {
				lua_Number const cur_speed_ = std::sqrt(vx * vx + vy * vy);
				lua_Number const new_angle_ = luaL_checknumber(L, 3) * L_DEG_TO_RAD;
				if (cur_speed_ <= DBL_MIN)
				{
					rot = new_angle_;
				}
				else
				{
					vx = cur_speed_ * std::cos(new_angle_);
					vy = cur_speed_ * std::sin(new_angle_);
				}
			} while (false);
			return 0;

			// 碰撞体

		case LuaSTG::GameObjectMember::GROUP:
			do {
				lua_Integer const group_ = luaL_checkinteger(L, 3);
				if (group == group_)
					return 0;
				if (0 <= group_ && group_ < LOBJPOOL_GROUPN)
					group = group_;
				else
					return luaL_error(L, "invalid argument for property 'group', required 0 <= group <= %d.", LOBJPOOL_GROUPN - 1);
			} while (false);
			return 1;
		case LuaSTG::GameObjectMember::BOUND:
			bound = lua_to_uint8_boolean(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::COLLI:
			colli = lua_to_uint8_boolean(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::RECT:
			rect = lua_to_uint8_boolean(L, 3);
			UpdateCollisionCircleRadius();
			return 0;
		case LuaSTG::GameObjectMember::A:
		#ifdef GLOBAL_SCALE_COLLI_SHAPE
			a = luaL_checknumber(L, 3) * LRES.GetGlobalImageScaleFactor();
		#else
			a = luaL_checknumber(L, 3);
		#endif // GLOBAL_SCALE_COLLI_SHAPE
			UpdateCollisionCircleRadius();
			return 0;
		case LuaSTG::GameObjectMember::B:
		#ifdef GLOBAL_SCALE_COLLI_SHAPE
			b = luaL_checknumber(L, 3) * LRES.GetGlobalImageScaleFactor();
		#else
			b = luaL_checknumber(L, 3);
		#endif // GLOBAL_SCALE_COLLI_SHAPE
			UpdateCollisionCircleRadius();
			return 0;

			// 渲染

		case LuaSTG::GameObjectMember::LAYER:
			do
			{
				lua_Number const layer_ = luaL_checknumber(L, 3);
				if (layer == layer_)
					return 0;
				nextlayer = layer_;
			} while (false);
			return 2;
		case LuaSTG::GameObjectMember::HSCALE:
			hscale = luaL_checknumber(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::VSCALE:
			vscale = luaL_checknumber(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::ROT:
			rot = luaL_checknumber(L, 3) * L_DEG_TO_RAD;
			return 0;
		case LuaSTG::GameObjectMember::OMEGA:
			omega = luaL_checknumber(L, 3) * L_DEG_TO_RAD;
			return 0;
		#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		case LuaSTG::GameObjectMember::_BLEND:
			if (luaclass.IsRenderClass)
				blendmode = TranslateBlendMode(L, 3);
			else
				lua_rawset(L, 1);
			return 0;
		case LuaSTG::GameObjectMember::_COLOR:
			if (luaclass.IsRenderClass)
				vertexcolor = LuaWrapper::ColorWrapper::Cast(L, 3)->color();
			else
				lua_rawset(L, 1);
			return 0;
		case LuaSTG::GameObjectMember::_A:
			if (luaclass.IsRenderClass)
				((uint8_t*)&vertexcolor)[3] = (uint8_t)luaL_checkinteger(L, 3);
			else
				lua_rawset(L, 1);
			return 0;
		case LuaSTG::GameObjectMember::_R:
			if (luaclass.IsRenderClass)
				((uint8_t*)&vertexcolor)[2] = (uint8_t)luaL_checkinteger(L, 3);
			else
				lua_rawset(L, 1);
			return 0;
		case LuaSTG::GameObjectMember::_G:
			if (luaclass.IsRenderClass)
				((uint8_t*)&vertexcolor)[1] = (uint8_t)luaL_checkinteger(L, 3);
			else
				lua_rawset(L, 1);
			return 0;
		case LuaSTG::GameObjectMember::_B:
			if (luaclass.IsRenderClass)
				((uint8_t*)&vertexcolor)[0] = (uint8_t)luaL_checkinteger(L, 3);
			else
				lua_rawset(L, 1);
			return 0;
		#endif // USING_ADVANCE_GAMEOBJECT_CLASS
		case LuaSTG::GameObjectMember::ANI:
			return luaL_error(L, "property 'ani' is readonly.");
		case LuaSTG::GameObjectMember::HIDE:
			hide = lua_to_uint8_boolean(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::NAVI:
			navi = lua_to_uint8_boolean(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::IMG:
			do {
				if (lua_isstring(L, 3))
				{
					std::string_view const value = luaL_check_string_view(L, 3);
					if (!res || value != res->GetResName())
					{
						ReleaseLuaRC(L, 1); // TODO: 默认 table 是第一个？
						ReleaseResource();
						if (!ChangeResource(value))
							return luaL_error(L, "can't find resource '%s' in image/animation/particle pool.", value.data());
						ChangeLuaRC(L, 1); // TODO: 默认 table 是第一个？
					}
				}
				else
				{
					ReleaseLuaRC(L, 1); // TODO: 默认 table 是第一个？
					ReleaseResource();
				}
			} while (false);
			return 0;
		#ifdef USING_ADVANCE_GAMEOBJECT_CLASS
		case LuaSTG::GameObjectMember::RES_RC:
			if (luaclass.IsRenderClass)
				return luaL_error(L, "property 'rc' is readonly.");
			else
				lua_rawset(L, 1);
			return 0;
		#endif // USING_ADVANCE_GAMEOBJECT_CLASS

			// 更新控制

		case LuaSTG::GameObjectMember::TIMER:
			timer = luaL_checkinteger(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::PAUSE:
			pause = luaL_checkinteger(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::RESOLVEMOVE:
			resolve_move = lua_to_uint8_boolean(L, 3);
			return 0;
		case LuaSTG::GameObjectMember::IGNORESUPERPAUSE:
			ignore_superpause = lua_to_uint8_boolean(L, 3);
			return 0;
		
			// 默认处理

		default:
			lua_rawset(L, 1);
			return 0;
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
