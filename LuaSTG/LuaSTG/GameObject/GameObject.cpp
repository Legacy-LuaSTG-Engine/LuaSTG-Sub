#include "GameObject/GameObject.hpp"
#include "GameResource/ResourceSprite.hpp"
#include "GameResource/ResourceAnimation.hpp"
#include "lua/plus.hpp"
#include "AppFrame.h"
#include <ranges>
#include <algorithm>

using std::string_view_literals::operator ""sv;

namespace luastg {
	std::pmr::unsynchronized_pool_resource GameObject::s_callbacks_resource;

	namespace {
		[[nodiscard]] bool isDirectCallbacks(GameObject const* const self) noexcept {
			return self->callbacks_count == 1 && self->callbacks_capacity == 0;
		}
		void setDirectCallbacks(GameObject* const self, IGameObjectCallbacks* const c) noexcept {
			self->callbacks = reinterpret_cast<IGameObjectCallbacks**>(c);
			self->callbacks_count = 1;
			self->callbacks_capacity = 0;
		}
		void clearCallbacks(GameObject* const self) noexcept {
			self->callbacks = nullptr;
			self->callbacks_count = 0;
			self->callbacks_capacity = 0;
		}
	}

	bool GameObject::containsCallbacks(IGameObjectCallbacks const* const c) const noexcept {
		if (isDirectCallbacks(this)) {
			return reinterpret_cast<IGameObjectCallbacks*>(callbacks) == c;
		}
		for (size_t i = 0; i < callbacks_count; i++) {
			if (callbacks[i] == c) {
				return true;
			}
		}
		return false;
	}
	void GameObject::addCallbacks(IGameObjectCallbacks* const c) {
		if (callbacks_count == 0) {
			setDirectCallbacks(this, c);
		}
		else if (!containsCallbacks(c)) {
			if (isDirectCallbacks(this)) {
				auto const last = reinterpret_cast<IGameObjectCallbacks*>(callbacks);
				callbacks_count = 2;
				callbacks_capacity = 2;
				callbacks = static_cast<IGameObjectCallbacks**>(s_callbacks_resource.allocate(sizeof(IGameObjectCallbacks*) * callbacks_capacity));
				callbacks[0] = last;
				callbacks[1] = c;
			}
			else if (callbacks_count == callbacks_capacity) {
				assert(false); // DEBUG: unlikely
				auto const data = callbacks;
				auto const size = callbacks_count;
				callbacks_capacity *= 2;
				callbacks = static_cast<IGameObjectCallbacks**>(s_callbacks_resource.allocate(sizeof(IGameObjectCallbacks*) * callbacks_capacity));
				std::memcpy(static_cast<void*>(callbacks), static_cast<void*>(data), sizeof(IGameObjectCallbacks*) * size);
				std::memset(static_cast<void*>(callbacks + size), 0, sizeof(IGameObjectCallbacks*) * size);
				s_callbacks_resource.deallocate(static_cast<void*>(data), sizeof(IGameObjectCallbacks*) * size);
			}
			callbacks[callbacks_count] = c;
			callbacks_count++;
		}
	}
	void GameObject::removeCallbacks(IGameObjectCallbacks* const c) {
		if (callbacks_count == 0) {
			return;
		}
		if (isDirectCallbacks(this) && reinterpret_cast<IGameObjectCallbacks*>(callbacks) == c) {
			clearCallbacks(this);
		}
		if (callbacks_count == 1 && callbacks[0] == c) {
			callbacks[0] = nullptr;
			callbacks_count = 0;
		}
		if (auto const padding = std::ranges::remove(callbacks, callbacks + callbacks_count, c); !padding.empty()) {
			std::ranges::fill(padding, nullptr);
			callbacks_count -= static_cast<uint32_t>(padding.size());
		}
	}
	void GameObject::removeAllCallbacks() {
		if (callbacks != nullptr && !isDirectCallbacks(this)) {
			s_callbacks_resource.deallocate(static_cast<void*>(callbacks), sizeof(IGameObjectCallbacks*) * callbacks_capacity);
		}
		clearCallbacks(this);
	}

#define FOR_EACH_CALLBACKS(S) \
	if (isDirectCallbacks(this)) { reinterpret_cast<IGameObjectCallbacks*>(callbacks)-> S return; } \
	for (uint32_t i = 0; i < callbacks_count; i++) { callbacks[i]-> S }

	void GameObject::dispatchOnQueueToDestroy(std::string_view const reason) { FOR_EACH_CALLBACKS(onQueueToDestroy(this, reason);) }
	void GameObject::dispatchOnUpdate() { FOR_EACH_CALLBACKS(onUpdate(this);) }
	void GameObject::dispatchOnLateUpdate() { FOR_EACH_CALLBACKS(onLateUpdate(this);) }
	void GameObject::dispatchOnRender() { FOR_EACH_CALLBACKS(onRender(this);) }
	void GameObject::dispatchOnTrigger(GameObject* const other) { FOR_EACH_CALLBACKS(onTrigger(this, other);) }

#undef FOR_EACH_CALLBACKS
}

