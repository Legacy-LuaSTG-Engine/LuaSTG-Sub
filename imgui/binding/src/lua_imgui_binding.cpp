#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace imgui::binding {
    std::string_view const module_name{ "imgui" };

	void registerGlobal(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.create_module(module_name);
		ctx.set_map_value(m, "IMGUI_VERSION"sv, IMGUI_VERSION ""sv);
		ctx.set_map_value(m, "IMGUI_VERSION_NUM"sv, IMGUI_VERSION_NUM);
		ctx.set_map_value(m, "ImTextureID_Invalid"sv, /* ImTextureID_Invalid */ 0);
	}

	void registerAll(lua_State* const vm) {
		registerGlobal(vm);

		ImVec2Binding::registerClass(vm);

		ImVec4Binding::registerClass(vm);

		ImTextureRefBinding::registerClass(vm);

		registerImGuiContextCreationAndAccess(vm);
		registerImGuiMain(vm);
		registerImGuiDemoDebugInformation(vm);
		registerImGuiStyles(vm);
		registerImGuiWindows(vm);
		registerImGuiChildWindows(vm);
		registerImGuiWindowsUtilities(vm);
		registerImGuiWindowManipulation(vm);
		registerImGuiWindowsScrolling(vm);
		registerImGuiParametersStacksFont(vm);
		registerImGuiParametersStacksShared(vm);
		registerImGuiParametersStacksCurrentWindow(vm);
		registerImGuiStyleReadAccess(vm);
		registerImGuiLayoutCursorPositioning(vm);
		registerImGuiOtherLayoutFunctions(vm);
		registerImGuiIdStackScopes(vm);
		registerImGuiWidgetsText(vm);
		registerImGuiWidgetsMain(vm);
		registerImGuiWidgetsImages(vm);
		registerImGuiWidgetsComboBoxDropdown(vm);
		registerImGuiWidgetsDragSliders(vm);
		registerImGuiWidgetsRegularSliders(vm);
		registerImGuiWidgetsInputWithKeyboardInputText(vm);
		registerImGuiWidgetsInputWithKeyboard(vm);
		registerImGuiWidgetsColorEditorPicker(vm);
		registerImGuiWidgetsTrees(vm);
		registerImGuiWidgetsSelectables(vm);
		registerImGuiMultiSelectionSystem(vm);
		registerImGuiWidgetsListBoxes(vm);
		registerImGuiWidgetsDataPlotting(vm);
		registerImGuiWidgetsValueHelpers(vm);
		registerImGuiWidgetsMenus(vm);
		registerImGuiTooltips(vm);
		registerImGuiPopupsModals(vm);
		registerImGuiTables(vm);
		registerImGuiLegacyColumns(vm);
		registerImGuiTabBarsTabs(vm);
		registerImGuiLoggingCapture(vm);
		registerImGuiDragAndDrop(vm);
		registerImGuiDisabling(vm);
		registerImGuiClipping(vm);
		registerImGuiFocusActivation(vm);
		registerImGuiKeyboardGamepadNavigation(vm);
		registerImGuiOverlappingMode(vm);
		registerImGuiItemWidgetsUtilitiesAndQuery(vm);
		registerImGuiViewports(vm);
		registerImGuiBackgroundForegroundDrawLists(vm);
		registerImGuiMiscellaneousUtilities(vm);
		registerImGuiTextUtilities(vm);
		registerImGuiColorUtilities(vm);
		registerImGuiInputsUtilities(vm);
		registerImGuiClipboardUtilities(vm);
		registerImGuiSettingsIniUtilities(vm);
		registerImGuiDebugUtilities(vm);
		registerImGuiMemoryAllocators(vm);

		ImGuiStyleBinding::registerClass(vm);

		ImGuiTextBufferBinding::registerClass(vm);
	}
}
