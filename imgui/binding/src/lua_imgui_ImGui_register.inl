// ImGui Lua API

#define MAKEF(NAME) {#NAME, &lib_##NAME##}

static const luaL_Reg lib_fun[] = {
    // Context creation and access
    MAKEF(CreateContext),
    MAKEF(DestroyContext),
    MAKEF(GetCurrentContext),
    MAKEF(SetCurrentContext),
    
    // Main
    MAKEF(GetIO),
    MAKEF(GetPlatformIO),
    MAKEF(GetStyle),
    MAKEF(NewFrame),
    MAKEF(EndFrame),
    MAKEF(Render),
    MAKEF(GetDrawData),
    
    // Demo, Debug, Information
    MAKEF(ShowDemoWindow),
    MAKEF(ShowMetricsWindow),
    MAKEF(ShowDebugLogWindow),
    MAKEF(ShowIDStackToolWindow),
    MAKEF(ShowAboutWindow),
    MAKEF(ShowStyleEditor),
    MAKEF(ShowStyleSelector),
    MAKEF(ShowFontSelector),
    MAKEF(ShowUserGuide),
    MAKEF(GetVersion),
    
    // Styles
    MAKEF(StyleColorsDark),
    MAKEF(StyleColorsLight),
    MAKEF(StyleColorsClassic),
    
    // Windows
    MAKEF(Begin),
    MAKEF(End),
    
    // Child Windows
    MAKEF(BeginChild),
    MAKEF(EndChild),
    
    // Windows Utilities
    MAKEF(IsWindowAppearing),
    MAKEF(IsWindowCollapsed),
    MAKEF(IsWindowFocused),
    MAKEF(IsWindowHovered),
    MAKEF(GetWindowDrawList),
    MAKEF(GetWindowPos),
    MAKEF(GetWindowSize),
    MAKEF(GetWindowWidth),
    MAKEF(GetWindowHeight),
    
    MAKEF(SetNextWindowPos),
    MAKEF(SetNextWindowSize),
    MAKEF(SetNextWindowSizeConstraints),
    MAKEF(SetNextWindowContentSize),
    MAKEF(SetNextWindowCollapsed),
    MAKEF(SetNextWindowFocus),
    MAKEF(SetNextWindowBgAlpha),
    MAKEF(SetWindowPos),
    MAKEF(SetWindowSize),
    MAKEF(SetWindowCollapsed),
    MAKEF(SetWindowFocus),
    MAKEF(SetWindowFontScale),
    
    // Windows Scrolling
    MAKEF(GetScrollX),
    MAKEF(GetScrollY),
    MAKEF(SetScrollX),
    MAKEF(SetScrollY),
    MAKEF(GetScrollMaxX),
    MAKEF(GetScrollMaxY),
    MAKEF(SetScrollHereX),
    MAKEF(SetScrollHereY),
    MAKEF(SetScrollFromPosX),
    MAKEF(SetScrollFromPosY),
    
    // Parameters stacks (shared)
    MAKEF(PushFont),
    MAKEF(PopFont),
    MAKEF(PushStyleColor),
    MAKEF(PopStyleColor),
    MAKEF(PushStyleVar),
    MAKEF(PushStyleVarX),
    MAKEF(PushStyleVarY),
    MAKEF(PopStyleVar),
    MAKEF(PushItemFlag),
    MAKEF(PopItemFlag),
    
    // Parameters stacks (current window)
    MAKEF(PushItemWidth),
    MAKEF(PopItemWidth),
    MAKEF(SetNextItemWidth),
    MAKEF(CalcItemWidth),
    MAKEF(PushTextWrapPos),
    MAKEF(PopTextWrapPos),
    
    // Style read access
    MAKEF(GetFont),
    MAKEF(GetFontSize),
    MAKEF(GetFontTexUvWhitePixel),
    MAKEF(GetColorU32),
    MAKEF(GetStyleColorVec4),
    
    // Cursor / Layout
    MAKEF(GetCursorScreenPos),
    MAKEF(SetCursorScreenPos),
    MAKEF(GetContentRegionAvail),
    MAKEF(GetCursorPos),
    MAKEF(GetCursorPosX),
    MAKEF(GetCursorPosY),
    MAKEF(SetCursorPos),
    MAKEF(SetCursorPosX),
    MAKEF(SetCursorPosY),
    MAKEF(GetCursorStartPos),

    // Other layout functions
    MAKEF(Separator),
    MAKEF(SameLine),
    MAKEF(NewLine),
    MAKEF(Spacing),
    MAKEF(Dummy),
    MAKEF(Indent),
    MAKEF(Unindent),
    MAKEF(BeginGroup),
    MAKEF(EndGroup),
    MAKEF(AlignTextToFramePadding),
    MAKEF(GetTextLineHeight),
    MAKEF(GetTextLineHeightWithSpacing),
    MAKEF(GetFrameHeight),
    MAKEF(GetFrameHeightWithSpacing),
    
    // ID stack/scopes
    MAKEF(PushID),
    MAKEF(PopID),
    MAKEF(GetID),
    
    // Widgets: Text
    MAKEF(TextUnformatted),
    MAKEF(Text),
    MAKEF(TextV),
    MAKEF(TextColored),
    MAKEF(TextColoredV),
    MAKEF(TextDisabled),
    MAKEF(TextDisabledV),
    MAKEF(TextWrapped),
    MAKEF(TextWrappedV),
    MAKEF(LabelText),
    MAKEF(LabelTextV),
    MAKEF(BulletText),
    MAKEF(BulletTextV),
    MAKEF(SeparatorText),
    
    // Widgets: Main
    MAKEF(Button),
    MAKEF(SmallButton),
    MAKEF(InvisibleButton),
    MAKEF(ArrowButton),
    MAKEF(Checkbox),
    MAKEF(CheckboxFlags),
    MAKEF(RadioButton),
    MAKEF(ProgressBar),
    MAKEF(Bullet),
    MAKEF(TextLink),
    MAKEF(TextLinkOpenURL),
    
    // Widgets: Images
    MAKEF(Image),
    MAKEF(ImageButton),

    // Widgets: Combo Box (Dropdown)
    MAKEF(BeginCombo),
    MAKEF(EndCombo),
    MAKEF(Combo),
    
    // Widgets: Drag Sliders
    MAKEF(DragFloat),
    MAKEF(DragFloat2),
    MAKEF(DragFloat3),
    MAKEF(DragFloat4),
    MAKEF(DragFloatRange2),
    MAKEF(DragInt),
    MAKEF(DragInt2),
    MAKEF(DragInt3),
    MAKEF(DragInt4),
    MAKEF(DragIntRange2),
    MAKEF(DragScalar),
    MAKEF(DragScalarN),
    
    // Widgets: Regular Sliders
    MAKEF(SliderFloat),
    MAKEF(SliderFloat2),
    MAKEF(SliderFloat3),
    MAKEF(SliderFloat4),
    MAKEF(SliderAngle),
    MAKEF(SliderInt),
    MAKEF(SliderInt2),
    MAKEF(SliderInt3),
    MAKEF(SliderInt4),
    MAKEF(SliderScalar),
    MAKEF(SliderScalarN),
    MAKEF(VSliderFloat),
    MAKEF(VSliderInt),
    MAKEF(VSliderScalar),
    
    // Widgets: Input with Keyboard
    MAKEF(InputText),
    MAKEF(InputTextMultiline),
    MAKEF(InputTextWithHint),
    MAKEF(InputFloat),
    MAKEF(InputFloat2),
    MAKEF(InputFloat3),
    MAKEF(InputFloat4),
    MAKEF(InputInt),
    MAKEF(InputInt2),
    MAKEF(InputInt3),
    MAKEF(InputInt4),
    MAKEF(InputDouble),
    MAKEF(InputScalar),
    MAKEF(InputScalarN),
    
    // Widgets: Color Editor/Picker
    MAKEF(ColorEdit3),
    MAKEF(ColorEdit4),
    MAKEF(ColorPicker3),
    MAKEF(ColorPicker4),
    MAKEF(ColorButton),
    MAKEF(SetColorEditOptions),
    
    // Widgets: Trees
    MAKEF(TreeNode),
    MAKEF(TreeNodeV),
    MAKEF(TreeNodeEx),
    MAKEF(TreeNodeExV),
    MAKEF(TreePush),
    MAKEF(TreePop),
    MAKEF(GetTreeNodeToLabelSpacing),
    MAKEF(CollapsingHeader),
    MAKEF(SetNextItemOpen),
    MAKEF(SetNextItemStorageID),
    
    // Widgets: Selectables
    MAKEF(Selectable),
    
    // Widgets: List Boxes
    MAKEF(BeginListBox),
    MAKEF(EndListBox),
    MAKEF(ListBox),
    
    // Widgets: Data Plotting
    MAKEF(PlotLines),
    MAKEF(PlotHistogram),
    
    // Widgets: Value() Helpers.
    MAKEF(ValueB),
    MAKEF(ValueI),
    MAKEF(ValueF),
    MAKEF(Value),
    
    // Widgets: Menus
    MAKEF(BeginMenuBar),
    MAKEF(EndMenuBar),
    MAKEF(BeginMainMenuBar),
    MAKEF(EndMainMenuBar),
    MAKEF(BeginMenu),
    MAKEF(EndMenu),
    MAKEF(MenuItem),
    
    // Tooltips
    MAKEF(BeginTooltip),
    MAKEF(EndTooltip),
    MAKEF(SetTooltip),
    MAKEF(SetTooltipV),

    MAKEF(BeginItemTooltip),
    MAKEF(SetItemTooltip),
    MAKEF(SetItemTooltipV),
    
    // Popups, Modals
    MAKEF(BeginPopup),
    MAKEF(BeginPopupModal),
    MAKEF(EndPopup),
    // Popups: open/close functions
    MAKEF(OpenPopup),
    MAKEF(OpenPopupOnItemClick),
    MAKEF(CloseCurrentPopup),
    // Popups: open+begin combined functions helpers
    MAKEF(BeginPopupContextItem),
    MAKEF(BeginPopupContextWindow),
    MAKEF(BeginPopupContextVoid),
    // Popups: test function
    MAKEF(IsPopupOpen),
    
    // Tables
    MAKEF(BeginTable),
    MAKEF(EndTable),
    MAKEF(TableNextRow),
    MAKEF(TableNextColumn),
    MAKEF(TableSetColumnIndex),
    // Tables: Headers & Columns declaration
    MAKEF(TableSetupColumn),
    MAKEF(TableSetupScrollFreeze),
    MAKEF(TableHeader),
    MAKEF(TableHeadersRow),
    MAKEF(TableAngledHeadersRow),
    // Tables: Sorting & Miscellaneous functions
    MAKEF(TableGetSortSpecs),
    MAKEF(TableGetColumnCount),
    MAKEF(TableGetColumnIndex),
    MAKEF(TableGetRowIndex),
    MAKEF(TableGetColumnName),
    MAKEF(TableGetColumnFlags),
    MAKEF(TableSetColumnEnabled),
    MAKEF(TableGetHoveredColumn),
    MAKEF(TableSetBgColor),
    
    // Legacy Columns API
    MAKEF(Columns),
    MAKEF(NextColumn),
    MAKEF(GetColumnIndex),
    MAKEF(GetColumnWidth),
    MAKEF(SetColumnWidth),
    MAKEF(GetColumnOffset),
    MAKEF(SetColumnOffset),
    MAKEF(GetColumnsCount),
    
    // Tab Bars, Tabs
    MAKEF(BeginTabBar),
    MAKEF(EndTabBar),
    MAKEF(BeginTabItem),
    MAKEF(EndTabItem),
    MAKEF(TabItemButton),
    MAKEF(SetTabItemClosed),
    
    // Logging/Capture
    MAKEF(LogToTTY),
    MAKEF(LogToFile),
    MAKEF(LogToClipboard),
    MAKEF(LogFinish),
    MAKEF(LogButtons),
    MAKEF(LogText),
    
    // Drag and Drop
    MAKEF(BeginDragDropSource),
    MAKEF(SetDragDropPayload),
    MAKEF(EndDragDropSource),
    MAKEF(BeginDragDropTarget),
    MAKEF(AcceptDragDropPayload),
    MAKEF(EndDragDropTarget),
    MAKEF(GetDragDropPayload),
    
    // Disabling
    MAKEF(BeginDisabled),
    MAKEF(EndDisabled),
    
    // Clipping
    MAKEF(PushClipRect),
    MAKEF(PopClipRect),
    
    // Focus, Activation
    MAKEF(SetItemDefaultFocus),
    MAKEF(SetKeyboardFocusHere),
    
    // Keyboard/Gamepad Navigation
    MAKEF(SetNavCursorVisible),

    // Overlapping mode
    MAKEF(SetNextItemAllowOverlap),

    // Item/Widgets Utilities
    MAKEF(IsItemHovered),
    MAKEF(IsItemActive),
    MAKEF(IsItemFocused),
    MAKEF(IsItemClicked),
    MAKEF(IsItemVisible),
    MAKEF(IsItemEdited),
    MAKEF(IsItemActivated),
    MAKEF(IsItemDeactivated),
    MAKEF(IsItemDeactivatedAfterEdit),
    MAKEF(IsItemToggledOpen),
    MAKEF(IsAnyItemHovered),
    MAKEF(IsAnyItemActive),
    MAKEF(IsAnyItemFocused),
    MAKEF(GetItemID),
    MAKEF(GetItemRectMin),
    MAKEF(GetItemRectMax),
    MAKEF(GetItemRectSize),
    
    // Background/Foreground Draw Lists
    MAKEF(GetBackgroundDrawList),
    MAKEF(GetForegroundDrawList),

    // Miscellaneous Utilities
    MAKEF(IsRectVisible),
    MAKEF(GetTime),
    MAKEF(GetFrameCount),
    MAKEF(GetDrawListSharedData),
    MAKEF(GetStyleColorName),
    MAKEF(SetStateStorage),
    MAKEF(GetStateStorage),

    // Text Utilities
    MAKEF(CalcTextSize),
    
    // Color Utilities
    MAKEF(ColorConvertU32ToFloat4),
    MAKEF(ColorConvertFloat4ToU32),
    MAKEF(ColorConvertRGBtoHSV),
    MAKEF(ColorConvertHSVtoRGB),
    
    // Inputs Utilities: Keyboard/Mouse/Gamepad
    MAKEF(IsKeyDown),
    MAKEF(IsKeyPressed),
    MAKEF(IsKeyReleased),
    MAKEF(IsKeyChordPressed),
    MAKEF(GetKeyPressedAmount),
    MAKEF(GetKeyName),
    MAKEF(SetNextFrameWantCaptureKeyboard),
    
    // Inputs Utilities: Shortcut Testing & Routing [BETA]
    MAKEF(Shortcut),
    MAKEF(SetNextItemShortcut),

    // Inputs Utilities: Key/Input Ownership [BETA]
    MAKEF(SetItemKeyOwner),

    // Inputs Utilities: Mouse specific
    MAKEF(IsMouseDown),
    MAKEF(IsMouseClicked),
    MAKEF(IsMouseReleased),
    MAKEF(IsMouseDoubleClicked),
    MAKEF(IsMouseReleasedWithDelay),
    MAKEF(GetMouseClickedCount),
    MAKEF(IsMouseHoveringRect),
    MAKEF(IsMousePosValid),
    MAKEF(IsAnyMouseDown),
    MAKEF(GetMousePos),
    MAKEF(GetMousePosOnOpeningCurrentPopup),
    MAKEF(IsMouseDragging),
    MAKEF(GetMouseDragDelta),
    MAKEF(ResetMouseDragDelta),
    MAKEF(GetMouseCursor),
    MAKEF(SetMouseCursor),
    MAKEF(SetNextFrameWantCaptureMouse),
    
    // Clipboard Utilities
    MAKEF(GetClipboardText),
    MAKEF(SetClipboardText),
    
    // Settings/.Ini Utilities
    MAKEF(LoadIniSettingsFromDisk),
    MAKEF(LoadIniSettingsFromMemory),
    MAKEF(SaveIniSettingsToDisk),
    MAKEF(SaveIniSettingsToMemory),
    
    // Debug Utilities
    MAKEF(DebugTextEncoding),
    MAKEF(DebugFlashStyleColor),
    MAKEF(DebugStartItemPicker),
    MAKEF(DebugCheckVersionAndDataLayout),
    MAKEF(DebugLog),
    MAKEF(DebugLogV),
    
    // Memory Allocators
    MAKEF(SetAllocatorFunctions),
    MAKEF(GetAllocatorFunctions),
    MAKEF(MemAlloc),
    MAKEF(MemFree),
    
    // NULL END
    {NULL, NULL},
};

#undef MAKEF

static const int lib_func = (sizeof(lib_fun) / sizeof(luaL_Reg)) - 1;