namespace luastg {
	void GameObject::Reset() {
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
	void GameObject::DirtReset() {
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

	bool GameObject::ChangeResource(std::string_view const& res_name) {
		core::SmartReference<IResourceSprite> tSprite = LRES.FindSprite(res_name.data());
		if (tSprite) {
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
		if (tAnimation) {
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
		if (tParticle) {
			// 分配粒子池
			if (!tParticle->CreateInstance(&ps)) {
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
	void GameObject::ReleaseResource() {
		if (res) {
			if (res->GetType() == ResourceType::Particle) {
				assert(ps);
				static_cast<IResourceParticle*>(res)->DestroyInstance(ps);
				ps = nullptr;
			}
			res->release();
			res = nullptr;
		}
	}

	void GameObject::Update() {
	#ifdef LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		if (pause > 0) {
			pause -= 1;
		}
		else {
			if (resolve_move) {
				if (last_xy_touched) {
					vx = x - last_x;
					vy = y - last_y;
				}
				else {
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
				if (max_v <= DBL_MIN) {
					vx = 0.0;
					vy = 0.0;
				}
				else {
					lua_Number const speed_ = std::sqrt(vx * vx + vy * vy);
					if (max_v < speed_ && speed_ > DBL_MIN) {
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
			if (res && res->GetType() == ResourceType::Particle) {
				ps->SetRotation((float)rot);
				if (ps->IsActived()) // 兼容性处理
				{
					ps->SetActive(false);
					ps->SetCenter(core::Vector2F((float)x, (float)y));
					ps->SetActive(true);
				}
				else {
					ps->SetCenter(core::Vector2F((float)x, (float)y));
				}
				ps->Update(1.0f / 60.f);
			}
		#ifdef LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		}
	#endif // LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
	}
	void GameObject::UpdateLast() {
		if (last_xy_touched) {
			dx = x - last_x;
			dy = y - last_y;
		}
		else {
			dx = 0.0;
			dy = 0.0;
		}
		last_x = x;
		last_y = y;
		last_xy_touched = true;
		if (navi && (std::abs(dx) > DBL_MIN || std::abs(dy) > DBL_MIN)) {
			rot = std::atan2(dy, dx);
		}
	}
	void GameObject::UpdateTimer() {
		timer += 1;
		ani_timer += 1;
	}

	void GameObject::UpdateV2() {
	#ifdef LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		if (pause > 0) {
			pause -= 1;
		}
		else {
			if (resolve_move) {
				if (last_xy_touched) {
					vx = x - last_x;
					vy = y - last_y;
				}
				else {
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
				if (max_v <= DBL_MIN) {
					vx = 0.0;
					vy = 0.0;
				}
				else {
					lua_Number const speed_ = std::sqrt(vx * vx + vy * vy);
					if (max_v < speed_ && speed_ > DBL_MIN) {
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
			if (res && res->GetType() == ResourceType::Particle) {
				ps->SetRotation((float)rot);
				if (ps->IsActived()) // 兼容性处理
				{
					ps->SetActive(false);
					ps->SetCenter(core::Vector2F((float)x, (float)y));
					ps->SetActive(true);
				}
				else {
					ps->SetCenter(core::Vector2F((float)x, (float)y));
				}
				ps->Update(1.0f / 60.f);
			}
		#ifdef LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
		}
	#endif // LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
	}
	void GameObject::UpdateLastV2() {
		if (last_xy_touched) {
			dx = x - last_x;
			dy = y - last_y;
		}
		else {
			dx = 0.0;
			dy = 0.0;
		}
		last_x = x;
		last_y = y;
		last_xy_touched = true;
		timer += 1;
		ani_timer += 1;
	}

	void GameObject::Render() {
		if (res) {
			float const gscale = LRES.GetGlobalImageScaleFactor();
			if (!features.is_render_class) {
				switch (res->GetType()) {
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
					if (ps) {
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
				switch (res->GetType()) {
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
					break;
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
					if (ps) {
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
	void GameObject::setResourceRenderState(BlendMode const blend, core::Color4B const color) const {
		if (res == nullptr) {
			return;
		}
		if (auto const resource_type = res->GetType(); resource_type == ResourceType::Sprite) {
			auto const sprite = static_cast<IResourceSprite*>(res);
			sprite->SetBlendMode(blend);
			sprite->SetColor(color);
		}
		else if (resource_type == ResourceType::Animation) {
			auto const sprite_sequence = static_cast<IResourceAnimation*>(res);
			sprite_sequence->SetBlendMode(blend);
			sprite_sequence->SetVertexColor(color);
		}
	}
	void GameObject::setParticleRenderState(BlendMode const blend, core::Color4B const color) const {
		if (res == nullptr || res->GetType() != ResourceType::Particle) {
			return;
		}
		ps->SetBlendMode(blend);
		ps->SetVertexColor(color);
	}
	void GameObject::stopParticle() const {
		if (!hasParticlePool()) {
			return;
		}
		ps->SetActive(false);
	}
	void GameObject::startParticle() const {
		if (!hasParticlePool()) {
			return;
		}
		ps->SetActive(true);
	}
	size_t GameObject::getParticleCount() const {
		if (!hasParticlePool()) {
			return 0;
		}
		return ps->GetAliveCount();
	}
	int32_t GameObject::getParticleEmission() const {
		if (!hasParticlePool()) {
			return 0;
		}
		return ps->GetEmission();
	}
	void GameObject::setParticleEmission(int32_t const value) const {
		if (!hasParticlePool()) {
			return;
		}
		ps->SetEmission(value);
	}
}
