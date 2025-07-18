// ImGui Lua API

#define MAKEF(NAME) {#NAME, &lib_##NAME##}

static const luaL_Reg lib_fun[] = {
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
