#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"
#include <cassert>

using std::string_view_literals::operator ""sv;

namespace {
	int notSupported(lua_State* const vm) {
		return luaL_error(vm, "not supported");
	}
}

namespace imgui::binding {
	std::string_view const ImGuiTextBufferBinding::class_name{ "imgui.ImGuiTextBuffer"sv };

	constexpr auto mask = ~static_cast<size_t>(1);

	void ImGuiTextBufferBinding::set(ImGuiTextBuffer* const ptr, bool const is_reference) {
		assert(reinterpret_cast<size_t>(ptr) % 2 == 0); // unlikely
		data = reinterpret_cast<ImGuiTextBuffer*>((reinterpret_cast<size_t>(ptr) & mask) | (is_reference ? 0x1 : 0x0));
	}
	ImGuiTextBuffer* ImGuiTextBufferBinding::get() {
		return reinterpret_cast<ImGuiTextBuffer*>(reinterpret_cast<size_t>(data) & mask);
	}
	bool ImGuiTextBufferBinding::isReference() const noexcept {
		return (reinterpret_cast<size_t>(data) & 0x1) == 0x1;
	}

	struct ImGuiTextBufferWrapper : ImGuiTextBufferBinding {
		static int gc(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = ctx.as_userdata<ImGuiTextBufferBinding>(1);
			if (self->get() != nullptr && !self->isReference()) {
				IM_DELETE(self->get());
				self->data = nullptr;
			}
			return 0;
		}
		static int toString(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			[[maybe_unused]] auto const self = as(vm, 1);
			ctx.push_value(class_name);
			return 1;
		}

		static int size(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const result = self->size();
			ctx.push_value(result);
			return 1;
		}
		static int empty(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const result = self->empty();
			ctx.push_value(result);
			return 1;
		}
		static int clear(lua_State* const vm) {
			auto const self = as(vm, 1);
			self->clear();
			return 0;
		}
		static int resize(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const size = ctx.get_value<int>(1 + 1);
			self->resize(size);
			return 0;
		}
		static int reserve(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const size = ctx.get_value<int>(1 + 1);
			self->reserve(size);
			return 0;
		}
		static int c_str(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const result = self->c_str();
			ctx.push_value(std::string_view(result, strnlen_s(result, self->size())));
			return 1;
		}
		static int append(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const str = ctx.get_value<std::string_view>(1 + 1);
			self->append(str.data(), str.data() + str.size());
			return 0;
		}

		static int constructor(lua_State* const vm) {
			std::ignore = create(vm);
			return 1;
		}
	};

	bool ImGuiTextBufferBinding::is(lua_State* const vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	ImGuiTextBuffer* ImGuiTextBufferBinding::as(lua_State* const vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<ImGuiTextBufferBinding>(index)->get();
	}
	ImGuiTextBuffer* ImGuiTextBufferBinding::create(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<ImGuiTextBufferBinding>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->set(IM_NEW(ImGuiTextBuffer)(), false);
		return self->get();
	}
	void ImGuiTextBufferBinding::registerClass(lua_State* vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const methods = ctx.create_module(class_name);
		ctx.set_map_value(methods, "begin"sv, &notSupported);
		ctx.set_map_value(methods, "end"sv, &notSupported);
		ctx.set_map_value(methods, "size"sv, &ImGuiTextBufferWrapper::size);
		ctx.set_map_value(methods, "empty"sv, &ImGuiTextBufferWrapper::empty);
		ctx.set_map_value(methods, "clear"sv, &ImGuiTextBufferWrapper::clear);
		ctx.set_map_value(methods, "resize"sv, &ImGuiTextBufferWrapper::resize);
		ctx.set_map_value(methods, "reserve"sv, &ImGuiTextBufferWrapper::reserve);
		ctx.set_map_value(methods, "c_str"sv, &ImGuiTextBufferWrapper::c_str);
		ctx.set_map_value(methods, "append"sv, &ImGuiTextBufferWrapper::append);
		//ctx.set_map_value(methods, "appendf"sv, &appendf);
		ctx.set_map_value(methods, "appendfv"sv, &notSupported);

		auto const mt = ctx.create_metatable(class_name);
		ctx.set_map_value(mt, "__gc"sv, &ImGuiTextBufferWrapper::gc);
		ctx.set_map_value(mt, "__tostring"sv, &ImGuiTextBufferWrapper::toString);
		ctx.set_map_value(mt, "__index"sv, methods);

		auto const m = ctx.push_module(module_name);
		ctx.set_map_value(m, "ImGuiTextBuffer"sv, &ImGuiTextBufferWrapper::constructor);

		luaL_dostring(vm, R"(
local select = select
local ImGuiTextBuffer = require("imgui.ImGuiTextBuffer")
function ImGuiTextBuffer:appendf(fmt, ...)
	if select("#", ...) > 0 then
		self:append(fmt:format(...))
	else
		self:append(fmt)
	end
end
		)");
	}
}
