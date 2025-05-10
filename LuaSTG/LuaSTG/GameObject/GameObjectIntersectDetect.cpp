#include "GameObject/GameObject.hpp"
#include "XCollision.h"

namespace {
	template<typename T>
	constexpr bool isAxisAlignedBoundingBoxIntersect(
		const T x1, const T y1, const T a1, const T b1,
		const T x2, const T y2, const T a2, const T b2
	) noexcept {
		return (x1 + a1) >= (x2 - a2)
			&& (x1 - a1) <= (x2 + a2)
			&& (y1 + b1) >= (y2 - b2)
			&& (y1 - b1) <= (y2 + b2);
	}

	bool isAxisAlignedBoundingBoxIntersect(
		luastg::GameObject const* const p1,
		luastg::GameObject const* const p2
	) noexcept {
		return isAxisAlignedBoundingBoxIntersect(
			p1->x, p1->y, p1->col_r, p1->col_r,
			p2->x, p2->y, p2->col_r, p2->col_r);
	}

	template<typename T>
	constexpr bool isAxisAlignedBoundingBoxNotIntersect(
		const T x1, const T y1, const T a1, const T b1,
		const T x2, const T y2, const T a2, const T b2
	) noexcept {
		return (x1 + a1) < (x2 - a2)
			|| (x1 - a1) > (x2 + a2)
			|| (y1 + b1) < (y2 - b2)
			|| (y1 - b1) > (y2 + b2);
	}

	bool isAxisAlignedBoundingBoxNotIntersect(
		luastg::GameObject const* const p1,
		luastg::GameObject const* const p2
	) noexcept {
		return isAxisAlignedBoundingBoxIntersect(
			p1->x, p1->y, p1->col_r, p1->col_r,
			p2->x, p2->y, p2->col_r, p2->col_r);
	}

	xmath::collision::ColliderType getColliderType(luastg::GameObject const* const p) noexcept {
		return p->rect
			? xmath::collision::ColliderType::OBB
			: p->a == p->b
			? xmath::collision::ColliderType::Circle
			: xmath::collision::ColliderType::Ellipse;
	}
}

namespace luastg {
	bool GameObject::isIntersect(GameObject const* const p1, GameObject const* const p2) noexcept {
		//忽略不碰撞对象
		if (!p1->colli || !p2->colli) {
			return false;
		}

		//快速AABB检测
		if (isAxisAlignedBoundingBoxNotIntersect(p1, p2)) {
			return false;
		}

		cocos2d::Vec2 const xy1(static_cast<float>(p1->x), static_cast<float>(p1->y));
		auto const r1 = static_cast<float>(p1->col_r);
		cocos2d::Vec2 const xy2(static_cast<float>(p2->x), static_cast<float>(p2->y));
		auto const r2 = static_cast<float>(p2->col_r);

		auto const a1 = static_cast<float>(p1->a);
		auto const b1 = static_cast<float>(p1->b);
		auto const rot1 = static_cast<float>(p1->rot);
		auto const a2 = static_cast<float>(p2->a);
		auto const b2 = static_cast<float>(p2->b);
		auto const rot2 = static_cast<float>(p2->rot);

		//外接圆碰撞检测
		if (!xmath::collision::check(
			xy1, r1, r1, rot1, xmath::collision::ColliderType::Circle,
			xy2, r2, r2, rot2, xmath::collision::ColliderType::Circle)) {
			return false;
		}

		// 精确碰撞检测
		return xmath::collision::check(
			xy1, a1, b1, rot1, getColliderType(p1),
			xy2, a2, b2, rot2, getColliderType(p2));
	}
}
