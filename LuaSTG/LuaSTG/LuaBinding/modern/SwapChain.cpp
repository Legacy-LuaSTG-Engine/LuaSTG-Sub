#include "SwapChain.hpp"
#include "lua/plus.hpp"
#include "AppFrame.h"

static void pushSize(lua_State* L, lua_Number const width, lua_Number const height) {
	lua::stack_t S(L);
	auto const index = S.create_map(2);
	S.set_map_value(index, "width", width);
	S.set_map_value(index, "height", height);
}

namespace luastg::binding {

	std::string_view SwapChain::class_name{ "lstg.SwapChain" };

	struct SwapChainBinding : public SwapChain {

		// meta methods

		static int __gc(lua_State* L) {
			auto self = as(L, 1);
			if (self->data) {
				self->data->release();
				self->data = nullptr;
			}
			return 0;
		}

		static int __tostring(lua_State* L) {
			lua::stack_t S(L);
			[[maybe_unused]] auto self = as(L, 1);
			S.push_value(class_name);
			return 1;
		}

		static int __eq(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			if (is(L, 2)) {
				auto other = as(L, 2);
				S.push_value(self->data == other->data);
			}
			else {
				S.push_value(false);
			}
			return 1;
		}

		// instance methods

		static int setWindowed(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const width = S.get_value<uint32_t>(2);
			auto const height = S.get_value<uint32_t>(3);
			auto const size = Core::Vector2U(width, height);
			auto const result = self->data->setWindowMode(size);
			S.push_value(result);
			return 1;
		}

		static int getSize(lua_State* L) {
			auto self = as(L, 1);
			auto const size = self->data->getCanvasSize();
			pushSize(L, size.x, size.y);
			return 1;
		}

		static int setSize(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const width = S.get_value<uint32_t>(2);
			auto const height = S.get_value<uint32_t>(3);
			auto const size = Core::Vector2U(width, height);
			auto const result = self->data->setCanvasSize(size);
			S.push_value(result);
			return 1;
		}

		static int getVSyncPreference(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const allow = self->data->getVSync();
			S.push_value(allow);
			return 1;
		}

		static int setVSyncPreference(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const allow = S.get_value<bool>(2);
			self->data->setVSync(allow);
			return 0;
		}

		static int getScalingMode(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const mode = static_cast<int32_t>(self->data->getScalingMode());
			S.push_value(mode);
			return 1;
		}

		static int setScalingMode(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const mode = static_cast<Core::Graphics::SwapChainScalingMode>(S.get_value<int32_t>(2));
			self->data->setScalingMode(mode);
			return 0;
		}

		// static methods

		static int getMain(lua_State* L) {
			lua::stack_t S(L);
			auto self = create(L);
			self->data = LAPP.GetAppModel()->getSwapChain();
			self->data->retain();
			return 1;
		}

	};

	bool SwapChain::is(lua_State* L, int index) {
		return nullptr != luaL_testudata(L, index, class_name.data());
	}

	SwapChain* SwapChain::as(lua_State* L, int index) {
		return static_cast<SwapChain*>(luaL_checkudata(L, index, class_name.data()));
	}

	SwapChain* SwapChain::create(lua_State* L) {
		lua::stack_t S(L);
		auto self = S.create_userdata<SwapChain>();
		auto const self_index = S.index_of_top();
		S.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}

	void SwapChain::registerClass(lua_State* L) {
		[[maybe_unused]] lua::stack_balancer_t SB(L);
		lua::stack_t S(L);

		// lstg.Window.FrameStyle

		{
			auto const e = S.create_module("lstg.SwapChain.ScalingMode");
			S.set_map_value(e, "stretch", static_cast<int32_t>(Core::Graphics::SwapChainScalingMode::Stretch));
			S.set_map_value(e, "aspect_ratio", static_cast<int32_t>(Core::Graphics::SwapChainScalingMode::AspectRatio));
		}

		// method

		auto const method_table = S.create_module(class_name);
		S.set_map_value(method_table, "setWindowed", &SwapChainBinding::setWindowed);
		S.set_map_value(method_table, "getSize", &SwapChainBinding::getSize);
		S.set_map_value(method_table, "setSize", &SwapChainBinding::setSize);
		S.set_map_value(method_table, "getVSyncPreference", &SwapChainBinding::getVSyncPreference);
		S.set_map_value(method_table, "setVSyncPreference", &SwapChainBinding::setVSyncPreference);
		S.set_map_value(method_table, "getScalingMode", &SwapChainBinding::getScalingMode);
		S.set_map_value(method_table, "setScalingMode", &SwapChainBinding::setScalingMode);
		S.set_map_value(method_table, "getMain", &SwapChainBinding::getMain);

		// metatable

		auto const metatable = S.create_metatable(class_name);
		S.set_map_value(metatable, "__gc", &SwapChainBinding::__gc);
		S.set_map_value(metatable, "__tostring", &SwapChainBinding::__tostring);
		S.set_map_value(metatable, "__eq", &SwapChainBinding::__eq);
		S.set_map_value(metatable, "__index", method_table);

		// register

		//auto const module_table = S.push_module("lstg");
		//S.set_map_value(module_table, "SwapChain", method_table);
	}

}
