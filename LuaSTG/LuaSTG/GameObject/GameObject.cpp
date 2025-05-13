#include "GameObject/GameObject.hpp"
#include "GameResource/ResourceSprite.hpp"
#include "GameResource/ResourceAnimation.hpp"
#include "lua/plus.hpp"
#include "AppFrame.h"

using std::string_view_literals::operator ""sv;

namespace luastg {
	void GameObjectFeatures::read(lua_State* const vm, int const index) {
		lua::stack_t const ctx(vm);
		reset();
		if (!ctx.is_table(index)) {
			return;
		}
		is_class = ctx.get_map_value<bool>(index, "is_class"sv, false);
		if (!is_class) {
			return;
		}
		is_render_class = ctx.get_map_value<bool>(index, ".render"sv, false);
		auto const default_function_mask = ctx.get_map_value<int32_t>(index, "default_function"sv, 0);
	#define TEST_CALLBACK(CALLBACK) (default_function_mask & (1 << (CALLBACK))) ? false : true
		has_callback_create = TEST_CALLBACK(LGOBJ_CC_INIT);
		has_callback_destroy = TEST_CALLBACK(LGOBJ_CC_DEL);
		has_callback_update = TEST_CALLBACK(LGOBJ_CC_FRAME);
		has_callback_render = TEST_CALLBACK(LGOBJ_CC_RENDER);
		has_callback_trigger = TEST_CALLBACK(LGOBJ_CC_COLLI);
		has_callback_legacy_kill = TEST_CALLBACK(LGOBJ_CC_KILL);
	#undef TEST_CALLBACK
	}
}

namespace luastg
{
	void GameObject::Reset()
	{
		update_list_previous = update_list_next = nullptr;
		detect_list_previous = detect_list_next = nullptr;

		status = GameObjectStatus::Free;
		id = max_id;
		unique_id = 0;
		features.reset();

		x = y = 0.0;
		last_x = last_y = 0.0;
		dx = dy = 0.0;
		rot = omega = 0.0;
		vx = vy = 0.0;
		ax = ay = 0.0;
		layer = 0.0;
		hscale = vscale = 1.0;
#ifdef USER_SYSTEM_OPERATION
		max_v = DBL_MAX * 0.5; // 平时应该不会有人弄那么大的速度吧，希望计算时不会溢出（
		max_vx = max_vy = DBL_MAX;
		ag = 0.0;
#endif

		colli = bound = true;
		hide = navi = false;

		group = 0;
		timer = ani_timer = 0;

		res = nullptr;
		ps = nullptr;

	#ifdef LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		resolve_move = false;
		pause = 0;
	#endif // LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		ignore_super_pause = false;
		last_xy_touched = false;

#ifdef USING_MULTI_GAME_WORLD
		world = 15;
#endif // USING_MULTI_GAME_WORLD

		rect = false;
		a = b = 0.0;
		col_r = 0.0;

		blend_mode = BlendMode::MulAlpha;
		vertex_color = core::Color4B::white();
	}
	void GameObject::DirtReset()
	{
		status = GameObjectStatus::Active;

		x = y = 0.;
		last_x = last_y = 0.;
		dx = dy = 0.;
		rot = omega = 0.;
		vx = vy = 0.;
		ax = ay = 0.;
		layer = 0.;
		hscale = vscale = 1.;
#ifdef USER_SYSTEM_OPERATION
		max_v = DBL_MAX * 0.5; // 平时应该不会有人弄那么大的速度吧，希望计算时不会溢出（
		max_vx = max_vy = DBL_MAX;
		ag = 0.;
#endif

		colli = bound = true;
		hide = navi = false;

		group = 0;
		timer = ani_timer = 0;

		ReleaseResource();

	#ifdef LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		resolve_move = false;
		pause = 0;
	#endif // LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		ignore_super_pause = false;
		last_xy_touched = false;

#ifdef USING_MULTI_GAME_WORLD
		world = 15;
#endif // USING_MULTI_GAME_WORLD

		rect = false;
		a = b = 0.;
		col_r = 0.;

		blend_mode = BlendMode::MulAlpha;
		vertex_color = core::Color4B::white();
	}
	
