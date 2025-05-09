#include "Mesh.hpp"
#include "Texture2D.hpp"
#include "MeshRenderer.hpp"
#include "lua/plus.hpp"
#include "LuaWrapperMisc.hpp"
#include "AppFrame.h"
#include "GameResource/LegacyBlendStateHelper.hpp"
#include <DirectXMath.h>

using std::string_view_literals::operator ""sv;

namespace luastg::binding {
	std::string_view MeshRenderer::class_name{ "lstg.MeshRenderer" };

	struct MeshRendererBinding : MeshRenderer {
		// meta methods

		// NOLINTBEGIN(*-reserved-identifier)

		static int __gc(lua_State* vm) {
			if (auto const self = as(vm, 1); self->data) {
				self->data->release();
				self->data = nullptr;
			}
			return 0;
		}
		static int __tostring(lua_State* vm) {
			lua::stack_t const ctx(vm);
			[[maybe_unused]] auto const self = as(vm, 1);
			ctx.push_value(class_name);
			return 1;
		}
		static int __eq(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (is(vm, 2)) {
				auto const other = as(vm, 2);
				ctx.push_value(self->data == other->data);
			}
			else {
				ctx.push_value(false);
			}
			return 1;
		}

		// NOLINTEND(*-reserved-identifier)

		// method

		static void applyTransform(MeshRenderer const* const self) {
			auto const scale = DirectX::XMMatrixScaling(self->scale.x, self->scale.y, self->scale.z);
			auto const rotation = DirectX::XMMatrixRotationRollPitchYaw(self->rotation_yaw_pitch_roll.y, self->rotation_yaw_pitch_roll.x, self->rotation_yaw_pitch_roll.z);
			auto const position = DirectX::XMMatrixTranslation(self->position.x, self->position.y, self->position.z);
			auto const transform = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(scale, rotation), position);
			DirectX::XMFLOAT4X4A matrix;
			DirectX::XMStoreFloat4x4(&matrix, transform);
			self->data->setTransform(*reinterpret_cast<core::Matrix4F*>(&matrix));
		}
		static int setPosition(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);

			auto const x = ctx.get_value<float>(1 + 1);
			auto const y = ctx.get_value<float>(1 + 2);
			if (ctx.index_of_top() >= 1 + 3) {
				auto const z = ctx.get_value<float>(1 + 3);
				self->position = core::Vector3F(x, y, z);
			}
			else {
				self->position = core::Vector3F(x, y, 0.0f);
			}
			applyTransform(self);

			ctx.push_value(lua::stack_index_t(1)); // return self
			return 1;
		}
		static int setScale(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);

			auto const x = ctx.get_value<float>(1 + 1);
			auto const y = ctx.get_value<float>(1 + 2);
			if (ctx.index_of_top() >= 1 + 3) {
				auto const z = ctx.get_value<float>(1 + 3);
				self->scale = core::Vector3F(x, y, z);
			}
			else {
				self->scale = core::Vector3F(x, y, 1.0f);
			}
			applyTransform(self);

			ctx.push_value(lua::stack_index_t(1)); // return self
			return 1;
		}
		static int setRotationYawPitchRoll(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);

			auto const yaw = ctx.get_value<float>(1 + 1);
			auto const pitch = ctx.get_value<float>(1 + 2);
			auto const roll = ctx.get_value<float>(1 + 3);
			self->rotation_yaw_pitch_roll = core::Vector3F(yaw, pitch, roll);
			applyTransform(self);

			ctx.push_value(lua::stack_index_t(1)); // return self
			return 1;
		}
		static int setMesh(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);

			if (ctx.is_userdata(1 + 1)) {
				auto const mesh = Mesh::as(vm, 1 + 1);
				self->data->setMesh(mesh->data);
			}
			else {
				self->data->setMesh(nullptr);
			}

			ctx.push_value(lua::stack_index_t(1)); // return self
			return 1;
		}
		static int setTexture(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);

			if (ctx.is_userdata(1 + 1)) {
				auto const texture = Texture2D::as(vm, 1 + 1);
				self->data->setTexture(texture->data);
			}
			else if (ctx.is_string(1 + 1)) {
				// work with resource pool
				// remove this workaround in the future
				auto const texture_resource_name = ctx.get_value<std::string_view>(1 + 1);
				auto texture_resource = LRES.FindTexture(texture_resource_name.data());
				self->data->setTexture(texture_resource->GetTexture());
			}
			else {
				self->data->setTexture(nullptr);
			}

			ctx.push_value(lua::stack_index_t(1)); // return self
			return 1;
		}
		static int setLegacyBlendState(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const blend = luastg::TranslateBlendMode(vm, 1 + 1);
			[[maybe_unused]] auto const [v, b] = luastg::translateLegacyBlendState(blend);
			self->data->setLegacyBlendState(v, b);
			ctx.push_value(lua::stack_index_t(1)); // return self
			return 1;
		}
		static int draw(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);

			auto const renderer = LAPP.GetAppModel()->getRenderer();
			self->data->draw(renderer);

			ctx.push_value(lua::stack_index_t(1)); // return self
			return 1;
		}

		// static method

		static int create(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const device = LAPP.GetAppModel()->getDevice();
			if (ctx.index_of_top() >= 2) {
				auto const mesh = Mesh::as(vm, 1);
				auto const texture = Texture2D::as(vm, 2);
				auto const self = MeshRenderer::create(vm);
				if (!core::Graphics::IMeshRenderer::create(device, &self->data)) {
					return luaL_error(vm, "create MeshRenderer failed.");
				}
				self->data->setMesh(mesh->data);
				self->data->setTexture(texture->data);
			}
			else {
				auto const self = MeshRenderer::create(vm);
				if (!core::Graphics::IMeshRenderer::create(device, &self->data)) {
					return luaL_error(vm, "create MeshRenderer failed.");
				}
			}
			return 1;
		}
	};

	bool MeshRenderer::is(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	MeshRenderer* MeshRenderer::as(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<MeshRenderer>(index);
	}
	MeshRenderer* MeshRenderer::create(lua_State* vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<MeshRenderer>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}
	void MeshRenderer::registerClass(lua_State* vm) {
		[[maybe_unused]] lua::stack_balancer_t stack_balancer(vm);
		lua::stack_t const ctx(vm);

		// method

		auto const method_table = ctx.create_module(class_name);
		ctx.set_map_value(method_table, "setPosition", &MeshRendererBinding::setPosition);
		ctx.set_map_value(method_table, "setScale", &MeshRendererBinding::setScale);
		ctx.set_map_value(method_table, "setRotationYawPitchRoll", &MeshRendererBinding::setRotationYawPitchRoll);
		ctx.set_map_value(method_table, "setMesh", &MeshRendererBinding::setMesh);
		ctx.set_map_value(method_table, "setTexture", &MeshRendererBinding::setTexture);
		ctx.set_map_value(method_table, "setLegacyBlendState", &MeshRendererBinding::setLegacyBlendState);
		ctx.set_map_value(method_table, "draw", &MeshRendererBinding::draw);
		ctx.set_map_value(method_table, "create", &MeshRendererBinding::create);

		// metatable

		auto const metatable = ctx.create_metatable(class_name);
		ctx.set_map_value(metatable, "__gc", &MeshRendererBinding::__gc);
		ctx.set_map_value(metatable, "__tostring", &MeshRendererBinding::__tostring);
		ctx.set_map_value(metatable, "__eq", &MeshRendererBinding::__eq);
		ctx.set_map_value(metatable, "__index", method_table);
	}
}
