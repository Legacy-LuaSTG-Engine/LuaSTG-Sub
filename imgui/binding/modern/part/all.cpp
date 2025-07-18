#include "lua_imgui_binding.hpp"

namespace imgui::binding {
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

		ImGuiStyleBinding::registerClass(vm);
	}
}