	void GameObject::UpdateCollisionCircleRadius() {
		if (rect) {
			//矩形
			col_r = std::hypot(a, b);
		}
		else if (a != b) {
			//椭圆
			col_r = a > b ? a : b;
		}
		else {
			//严格的正圆
			col_r = a;
		}
	}
	
	bool GameObject::ChangeResource(std::string_view const& res_name)
	{
		core::SmartReference<IResourceSprite> tSprite = LRES.FindSprite(res_name.data());
		if (tSprite)
		{
			res = *tSprite;
			res->retain();
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

		core::SmartReference<IResourceAnimation> tAnimation = LRES.FindAnimation(res_name.data());
		if (tAnimation)
		{
			res = *tAnimation;
			res->retain();
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

		core::SmartReference<IResourceParticle> tParticle = LRES.FindParticle(res_name.data());
		if (tParticle)
		{
			// 分配粒子池
			if (!tParticle->CreateInstance(&ps))
			{
				res = nullptr;
				spdlog::error("[luastg] ResParticle: 无法分配粒子池，内存不足");
				return false;
			}
			ps->SetActive(false);
			ps->SetCenter(core::Vector2F((float)x, (float)y));
			ps->SetRotation((float)rot);
			ps->SetActive(true);
			// 设置资源
			res = *tParticle;
			res->retain();
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
				static_cast<IResourceParticle*>(res)->DestroyInstance(ps);
				ps = nullptr;
			}
			res->release();
			res = nullptr;
		}
	}

	void GameObject::Update()
	{
	#ifdef LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		if (pause > 0)
		{
			pause -= 1;
		}
		else
		{
			if (resolve_move)
			{
				if (last_xy_touched)
				{
					vx = x - last_x;
					vy = y - last_y;
				}
				else
				{
					vx = 0.0;
					vy = 0.0;
				}
			}
			else
	#endif // LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
			{
				// 更新速度
				vx += ax;
				vy += ay;
			#ifdef USER_SYSTEM_OPERATION
				// 单独应用重力加速度
				vy -= ag;
				// 速度限制，来自lua层
				if (max_v <= DBL_MIN)
				{
					vx = 0.0;
					vy = 0.0;
				}
				else
				{
					lua_Number const speed_ = std::sqrt(vx * vx + vy * vy);
					if (max_v < speed_ && speed_ > DBL_MIN)
					{
						lua_Number const scale_ = max_v / speed_;
						vx = scale_ * vx;
						vy = scale_ * vy;
					}
				}
				//针对x、y方向单独限制
				vx = std::clamp(vx, -max_vx, max_vx);
				vy = std::clamp(vy, -max_vy, max_vy);
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
					ps->SetCenter(core::Vector2F((float)x, (float)y));
					ps->SetActive(true);
				}
				else
				{
					ps->SetCenter(core::Vector2F((float)x, (float)y));
				}
				ps->Update(1.0f / 60.f);
			}
	#ifdef LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		}
	#endif // LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
	}
	void GameObject::UpdateLast()
	{
		if (last_xy_touched)
		{
			dx = x - last_x;
			dy = y - last_y;
		}
		else
		{
			dx = 0.0;
			dy = 0.0;
		}
		last_x = x;
		last_y = y;
		last_xy_touched = true;
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

	void GameObject::UpdateV2()
	{
	#ifdef LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		if (pause > 0)
		{
			pause -= 1;
		}
		else
		{
			if (resolve_move)
			{
				if (last_xy_touched)
				{
					vx = x - last_x;
					vy = y - last_y;
				}
				else
				{
					vx = 0.0;
					vy = 0.0;
				}
			}
			else
	#endif // LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
			{
				// 更新速度
				vx += ax;
				vy += ay;
			#ifdef USER_SYSTEM_OPERATION
				// 单独应用重力加速度
				vy -= ag;
				// 速度限制，来自lua层
				if (max_v <= DBL_MIN)
				{
					vx = 0.0;
					vy = 0.0;
				}
				else
				{
					lua_Number const speed_ = std::sqrt(vx * vx + vy * vy);
					if (max_v < speed_ && speed_ > DBL_MIN)
					{
						lua_Number const scale_ = max_v / speed_;
						vx = scale_ * vx;
						vy = scale_ * vy;
					}
				}
				//针对x、y方向单独限制
				vx = std::clamp(vx, -max_vx, max_vx);
				vy = std::clamp(vy, -max_vy, max_vy);
			#endif
				x += vx;
				y += vy;
			}

			rot += omega;

			// 自动旋转

			if (navi && last_xy_touched) {
				auto const dx_ = x - last_x;
				auto const dy_ = y - last_y;
				if (std::abs(dx_) > DBL_MIN || std::abs(dy_) > DBL_MIN) {
					rot = std::atan2(dy_, dx_);
				}
			}
			
			// 更新粒子系统（若有）
			if (res && res->GetType() == ResourceType::Particle)
			{
				ps->SetRotation((float)rot);
				if (ps->IsActived()) // 兼容性处理
				{
					ps->SetActive(false);
					ps->SetCenter(core::Vector2F((float)x, (float)y));
					ps->SetActive(true);
				}
				else
				{
					ps->SetCenter(core::Vector2F((float)x, (float)y));
				}
				ps->Update(1.0f / 60.f);
			}
	#ifdef LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		}
	#endif // LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
	}
	void GameObject::UpdateLastV2() {
		if (last_xy_touched)
		{
			dx = x - last_x;
			dy = y - last_y;
		}
		else
		{
			dx = 0.0;
			dy = 0.0;
		}
		last_x = x;
		last_y = y;
		last_xy_touched = true;
		timer += 1;
		ani_timer += 1;
	}

	void GameObject::Render()
	{
		if (res)
		{
			float const gscale = LRES.GetGlobalImageScaleFactor();
			if (!features.is_render_class) {
				switch (res->GetType())
				{
				case ResourceType::Sprite:
					static_cast<IResourceSprite*>(res)->Render(
						static_cast<float>(x),
						static_cast<float>(y),
						static_cast<float>(rot),
						static_cast<float>(hscale) * gscale,
						static_cast<float>(vscale) * gscale
					);
					break;
				case ResourceType::Animation:
					static_cast<IResourceAnimation*>(res)->Render(
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
			}
			else {
				switch (res->GetType())
				{
				case ResourceType::Sprite:
					static_cast<IResourceSprite*>(res)->Render(
							static_cast<float>(x),
							static_cast<float>(y),
							static_cast<float>(rot),
							static_cast<float>(hscale) * gscale,
						static_cast<float>(vscale) * gscale,
						blend_mode,
						vertex_color
						);
				case ResourceType::Animation:
					static_cast<IResourceAnimation*>(res)->Render(
						static_cast<int>(ani_timer),
						static_cast<float>(x),
						static_cast<float>(y),
						static_cast<float>(rot),
						static_cast<float>(hscale) * gscale,
						static_cast<float>(vscale) * gscale,
						blend_mode,
						vertex_color
					);
					break;
				case ResourceType::Particle:
					if (ps)
					{
						ps->SetBlendMode(blend_mode);
						ps->SetVertexColor(vertex_color);
						LAPP.Render(
							ps,
							static_cast<float>(hscale) * gscale,
							static_cast<float>(vscale) * gscale
						);
					}
					break;
				}
			}
		}
	}

	void GameObject::setGroup(int64_t const new_group) {
		LPOOL.setGroup(this, static_cast<size_t>(new_group));
	}
	void GameObject::setLayer(double const new_layer) {
		LPOOL.setLayer(this, new_layer);
	}
	void GameObject::setResourceRenderState(BlendMode const blend, core::Color4B const color) {
		if (res == nullptr) {
			return;
		}
		if (auto const resource_type = res->GetType(); resource_type == ResourceType::Sprite) {
			auto const sprite = static_cast<IResourceSprite*>(res);
			sprite->SetBlendMode(blend);
			sprite->GetSprite()->setColor(color);
		}
		else if (resource_type == ResourceType::Animation) {
			auto const sprite_sequence = static_cast<IResourceAnimation*>(res);
			sprite_sequence->SetBlendMode(blend);
			sprite_sequence->SetVertexColor(color);
		}
	}
	void GameObject::setParticleRenderState(BlendMode const blend, core::Color4B const color) {
		if (res == nullptr || res->GetType() != ResourceType::Particle) {
			return;
		}
		ps->SetBlendMode(blend);
		ps->SetVertexColor(color);
	}
}
