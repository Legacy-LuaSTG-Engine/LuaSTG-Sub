#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"
#include <cassert>

using std::string_view_literals::operator ""sv;

namespace imgui::binding {
	struct ImGuiStyleColorsBinding {
		static constexpr auto class_name{ "imgui.ImGuiStyle.Colors"sv };

		static int toString(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			[[maybe_unused]] auto const self = as(vm, 1);
			ctx.push_value(class_name);
			return 1;
		}
		static int getter(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const key = ctx.get_value<int32_t>(2);
			if (key >= ImGuiCol_Text && key < ImGuiCol_COUNT) {
				ImVec4Binding::create(vm, (*self)[key]);
				return 1;
			}
			return luaL_error(vm, "index [%d] out of bound", key);
		}
		static int setter(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const key = ctx.get_value<int32_t>(2);
			if (key >= ImGuiCol_Text && key < ImGuiCol_COUNT) {
				auto const value = ImVec4Binding::as(vm, 3);
				(*self)[key] = *value;
				return 0;
			}
			return luaL_error(vm, "index [%d] out of bound", key);
		}

		static bool is(lua_State* const vm, int const index) {
			lua::stack_t const ctx(vm);
			return ctx.is_metatable(index, class_name);
		}
		static ImVec4** as(lua_State* const vm, int const index) {
			lua::stack_t const ctx(vm);
			return ctx.as_userdata<ImVec4*>(index, class_name);
		}
		static ImVec4** reference(lua_State* const vm, ImVec4* const value) {
			lua::stack_t const ctx(vm);
			auto const self = ctx.create_userdata<ImVec4*>();
			auto const self_index = ctx.index_of_top();
			ctx.set_metatable(self_index, class_name);
			*self = value;
			return self;
		}
		static void registerClass(lua_State* const vm) {
			lua::stack_balancer_t const sb(vm);
			lua::stack_t const ctx(vm);

			auto const mt = ctx.create_metatable(class_name);
			ctx.set_map_value(mt, "__tostring"sv, &ImGuiStyleColorsBinding::toString);
			ctx.set_map_value(mt, "__index"sv, &ImGuiStyleColorsBinding::getter);
			ctx.set_map_value(mt, "__newindex"sv, &ImGuiStyleColorsBinding::setter);
		}
	};

	std::string_view const ImGuiStyleBinding::class_name{ "imgui.ImGuiStyle"sv };

	constexpr auto mask = ~static_cast<size_t>(1);

	void ImGuiStyleBinding::set(ImGuiStyle* const ptr, bool const is_reference) {
		assert(reinterpret_cast<size_t>(ptr) % 2 == 0); // unlikely
		data = reinterpret_cast<ImGuiStyle*>((reinterpret_cast<size_t>(ptr) & mask) | (is_reference ? 0x1 : 0x0));
	}
	ImGuiStyle* ImGuiStyleBinding::get() {
		return reinterpret_cast<ImGuiStyle*>(reinterpret_cast<size_t>(data) & mask);
	}
	bool ImGuiStyleBinding::isReference() const noexcept {
		return (reinterpret_cast<size_t>(data) & 0x1) == 0x1;
	}

