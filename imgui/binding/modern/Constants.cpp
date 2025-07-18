#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

void imgui::binding::registerConstants(lua_State* const vm) {
	lua::stack_t const ctx(vm);
	auto const m = ctx.push_module(module_name); // imgui
	{
		auto const e = ctx.create_map(30);
		ctx.set_map_value(m, "ImGuiWindowFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiWindowFlags_None);
		ctx.set_map_value(e, "NoTitleBar"sv, ImGuiWindowFlags_NoTitleBar);
		ctx.set_map_value(e, "NoResize"sv, ImGuiWindowFlags_NoResize);
		ctx.set_map_value(e, "NoMove"sv, ImGuiWindowFlags_NoMove);
		ctx.set_map_value(e, "NoScrollbar"sv, ImGuiWindowFlags_NoScrollbar);
		ctx.set_map_value(e, "NoScrollWithMouse"sv, ImGuiWindowFlags_NoScrollWithMouse);
		ctx.set_map_value(e, "NoCollapse"sv, ImGuiWindowFlags_NoCollapse);
		ctx.set_map_value(e, "AlwaysAutoResize"sv, ImGuiWindowFlags_AlwaysAutoResize);
		ctx.set_map_value(e, "NoBackground"sv, ImGuiWindowFlags_NoBackground);
		ctx.set_map_value(e, "NoSavedSettings"sv, ImGuiWindowFlags_NoSavedSettings);
		ctx.set_map_value(e, "NoMouseInputs"sv, ImGuiWindowFlags_NoMouseInputs);
		ctx.set_map_value(e, "MenuBar"sv, ImGuiWindowFlags_MenuBar);
		ctx.set_map_value(e, "HorizontalScrollbar"sv, ImGuiWindowFlags_HorizontalScrollbar);
		ctx.set_map_value(e, "NoFocusOnAppearing"sv, ImGuiWindowFlags_NoFocusOnAppearing);
		ctx.set_map_value(e, "NoBringToFrontOnFocus"sv, ImGuiWindowFlags_NoBringToFrontOnFocus);
		ctx.set_map_value(e, "AlwaysVerticalScrollbar"sv, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		ctx.set_map_value(e, "AlwaysHorizontalScrollbar"sv, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
		ctx.set_map_value(e, "NoNavInputs"sv, ImGuiWindowFlags_NoNavInputs);
		ctx.set_map_value(e, "NoNavFocus"sv, ImGuiWindowFlags_NoNavFocus);
		ctx.set_map_value(e, "UnsavedDocument"sv, ImGuiWindowFlags_UnsavedDocument);
		ctx.set_map_value(e, "NoNav"sv, ImGuiWindowFlags_NoNav);
		ctx.set_map_value(e, "NoDecoration"sv, ImGuiWindowFlags_NoDecoration);
		ctx.set_map_value(e, "NoInputs"sv, ImGuiWindowFlags_NoInputs);
		ctx.set_map_value(e, "ChildWindow"sv, ImGuiWindowFlags_ChildWindow);
		ctx.set_map_value(e, "Tooltip"sv, ImGuiWindowFlags_Tooltip);
		ctx.set_map_value(e, "Popup"sv, ImGuiWindowFlags_Popup);
		ctx.set_map_value(e, "Modal"sv, ImGuiWindowFlags_Modal);
		ctx.set_map_value(e, "ChildMenu"sv, ImGuiWindowFlags_ChildMenu);
	}
	{
		auto const e = ctx.create_map(11);
		ctx.set_map_value(m, "ImGuiChildFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiChildFlags_None);
		ctx.set_map_value(e, "Borders"sv, ImGuiChildFlags_Borders);
		ctx.set_map_value(e, "AlwaysUseWindowPadding"sv, ImGuiChildFlags_AlwaysUseWindowPadding);
		ctx.set_map_value(e, "ResizeX"sv, ImGuiChildFlags_ResizeX);
		ctx.set_map_value(e, "ResizeY"sv, ImGuiChildFlags_ResizeY);
		ctx.set_map_value(e, "AutoResizeX"sv, ImGuiChildFlags_AutoResizeX);
		ctx.set_map_value(e, "AutoResizeY"sv, ImGuiChildFlags_AutoResizeY);
		ctx.set_map_value(e, "AlwaysAutoResize"sv, ImGuiChildFlags_AlwaysAutoResize);
		ctx.set_map_value(e, "FrameStyle"sv, ImGuiChildFlags_FrameStyle);
		ctx.set_map_value(e, "NavFlattened"sv, ImGuiChildFlags_NavFlattened);
	}
	{
		auto const e = ctx.create_map(7);
		ctx.set_map_value(m, "ImGuiItemFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiItemFlags_None);
		ctx.set_map_value(e, "NoTabStop"sv, ImGuiItemFlags_NoTabStop);
		ctx.set_map_value(e, "NoNav"sv, ImGuiItemFlags_NoNav);
		ctx.set_map_value(e, "NoNavDefaultFocus"sv, ImGuiItemFlags_NoNavDefaultFocus);
		ctx.set_map_value(e, "ButtonRepeat"sv, ImGuiItemFlags_ButtonRepeat);
		ctx.set_map_value(e, "AutoClosePopups"sv, ImGuiItemFlags_AutoClosePopups);
		ctx.set_map_value(e, "AllowDuplicateId"sv, ImGuiItemFlags_AllowDuplicateId);
	}
	{
		auto const e = ctx.create_map(25);
		ctx.set_map_value(m, "ImGuiInputTextFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiInputTextFlags_None);
		ctx.set_map_value(e, "CharsDecimal"sv, ImGuiInputTextFlags_CharsDecimal);
		ctx.set_map_value(e, "CharsHexadecimal"sv, ImGuiInputTextFlags_CharsHexadecimal);
		ctx.set_map_value(e, "CharsScientific"sv, ImGuiInputTextFlags_CharsScientific);
		ctx.set_map_value(e, "CharsUppercase"sv, ImGuiInputTextFlags_CharsUppercase);
		ctx.set_map_value(e, "CharsNoBlank"sv, ImGuiInputTextFlags_CharsNoBlank);
		ctx.set_map_value(e, "AllowTabInput"sv, ImGuiInputTextFlags_AllowTabInput);
		ctx.set_map_value(e, "EnterReturnsTrue"sv, ImGuiInputTextFlags_EnterReturnsTrue);
		ctx.set_map_value(e, "EscapeClearsAll"sv, ImGuiInputTextFlags_EscapeClearsAll);
		ctx.set_map_value(e, "CtrlEnterForNewLine"sv, ImGuiInputTextFlags_CtrlEnterForNewLine);
		ctx.set_map_value(e, "ReadOnly"sv, ImGuiInputTextFlags_ReadOnly);
		ctx.set_map_value(e, "Password"sv, ImGuiInputTextFlags_Password);
		ctx.set_map_value(e, "AlwaysOverwrite"sv, ImGuiInputTextFlags_AlwaysOverwrite);
		ctx.set_map_value(e, "AutoSelectAll"sv, ImGuiInputTextFlags_AutoSelectAll);
		ctx.set_map_value(e, "ParseEmptyRefVal"sv, ImGuiInputTextFlags_ParseEmptyRefVal);
		ctx.set_map_value(e, "DisplayEmptyRefVal"sv, ImGuiInputTextFlags_DisplayEmptyRefVal);
		ctx.set_map_value(e, "NoHorizontalScroll"sv, ImGuiInputTextFlags_NoHorizontalScroll);
		ctx.set_map_value(e, "NoUndoRedo"sv, ImGuiInputTextFlags_NoUndoRedo);
		ctx.set_map_value(e, "ElideLeft"sv, ImGuiInputTextFlags_ElideLeft);
		ctx.set_map_value(e, "CallbackCompletion"sv, ImGuiInputTextFlags_CallbackCompletion);
		ctx.set_map_value(e, "CallbackHistory"sv, ImGuiInputTextFlags_CallbackHistory);
		ctx.set_map_value(e, "CallbackAlways"sv, ImGuiInputTextFlags_CallbackAlways);
		ctx.set_map_value(e, "CallbackCharFilter"sv, ImGuiInputTextFlags_CallbackCharFilter);
		ctx.set_map_value(e, "CallbackResize"sv, ImGuiInputTextFlags_CallbackResize);
		ctx.set_map_value(e, "CallbackEdit"sv, ImGuiInputTextFlags_CallbackEdit);
	}
	{
		auto const e = ctx.create_map(25);
		ctx.set_map_value(m, "ImGuiTreeNodeFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiTreeNodeFlags_None);
		ctx.set_map_value(e, "Selected"sv, ImGuiTreeNodeFlags_Selected);
		ctx.set_map_value(e, "Framed"sv, ImGuiTreeNodeFlags_Framed);
		ctx.set_map_value(e, "AllowOverlap"sv, ImGuiTreeNodeFlags_AllowOverlap);
		ctx.set_map_value(e, "NoTreePushOnOpen"sv, ImGuiTreeNodeFlags_NoTreePushOnOpen);
		ctx.set_map_value(e, "NoAutoOpenOnLog"sv, ImGuiTreeNodeFlags_NoAutoOpenOnLog);
		ctx.set_map_value(e, "DefaultOpen"sv, ImGuiTreeNodeFlags_DefaultOpen);
		ctx.set_map_value(e, "OpenOnDoubleClick"sv, ImGuiTreeNodeFlags_OpenOnDoubleClick);
		ctx.set_map_value(e, "OpenOnArrow"sv, ImGuiTreeNodeFlags_OpenOnArrow);
		ctx.set_map_value(e, "Leaf"sv, ImGuiTreeNodeFlags_Leaf);
		ctx.set_map_value(e, "Bullet"sv, ImGuiTreeNodeFlags_Bullet);
		ctx.set_map_value(e, "FramePadding"sv, ImGuiTreeNodeFlags_FramePadding);
		ctx.set_map_value(e, "SpanAvailWidth"sv, ImGuiTreeNodeFlags_SpanAvailWidth);
		ctx.set_map_value(e, "SpanFullWidth"sv, ImGuiTreeNodeFlags_SpanFullWidth);
		ctx.set_map_value(e, "SpanLabelWidth"sv, ImGuiTreeNodeFlags_SpanLabelWidth);
		ctx.set_map_value(e, "SpanAllColumns"sv, ImGuiTreeNodeFlags_SpanAllColumns);
		ctx.set_map_value(e, "LabelSpanAllColumns"sv, ImGuiTreeNodeFlags_LabelSpanAllColumns);
		ctx.set_map_value(e, "NavLeftJumpsToParent"sv, ImGuiTreeNodeFlags_NavLeftJumpsToParent);
		ctx.set_map_value(e, "CollapsingHeader"sv, ImGuiTreeNodeFlags_CollapsingHeader);
		ctx.set_map_value(e, "DrawLinesNone"sv, ImGuiTreeNodeFlags_DrawLinesNone);
		ctx.set_map_value(e, "DrawLinesFull"sv, ImGuiTreeNodeFlags_DrawLinesFull);
		ctx.set_map_value(e, "DrawLinesToNodes"sv, ImGuiTreeNodeFlags_DrawLinesToNodes);
	}
	{
		auto const e = ctx.create_map(12);
		ctx.set_map_value(m, "ImGuiPopupFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiPopupFlags_None);
		ctx.set_map_value(e, "MouseButtonLeft"sv, ImGuiPopupFlags_MouseButtonLeft);
		ctx.set_map_value(e, "MouseButtonRight"sv, ImGuiPopupFlags_MouseButtonRight);
		ctx.set_map_value(e, "MouseButtonMiddle"sv, ImGuiPopupFlags_MouseButtonMiddle);
		ctx.set_map_value(e, "MouseButtonMask_"sv, ImGuiPopupFlags_MouseButtonMask_);
		ctx.set_map_value(e, "MouseButtonDefault_"sv, ImGuiPopupFlags_MouseButtonDefault_);
		ctx.set_map_value(e, "NoReopen"sv, ImGuiPopupFlags_NoReopen);
		ctx.set_map_value(e, "NoOpenOverExistingPopup"sv, ImGuiPopupFlags_NoOpenOverExistingPopup);
		ctx.set_map_value(e, "NoOpenOverItems"sv, ImGuiPopupFlags_NoOpenOverItems);
		ctx.set_map_value(e, "AnyPopupId"sv, ImGuiPopupFlags_AnyPopupId);
		ctx.set_map_value(e, "AnyPopupLevel"sv, ImGuiPopupFlags_AnyPopupLevel);
		ctx.set_map_value(e, "AnyPopup"sv, ImGuiPopupFlags_AnyPopup);
	}
	{
		auto const e = ctx.create_map(9);
		ctx.set_map_value(m, "ImGuiSelectableFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiSelectableFlags_None);
		ctx.set_map_value(e, "NoAutoClosePopups"sv, ImGuiSelectableFlags_NoAutoClosePopups);
		ctx.set_map_value(e, "SpanAllColumns"sv, ImGuiSelectableFlags_SpanAllColumns);
		ctx.set_map_value(e, "AllowDoubleClick"sv, ImGuiSelectableFlags_AllowDoubleClick);
		ctx.set_map_value(e, "Disabled"sv, ImGuiSelectableFlags_Disabled);
		ctx.set_map_value(e, "AllowOverlap"sv, ImGuiSelectableFlags_AllowOverlap);
		ctx.set_map_value(e, "Highlight"sv, ImGuiSelectableFlags_Highlight);
	}
	{
		auto const e = ctx.create_map(10);
		ctx.set_map_value(m, "ImGuiComboFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiComboFlags_None);
		ctx.set_map_value(e, "PopupAlignLeft"sv, ImGuiComboFlags_PopupAlignLeft);
		ctx.set_map_value(e, "HeightSmall"sv, ImGuiComboFlags_HeightSmall);
		ctx.set_map_value(e, "HeightRegular"sv, ImGuiComboFlags_HeightRegular);
		ctx.set_map_value(e, "HeightLarge"sv, ImGuiComboFlags_HeightLarge);
		ctx.set_map_value(e, "HeightLargest"sv, ImGuiComboFlags_HeightLargest);
		ctx.set_map_value(e, "NoArrowButton"sv, ImGuiComboFlags_NoArrowButton);
		ctx.set_map_value(e, "NoPreview"sv, ImGuiComboFlags_NoPreview);
		ctx.set_map_value(e, "WidthFitPreview"sv, ImGuiComboFlags_WidthFitPreview);
		ctx.set_map_value(e, "HeightMask_"sv, ImGuiComboFlags_HeightMask_);
	}
	{
		auto const e = ctx.create_map(12);
		ctx.set_map_value(m, "ImGuiTabBarFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiTabBarFlags_None);
		ctx.set_map_value(e, "Reorderable"sv, ImGuiTabBarFlags_Reorderable);
		ctx.set_map_value(e, "AutoSelectNewTabs"sv, ImGuiTabBarFlags_AutoSelectNewTabs);
		ctx.set_map_value(e, "TabListPopupButton"sv, ImGuiTabBarFlags_TabListPopupButton);
		ctx.set_map_value(e, "NoCloseWithMiddleMouseButton"sv, ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);
		ctx.set_map_value(e, "NoTabListScrollingButtons"sv, ImGuiTabBarFlags_NoTabListScrollingButtons);
		ctx.set_map_value(e, "NoTooltip"sv, ImGuiTabBarFlags_NoTooltip);
		ctx.set_map_value(e, "DrawSelectedOverline"sv, ImGuiTabBarFlags_DrawSelectedOverline);
		ctx.set_map_value(e, "FittingPolicyResizeDown"sv, ImGuiTabBarFlags_FittingPolicyResizeDown);
		ctx.set_map_value(e, "FittingPolicyScroll"sv, ImGuiTabBarFlags_FittingPolicyScroll);
		ctx.set_map_value(e, "FittingPolicyMask_"sv, ImGuiTabBarFlags_FittingPolicyMask_);
		ctx.set_map_value(e, "FittingPolicyDefault_"sv, ImGuiTabBarFlags_FittingPolicyDefault_);
	}
	{
		auto const e = ctx.create_map(10);
		ctx.set_map_value(m, "ImGuiTabItemFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiTabItemFlags_None);
		ctx.set_map_value(e, "UnsavedDocument"sv, ImGuiTabItemFlags_UnsavedDocument);
		ctx.set_map_value(e, "SetSelected"sv, ImGuiTabItemFlags_SetSelected);
		ctx.set_map_value(e, "NoCloseWithMiddleMouseButton"sv, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton);
		ctx.set_map_value(e, "NoPushId"sv, ImGuiTabItemFlags_NoPushId);
		ctx.set_map_value(e, "NoTooltip"sv, ImGuiTabItemFlags_NoTooltip);
		ctx.set_map_value(e, "NoReorder"sv, ImGuiTabItemFlags_NoReorder);
		ctx.set_map_value(e, "Leading"sv, ImGuiTabItemFlags_Leading);
		ctx.set_map_value(e, "Trailing"sv, ImGuiTabItemFlags_Trailing);
		ctx.set_map_value(e, "NoAssumedClosure"sv, ImGuiTabItemFlags_NoAssumedClosure);
	}
	{
		auto const e = ctx.create_map(6);
		ctx.set_map_value(m, "ImGuiFocusedFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiFocusedFlags_None);
		ctx.set_map_value(e, "ChildWindows"sv, ImGuiFocusedFlags_ChildWindows);
		ctx.set_map_value(e, "RootWindow"sv, ImGuiFocusedFlags_RootWindow);
		ctx.set_map_value(e, "AnyWindow"sv, ImGuiFocusedFlags_AnyWindow);
		ctx.set_map_value(e, "NoPopupHierarchy"sv, ImGuiFocusedFlags_NoPopupHierarchy);
		ctx.set_map_value(e, "RootAndChildWindows"sv, ImGuiFocusedFlags_RootAndChildWindows);
	}
	{
		auto const e = ctx.create_map(20);
		ctx.set_map_value(m, "ImGuiHoveredFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiHoveredFlags_None);
		ctx.set_map_value(e, "ChildWindows"sv, ImGuiHoveredFlags_ChildWindows);
		ctx.set_map_value(e, "RootWindow"sv, ImGuiHoveredFlags_RootWindow);
		ctx.set_map_value(e, "AnyWindow"sv, ImGuiHoveredFlags_AnyWindow);
		ctx.set_map_value(e, "NoPopupHierarchy"sv, ImGuiHoveredFlags_NoPopupHierarchy);
		ctx.set_map_value(e, "AllowWhenBlockedByPopup"sv, ImGuiHoveredFlags_AllowWhenBlockedByPopup);
		ctx.set_map_value(e, "AllowWhenBlockedByActiveItem"sv, ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
		ctx.set_map_value(e, "AllowWhenOverlappedByItem"sv, ImGuiHoveredFlags_AllowWhenOverlappedByItem);
		ctx.set_map_value(e, "AllowWhenOverlappedByWindow"sv, ImGuiHoveredFlags_AllowWhenOverlappedByWindow);
		ctx.set_map_value(e, "AllowWhenDisabled"sv, ImGuiHoveredFlags_AllowWhenDisabled);
		ctx.set_map_value(e, "NoNavOverride"sv, ImGuiHoveredFlags_NoNavOverride);
		ctx.set_map_value(e, "AllowWhenOverlapped"sv, ImGuiHoveredFlags_AllowWhenOverlapped);
		ctx.set_map_value(e, "RectOnly"sv, ImGuiHoveredFlags_RectOnly);
		ctx.set_map_value(e, "RootAndChildWindows"sv, ImGuiHoveredFlags_RootAndChildWindows);
		ctx.set_map_value(e, "ForTooltip"sv, ImGuiHoveredFlags_ForTooltip);
		ctx.set_map_value(e, "Stationary"sv, ImGuiHoveredFlags_Stationary);
		ctx.set_map_value(e, "DelayNone"sv, ImGuiHoveredFlags_DelayNone);
		ctx.set_map_value(e, "DelayShort"sv, ImGuiHoveredFlags_DelayShort);
		ctx.set_map_value(e, "DelayNormal"sv, ImGuiHoveredFlags_DelayNormal);
		ctx.set_map_value(e, "NoSharedDelay"sv, ImGuiHoveredFlags_NoSharedDelay);
	}
	{
		auto const e = ctx.create_map(14);
		ctx.set_map_value(m, "ImGuiDragDropFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiDragDropFlags_None);
		ctx.set_map_value(e, "SourceNoPreviewTooltip"sv, ImGuiDragDropFlags_SourceNoPreviewTooltip);
		ctx.set_map_value(e, "SourceNoDisableHover"sv, ImGuiDragDropFlags_SourceNoDisableHover);
		ctx.set_map_value(e, "SourceNoHoldToOpenOthers"sv, ImGuiDragDropFlags_SourceNoHoldToOpenOthers);
		ctx.set_map_value(e, "SourceAllowNullID"sv, ImGuiDragDropFlags_SourceAllowNullID);
		ctx.set_map_value(e, "SourceExtern"sv, ImGuiDragDropFlags_SourceExtern);
		ctx.set_map_value(e, "PayloadAutoExpire"sv, ImGuiDragDropFlags_PayloadAutoExpire);
		ctx.set_map_value(e, "PayloadNoCrossContext"sv, ImGuiDragDropFlags_PayloadNoCrossContext);
		ctx.set_map_value(e, "PayloadNoCrossProcess"sv, ImGuiDragDropFlags_PayloadNoCrossProcess);
		ctx.set_map_value(e, "AcceptBeforeDelivery"sv, ImGuiDragDropFlags_AcceptBeforeDelivery);
		ctx.set_map_value(e, "AcceptNoDrawDefaultRect"sv, ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
		ctx.set_map_value(e, "AcceptNoPreviewTooltip"sv, ImGuiDragDropFlags_AcceptNoPreviewTooltip);
		ctx.set_map_value(e, "AcceptPeekOnly"sv, ImGuiDragDropFlags_AcceptPeekOnly);
	}
	{
		auto const e = ctx.create_map(13);
		ctx.set_map_value(m, "ImGuiDataType"sv, e);
		ctx.set_map_value(e, "S8"sv, ImGuiDataType_S8);
		ctx.set_map_value(e, "U8"sv, ImGuiDataType_U8);
		ctx.set_map_value(e, "S16"sv, ImGuiDataType_S16);
		ctx.set_map_value(e, "U16"sv, ImGuiDataType_U16);
		ctx.set_map_value(e, "S32"sv, ImGuiDataType_S32);
		ctx.set_map_value(e, "U32"sv, ImGuiDataType_U32);
		ctx.set_map_value(e, "S64"sv, ImGuiDataType_S64);
		ctx.set_map_value(e, "U64"sv, ImGuiDataType_U64);
		ctx.set_map_value(e, "Float"sv, ImGuiDataType_Float);
		ctx.set_map_value(e, "Double"sv, ImGuiDataType_Double);
		ctx.set_map_value(e, "Bool"sv, ImGuiDataType_Bool);
		ctx.set_map_value(e, "String"sv, ImGuiDataType_String);
		ctx.set_map_value(e, "COUNT"sv, ImGuiDataType_COUNT);
	}
	{
		auto const e = ctx.create_map(6);
		ctx.set_map_value(m, "ImGuiDir"sv, e);
		ctx.set_map_value(e, "_None"sv, ImGuiDir_None);
		ctx.set_map_value(e, "_Left"sv, ImGuiDir_Left);
		ctx.set_map_value(e, "_Right"sv, ImGuiDir_Right);
		ctx.set_map_value(e, "_Up"sv, ImGuiDir_Up);
		ctx.set_map_value(e, "_Down"sv, ImGuiDir_Down);
		ctx.set_map_value(e, "_COUNT"sv, ImGuiDir_COUNT);
	}
	{
		auto const e = ctx.create_map(3);
		ctx.set_map_value(m, "ImGuiSortDirection"sv, e);
		ctx.set_map_value(e, "_None"sv, ImGuiSortDirection_None);
		ctx.set_map_value(e, "_Ascending"sv, ImGuiSortDirection_Ascending);
		ctx.set_map_value(e, "_Descending"sv, ImGuiSortDirection_Descending);
	}
	{
		auto const e = ctx.create_map(171);
		ctx.set_map_value(m, "ImGuiKey"sv, e);
		ctx.set_map_value(e, "_None"sv, ImGuiKey_None);
		ctx.set_map_value(e, "_NamedKey_BEGIN"sv, ImGuiKey_NamedKey_BEGIN);
		ctx.set_map_value(e, "_Tab"sv, ImGuiKey_Tab);
		ctx.set_map_value(e, "_LeftArrow"sv, ImGuiKey_LeftArrow);
		ctx.set_map_value(e, "_RightArrow"sv, ImGuiKey_RightArrow);
		ctx.set_map_value(e, "_UpArrow"sv, ImGuiKey_UpArrow);
		ctx.set_map_value(e, "_DownArrow"sv, ImGuiKey_DownArrow);
		ctx.set_map_value(e, "_PageUp"sv, ImGuiKey_PageUp);
		ctx.set_map_value(e, "_PageDown"sv, ImGuiKey_PageDown);
		ctx.set_map_value(e, "_Home"sv, ImGuiKey_Home);
		ctx.set_map_value(e, "_End"sv, ImGuiKey_End);
		ctx.set_map_value(e, "_Insert"sv, ImGuiKey_Insert);
		ctx.set_map_value(e, "_Delete"sv, ImGuiKey_Delete);
		ctx.set_map_value(e, "_Backspace"sv, ImGuiKey_Backspace);
		ctx.set_map_value(e, "_Space"sv, ImGuiKey_Space);
		ctx.set_map_value(e, "_Enter"sv, ImGuiKey_Enter);
		ctx.set_map_value(e, "_Escape"sv, ImGuiKey_Escape);
		ctx.set_map_value(e, "_LeftCtrl"sv, ImGuiKey_LeftCtrl);
		ctx.set_map_value(e, "_LeftShift"sv, ImGuiKey_LeftShift);
		ctx.set_map_value(e, "_LeftAlt"sv, ImGuiKey_LeftAlt);
		ctx.set_map_value(e, "_LeftSuper"sv, ImGuiKey_LeftSuper);
		ctx.set_map_value(e, "_RightCtrl"sv, ImGuiKey_RightCtrl);
		ctx.set_map_value(e, "_RightShift"sv, ImGuiKey_RightShift);
		ctx.set_map_value(e, "_RightAlt"sv, ImGuiKey_RightAlt);
		ctx.set_map_value(e, "_RightSuper"sv, ImGuiKey_RightSuper);
		ctx.set_map_value(e, "_Menu"sv, ImGuiKey_Menu);
		ctx.set_map_value(e, "_0"sv, ImGuiKey_0);
		ctx.set_map_value(e, "_1"sv, ImGuiKey_1);
		ctx.set_map_value(e, "_2"sv, ImGuiKey_2);
		ctx.set_map_value(e, "_3"sv, ImGuiKey_3);
		ctx.set_map_value(e, "_4"sv, ImGuiKey_4);
		ctx.set_map_value(e, "_5"sv, ImGuiKey_5);
		ctx.set_map_value(e, "_6"sv, ImGuiKey_6);
		ctx.set_map_value(e, "_7"sv, ImGuiKey_7);
		ctx.set_map_value(e, "_8"sv, ImGuiKey_8);
		ctx.set_map_value(e, "_9"sv, ImGuiKey_9);
		ctx.set_map_value(e, "_A"sv, ImGuiKey_A);
		ctx.set_map_value(e, "_B"sv, ImGuiKey_B);
		ctx.set_map_value(e, "_C"sv, ImGuiKey_C);
		ctx.set_map_value(e, "_D"sv, ImGuiKey_D);
		ctx.set_map_value(e, "_E"sv, ImGuiKey_E);
		ctx.set_map_value(e, "_F"sv, ImGuiKey_F);
		ctx.set_map_value(e, "_G"sv, ImGuiKey_G);
		ctx.set_map_value(e, "_H"sv, ImGuiKey_H);
		ctx.set_map_value(e, "_I"sv, ImGuiKey_I);
		ctx.set_map_value(e, "_J"sv, ImGuiKey_J);
		ctx.set_map_value(e, "_K"sv, ImGuiKey_K);
		ctx.set_map_value(e, "_L"sv, ImGuiKey_L);
		ctx.set_map_value(e, "_M"sv, ImGuiKey_M);
		ctx.set_map_value(e, "_N"sv, ImGuiKey_N);
		ctx.set_map_value(e, "_O"sv, ImGuiKey_O);
		ctx.set_map_value(e, "_P"sv, ImGuiKey_P);
		ctx.set_map_value(e, "_Q"sv, ImGuiKey_Q);
		ctx.set_map_value(e, "_R"sv, ImGuiKey_R);
		ctx.set_map_value(e, "_S"sv, ImGuiKey_S);
		ctx.set_map_value(e, "_T"sv, ImGuiKey_T);
		ctx.set_map_value(e, "_U"sv, ImGuiKey_U);
		ctx.set_map_value(e, "_V"sv, ImGuiKey_V);
		ctx.set_map_value(e, "_W"sv, ImGuiKey_W);
		ctx.set_map_value(e, "_X"sv, ImGuiKey_X);
		ctx.set_map_value(e, "_Y"sv, ImGuiKey_Y);
		ctx.set_map_value(e, "_Z"sv, ImGuiKey_Z);
		ctx.set_map_value(e, "_F1"sv, ImGuiKey_F1);
		ctx.set_map_value(e, "_F2"sv, ImGuiKey_F2);
		ctx.set_map_value(e, "_F3"sv, ImGuiKey_F3);
		ctx.set_map_value(e, "_F4"sv, ImGuiKey_F4);
		ctx.set_map_value(e, "_F5"sv, ImGuiKey_F5);
		ctx.set_map_value(e, "_F6"sv, ImGuiKey_F6);
		ctx.set_map_value(e, "_F7"sv, ImGuiKey_F7);
		ctx.set_map_value(e, "_F8"sv, ImGuiKey_F8);
		ctx.set_map_value(e, "_F9"sv, ImGuiKey_F9);
		ctx.set_map_value(e, "_F10"sv, ImGuiKey_F10);
		ctx.set_map_value(e, "_F11"sv, ImGuiKey_F11);
		ctx.set_map_value(e, "_F12"sv, ImGuiKey_F12);
		ctx.set_map_value(e, "_F13"sv, ImGuiKey_F13);
		ctx.set_map_value(e, "_F14"sv, ImGuiKey_F14);
		ctx.set_map_value(e, "_F15"sv, ImGuiKey_F15);
		ctx.set_map_value(e, "_F16"sv, ImGuiKey_F16);
		ctx.set_map_value(e, "_F17"sv, ImGuiKey_F17);
		ctx.set_map_value(e, "_F18"sv, ImGuiKey_F18);
		ctx.set_map_value(e, "_F19"sv, ImGuiKey_F19);
		ctx.set_map_value(e, "_F20"sv, ImGuiKey_F20);
		ctx.set_map_value(e, "_F21"sv, ImGuiKey_F21);
		ctx.set_map_value(e, "_F22"sv, ImGuiKey_F22);
		ctx.set_map_value(e, "_F23"sv, ImGuiKey_F23);
		ctx.set_map_value(e, "_F24"sv, ImGuiKey_F24);
		ctx.set_map_value(e, "_Apostrophe"sv, ImGuiKey_Apostrophe);
		ctx.set_map_value(e, "_Comma"sv, ImGuiKey_Comma);
		ctx.set_map_value(e, "_Minus"sv, ImGuiKey_Minus);
		ctx.set_map_value(e, "_Period"sv, ImGuiKey_Period);
		ctx.set_map_value(e, "_Slash"sv, ImGuiKey_Slash);
		ctx.set_map_value(e, "_Semicolon"sv, ImGuiKey_Semicolon);
		ctx.set_map_value(e, "_Equal"sv, ImGuiKey_Equal);
		ctx.set_map_value(e, "_LeftBracket"sv, ImGuiKey_LeftBracket);
		ctx.set_map_value(e, "_Backslash"sv, ImGuiKey_Backslash);
		ctx.set_map_value(e, "_RightBracket"sv, ImGuiKey_RightBracket);
		ctx.set_map_value(e, "_GraveAccent"sv, ImGuiKey_GraveAccent);
		ctx.set_map_value(e, "_CapsLock"sv, ImGuiKey_CapsLock);
		ctx.set_map_value(e, "_ScrollLock"sv, ImGuiKey_ScrollLock);
		ctx.set_map_value(e, "_NumLock"sv, ImGuiKey_NumLock);
		ctx.set_map_value(e, "_PrintScreen"sv, ImGuiKey_PrintScreen);
		ctx.set_map_value(e, "_Pause"sv, ImGuiKey_Pause);
		ctx.set_map_value(e, "_Keypad0"sv, ImGuiKey_Keypad0);
		ctx.set_map_value(e, "_Keypad1"sv, ImGuiKey_Keypad1);
		ctx.set_map_value(e, "_Keypad2"sv, ImGuiKey_Keypad2);
		ctx.set_map_value(e, "_Keypad3"sv, ImGuiKey_Keypad3);
		ctx.set_map_value(e, "_Keypad4"sv, ImGuiKey_Keypad4);
		ctx.set_map_value(e, "_Keypad5"sv, ImGuiKey_Keypad5);
		ctx.set_map_value(e, "_Keypad6"sv, ImGuiKey_Keypad6);
		ctx.set_map_value(e, "_Keypad7"sv, ImGuiKey_Keypad7);
		ctx.set_map_value(e, "_Keypad8"sv, ImGuiKey_Keypad8);
		ctx.set_map_value(e, "_Keypad9"sv, ImGuiKey_Keypad9);
		ctx.set_map_value(e, "_KeypadDecimal"sv, ImGuiKey_KeypadDecimal);
		ctx.set_map_value(e, "_KeypadDivide"sv, ImGuiKey_KeypadDivide);
		ctx.set_map_value(e, "_KeypadMultiply"sv, ImGuiKey_KeypadMultiply);
		ctx.set_map_value(e, "_KeypadSubtract"sv, ImGuiKey_KeypadSubtract);
		ctx.set_map_value(e, "_KeypadAdd"sv, ImGuiKey_KeypadAdd);
		ctx.set_map_value(e, "_KeypadEnter"sv, ImGuiKey_KeypadEnter);
		ctx.set_map_value(e, "_KeypadEqual"sv, ImGuiKey_KeypadEqual);
		ctx.set_map_value(e, "_AppBack"sv, ImGuiKey_AppBack);
		ctx.set_map_value(e, "_AppForward"sv, ImGuiKey_AppForward);
		ctx.set_map_value(e, "_Oem102"sv, ImGuiKey_Oem102);
		ctx.set_map_value(e, "_GamepadStart"sv, ImGuiKey_GamepadStart);
		ctx.set_map_value(e, "_GamepadBack"sv, ImGuiKey_GamepadBack);
		ctx.set_map_value(e, "_GamepadFaceLeft"sv, ImGuiKey_GamepadFaceLeft);
		ctx.set_map_value(e, "_GamepadFaceRight"sv, ImGuiKey_GamepadFaceRight);
		ctx.set_map_value(e, "_GamepadFaceUp"sv, ImGuiKey_GamepadFaceUp);
		ctx.set_map_value(e, "_GamepadFaceDown"sv, ImGuiKey_GamepadFaceDown);
		ctx.set_map_value(e, "_GamepadDpadLeft"sv, ImGuiKey_GamepadDpadLeft);
		ctx.set_map_value(e, "_GamepadDpadRight"sv, ImGuiKey_GamepadDpadRight);
		ctx.set_map_value(e, "_GamepadDpadUp"sv, ImGuiKey_GamepadDpadUp);
		ctx.set_map_value(e, "_GamepadDpadDown"sv, ImGuiKey_GamepadDpadDown);
		ctx.set_map_value(e, "_GamepadL1"sv, ImGuiKey_GamepadL1);
		ctx.set_map_value(e, "_GamepadR1"sv, ImGuiKey_GamepadR1);
		ctx.set_map_value(e, "_GamepadL2"sv, ImGuiKey_GamepadL2);
		ctx.set_map_value(e, "_GamepadR2"sv, ImGuiKey_GamepadR2);
		ctx.set_map_value(e, "_GamepadL3"sv, ImGuiKey_GamepadL3);
		ctx.set_map_value(e, "_GamepadR3"sv, ImGuiKey_GamepadR3);
		ctx.set_map_value(e, "_GamepadLStickLeft"sv, ImGuiKey_GamepadLStickLeft);
		ctx.set_map_value(e, "_GamepadLStickRight"sv, ImGuiKey_GamepadLStickRight);
		ctx.set_map_value(e, "_GamepadLStickUp"sv, ImGuiKey_GamepadLStickUp);
		ctx.set_map_value(e, "_GamepadLStickDown"sv, ImGuiKey_GamepadLStickDown);
		ctx.set_map_value(e, "_GamepadRStickLeft"sv, ImGuiKey_GamepadRStickLeft);
		ctx.set_map_value(e, "_GamepadRStickRight"sv, ImGuiKey_GamepadRStickRight);
		ctx.set_map_value(e, "_GamepadRStickUp"sv, ImGuiKey_GamepadRStickUp);
		ctx.set_map_value(e, "_GamepadRStickDown"sv, ImGuiKey_GamepadRStickDown);
		ctx.set_map_value(e, "_MouseLeft"sv, ImGuiKey_MouseLeft);
		ctx.set_map_value(e, "_MouseRight"sv, ImGuiKey_MouseRight);
		ctx.set_map_value(e, "_MouseMiddle"sv, ImGuiKey_MouseMiddle);
		ctx.set_map_value(e, "_MouseX1"sv, ImGuiKey_MouseX1);
		ctx.set_map_value(e, "_MouseX2"sv, ImGuiKey_MouseX2);
		ctx.set_map_value(e, "_MouseWheelX"sv, ImGuiKey_MouseWheelX);
		ctx.set_map_value(e, "_MouseWheelY"sv, ImGuiKey_MouseWheelY);
		ctx.set_map_value(e, "_ReservedForModCtrl"sv, ImGuiKey_ReservedForModCtrl);
		ctx.set_map_value(e, "_ReservedForModShift"sv, ImGuiKey_ReservedForModShift);
		ctx.set_map_value(e, "_ReservedForModAlt"sv, ImGuiKey_ReservedForModAlt);
		ctx.set_map_value(e, "_ReservedForModSuper"sv, ImGuiKey_ReservedForModSuper);
		ctx.set_map_value(e, "_NamedKey_END"sv, ImGuiKey_NamedKey_END);
		ctx.set_map_value(e, "_NamedKey_COUNT"sv, ImGuiKey_NamedKey_COUNT);
		ctx.set_map_value(e, "_None"sv, ImGuiMod_None);
		ctx.set_map_value(e, "_Ctrl"sv, ImGuiMod_Ctrl);
		ctx.set_map_value(e, "_Shift"sv, ImGuiMod_Shift);
		ctx.set_map_value(e, "_Alt"sv, ImGuiMod_Alt);
		ctx.set_map_value(e, "_Super"sv, ImGuiMod_Super);
		ctx.set_map_value(e, "_Mask_"sv, ImGuiMod_Mask_);
	}
	{
		auto const e = ctx.create_map(11);
		ctx.set_map_value(m, "ImGuiInputFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiInputFlags_None);
		ctx.set_map_value(e, "Repeat"sv, ImGuiInputFlags_Repeat);
		ctx.set_map_value(e, "RouteActive"sv, ImGuiInputFlags_RouteActive);
		ctx.set_map_value(e, "RouteFocused"sv, ImGuiInputFlags_RouteFocused);
		ctx.set_map_value(e, "RouteGlobal"sv, ImGuiInputFlags_RouteGlobal);
		ctx.set_map_value(e, "RouteAlways"sv, ImGuiInputFlags_RouteAlways);
		ctx.set_map_value(e, "RouteOverFocused"sv, ImGuiInputFlags_RouteOverFocused);
		ctx.set_map_value(e, "RouteOverActive"sv, ImGuiInputFlags_RouteOverActive);
		ctx.set_map_value(e, "RouteUnlessBgFocused"sv, ImGuiInputFlags_RouteUnlessBgFocused);
		ctx.set_map_value(e, "RouteFromRootWindow"sv, ImGuiInputFlags_RouteFromRootWindow);
		ctx.set_map_value(e, "Tooltip"sv, ImGuiInputFlags_Tooltip);
	}
	{
		auto const e = ctx.create_map(10);
		ctx.set_map_value(m, "ImGuiConfigFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiConfigFlags_None);
		ctx.set_map_value(e, "NavEnableKeyboard"sv, ImGuiConfigFlags_NavEnableKeyboard);
		ctx.set_map_value(e, "NavEnableGamepad"sv, ImGuiConfigFlags_NavEnableGamepad);
		ctx.set_map_value(e, "NoMouse"sv, ImGuiConfigFlags_NoMouse);
		ctx.set_map_value(e, "NoMouseCursorChange"sv, ImGuiConfigFlags_NoMouseCursorChange);
		ctx.set_map_value(e, "NoKeyboard"sv, ImGuiConfigFlags_NoKeyboard);
		ctx.set_map_value(e, "IsSRGB"sv, ImGuiConfigFlags_IsSRGB);
		ctx.set_map_value(e, "IsTouchScreen"sv, ImGuiConfigFlags_IsTouchScreen);
	}
	{
		auto const e = ctx.create_map(6);
		ctx.set_map_value(m, "ImGuiBackendFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiBackendFlags_None);
		ctx.set_map_value(e, "HasGamepad"sv, ImGuiBackendFlags_HasGamepad);
		ctx.set_map_value(e, "HasMouseCursors"sv, ImGuiBackendFlags_HasMouseCursors);
		ctx.set_map_value(e, "HasSetMousePos"sv, ImGuiBackendFlags_HasSetMousePos);
		ctx.set_map_value(e, "RendererHasVtxOffset"sv, ImGuiBackendFlags_RendererHasVtxOffset);
		ctx.set_map_value(e, "RendererHasTextures"sv, ImGuiBackendFlags_RendererHasTextures);
	}
	{
		auto const e = ctx.create_map(63);
		ctx.set_map_value(m, "ImGuiCol"sv, e);
		ctx.set_map_value(e, "Text"sv, ImGuiCol_Text);
		ctx.set_map_value(e, "TextDisabled"sv, ImGuiCol_TextDisabled);
		ctx.set_map_value(e, "WindowBg"sv, ImGuiCol_WindowBg);
		ctx.set_map_value(e, "ChildBg"sv, ImGuiCol_ChildBg);
		ctx.set_map_value(e, "PopupBg"sv, ImGuiCol_PopupBg);
		ctx.set_map_value(e, "Border"sv, ImGuiCol_Border);
		ctx.set_map_value(e, "BorderShadow"sv, ImGuiCol_BorderShadow);
		ctx.set_map_value(e, "FrameBg"sv, ImGuiCol_FrameBg);
		ctx.set_map_value(e, "FrameBgHovered"sv, ImGuiCol_FrameBgHovered);
		ctx.set_map_value(e, "FrameBgActive"sv, ImGuiCol_FrameBgActive);
		ctx.set_map_value(e, "TitleBg"sv, ImGuiCol_TitleBg);
		ctx.set_map_value(e, "TitleBgActive"sv, ImGuiCol_TitleBgActive);
		ctx.set_map_value(e, "TitleBgCollapsed"sv, ImGuiCol_TitleBgCollapsed);
		ctx.set_map_value(e, "MenuBarBg"sv, ImGuiCol_MenuBarBg);
		ctx.set_map_value(e, "ScrollbarBg"sv, ImGuiCol_ScrollbarBg);
		ctx.set_map_value(e, "ScrollbarGrab"sv, ImGuiCol_ScrollbarGrab);
		ctx.set_map_value(e, "ScrollbarGrabHovered"sv, ImGuiCol_ScrollbarGrabHovered);
		ctx.set_map_value(e, "ScrollbarGrabActive"sv, ImGuiCol_ScrollbarGrabActive);
		ctx.set_map_value(e, "CheckMark"sv, ImGuiCol_CheckMark);
		ctx.set_map_value(e, "SliderGrab"sv, ImGuiCol_SliderGrab);
		ctx.set_map_value(e, "SliderGrabActive"sv, ImGuiCol_SliderGrabActive);
		ctx.set_map_value(e, "Button"sv, ImGuiCol_Button);
		ctx.set_map_value(e, "ButtonHovered"sv, ImGuiCol_ButtonHovered);
		ctx.set_map_value(e, "ButtonActive"sv, ImGuiCol_ButtonActive);
		ctx.set_map_value(e, "Header"sv, ImGuiCol_Header);
		ctx.set_map_value(e, "HeaderHovered"sv, ImGuiCol_HeaderHovered);
		ctx.set_map_value(e, "HeaderActive"sv, ImGuiCol_HeaderActive);
		ctx.set_map_value(e, "Separator"sv, ImGuiCol_Separator);
		ctx.set_map_value(e, "SeparatorHovered"sv, ImGuiCol_SeparatorHovered);
		ctx.set_map_value(e, "SeparatorActive"sv, ImGuiCol_SeparatorActive);
		ctx.set_map_value(e, "ResizeGrip"sv, ImGuiCol_ResizeGrip);
		ctx.set_map_value(e, "ResizeGripHovered"sv, ImGuiCol_ResizeGripHovered);
		ctx.set_map_value(e, "ResizeGripActive"sv, ImGuiCol_ResizeGripActive);
		ctx.set_map_value(e, "InputTextCursor"sv, ImGuiCol_InputTextCursor);
		ctx.set_map_value(e, "TabHovered"sv, ImGuiCol_TabHovered);
		ctx.set_map_value(e, "Tab"sv, ImGuiCol_Tab);
		ctx.set_map_value(e, "TabSelected"sv, ImGuiCol_TabSelected);
		ctx.set_map_value(e, "TabSelectedOverline"sv, ImGuiCol_TabSelectedOverline);
		ctx.set_map_value(e, "TabDimmed"sv, ImGuiCol_TabDimmed);
		ctx.set_map_value(e, "TabDimmedSelected"sv, ImGuiCol_TabDimmedSelected);
		ctx.set_map_value(e, "TabDimmedSelectedOverline"sv, ImGuiCol_TabDimmedSelectedOverline);
		ctx.set_map_value(e, "PlotLines"sv, ImGuiCol_PlotLines);
		ctx.set_map_value(e, "PlotLinesHovered"sv, ImGuiCol_PlotLinesHovered);
		ctx.set_map_value(e, "PlotHistogram"sv, ImGuiCol_PlotHistogram);
		ctx.set_map_value(e, "PlotHistogramHovered"sv, ImGuiCol_PlotHistogramHovered);
		ctx.set_map_value(e, "TableHeaderBg"sv, ImGuiCol_TableHeaderBg);
		ctx.set_map_value(e, "TableBorderStrong"sv, ImGuiCol_TableBorderStrong);
		ctx.set_map_value(e, "TableBorderLight"sv, ImGuiCol_TableBorderLight);
		ctx.set_map_value(e, "TableRowBg"sv, ImGuiCol_TableRowBg);
		ctx.set_map_value(e, "TableRowBgAlt"sv, ImGuiCol_TableRowBgAlt);
		ctx.set_map_value(e, "TextLink"sv, ImGuiCol_TextLink);
		ctx.set_map_value(e, "TextSelectedBg"sv, ImGuiCol_TextSelectedBg);
		ctx.set_map_value(e, "TreeLines"sv, ImGuiCol_TreeLines);
		ctx.set_map_value(e, "DragDropTarget"sv, ImGuiCol_DragDropTarget);
		ctx.set_map_value(e, "NavCursor"sv, ImGuiCol_NavCursor);
		ctx.set_map_value(e, "NavWindowingHighlight"sv, ImGuiCol_NavWindowingHighlight);
		ctx.set_map_value(e, "NavWindowingDimBg"sv, ImGuiCol_NavWindowingDimBg);
		ctx.set_map_value(e, "ModalWindowDimBg"sv, ImGuiCol_ModalWindowDimBg);
		ctx.set_map_value(e, "COUNT"sv, ImGuiCol_COUNT);
	}
	{
		auto const e = ctx.create_map(37);
		ctx.set_map_value(m, "ImGuiStyleVar"sv, e);
		ctx.set_map_value(e, "Alpha"sv, ImGuiStyleVar_Alpha);
		ctx.set_map_value(e, "DisabledAlpha"sv, ImGuiStyleVar_DisabledAlpha);
		ctx.set_map_value(e, "WindowPadding"sv, ImGuiStyleVar_WindowPadding);
		ctx.set_map_value(e, "WindowRounding"sv, ImGuiStyleVar_WindowRounding);
		ctx.set_map_value(e, "WindowBorderSize"sv, ImGuiStyleVar_WindowBorderSize);
		ctx.set_map_value(e, "WindowMinSize"sv, ImGuiStyleVar_WindowMinSize);
		ctx.set_map_value(e, "WindowTitleAlign"sv, ImGuiStyleVar_WindowTitleAlign);
		ctx.set_map_value(e, "ChildRounding"sv, ImGuiStyleVar_ChildRounding);
		ctx.set_map_value(e, "ChildBorderSize"sv, ImGuiStyleVar_ChildBorderSize);
		ctx.set_map_value(e, "PopupRounding"sv, ImGuiStyleVar_PopupRounding);
		ctx.set_map_value(e, "PopupBorderSize"sv, ImGuiStyleVar_PopupBorderSize);
		ctx.set_map_value(e, "FramePadding"sv, ImGuiStyleVar_FramePadding);
		ctx.set_map_value(e, "FrameRounding"sv, ImGuiStyleVar_FrameRounding);
		ctx.set_map_value(e, "FrameBorderSize"sv, ImGuiStyleVar_FrameBorderSize);
		ctx.set_map_value(e, "ItemSpacing"sv, ImGuiStyleVar_ItemSpacing);
		ctx.set_map_value(e, "ItemInnerSpacing"sv, ImGuiStyleVar_ItemInnerSpacing);
		ctx.set_map_value(e, "IndentSpacing"sv, ImGuiStyleVar_IndentSpacing);
		ctx.set_map_value(e, "CellPadding"sv, ImGuiStyleVar_CellPadding);
		ctx.set_map_value(e, "ScrollbarSize"sv, ImGuiStyleVar_ScrollbarSize);
		ctx.set_map_value(e, "ScrollbarRounding"sv, ImGuiStyleVar_ScrollbarRounding);
		ctx.set_map_value(e, "GrabMinSize"sv, ImGuiStyleVar_GrabMinSize);
		ctx.set_map_value(e, "GrabRounding"sv, ImGuiStyleVar_GrabRounding);
		ctx.set_map_value(e, "ImageBorderSize"sv, ImGuiStyleVar_ImageBorderSize);
		ctx.set_map_value(e, "TabRounding"sv, ImGuiStyleVar_TabRounding);
		ctx.set_map_value(e, "TabBorderSize"sv, ImGuiStyleVar_TabBorderSize);
		ctx.set_map_value(e, "TabBarBorderSize"sv, ImGuiStyleVar_TabBarBorderSize);
		ctx.set_map_value(e, "TabBarOverlineSize"sv, ImGuiStyleVar_TabBarOverlineSize);
		ctx.set_map_value(e, "TableAngledHeadersAngle"sv, ImGuiStyleVar_TableAngledHeadersAngle);
		ctx.set_map_value(e, "TableAngledHeadersTextAlign"sv, ImGuiStyleVar_TableAngledHeadersTextAlign);
		ctx.set_map_value(e, "TreeLinesSize"sv, ImGuiStyleVar_TreeLinesSize);
		ctx.set_map_value(e, "TreeLinesRounding"sv, ImGuiStyleVar_TreeLinesRounding);
		ctx.set_map_value(e, "ButtonTextAlign"sv, ImGuiStyleVar_ButtonTextAlign);
		ctx.set_map_value(e, "SelectableTextAlign"sv, ImGuiStyleVar_SelectableTextAlign);
		ctx.set_map_value(e, "SeparatorTextBorderSize"sv, ImGuiStyleVar_SeparatorTextBorderSize);
		ctx.set_map_value(e, "SeparatorTextAlign"sv, ImGuiStyleVar_SeparatorTextAlign);
		ctx.set_map_value(e, "SeparatorTextPadding"sv, ImGuiStyleVar_SeparatorTextPadding);
		ctx.set_map_value(e, "COUNT"sv, ImGuiStyleVar_COUNT);
	}
	{
		auto const e = ctx.create_map(6);
		ctx.set_map_value(m, "ImGuiButtonFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiButtonFlags_None);
		ctx.set_map_value(e, "MouseButtonLeft"sv, ImGuiButtonFlags_MouseButtonLeft);
		ctx.set_map_value(e, "MouseButtonRight"sv, ImGuiButtonFlags_MouseButtonRight);
		ctx.set_map_value(e, "MouseButtonMiddle"sv, ImGuiButtonFlags_MouseButtonMiddle);
		ctx.set_map_value(e, "MouseButtonMask_"sv, ImGuiButtonFlags_MouseButtonMask_);
		ctx.set_map_value(e, "EnableNav"sv, ImGuiButtonFlags_EnableNav);
	}
	{
		auto const e = ctx.create_map(32);
		ctx.set_map_value(m, "ImGuiColorEditFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiColorEditFlags_None);
		ctx.set_map_value(e, "NoAlpha"sv, ImGuiColorEditFlags_NoAlpha);
		ctx.set_map_value(e, "NoPicker"sv, ImGuiColorEditFlags_NoPicker);
		ctx.set_map_value(e, "NoOptions"sv, ImGuiColorEditFlags_NoOptions);
		ctx.set_map_value(e, "NoSmallPreview"sv, ImGuiColorEditFlags_NoSmallPreview);
		ctx.set_map_value(e, "NoInputs"sv, ImGuiColorEditFlags_NoInputs);
		ctx.set_map_value(e, "NoTooltip"sv, ImGuiColorEditFlags_NoTooltip);
		ctx.set_map_value(e, "NoLabel"sv, ImGuiColorEditFlags_NoLabel);
		ctx.set_map_value(e, "NoSidePreview"sv, ImGuiColorEditFlags_NoSidePreview);
		ctx.set_map_value(e, "NoDragDrop"sv, ImGuiColorEditFlags_NoDragDrop);
		ctx.set_map_value(e, "NoBorder"sv, ImGuiColorEditFlags_NoBorder);
		ctx.set_map_value(e, "AlphaOpaque"sv, ImGuiColorEditFlags_AlphaOpaque);
		ctx.set_map_value(e, "AlphaNoBg"sv, ImGuiColorEditFlags_AlphaNoBg);
		ctx.set_map_value(e, "AlphaPreviewHalf"sv, ImGuiColorEditFlags_AlphaPreviewHalf);
		ctx.set_map_value(e, "AlphaBar"sv, ImGuiColorEditFlags_AlphaBar);
		ctx.set_map_value(e, "HDR"sv, ImGuiColorEditFlags_HDR);
		ctx.set_map_value(e, "DisplayRGB"sv, ImGuiColorEditFlags_DisplayRGB);
		ctx.set_map_value(e, "DisplayHSV"sv, ImGuiColorEditFlags_DisplayHSV);
		ctx.set_map_value(e, "DisplayHex"sv, ImGuiColorEditFlags_DisplayHex);
		ctx.set_map_value(e, "Uint8"sv, ImGuiColorEditFlags_Uint8);
		ctx.set_map_value(e, "Float"sv, ImGuiColorEditFlags_Float);
		ctx.set_map_value(e, "PickerHueBar"sv, ImGuiColorEditFlags_PickerHueBar);
		ctx.set_map_value(e, "PickerHueWheel"sv, ImGuiColorEditFlags_PickerHueWheel);
		ctx.set_map_value(e, "InputRGB"sv, ImGuiColorEditFlags_InputRGB);
		ctx.set_map_value(e, "InputHSV"sv, ImGuiColorEditFlags_InputHSV);
		ctx.set_map_value(e, "DefaultOptions_"sv, ImGuiColorEditFlags_DefaultOptions_);
		ctx.set_map_value(e, "AlphaMask_"sv, ImGuiColorEditFlags_AlphaMask_);
		ctx.set_map_value(e, "DisplayMask_"sv, ImGuiColorEditFlags_DisplayMask_);
		ctx.set_map_value(e, "DataTypeMask_"sv, ImGuiColorEditFlags_DataTypeMask_);
		ctx.set_map_value(e, "PickerMask_"sv, ImGuiColorEditFlags_PickerMask_);
		ctx.set_map_value(e, "InputMask_"sv, ImGuiColorEditFlags_InputMask_);
	}
	{
		auto const e = ctx.create_map(10);
		ctx.set_map_value(m, "ImGuiSliderFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiSliderFlags_None);
		ctx.set_map_value(e, "Logarithmic"sv, ImGuiSliderFlags_Logarithmic);
		ctx.set_map_value(e, "NoRoundToFormat"sv, ImGuiSliderFlags_NoRoundToFormat);
		ctx.set_map_value(e, "NoInput"sv, ImGuiSliderFlags_NoInput);
		ctx.set_map_value(e, "WrapAround"sv, ImGuiSliderFlags_WrapAround);
		ctx.set_map_value(e, "ClampOnInput"sv, ImGuiSliderFlags_ClampOnInput);
		ctx.set_map_value(e, "ClampZeroRange"sv, ImGuiSliderFlags_ClampZeroRange);
		ctx.set_map_value(e, "NoSpeedTweaks"sv, ImGuiSliderFlags_NoSpeedTweaks);
		ctx.set_map_value(e, "AlwaysClamp"sv, ImGuiSliderFlags_AlwaysClamp);
		ctx.set_map_value(e, "InvalidMask_"sv, ImGuiSliderFlags_InvalidMask_);
	}
	{
		auto const e = ctx.create_map(4);
		ctx.set_map_value(m, "ImGuiMouseButton"sv, e);
		ctx.set_map_value(e, "Left"sv, ImGuiMouseButton_Left);
		ctx.set_map_value(e, "Right"sv, ImGuiMouseButton_Right);
		ctx.set_map_value(e, "Middle"sv, ImGuiMouseButton_Middle);
		ctx.set_map_value(e, "COUNT"sv, ImGuiMouseButton_COUNT);
	}
	{
		auto const e = ctx.create_map(13);
		ctx.set_map_value(m, "ImGuiMouseCursor"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiMouseCursor_None);
		ctx.set_map_value(e, "Arrow"sv, ImGuiMouseCursor_Arrow);
		ctx.set_map_value(e, "TextInput"sv, ImGuiMouseCursor_TextInput);
		ctx.set_map_value(e, "ResizeAll"sv, ImGuiMouseCursor_ResizeAll);
		ctx.set_map_value(e, "ResizeNS"sv, ImGuiMouseCursor_ResizeNS);
		ctx.set_map_value(e, "ResizeEW"sv, ImGuiMouseCursor_ResizeEW);
		ctx.set_map_value(e, "ResizeNESW"sv, ImGuiMouseCursor_ResizeNESW);
		ctx.set_map_value(e, "ResizeNWSE"sv, ImGuiMouseCursor_ResizeNWSE);
		ctx.set_map_value(e, "Hand"sv, ImGuiMouseCursor_Hand);
		ctx.set_map_value(e, "Wait"sv, ImGuiMouseCursor_Wait);
		ctx.set_map_value(e, "Progress"sv, ImGuiMouseCursor_Progress);
		ctx.set_map_value(e, "NotAllowed"sv, ImGuiMouseCursor_NotAllowed);
		ctx.set_map_value(e, "COUNT"sv, ImGuiMouseCursor_COUNT);
	}
	{
		auto const e = ctx.create_map(4);
		ctx.set_map_value(m, "ImGuiMouseSource"sv, e);
		ctx.set_map_value(e, "_Mouse"sv, ImGuiMouseSource_Mouse);
		ctx.set_map_value(e, "_TouchScreen"sv, ImGuiMouseSource_TouchScreen);
		ctx.set_map_value(e, "_Pen"sv, ImGuiMouseSource_Pen);
		ctx.set_map_value(e, "_COUNT"sv, ImGuiMouseSource_COUNT);
	}
	{
		auto const e = ctx.create_map(5);
		ctx.set_map_value(m, "ImGuiCond"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiCond_None);
		ctx.set_map_value(e, "Always"sv, ImGuiCond_Always);
		ctx.set_map_value(e, "Once"sv, ImGuiCond_Once);
		ctx.set_map_value(e, "FirstUseEver"sv, ImGuiCond_FirstUseEver);
		ctx.set_map_value(e, "Appearing"sv, ImGuiCond_Appearing);
	}
	{
		auto const e = ctx.create_map(37);
		ctx.set_map_value(m, "ImGuiTableFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiTableFlags_None);
		ctx.set_map_value(e, "Resizable"sv, ImGuiTableFlags_Resizable);
		ctx.set_map_value(e, "Reorderable"sv, ImGuiTableFlags_Reorderable);
		ctx.set_map_value(e, "Hideable"sv, ImGuiTableFlags_Hideable);
		ctx.set_map_value(e, "Sortable"sv, ImGuiTableFlags_Sortable);
		ctx.set_map_value(e, "NoSavedSettings"sv, ImGuiTableFlags_NoSavedSettings);
		ctx.set_map_value(e, "ContextMenuInBody"sv, ImGuiTableFlags_ContextMenuInBody);
		ctx.set_map_value(e, "RowBg"sv, ImGuiTableFlags_RowBg);
		ctx.set_map_value(e, "BordersInnerH"sv, ImGuiTableFlags_BordersInnerH);
		ctx.set_map_value(e, "BordersOuterH"sv, ImGuiTableFlags_BordersOuterH);
		ctx.set_map_value(e, "BordersInnerV"sv, ImGuiTableFlags_BordersInnerV);
		ctx.set_map_value(e, "BordersOuterV"sv, ImGuiTableFlags_BordersOuterV);
		ctx.set_map_value(e, "BordersH"sv, ImGuiTableFlags_BordersH);
		ctx.set_map_value(e, "BordersV"sv, ImGuiTableFlags_BordersV);
		ctx.set_map_value(e, "BordersInner"sv, ImGuiTableFlags_BordersInner);
		ctx.set_map_value(e, "BordersOuter"sv, ImGuiTableFlags_BordersOuter);
		ctx.set_map_value(e, "Borders"sv, ImGuiTableFlags_Borders);
		ctx.set_map_value(e, "NoBordersInBody"sv, ImGuiTableFlags_NoBordersInBody);
		ctx.set_map_value(e, "NoBordersInBodyUntilResize"sv, ImGuiTableFlags_NoBordersInBodyUntilResize);
		ctx.set_map_value(e, "SizingFixedFit"sv, ImGuiTableFlags_SizingFixedFit);
		ctx.set_map_value(e, "SizingFixedSame"sv, ImGuiTableFlags_SizingFixedSame);
		ctx.set_map_value(e, "SizingStretchProp"sv, ImGuiTableFlags_SizingStretchProp);
		ctx.set_map_value(e, "SizingStretchSame"sv, ImGuiTableFlags_SizingStretchSame);
		ctx.set_map_value(e, "NoHostExtendX"sv, ImGuiTableFlags_NoHostExtendX);
		ctx.set_map_value(e, "NoHostExtendY"sv, ImGuiTableFlags_NoHostExtendY);
		ctx.set_map_value(e, "NoKeepColumnsVisible"sv, ImGuiTableFlags_NoKeepColumnsVisible);
		ctx.set_map_value(e, "PreciseWidths"sv, ImGuiTableFlags_PreciseWidths);
		ctx.set_map_value(e, "NoClip"sv, ImGuiTableFlags_NoClip);
		ctx.set_map_value(e, "PadOuterX"sv, ImGuiTableFlags_PadOuterX);
		ctx.set_map_value(e, "NoPadOuterX"sv, ImGuiTableFlags_NoPadOuterX);
		ctx.set_map_value(e, "NoPadInnerX"sv, ImGuiTableFlags_NoPadInnerX);
		ctx.set_map_value(e, "ScrollX"sv, ImGuiTableFlags_ScrollX);
		ctx.set_map_value(e, "ScrollY"sv, ImGuiTableFlags_ScrollY);
		ctx.set_map_value(e, "SortMulti"sv, ImGuiTableFlags_SortMulti);
		ctx.set_map_value(e, "SortTristate"sv, ImGuiTableFlags_SortTristate);
		ctx.set_map_value(e, "HighlightHoveredColumn"sv, ImGuiTableFlags_HighlightHoveredColumn);
		ctx.set_map_value(e, "SizingMask_"sv, ImGuiTableFlags_SizingMask_);
	}
	{
		auto const e = ctx.create_map(28);
		ctx.set_map_value(m, "ImGuiTableColumnFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiTableColumnFlags_None);
		ctx.set_map_value(e, "Disabled"sv, ImGuiTableColumnFlags_Disabled);
		ctx.set_map_value(e, "DefaultHide"sv, ImGuiTableColumnFlags_DefaultHide);
		ctx.set_map_value(e, "DefaultSort"sv, ImGuiTableColumnFlags_DefaultSort);
		ctx.set_map_value(e, "WidthStretch"sv, ImGuiTableColumnFlags_WidthStretch);
		ctx.set_map_value(e, "WidthFixed"sv, ImGuiTableColumnFlags_WidthFixed);
		ctx.set_map_value(e, "NoResize"sv, ImGuiTableColumnFlags_NoResize);
		ctx.set_map_value(e, "NoReorder"sv, ImGuiTableColumnFlags_NoReorder);
		ctx.set_map_value(e, "NoHide"sv, ImGuiTableColumnFlags_NoHide);
		ctx.set_map_value(e, "NoClip"sv, ImGuiTableColumnFlags_NoClip);
		ctx.set_map_value(e, "NoSort"sv, ImGuiTableColumnFlags_NoSort);
		ctx.set_map_value(e, "NoSortAscending"sv, ImGuiTableColumnFlags_NoSortAscending);
		ctx.set_map_value(e, "NoSortDescending"sv, ImGuiTableColumnFlags_NoSortDescending);
		ctx.set_map_value(e, "NoHeaderLabel"sv, ImGuiTableColumnFlags_NoHeaderLabel);
		ctx.set_map_value(e, "NoHeaderWidth"sv, ImGuiTableColumnFlags_NoHeaderWidth);
		ctx.set_map_value(e, "PreferSortAscending"sv, ImGuiTableColumnFlags_PreferSortAscending);
		ctx.set_map_value(e, "PreferSortDescending"sv, ImGuiTableColumnFlags_PreferSortDescending);
		ctx.set_map_value(e, "IndentEnable"sv, ImGuiTableColumnFlags_IndentEnable);
		ctx.set_map_value(e, "IndentDisable"sv, ImGuiTableColumnFlags_IndentDisable);
		ctx.set_map_value(e, "AngledHeader"sv, ImGuiTableColumnFlags_AngledHeader);
		ctx.set_map_value(e, "IsEnabled"sv, ImGuiTableColumnFlags_IsEnabled);
		ctx.set_map_value(e, "IsVisible"sv, ImGuiTableColumnFlags_IsVisible);
		ctx.set_map_value(e, "IsSorted"sv, ImGuiTableColumnFlags_IsSorted);
		ctx.set_map_value(e, "IsHovered"sv, ImGuiTableColumnFlags_IsHovered);
		ctx.set_map_value(e, "WidthMask_"sv, ImGuiTableColumnFlags_WidthMask_);
		ctx.set_map_value(e, "IndentMask_"sv, ImGuiTableColumnFlags_IndentMask_);
		ctx.set_map_value(e, "StatusMask_"sv, ImGuiTableColumnFlags_StatusMask_);
		ctx.set_map_value(e, "NoDirectResize_"sv, ImGuiTableColumnFlags_NoDirectResize_);
	}
	{
		auto const e = ctx.create_map(2);
		ctx.set_map_value(m, "ImGuiTableRowFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiTableRowFlags_None);
		ctx.set_map_value(e, "Headers"sv, ImGuiTableRowFlags_Headers);
	}
	{
		auto const e = ctx.create_map(4);
		ctx.set_map_value(m, "ImGuiTableBgTarget"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiTableBgTarget_None);
		ctx.set_map_value(e, "RowBg0"sv, ImGuiTableBgTarget_RowBg0);
		ctx.set_map_value(e, "RowBg1"sv, ImGuiTableBgTarget_RowBg1);
		ctx.set_map_value(e, "CellBg"sv, ImGuiTableBgTarget_CellBg);
	}
	{
		auto const e = ctx.create_map(17);
		ctx.set_map_value(m, "ImGuiMultiSelectFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiMultiSelectFlags_None);
		ctx.set_map_value(e, "SingleSelect"sv, ImGuiMultiSelectFlags_SingleSelect);
		ctx.set_map_value(e, "NoSelectAll"sv, ImGuiMultiSelectFlags_NoSelectAll);
		ctx.set_map_value(e, "NoRangeSelect"sv, ImGuiMultiSelectFlags_NoRangeSelect);
		ctx.set_map_value(e, "NoAutoSelect"sv, ImGuiMultiSelectFlags_NoAutoSelect);
		ctx.set_map_value(e, "NoAutoClear"sv, ImGuiMultiSelectFlags_NoAutoClear);
		ctx.set_map_value(e, "NoAutoClearOnReselect"sv, ImGuiMultiSelectFlags_NoAutoClearOnReselect);
		ctx.set_map_value(e, "BoxSelect1d"sv, ImGuiMultiSelectFlags_BoxSelect1d);
		ctx.set_map_value(e, "BoxSelect2d"sv, ImGuiMultiSelectFlags_BoxSelect2d);
		ctx.set_map_value(e, "BoxSelectNoScroll"sv, ImGuiMultiSelectFlags_BoxSelectNoScroll);
		ctx.set_map_value(e, "ClearOnEscape"sv, ImGuiMultiSelectFlags_ClearOnEscape);
		ctx.set_map_value(e, "ClearOnClickVoid"sv, ImGuiMultiSelectFlags_ClearOnClickVoid);
		ctx.set_map_value(e, "ScopeWindow"sv, ImGuiMultiSelectFlags_ScopeWindow);
		ctx.set_map_value(e, "ScopeRect"sv, ImGuiMultiSelectFlags_ScopeRect);
		ctx.set_map_value(e, "SelectOnClick"sv, ImGuiMultiSelectFlags_SelectOnClick);
		ctx.set_map_value(e, "SelectOnClickRelease"sv, ImGuiMultiSelectFlags_SelectOnClickRelease);
		ctx.set_map_value(e, "NavWrapX"sv, ImGuiMultiSelectFlags_NavWrapX);
	}
	{
		auto const e = ctx.create_map(3);
		ctx.set_map_value(m, "ImGuiSelectionRequestType"sv, e);
		ctx.set_map_value(e, "_None"sv, ImGuiSelectionRequestType_None);
		ctx.set_map_value(e, "_SetAll"sv, ImGuiSelectionRequestType_SetAll);
		ctx.set_map_value(e, "_SetRange"sv, ImGuiSelectionRequestType_SetRange);
	}
	{
		auto const e = ctx.create_map(14);
		ctx.set_map_value(m, "ImDrawFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImDrawFlags_None);
		ctx.set_map_value(e, "Closed"sv, ImDrawFlags_Closed);
		ctx.set_map_value(e, "RoundCornersTopLeft"sv, ImDrawFlags_RoundCornersTopLeft);
		ctx.set_map_value(e, "RoundCornersTopRight"sv, ImDrawFlags_RoundCornersTopRight);
		ctx.set_map_value(e, "RoundCornersBottomLeft"sv, ImDrawFlags_RoundCornersBottomLeft);
		ctx.set_map_value(e, "RoundCornersBottomRight"sv, ImDrawFlags_RoundCornersBottomRight);
		ctx.set_map_value(e, "RoundCornersNone"sv, ImDrawFlags_RoundCornersNone);
		ctx.set_map_value(e, "RoundCornersTop"sv, ImDrawFlags_RoundCornersTop);
		ctx.set_map_value(e, "RoundCornersBottom"sv, ImDrawFlags_RoundCornersBottom);
		ctx.set_map_value(e, "RoundCornersLeft"sv, ImDrawFlags_RoundCornersLeft);
		ctx.set_map_value(e, "RoundCornersRight"sv, ImDrawFlags_RoundCornersRight);
		ctx.set_map_value(e, "RoundCornersAll"sv, ImDrawFlags_RoundCornersAll);
		ctx.set_map_value(e, "RoundCornersDefault_"sv, ImDrawFlags_RoundCornersDefault_);
		ctx.set_map_value(e, "RoundCornersMask_"sv, ImDrawFlags_RoundCornersMask_);
	}
	{
		auto const e = ctx.create_map(5);
		ctx.set_map_value(m, "ImDrawListFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImDrawListFlags_None);
		ctx.set_map_value(e, "AntiAliasedLines"sv, ImDrawListFlags_AntiAliasedLines);
		ctx.set_map_value(e, "AntiAliasedLinesUseTex"sv, ImDrawListFlags_AntiAliasedLinesUseTex);
		ctx.set_map_value(e, "AntiAliasedFill"sv, ImDrawListFlags_AntiAliasedFill);
		ctx.set_map_value(e, "AllowVtxOffset"sv, ImDrawListFlags_AllowVtxOffset);
	}
	{
		auto const e = ctx.create_map(2);
		ctx.set_map_value(m, "ImTextureFormat"sv, e);
		ctx.set_map_value(e, "_RGBA32"sv, ImTextureFormat_RGBA32);
		ctx.set_map_value(e, "_Alpha8"sv, ImTextureFormat_Alpha8);
	}
	{
		auto const e = ctx.create_map(5);
		ctx.set_map_value(m, "ImTextureStatus"sv, e);
		ctx.set_map_value(e, "_OK"sv, ImTextureStatus_OK);
		ctx.set_map_value(e, "_Destroyed"sv, ImTextureStatus_Destroyed);
		ctx.set_map_value(e, "_WantCreate"sv, ImTextureStatus_WantCreate);
		ctx.set_map_value(e, "_WantUpdates"sv, ImTextureStatus_WantUpdates);
		ctx.set_map_value(e, "_WantDestroy"sv, ImTextureStatus_WantDestroy);
	}
	{
		auto const e = ctx.create_map(4);
		ctx.set_map_value(m, "ImFontAtlasFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImFontAtlasFlags_None);
		ctx.set_map_value(e, "NoPowerOfTwoHeight"sv, ImFontAtlasFlags_NoPowerOfTwoHeight);
		ctx.set_map_value(e, "NoMouseCursors"sv, ImFontAtlasFlags_NoMouseCursors);
		ctx.set_map_value(e, "NoBakedLines"sv, ImFontAtlasFlags_NoBakedLines);
	}
	{
		auto const e = ctx.create_map(4);
		ctx.set_map_value(m, "ImFontFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImFontFlags_None);
		ctx.set_map_value(e, "NoLoadError"sv, ImFontFlags_NoLoadError);
		ctx.set_map_value(e, "NoLoadGlyphs"sv, ImFontFlags_NoLoadGlyphs);
		ctx.set_map_value(e, "LockBakedSizes"sv, ImFontFlags_LockBakedSizes);
	}
	{
		auto const e = ctx.create_map(4);
		ctx.set_map_value(m, "ImGuiViewportFlags"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiViewportFlags_None);
		ctx.set_map_value(e, "IsPlatformWindow"sv, ImGuiViewportFlags_IsPlatformWindow);
		ctx.set_map_value(e, "IsPlatformMonitor"sv, ImGuiViewportFlags_IsPlatformMonitor);
		ctx.set_map_value(e, "OwnedByApp"sv, ImGuiViewportFlags_OwnedByApp);
	}
}