	struct ImGuiStyleWrapper : ImGuiStyleBinding {
		static int gc(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = ctx.as_userdata<ImGuiStyleBinding>(1);
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
		static int getter(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const key = ctx.get_value<std::string_view>(2);

		#define GET_FLAG(name) if (key == (#name ""sv)) { ctx.push_value(static_cast<int32_t>(self->name)); return 1; } (void)0
		#define GET_SCALAR(name) if (key == (#name ""sv)) { ctx.push_value(self->name); return 1; } (void)0
		#define GET_VEC2(name) if (key == (#name ""sv)) { ImVec2Binding::create(vm, self->name); return 1; } (void)0
		#define GET_COLOR(name) if (key == ("Colors" #name ""sv)) { ImVec4Binding::create(vm, self->Colors[ImGuiCol_##name]); return 1; } (void)0

			GET_SCALAR(FontSizeBase);
			GET_SCALAR(FontScaleMain);
			GET_SCALAR(FontScaleDpi);

			GET_SCALAR(Alpha);
			GET_SCALAR(DisabledAlpha);
			GET_VEC2(WindowPadding);
			GET_SCALAR(WindowRounding);
			GET_SCALAR(WindowBorderSize);
			GET_SCALAR(WindowBorderHoverPadding);
			GET_VEC2(WindowMinSize);
			GET_VEC2(WindowTitleAlign);
			GET_FLAG(WindowMenuButtonPosition);
			GET_SCALAR(ChildRounding);
			GET_SCALAR(ChildBorderSize);
			GET_SCALAR(PopupRounding);
			GET_SCALAR(PopupBorderSize);
			GET_VEC2(FramePadding);
			GET_SCALAR(FrameRounding);
			GET_SCALAR(FrameBorderSize);
			GET_VEC2(ItemSpacing);
			GET_VEC2(ItemInnerSpacing);
			GET_VEC2(CellPadding);
			GET_VEC2(TouchExtraPadding);
			GET_SCALAR(IndentSpacing);
			GET_SCALAR(ColumnsMinSpacing);
			GET_SCALAR(ScrollbarSize);
			GET_SCALAR(ScrollbarRounding);
			GET_SCALAR(ScrollbarPadding);
			GET_SCALAR(GrabMinSize);
			GET_SCALAR(GrabRounding);
			GET_SCALAR(LogSliderDeadzone);
			GET_SCALAR(ImageBorderSize);
			GET_SCALAR(TabRounding);
			GET_SCALAR(TabBorderSize);
			GET_SCALAR(TabMinWidthBase);
			GET_SCALAR(TabMinWidthShrink);
			GET_SCALAR(TabCloseButtonMinWidthSelected);
			GET_SCALAR(TabCloseButtonMinWidthUnselected);
			GET_SCALAR(TabBarBorderSize);
			GET_SCALAR(TabBarOverlineSize);
			GET_SCALAR(TableAngledHeadersAngle);
			GET_VEC2(TableAngledHeadersTextAlign);
			GET_FLAG(TreeLinesFlags);
			GET_SCALAR(TreeLinesSize);
			GET_SCALAR(TreeLinesRounding);
			GET_FLAG(ColorButtonPosition);
			GET_VEC2(ButtonTextAlign);
			GET_VEC2(SelectableTextAlign);
			GET_SCALAR(SeparatorTextBorderSize);
			GET_VEC2(SeparatorTextAlign);
			GET_VEC2(SeparatorTextPadding);
			GET_VEC2(DisplayWindowPadding);
			GET_VEC2(DisplaySafeAreaPadding);
			GET_SCALAR(MouseCursorScale);
			GET_SCALAR(AntiAliasedLines);
			GET_SCALAR(AntiAliasedLinesUseTex);
			GET_SCALAR(AntiAliasedFill);
			GET_SCALAR(CurveTessellationTol);
			GET_SCALAR(CircleTessellationMaxError);

			if (key == "Colors"sv) {
				ImGuiStyleColorsBinding::reference(vm, self->Colors);
				return 1;
			}
			GET_COLOR(Text);
			GET_COLOR(TextDisabled);
			GET_COLOR(WindowBg);
			GET_COLOR(ChildBg);
			GET_COLOR(PopupBg);
			GET_COLOR(Border);
			GET_COLOR(BorderShadow);
			GET_COLOR(FrameBg);
			GET_COLOR(FrameBgHovered);
			GET_COLOR(FrameBgActive);
			GET_COLOR(TitleBg);
			GET_COLOR(TitleBgActive);
			GET_COLOR(TitleBgCollapsed);
			GET_COLOR(MenuBarBg);
			GET_COLOR(ScrollbarBg);
			GET_COLOR(ScrollbarGrab);
			GET_COLOR(ScrollbarGrabHovered);
			GET_COLOR(ScrollbarGrabActive);
			GET_COLOR(CheckMark);
			GET_COLOR(SliderGrab);
			GET_COLOR(SliderGrabActive);
			GET_COLOR(Button);
			GET_COLOR(ButtonHovered);
			GET_COLOR(ButtonActive);
			GET_COLOR(Header);
			GET_COLOR(HeaderHovered);
			GET_COLOR(HeaderActive);
			GET_COLOR(Separator);
			GET_COLOR(SeparatorHovered);
			GET_COLOR(SeparatorActive);
			GET_COLOR(ResizeGrip);
			GET_COLOR(ResizeGripHovered);
			GET_COLOR(ResizeGripActive);
			GET_COLOR(InputTextCursor);
			GET_COLOR(TabHovered);
			GET_COLOR(Tab);
			GET_COLOR(TabSelected);
			GET_COLOR(TabSelectedOverline);
			GET_COLOR(TabDimmed);
			GET_COLOR(TabDimmedSelected);
			GET_COLOR(TabDimmedSelectedOverline);
			GET_COLOR(PlotLines);
			GET_COLOR(PlotLinesHovered);
			GET_COLOR(PlotHistogram);
			GET_COLOR(PlotHistogramHovered);
			GET_COLOR(TableHeaderBg);
			GET_COLOR(TableBorderStrong);
			GET_COLOR(TableBorderLight);
			GET_COLOR(TableRowBg);
			GET_COLOR(TableRowBgAlt);
			GET_COLOR(TextLink);
			GET_COLOR(TextSelectedBg);
			GET_COLOR(TreeLines);
			GET_COLOR(DragDropTarget);
			GET_COLOR(NavCursor);
			GET_COLOR(NavWindowingHighlight);
			GET_COLOR(NavWindowingDimBg);
			GET_COLOR(ModalWindowDimBg);

			GET_SCALAR(HoverStationaryDelay);
			GET_SCALAR(HoverDelayShort);
			GET_SCALAR(HoverDelayNormal);
			GET_FLAG(HoverFlagsForTooltipMouse);
			GET_FLAG(HoverFlagsForTooltipNav);

			if (key == "ScaleAllSizes"sv) {
				ctx.push_value(&ScaleAllSizes);
				return 1;
			}

		#undef GET_FLAG
		#undef GET_SCALAR
		#undef GET_VEC2
		#undef GET_COLOR

			return luaL_error(vm, "field '%s' does not exist", key.data());
		}
		static int setter(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const key = ctx.get_value<std::string_view>(2);

		#define SET_FLAG(name) if (key == (#name ""sv)) { self->name = static_cast<decltype(self->name)>(ctx.get_value<int32_t>(3)); return 0; } (void)0
		#define SET_SCALAR(name) if (key == (#name ""sv)) { self->name = ctx.get_value<decltype(self->name)>(3); return 0; } (void)0
		#define SET_VEC2(name) if (key == (#name ""sv)) { self->name = *ImVec2Binding::as(vm, 3); return 0; } (void)0
		#define SET_COLOR(name) if (key == ("Colors" #name ""sv)) { self->Colors[ImGuiCol_##name] = *ImVec4Binding::as(vm, 3); return 0; } (void)0

			SET_SCALAR(FontSizeBase);
			SET_SCALAR(FontScaleMain);
			SET_SCALAR(FontScaleDpi);

			SET_SCALAR(Alpha);
			SET_SCALAR(DisabledAlpha);
			SET_VEC2(WindowPadding);
			SET_SCALAR(WindowRounding);
			SET_SCALAR(WindowBorderSize);
			SET_SCALAR(WindowBorderHoverPadding);
			SET_VEC2(WindowMinSize);
			SET_VEC2(WindowTitleAlign);
			SET_FLAG(WindowMenuButtonPosition);
			SET_SCALAR(ChildRounding);
			SET_SCALAR(ChildBorderSize);
			SET_SCALAR(PopupRounding);
			SET_SCALAR(PopupBorderSize);
			SET_VEC2(FramePadding);
			SET_SCALAR(FrameRounding);
			SET_SCALAR(FrameBorderSize);
			SET_VEC2(ItemSpacing);
			SET_VEC2(ItemInnerSpacing);
			SET_VEC2(CellPadding);
			SET_VEC2(TouchExtraPadding);
			SET_SCALAR(IndentSpacing);
			SET_SCALAR(ColumnsMinSpacing);
			SET_SCALAR(ScrollbarSize);
			SET_SCALAR(ScrollbarRounding);
			SET_SCALAR(ScrollbarPadding);
			SET_SCALAR(GrabMinSize);
			SET_SCALAR(GrabRounding);
			SET_SCALAR(LogSliderDeadzone);
			SET_SCALAR(ImageBorderSize);
			SET_SCALAR(TabRounding);
			SET_SCALAR(TabBorderSize);
			SET_SCALAR(TabMinWidthBase);
			SET_SCALAR(TabMinWidthShrink);
			SET_SCALAR(TabCloseButtonMinWidthSelected);
			SET_SCALAR(TabCloseButtonMinWidthUnselected);
			SET_SCALAR(TabBarBorderSize);
			SET_SCALAR(TabBarOverlineSize);
			SET_SCALAR(TableAngledHeadersAngle);
			SET_VEC2(TableAngledHeadersTextAlign);
			SET_FLAG(TreeLinesFlags);
			SET_SCALAR(TreeLinesSize);
			SET_SCALAR(TreeLinesRounding);
			SET_FLAG(ColorButtonPosition);
			SET_VEC2(ButtonTextAlign);
			SET_VEC2(SelectableTextAlign);
			SET_SCALAR(SeparatorTextBorderSize);
			SET_VEC2(SeparatorTextAlign);
			SET_VEC2(SeparatorTextPadding);
			SET_VEC2(DisplayWindowPadding);
			SET_VEC2(DisplaySafeAreaPadding);
			SET_SCALAR(MouseCursorScale);
			SET_SCALAR(AntiAliasedLines);
			SET_SCALAR(AntiAliasedLinesUseTex);
			SET_SCALAR(AntiAliasedFill);
			SET_SCALAR(CurveTessellationTol);
			SET_SCALAR(CircleTessellationMaxError);

			if (key == "Colors"sv) {
				return luaL_error(vm, "not supported");
			}
			SET_COLOR(Text);
			SET_COLOR(TextDisabled);
			SET_COLOR(WindowBg);
			SET_COLOR(ChildBg);
			SET_COLOR(PopupBg);
			SET_COLOR(Border);
			SET_COLOR(BorderShadow);
			SET_COLOR(FrameBg);
			SET_COLOR(FrameBgHovered);
			SET_COLOR(FrameBgActive);
			SET_COLOR(TitleBg);
			SET_COLOR(TitleBgActive);
			SET_COLOR(TitleBgCollapsed);
			SET_COLOR(MenuBarBg);
			SET_COLOR(ScrollbarBg);
			SET_COLOR(ScrollbarGrab);
			SET_COLOR(ScrollbarGrabHovered);
			SET_COLOR(ScrollbarGrabActive);
			SET_COLOR(CheckMark);
			SET_COLOR(SliderGrab);
			SET_COLOR(SliderGrabActive);
			SET_COLOR(Button);
			SET_COLOR(ButtonHovered);
			SET_COLOR(ButtonActive);
			SET_COLOR(Header);
			SET_COLOR(HeaderHovered);
			SET_COLOR(HeaderActive);
			SET_COLOR(Separator);
			SET_COLOR(SeparatorHovered);
			SET_COLOR(SeparatorActive);
			SET_COLOR(ResizeGrip);
			SET_COLOR(ResizeGripHovered);
			SET_COLOR(ResizeGripActive);
			SET_COLOR(InputTextCursor);
			SET_COLOR(TabHovered);
			SET_COLOR(Tab);
			SET_COLOR(TabSelected);
			SET_COLOR(TabSelectedOverline);
			SET_COLOR(TabDimmed);
			SET_COLOR(TabDimmedSelected);
			SET_COLOR(TabDimmedSelectedOverline);
			SET_COLOR(PlotLines);
			SET_COLOR(PlotLinesHovered);
			SET_COLOR(PlotHistogram);
			SET_COLOR(PlotHistogramHovered);
			SET_COLOR(TableHeaderBg);
			SET_COLOR(TableBorderStrong);
			SET_COLOR(TableBorderLight);
			SET_COLOR(TableRowBg);
			SET_COLOR(TableRowBgAlt);
			SET_COLOR(TextLink);
			SET_COLOR(TextSelectedBg);
			SET_COLOR(TreeLines);
			SET_COLOR(DragDropTarget);
			SET_COLOR(NavCursor);
			SET_COLOR(NavWindowingHighlight);
			SET_COLOR(NavWindowingDimBg);
			SET_COLOR(ModalWindowDimBg);

			SET_SCALAR(HoverStationaryDelay);
			SET_SCALAR(HoverDelayShort);
			SET_SCALAR(HoverDelayNormal);
			SET_FLAG(HoverFlagsForTooltipMouse);
			SET_FLAG(HoverFlagsForTooltipNav);

			if (key == "ScaleAllSizes"sv) {
				return luaL_error(vm, "invalid operation");
			}

		#undef SET_FLAG
		#undef SET_SCALAR
		#undef SET_VEC2
		#undef SET_COLOR

			return luaL_error(vm, "field '%s' does not exist", key.data());
		}

		static int ScaleAllSizes(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const scale_factor = ctx.get_value<float>(2);
			self->ScaleAllSizes(scale_factor);
			return 0;
		}

		static int constructor(lua_State* const vm) {
			std::ignore = create(vm);
			return 1;
		}
	};

	bool ImGuiStyleBinding::is(lua_State* const vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	ImGuiStyle* ImGuiStyleBinding::as(lua_State* const vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<ImGuiStyleBinding>(index)->get();
	}
	ImGuiStyle* ImGuiStyleBinding::create(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<ImGuiStyleBinding>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->set(IM_NEW(ImGuiStyle)(), false);
		return self->get();
	}
	ImGuiStyle* ImGuiStyleBinding::create(lua_State* const vm, ImGuiStyle const& value) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<ImGuiStyleBinding>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->set(IM_NEW(ImGuiStyle)(value), false);
		return self->get();
	}
	ImGuiStyle* ImGuiStyleBinding::reference(lua_State* const vm, ImGuiStyle* const value) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<ImGuiStyleBinding>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->set(value, true);
		return self->get();
	}
	void ImGuiStyleBinding::registerClass(lua_State* vm) {
		ImGuiStyleColorsBinding::registerClass(vm);

		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const mt = ctx.create_metatable(class_name);
		ctx.set_map_value(mt, "__gc"sv, &ImGuiStyleWrapper::gc);
		ctx.set_map_value(mt, "__tostring"sv, &ImGuiStyleWrapper::toString);
		ctx.set_map_value(mt, "__index"sv, &ImGuiStyleWrapper::getter);
		ctx.set_map_value(mt, "__newindex"sv, &ImGuiStyleWrapper::setter);

		auto const m = ctx.push_module(module_name);
		ctx.set_map_value(m, "ImGuiStyle"sv, &ImGuiStyleWrapper::constructor);
	}
}
