// This file was generated through a code generator, DO NOT edit it, please edit the code generator

#include "ImGuiStyleMember.hpp"

namespace imgui_binding_lua {

ImGuiStyleMember mapImGuiStyleMember(char const* const key, size_t const len) noexcept {
    if (key == nullptr || len == 0 || len > 32) {
        return ImGuiStyleMember::__unknown__;
    }
    int state{ 0 };
    #define make_condition(last_state, character) ((last_state << 8) | static_cast<int>(character))
    for (size_t i = 0; i < len; i += 1) {
        switch (i) {
        case 0:
            switch (make_condition(state, key[i])) {
            case make_condition(0, 'A'): state = 1; continue; // -> Alpha, AntiAliasedFill, AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(0, 'B'): state = 31; continue; // -> ButtonTextAlign
            case make_condition(0, 'C'): state = 46; continue; // -> CellPadding, ChildBorderSize, ChildRounding, CircleTessellationMaxError, ColorButtonPosition, Colors, ColumnsMinSpacing, CurveTessellationTol
            case make_condition(0, 'D'): state = 156; continue; // -> DisabledAlpha, DisplaySafeAreaPadding, DisplayWindowPadding
            case make_condition(0, 'F'): state = 201; continue; // -> FrameBorderSize, FramePadding, FrameRounding
            case make_condition(0, 'G'): state = 231; continue; // -> GrabMinSize, GrabRounding
            case make_condition(0, 'H'): state = 250; continue; // -> HoverDelayNormal, HoverDelayShort, HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav, HoverStationaryDelay
            case make_condition(0, 'I'): state = 309; continue; // -> ImageBorderSize, IndentSpacing, ItemInnerSpacing, ItemSpacing
            case make_condition(0, 'L'): state = 358; continue; // -> LogSliderDeadzone
            case make_condition(0, 'M'): state = 375; continue; // -> MouseCursorScale
            case make_condition(0, 'P'): state = 391; continue; // -> PopupBorderSize, PopupRounding
            case make_condition(0, 'S'): state = 414; continue; // -> ScaleAllSizes, ScrollbarRounding, ScrollbarSize, SelectableTextAlign, SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(0, 'T'): state = 497; continue; // -> TabBarBorderSize, TabBarOverlineSize, TabBorderSize, TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected, TabRounding, TableAngledHeadersAngle, TableAngledHeadersTextAlign, TouchExtraPadding
            case make_condition(0, 'W'): state = 624; continue; // -> WindowBorderHoverPadding, WindowBorderSize, WindowMenuButtonPosition, WindowMinSize, WindowPadding, WindowRounding, WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 1:
            switch (make_condition(state, key[i])) {
            case make_condition(1, 'l'): state = 2; continue; // -> Alpha
            case make_condition(1, 'n'): state = 6; continue; // -> AntiAliasedFill, AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(31, 'u'): state = 32; continue; // -> ButtonTextAlign
            case make_condition(46, 'e'): state = 47; continue; // -> CellPadding
            case make_condition(46, 'h'): state = 57; continue; // -> ChildBorderSize, ChildRounding
            case make_condition(46, 'i'): state = 79; continue; // -> CircleTessellationMaxError
            case make_condition(46, 'o'): state = 104; continue; // -> ColorButtonPosition, Colors, ColumnsMinSpacing
            case make_condition(46, 'u'): state = 137; continue; // -> CurveTessellationTol
            case make_condition(156, 'i'): state = 157; continue; // -> DisabledAlpha, DisplaySafeAreaPadding, DisplayWindowPadding
            case make_condition(201, 'r'): state = 202; continue; // -> FrameBorderSize, FramePadding, FrameRounding
            case make_condition(231, 'r'): state = 232; continue; // -> GrabMinSize, GrabRounding
            case make_condition(250, 'o'): state = 251; continue; // -> HoverDelayNormal, HoverDelayShort, HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav, HoverStationaryDelay
            case make_condition(309, 'm'): state = 310; continue; // -> ImageBorderSize
            case make_condition(309, 'n'): state = 324; continue; // -> IndentSpacing
            case make_condition(309, 't'): state = 336; continue; // -> ItemInnerSpacing, ItemSpacing
            case make_condition(358, 'o'): state = 359; continue; // -> LogSliderDeadzone
            case make_condition(375, 'o'): state = 376; continue; // -> MouseCursorScale
            case make_condition(391, 'o'): state = 392; continue; // -> PopupBorderSize, PopupRounding
            case make_condition(414, 'c'): state = 415; continue; // -> ScaleAllSizes, ScrollbarRounding, ScrollbarSize
            case make_condition(414, 'e'): state = 446; continue; // -> SelectableTextAlign, SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(497, 'a'): state = 498; continue; // -> TabBarBorderSize, TabBarOverlineSize, TabBorderSize, TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected, TabRounding, TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(497, 'o'): state = 608; continue; // -> TouchExtraPadding
            case make_condition(624, 'i'): state = 625; continue; // -> WindowBorderHoverPadding, WindowBorderSize, WindowMenuButtonPosition, WindowMinSize, WindowPadding, WindowRounding, WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 2:
            switch (make_condition(state, key[i])) {
            case make_condition(2, 'p'): state = 3; continue; // -> Alpha
            case make_condition(6, 't'): state = 7; continue; // -> AntiAliasedFill, AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(32, 't'): state = 33; continue; // -> ButtonTextAlign
            case make_condition(47, 'l'): state = 48; continue; // -> CellPadding
            case make_condition(57, 'i'): state = 58; continue; // -> ChildBorderSize, ChildRounding
            case make_condition(79, 'r'): state = 80; continue; // -> CircleTessellationMaxError
            case make_condition(104, 'l'): state = 105; continue; // -> ColorButtonPosition, Colors, ColumnsMinSpacing
            case make_condition(137, 'r'): state = 138; continue; // -> CurveTessellationTol
            case make_condition(157, 's'): state = 158; continue; // -> DisabledAlpha, DisplaySafeAreaPadding, DisplayWindowPadding
            case make_condition(202, 'a'): state = 203; continue; // -> FrameBorderSize, FramePadding, FrameRounding
            case make_condition(232, 'a'): state = 233; continue; // -> GrabMinSize, GrabRounding
            case make_condition(251, 'v'): state = 252; continue; // -> HoverDelayNormal, HoverDelayShort, HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav, HoverStationaryDelay
            case make_condition(310, 'a'): state = 311; continue; // -> ImageBorderSize
            case make_condition(324, 'd'): state = 325; continue; // -> IndentSpacing
            case make_condition(336, 'e'): state = 337; continue; // -> ItemInnerSpacing, ItemSpacing
            case make_condition(359, 'g'): state = 360; continue; // -> LogSliderDeadzone
            case make_condition(376, 'u'): state = 377; continue; // -> MouseCursorScale
            case make_condition(392, 'p'): state = 393; continue; // -> PopupBorderSize, PopupRounding
            case make_condition(415, 'a'): state = 416; continue; // -> ScaleAllSizes
            case make_condition(415, 'r'): state = 427; continue; // -> ScrollbarRounding, ScrollbarSize
            case make_condition(446, 'l'): state = 447; continue; // -> SelectableTextAlign
            case make_condition(446, 'p'): state = 464; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(498, 'b'): state = 499; continue; // -> TabBarBorderSize, TabBarOverlineSize, TabBorderSize, TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected, TabRounding, TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(608, 'u'): state = 609; continue; // -> TouchExtraPadding
            case make_condition(625, 'n'): state = 626; continue; // -> WindowBorderHoverPadding, WindowBorderSize, WindowMenuButtonPosition, WindowMinSize, WindowPadding, WindowRounding, WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 3:
            switch (make_condition(state, key[i])) {
            case make_condition(3, 'h'): state = 4; continue; // -> Alpha
            case make_condition(7, 'i'): state = 8; continue; // -> AntiAliasedFill, AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(33, 't'): state = 34; continue; // -> ButtonTextAlign
            case make_condition(48, 'l'): state = 49; continue; // -> CellPadding
            case make_condition(58, 'l'): state = 59; continue; // -> ChildBorderSize, ChildRounding
            case make_condition(80, 'c'): state = 81; continue; // -> CircleTessellationMaxError
            case make_condition(105, 'o'): state = 106; continue; // -> ColorButtonPosition, Colors
            case make_condition(105, 'u'): state = 123; continue; // -> ColumnsMinSpacing
            case make_condition(138, 'v'): state = 139; continue; // -> CurveTessellationTol
            case make_condition(158, 'a'): state = 159; continue; // -> DisabledAlpha
            case make_condition(158, 'p'): state = 169; continue; // -> DisplaySafeAreaPadding, DisplayWindowPadding
            case make_condition(203, 'm'): state = 204; continue; // -> FrameBorderSize, FramePadding, FrameRounding
            case make_condition(233, 'b'): state = 234; continue; // -> GrabMinSize, GrabRounding
            case make_condition(252, 'e'): state = 253; continue; // -> HoverDelayNormal, HoverDelayShort, HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav, HoverStationaryDelay
            case make_condition(311, 'g'): state = 312; continue; // -> ImageBorderSize
            case make_condition(325, 'e'): state = 326; continue; // -> IndentSpacing
            case make_condition(337, 'm'): state = 338; continue; // -> ItemInnerSpacing, ItemSpacing
            case make_condition(360, 'S'): state = 361; continue; // -> LogSliderDeadzone
            case make_condition(377, 's'): state = 378; continue; // -> MouseCursorScale
            case make_condition(393, 'u'): state = 394; continue; // -> PopupBorderSize, PopupRounding
            case make_condition(416, 'l'): state = 417; continue; // -> ScaleAllSizes
            case make_condition(427, 'o'): state = 428; continue; // -> ScrollbarRounding, ScrollbarSize
            case make_condition(447, 'e'): state = 448; continue; // -> SelectableTextAlign
            case make_condition(464, 'a'): state = 465; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(499, 'B'): state = 500; continue; // -> TabBarBorderSize, TabBarOverlineSize, TabBorderSize
            case make_condition(499, 'C'): state = 534; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(499, 'R'): state = 571; continue; // -> TabRounding
            case make_condition(499, 'l'): state = 579; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(609, 'c'): state = 610; continue; // -> TouchExtraPadding
            case make_condition(626, 'd'): state = 627; continue; // -> WindowBorderHoverPadding, WindowBorderSize, WindowMenuButtonPosition, WindowMinSize, WindowPadding, WindowRounding, WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 4:
            switch (make_condition(state, key[i])) {
            case make_condition(4, 'a'): state = 5; continue; // -> Alpha
            case make_condition(8, 'A'): state = 9; continue; // -> AntiAliasedFill, AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(34, 'o'): state = 35; continue; // -> ButtonTextAlign
            case make_condition(49, 'P'): state = 50; continue; // -> CellPadding
            case make_condition(59, 'd'): state = 60; continue; // -> ChildBorderSize, ChildRounding
            case make_condition(81, 'l'): state = 82; continue; // -> CircleTessellationMaxError
            case make_condition(106, 'r'): state = 107; continue; // -> ColorButtonPosition, Colors
            case make_condition(123, 'm'): state = 124; continue; // -> ColumnsMinSpacing
            case make_condition(139, 'e'): state = 140; continue; // -> CurveTessellationTol
            case make_condition(159, 'b'): state = 160; continue; // -> DisabledAlpha
            case make_condition(169, 'l'): state = 170; continue; // -> DisplaySafeAreaPadding, DisplayWindowPadding
            case make_condition(204, 'e'): state = 205; continue; // -> FrameBorderSize, FramePadding, FrameRounding
            case make_condition(234, 'M'): state = 235; continue; // -> GrabMinSize
            case make_condition(234, 'R'): state = 242; continue; // -> GrabRounding
            case make_condition(253, 'r'): state = 254; continue; // -> HoverDelayNormal, HoverDelayShort, HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav, HoverStationaryDelay
            case make_condition(312, 'e'): state = 313; continue; // -> ImageBorderSize
            case make_condition(326, 'n'): state = 327; continue; // -> IndentSpacing
            case make_condition(338, 'I'): state = 339; continue; // -> ItemInnerSpacing
            case make_condition(338, 'S'): state = 351; continue; // -> ItemSpacing
            case make_condition(361, 'l'): state = 362; continue; // -> LogSliderDeadzone
            case make_condition(378, 'e'): state = 379; continue; // -> MouseCursorScale
            case make_condition(394, 'p'): state = 395; continue; // -> PopupBorderSize, PopupRounding
            case make_condition(417, 'e'): state = 418; continue; // -> ScaleAllSizes
            case make_condition(428, 'l'): state = 429; continue; // -> ScrollbarRounding, ScrollbarSize
            case make_condition(448, 'c'): state = 449; continue; // -> SelectableTextAlign
            case make_condition(465, 'r'): state = 466; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(500, 'a'): state = 501; continue; // -> TabBarBorderSize, TabBarOverlineSize
            case make_condition(500, 'o'): state = 525; continue; // -> TabBorderSize
            case make_condition(534, 'l'): state = 535; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(571, 'o'): state = 572; continue; // -> TabRounding
            case make_condition(579, 'e'): state = 580; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(610, 'h'): state = 611; continue; // -> TouchExtraPadding
            case make_condition(627, 'o'): state = 628; continue; // -> WindowBorderHoverPadding, WindowBorderSize, WindowMenuButtonPosition, WindowMinSize, WindowPadding, WindowRounding, WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 5:
            switch (make_condition(state, key[i])) {
            case make_condition(9, 'l'): state = 10; continue; // -> AntiAliasedFill, AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(35, 'n'): state = 36; continue; // -> ButtonTextAlign
            case make_condition(50, 'a'): state = 51; continue; // -> CellPadding
            case make_condition(60, 'B'): state = 61; continue; // -> ChildBorderSize
            case make_condition(60, 'R'): state = 71; continue; // -> ChildRounding
            case make_condition(82, 'e'): state = 83; continue; // -> CircleTessellationMaxError
            case make_condition(107, 'B'): state = 108; continue; // -> ColorButtonPosition
            case make_condition(107, 's'): state = 122; continue; // -> Colors
            case make_condition(124, 'n'): state = 125; continue; // -> ColumnsMinSpacing
            case make_condition(140, 'T'): state = 141; continue; // -> CurveTessellationTol
            case make_condition(160, 'l'): state = 161; continue; // -> DisabledAlpha
            case make_condition(170, 'a'): state = 171; continue; // -> DisplaySafeAreaPadding, DisplayWindowPadding
            case make_condition(205, 'B'): state = 206; continue; // -> FrameBorderSize
            case make_condition(205, 'P'): state = 216; continue; // -> FramePadding
            case make_condition(205, 'R'): state = 223; continue; // -> FrameRounding
            case make_condition(235, 'i'): state = 236; continue; // -> GrabMinSize
            case make_condition(242, 'o'): state = 243; continue; // -> GrabRounding
            case make_condition(254, 'D'): state = 255; continue; // -> HoverDelayNormal, HoverDelayShort
            case make_condition(254, 'F'): state = 271; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(254, 'S'): state = 294; continue; // -> HoverStationaryDelay
            case make_condition(313, 'B'): state = 314; continue; // -> ImageBorderSize
            case make_condition(327, 't'): state = 328; continue; // -> IndentSpacing
            case make_condition(339, 'n'): state = 340; continue; // -> ItemInnerSpacing
            case make_condition(351, 'p'): state = 352; continue; // -> ItemSpacing
            case make_condition(362, 'i'): state = 363; continue; // -> LogSliderDeadzone
            case make_condition(379, 'C'): state = 380; continue; // -> MouseCursorScale
            case make_condition(395, 'B'): state = 396; continue; // -> PopupBorderSize
            case make_condition(395, 'R'): state = 406; continue; // -> PopupRounding
            case make_condition(418, 'A'): state = 419; continue; // -> ScaleAllSizes
            case make_condition(429, 'l'): state = 430; continue; // -> ScrollbarRounding, ScrollbarSize
            case make_condition(449, 't'): state = 450; continue; // -> SelectableTextAlign
            case make_condition(466, 'a'): state = 467; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(501, 'r'): state = 502; continue; // -> TabBarBorderSize, TabBarOverlineSize
            case make_condition(525, 'r'): state = 526; continue; // -> TabBorderSize
            case make_condition(535, 'o'): state = 536; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(572, 'u'): state = 573; continue; // -> TabRounding
            case make_condition(580, 'A'): state = 581; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(611, 'E'): state = 612; continue; // -> TouchExtraPadding
            case make_condition(628, 'w'): state = 629; continue; // -> WindowBorderHoverPadding, WindowBorderSize, WindowMenuButtonPosition, WindowMinSize, WindowPadding, WindowRounding, WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 6:
            switch (make_condition(state, key[i])) {
            case make_condition(10, 'i'): state = 11; continue; // -> AntiAliasedFill, AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(36, 'T'): state = 37; continue; // -> ButtonTextAlign
            case make_condition(51, 'd'): state = 52; continue; // -> CellPadding
            case make_condition(61, 'o'): state = 62; continue; // -> ChildBorderSize
            case make_condition(71, 'o'): state = 72; continue; // -> ChildRounding
            case make_condition(83, 'T'): state = 84; continue; // -> CircleTessellationMaxError
            case make_condition(108, 'u'): state = 109; continue; // -> ColorButtonPosition
            case make_condition(125, 's'): state = 126; continue; // -> ColumnsMinSpacing
            case make_condition(141, 'e'): state = 142; continue; // -> CurveTessellationTol
            case make_condition(161, 'e'): state = 162; continue; // -> DisabledAlpha
            case make_condition(171, 'y'): state = 172; continue; // -> DisplaySafeAreaPadding, DisplayWindowPadding
            case make_condition(206, 'o'): state = 207; continue; // -> FrameBorderSize
            case make_condition(216, 'a'): state = 217; continue; // -> FramePadding
            case make_condition(223, 'o'): state = 224; continue; // -> FrameRounding
            case make_condition(236, 'n'): state = 237; continue; // -> GrabMinSize
            case make_condition(243, 'u'): state = 244; continue; // -> GrabRounding
            case make_condition(255, 'e'): state = 256; continue; // -> HoverDelayNormal, HoverDelayShort
            case make_condition(271, 'l'): state = 272; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(294, 't'): state = 295; continue; // -> HoverStationaryDelay
            case make_condition(314, 'o'): state = 315; continue; // -> ImageBorderSize
            case make_condition(328, 'S'): state = 329; continue; // -> IndentSpacing
            case make_condition(340, 'n'): state = 341; continue; // -> ItemInnerSpacing
            case make_condition(352, 'a'): state = 353; continue; // -> ItemSpacing
            case make_condition(363, 'd'): state = 364; continue; // -> LogSliderDeadzone
            case make_condition(380, 'u'): state = 381; continue; // -> MouseCursorScale
            case make_condition(396, 'o'): state = 397; continue; // -> PopupBorderSize
            case make_condition(406, 'o'): state = 407; continue; // -> PopupRounding
            case make_condition(419, 'l'): state = 420; continue; // -> ScaleAllSizes
            case make_condition(430, 'b'): state = 431; continue; // -> ScrollbarRounding, ScrollbarSize
            case make_condition(450, 'a'): state = 451; continue; // -> SelectableTextAlign
            case make_condition(467, 't'): state = 468; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(502, 'B'): state = 503; continue; // -> TabBarBorderSize
            case make_condition(502, 'O'): state = 513; continue; // -> TabBarOverlineSize
            case make_condition(526, 'd'): state = 527; continue; // -> TabBorderSize
            case make_condition(536, 's'): state = 537; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(573, 'n'): state = 574; continue; // -> TabRounding
            case make_condition(581, 'n'): state = 582; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(612, 'x'): state = 613; continue; // -> TouchExtraPadding
            case make_condition(629, 'B'): state = 630; continue; // -> WindowBorderHoverPadding, WindowBorderSize
            case make_condition(629, 'M'): state = 652; continue; // -> WindowMenuButtonPosition, WindowMinSize
            case make_condition(629, 'P'): state = 676; continue; // -> WindowPadding
            case make_condition(629, 'R'): state = 683; continue; // -> WindowRounding
            case make_condition(629, 'T'): state = 691; continue; // -> WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 7:
            switch (make_condition(state, key[i])) {
            case make_condition(11, 'a'): state = 12; continue; // -> AntiAliasedFill, AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(37, 'e'): state = 38; continue; // -> ButtonTextAlign
            case make_condition(52, 'd'): state = 53; continue; // -> CellPadding
            case make_condition(62, 'r'): state = 63; continue; // -> ChildBorderSize
            case make_condition(72, 'u'): state = 73; continue; // -> ChildRounding
            case make_condition(84, 'e'): state = 85; continue; // -> CircleTessellationMaxError
            case make_condition(109, 't'): state = 110; continue; // -> ColorButtonPosition
            case make_condition(126, 'M'): state = 127; continue; // -> ColumnsMinSpacing
            case make_condition(142, 's'): state = 143; continue; // -> CurveTessellationTol
            case make_condition(162, 'd'): state = 163; continue; // -> DisabledAlpha
            case make_condition(172, 'S'): state = 173; continue; // -> DisplaySafeAreaPadding
            case make_condition(172, 'W'): state = 188; continue; // -> DisplayWindowPadding
            case make_condition(207, 'r'): state = 208; continue; // -> FrameBorderSize
            case make_condition(217, 'd'): state = 218; continue; // -> FramePadding
            case make_condition(224, 'u'): state = 225; continue; // -> FrameRounding
            case make_condition(237, 'S'): state = 238; continue; // -> GrabMinSize
            case make_condition(244, 'n'): state = 245; continue; // -> GrabRounding
            case make_condition(256, 'l'): state = 257; continue; // -> HoverDelayNormal, HoverDelayShort
            case make_condition(272, 'a'): state = 273; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(295, 'a'): state = 296; continue; // -> HoverStationaryDelay
            case make_condition(315, 'r'): state = 316; continue; // -> ImageBorderSize
            case make_condition(329, 'p'): state = 330; continue; // -> IndentSpacing
            case make_condition(341, 'e'): state = 342; continue; // -> ItemInnerSpacing
            case make_condition(353, 'c'): state = 354; continue; // -> ItemSpacing
            case make_condition(364, 'e'): state = 365; continue; // -> LogSliderDeadzone
            case make_condition(381, 'r'): state = 382; continue; // -> MouseCursorScale
            case make_condition(397, 'r'): state = 398; continue; // -> PopupBorderSize
            case make_condition(407, 'u'): state = 408; continue; // -> PopupRounding
            case make_condition(420, 'l'): state = 421; continue; // -> ScaleAllSizes
            case make_condition(431, 'a'): state = 432; continue; // -> ScrollbarRounding, ScrollbarSize
            case make_condition(451, 'b'): state = 452; continue; // -> SelectableTextAlign
            case make_condition(468, 'o'): state = 469; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(503, 'o'): state = 504; continue; // -> TabBarBorderSize
            case make_condition(513, 'v'): state = 514; continue; // -> TabBarOverlineSize
            case make_condition(527, 'e'): state = 528; continue; // -> TabBorderSize
            case make_condition(537, 'e'): state = 538; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(574, 'd'): state = 575; continue; // -> TabRounding
            case make_condition(582, 'g'): state = 583; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(613, 't'): state = 614; continue; // -> TouchExtraPadding
            case make_condition(630, 'o'): state = 631; continue; // -> WindowBorderHoverPadding, WindowBorderSize
            case make_condition(652, 'e'): state = 653; continue; // -> WindowMenuButtonPosition
            case make_condition(652, 'i'): state = 670; continue; // -> WindowMinSize
            case make_condition(676, 'a'): state = 677; continue; // -> WindowPadding
            case make_condition(683, 'o'): state = 684; continue; // -> WindowRounding
            case make_condition(691, 'i'): state = 692; continue; // -> WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 8:
            switch (make_condition(state, key[i])) {
            case make_condition(12, 's'): state = 13; continue; // -> AntiAliasedFill, AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(38, 'x'): state = 39; continue; // -> ButtonTextAlign
            case make_condition(53, 'i'): state = 54; continue; // -> CellPadding
            case make_condition(63, 'd'): state = 64; continue; // -> ChildBorderSize
            case make_condition(73, 'n'): state = 74; continue; // -> ChildRounding
            case make_condition(85, 's'): state = 86; continue; // -> CircleTessellationMaxError
            case make_condition(110, 't'): state = 111; continue; // -> ColorButtonPosition
            case make_condition(127, 'i'): state = 128; continue; // -> ColumnsMinSpacing
            case make_condition(143, 's'): state = 144; continue; // -> CurveTessellationTol
            case make_condition(163, 'A'): state = 164; continue; // -> DisabledAlpha
            case make_condition(173, 'a'): state = 174; continue; // -> DisplaySafeAreaPadding
            case make_condition(188, 'i'): state = 189; continue; // -> DisplayWindowPadding
            case make_condition(208, 'd'): state = 209; continue; // -> FrameBorderSize
            case make_condition(218, 'd'): state = 219; continue; // -> FramePadding
            case make_condition(225, 'n'): state = 226; continue; // -> FrameRounding
            case make_condition(238, 'i'): state = 239; continue; // -> GrabMinSize
            case make_condition(245, 'd'): state = 246; continue; // -> GrabRounding
            case make_condition(257, 'a'): state = 258; continue; // -> HoverDelayNormal, HoverDelayShort
            case make_condition(273, 'g'): state = 274; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(296, 't'): state = 297; continue; // -> HoverStationaryDelay
            case make_condition(316, 'd'): state = 317; continue; // -> ImageBorderSize
            case make_condition(330, 'a'): state = 331; continue; // -> IndentSpacing
            case make_condition(342, 'r'): state = 343; continue; // -> ItemInnerSpacing
            case make_condition(354, 'i'): state = 355; continue; // -> ItemSpacing
            case make_condition(365, 'r'): state = 366; continue; // -> LogSliderDeadzone
            case make_condition(382, 's'): state = 383; continue; // -> MouseCursorScale
            case make_condition(398, 'd'): state = 399; continue; // -> PopupBorderSize
            case make_condition(408, 'n'): state = 409; continue; // -> PopupRounding
            case make_condition(421, 'S'): state = 422; continue; // -> ScaleAllSizes
            case make_condition(432, 'r'): state = 433; continue; // -> ScrollbarRounding, ScrollbarSize
            case make_condition(452, 'l'): state = 453; continue; // -> SelectableTextAlign
            case make_condition(469, 'r'): state = 470; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(504, 'r'): state = 505; continue; // -> TabBarBorderSize
            case make_condition(514, 'e'): state = 515; continue; // -> TabBarOverlineSize
            case make_condition(528, 'r'): state = 529; continue; // -> TabBorderSize
            case make_condition(538, 'B'): state = 539; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(575, 'i'): state = 576; continue; // -> TabRounding
            case make_condition(583, 'l'): state = 584; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(614, 'r'): state = 615; continue; // -> TouchExtraPadding
            case make_condition(631, 'r'): state = 632; continue; // -> WindowBorderHoverPadding, WindowBorderSize
            case make_condition(653, 'n'): state = 654; continue; // -> WindowMenuButtonPosition
            case make_condition(670, 'n'): state = 671; continue; // -> WindowMinSize
            case make_condition(677, 'd'): state = 678; continue; // -> WindowPadding
            case make_condition(684, 'u'): state = 685; continue; // -> WindowRounding
            case make_condition(692, 't'): state = 693; continue; // -> WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 9:
            switch (make_condition(state, key[i])) {
            case make_condition(13, 'e'): state = 14; continue; // -> AntiAliasedFill, AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(39, 't'): state = 40; continue; // -> ButtonTextAlign
            case make_condition(54, 'n'): state = 55; continue; // -> CellPadding
            case make_condition(64, 'e'): state = 65; continue; // -> ChildBorderSize
            case make_condition(74, 'd'): state = 75; continue; // -> ChildRounding
            case make_condition(86, 's'): state = 87; continue; // -> CircleTessellationMaxError
            case make_condition(111, 'o'): state = 112; continue; // -> ColorButtonPosition
            case make_condition(128, 'n'): state = 129; continue; // -> ColumnsMinSpacing
            case make_condition(144, 'e'): state = 145; continue; // -> CurveTessellationTol
            case make_condition(164, 'l'): state = 165; continue; // -> DisabledAlpha
            case make_condition(174, 'f'): state = 175; continue; // -> DisplaySafeAreaPadding
            case make_condition(189, 'n'): state = 190; continue; // -> DisplayWindowPadding
            case make_condition(209, 'e'): state = 210; continue; // -> FrameBorderSize
            case make_condition(219, 'i'): state = 220; continue; // -> FramePadding
            case make_condition(226, 'd'): state = 227; continue; // -> FrameRounding
            case make_condition(239, 'z'): state = 240; continue; // -> GrabMinSize
            case make_condition(246, 'i'): state = 247; continue; // -> GrabRounding
            case make_condition(258, 'y'): state = 259; continue; // -> HoverDelayNormal, HoverDelayShort
            case make_condition(274, 's'): state = 275; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(297, 'i'): state = 298; continue; // -> HoverStationaryDelay
            case make_condition(317, 'e'): state = 318; continue; // -> ImageBorderSize
            case make_condition(331, 'c'): state = 332; continue; // -> IndentSpacing
            case make_condition(343, 'S'): state = 344; continue; // -> ItemInnerSpacing
            case make_condition(355, 'n'): state = 356; continue; // -> ItemSpacing
            case make_condition(366, 'D'): state = 367; continue; // -> LogSliderDeadzone
            case make_condition(383, 'o'): state = 384; continue; // -> MouseCursorScale
            case make_condition(399, 'e'): state = 400; continue; // -> PopupBorderSize
            case make_condition(409, 'd'): state = 410; continue; // -> PopupRounding
            case make_condition(422, 'i'): state = 423; continue; // -> ScaleAllSizes
            case make_condition(433, 'R'): state = 434; continue; // -> ScrollbarRounding
            case make_condition(433, 'S'): state = 442; continue; // -> ScrollbarSize
            case make_condition(453, 'e'): state = 454; continue; // -> SelectableTextAlign
            case make_condition(470, 'T'): state = 471; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(505, 'd'): state = 506; continue; // -> TabBarBorderSize
            case make_condition(515, 'r'): state = 516; continue; // -> TabBarOverlineSize
            case make_condition(529, 'S'): state = 530; continue; // -> TabBorderSize
            case make_condition(539, 'u'): state = 540; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(576, 'n'): state = 577; continue; // -> TabRounding
            case make_condition(584, 'e'): state = 585; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(615, 'a'): state = 616; continue; // -> TouchExtraPadding
            case make_condition(632, 'd'): state = 633; continue; // -> WindowBorderHoverPadding, WindowBorderSize
            case make_condition(654, 'u'): state = 655; continue; // -> WindowMenuButtonPosition
            case make_condition(671, 'S'): state = 672; continue; // -> WindowMinSize
            case make_condition(678, 'd'): state = 679; continue; // -> WindowPadding
            case make_condition(685, 'n'): state = 686; continue; // -> WindowRounding
            case make_condition(693, 'l'): state = 694; continue; // -> WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 10:
            switch (make_condition(state, key[i])) {
            case make_condition(14, 'd'): state = 15; continue; // -> AntiAliasedFill, AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(40, 'A'): state = 41; continue; // -> ButtonTextAlign
            case make_condition(55, 'g'): state = 56; continue; // -> CellPadding
            case make_condition(65, 'r'): state = 66; continue; // -> ChildBorderSize
            case make_condition(75, 'i'): state = 76; continue; // -> ChildRounding
            case make_condition(87, 'e'): state = 88; continue; // -> CircleTessellationMaxError
            case make_condition(112, 'n'): state = 113; continue; // -> ColorButtonPosition
            case make_condition(129, 'S'): state = 130; continue; // -> ColumnsMinSpacing
            case make_condition(145, 'l'): state = 146; continue; // -> CurveTessellationTol
            case make_condition(165, 'p'): state = 166; continue; // -> DisabledAlpha
            case make_condition(175, 'e'): state = 176; continue; // -> DisplaySafeAreaPadding
            case make_condition(190, 'd'): state = 191; continue; // -> DisplayWindowPadding
            case make_condition(210, 'r'): state = 211; continue; // -> FrameBorderSize
            case make_condition(220, 'n'): state = 221; continue; // -> FramePadding
            case make_condition(227, 'i'): state = 228; continue; // -> FrameRounding
            case make_condition(240, 'e'): state = 241; continue; // -> GrabMinSize
            case make_condition(247, 'n'): state = 248; continue; // -> GrabRounding
            case make_condition(259, 'N'): state = 260; continue; // -> HoverDelayNormal
            case make_condition(259, 'S'): state = 266; continue; // -> HoverDelayShort
            case make_condition(275, 'F'): state = 276; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(298, 'o'): state = 299; continue; // -> HoverStationaryDelay
            case make_condition(318, 'r'): state = 319; continue; // -> ImageBorderSize
            case make_condition(332, 'i'): state = 333; continue; // -> IndentSpacing
            case make_condition(344, 'p'): state = 345; continue; // -> ItemInnerSpacing
            case make_condition(356, 'g'): state = 357; continue; // -> ItemSpacing
            case make_condition(367, 'e'): state = 368; continue; // -> LogSliderDeadzone
            case make_condition(384, 'r'): state = 385; continue; // -> MouseCursorScale
            case make_condition(400, 'r'): state = 401; continue; // -> PopupBorderSize
            case make_condition(410, 'i'): state = 411; continue; // -> PopupRounding
            case make_condition(423, 'z'): state = 424; continue; // -> ScaleAllSizes
            case make_condition(434, 'o'): state = 435; continue; // -> ScrollbarRounding
            case make_condition(442, 'i'): state = 443; continue; // -> ScrollbarSize
            case make_condition(454, 'T'): state = 455; continue; // -> SelectableTextAlign
            case make_condition(471, 'e'): state = 472; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(506, 'e'): state = 507; continue; // -> TabBarBorderSize
            case make_condition(516, 'l'): state = 517; continue; // -> TabBarOverlineSize
            case make_condition(530, 'i'): state = 531; continue; // -> TabBorderSize
            case make_condition(540, 't'): state = 541; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(577, 'g'): state = 578; continue; // -> TabRounding
            case make_condition(585, 'd'): state = 586; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(616, 'P'): state = 617; continue; // -> TouchExtraPadding
            case make_condition(633, 'e'): state = 634; continue; // -> WindowBorderHoverPadding, WindowBorderSize
            case make_condition(655, 'B'): state = 656; continue; // -> WindowMenuButtonPosition
            case make_condition(672, 'i'): state = 673; continue; // -> WindowMinSize
            case make_condition(679, 'i'): state = 680; continue; // -> WindowPadding
            case make_condition(686, 'd'): state = 687; continue; // -> WindowRounding
            case make_condition(694, 'e'): state = 695; continue; // -> WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 11:
            switch (make_condition(state, key[i])) {
            case make_condition(15, 'F'): state = 16; continue; // -> AntiAliasedFill
            case make_condition(15, 'L'): state = 20; continue; // -> AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(41, 'l'): state = 42; continue; // -> ButtonTextAlign
            case make_condition(66, 'S'): state = 67; continue; // -> ChildBorderSize
            case make_condition(76, 'n'): state = 77; continue; // -> ChildRounding
            case make_condition(88, 'l'): state = 89; continue; // -> CircleTessellationMaxError
            case make_condition(113, 'P'): state = 114; continue; // -> ColorButtonPosition
            case make_condition(130, 'p'): state = 131; continue; // -> ColumnsMinSpacing
            case make_condition(146, 'l'): state = 147; continue; // -> CurveTessellationTol
            case make_condition(166, 'h'): state = 167; continue; // -> DisabledAlpha
            case make_condition(176, 'A'): state = 177; continue; // -> DisplaySafeAreaPadding
            case make_condition(191, 'o'): state = 192; continue; // -> DisplayWindowPadding
            case make_condition(211, 'S'): state = 212; continue; // -> FrameBorderSize
            case make_condition(221, 'g'): state = 222; continue; // -> FramePadding
            case make_condition(228, 'n'): state = 229; continue; // -> FrameRounding
            case make_condition(248, 'g'): state = 249; continue; // -> GrabRounding
            case make_condition(260, 'o'): state = 261; continue; // -> HoverDelayNormal
            case make_condition(266, 'h'): state = 267; continue; // -> HoverDelayShort
            case make_condition(276, 'o'): state = 277; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(299, 'n'): state = 300; continue; // -> HoverStationaryDelay
            case make_condition(319, 'S'): state = 320; continue; // -> ImageBorderSize
            case make_condition(333, 'n'): state = 334; continue; // -> IndentSpacing
            case make_condition(345, 'a'): state = 346; continue; // -> ItemInnerSpacing
            case make_condition(368, 'a'): state = 369; continue; // -> LogSliderDeadzone
            case make_condition(385, 'S'): state = 386; continue; // -> MouseCursorScale
            case make_condition(401, 'S'): state = 402; continue; // -> PopupBorderSize
            case make_condition(411, 'n'): state = 412; continue; // -> PopupRounding
            case make_condition(424, 'e'): state = 425; continue; // -> ScaleAllSizes
            case make_condition(435, 'u'): state = 436; continue; // -> ScrollbarRounding
            case make_condition(443, 'z'): state = 444; continue; // -> ScrollbarSize
            case make_condition(455, 'e'): state = 456; continue; // -> SelectableTextAlign
            case make_condition(472, 'x'): state = 473; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(507, 'r'): state = 508; continue; // -> TabBarBorderSize
            case make_condition(517, 'i'): state = 518; continue; // -> TabBarOverlineSize
            case make_condition(531, 'z'): state = 532; continue; // -> TabBorderSize
            case make_condition(541, 't'): state = 542; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(586, 'H'): state = 587; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(617, 'a'): state = 618; continue; // -> TouchExtraPadding
            case make_condition(634, 'r'): state = 635; continue; // -> WindowBorderHoverPadding, WindowBorderSize
            case make_condition(656, 'u'): state = 657; continue; // -> WindowMenuButtonPosition
            case make_condition(673, 'z'): state = 674; continue; // -> WindowMinSize
            case make_condition(680, 'n'): state = 681; continue; // -> WindowPadding
            case make_condition(687, 'i'): state = 688; continue; // -> WindowRounding
            case make_condition(695, 'A'): state = 696; continue; // -> WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 12:
            switch (make_condition(state, key[i])) {
            case make_condition(16, 'i'): state = 17; continue; // -> AntiAliasedFill
            case make_condition(20, 'i'): state = 21; continue; // -> AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(42, 'i'): state = 43; continue; // -> ButtonTextAlign
            case make_condition(67, 'i'): state = 68; continue; // -> ChildBorderSize
            case make_condition(77, 'g'): state = 78; continue; // -> ChildRounding
            case make_condition(89, 'l'): state = 90; continue; // -> CircleTessellationMaxError
            case make_condition(114, 'o'): state = 115; continue; // -> ColorButtonPosition
            case make_condition(131, 'a'): state = 132; continue; // -> ColumnsMinSpacing
            case make_condition(147, 'a'): state = 148; continue; // -> CurveTessellationTol
            case make_condition(167, 'a'): state = 168; continue; // -> DisabledAlpha
            case make_condition(177, 'r'): state = 178; continue; // -> DisplaySafeAreaPadding
            case make_condition(192, 'w'): state = 193; continue; // -> DisplayWindowPadding
            case make_condition(212, 'i'): state = 213; continue; // -> FrameBorderSize
            case make_condition(229, 'g'): state = 230; continue; // -> FrameRounding
            case make_condition(261, 'r'): state = 262; continue; // -> HoverDelayNormal
            case make_condition(267, 'o'): state = 268; continue; // -> HoverDelayShort
            case make_condition(277, 'r'): state = 278; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(300, 'a'): state = 301; continue; // -> HoverStationaryDelay
            case make_condition(320, 'i'): state = 321; continue; // -> ImageBorderSize
            case make_condition(334, 'g'): state = 335; continue; // -> IndentSpacing
            case make_condition(346, 'c'): state = 347; continue; // -> ItemInnerSpacing
            case make_condition(369, 'd'): state = 370; continue; // -> LogSliderDeadzone
            case make_condition(386, 'c'): state = 387; continue; // -> MouseCursorScale
            case make_condition(402, 'i'): state = 403; continue; // -> PopupBorderSize
            case make_condition(412, 'g'): state = 413; continue; // -> PopupRounding
            case make_condition(425, 's'): state = 426; continue; // -> ScaleAllSizes
            case make_condition(436, 'n'): state = 437; continue; // -> ScrollbarRounding
            case make_condition(444, 'e'): state = 445; continue; // -> ScrollbarSize
            case make_condition(456, 'x'): state = 457; continue; // -> SelectableTextAlign
            case make_condition(473, 't'): state = 474; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(508, 'S'): state = 509; continue; // -> TabBarBorderSize
            case make_condition(518, 'n'): state = 519; continue; // -> TabBarOverlineSize
            case make_condition(532, 'e'): state = 533; continue; // -> TabBorderSize
            case make_condition(542, 'o'): state = 543; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(587, 'e'): state = 588; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(618, 'd'): state = 619; continue; // -> TouchExtraPadding
            case make_condition(635, 'H'): state = 636; continue; // -> WindowBorderHoverPadding
            case make_condition(635, 'S'): state = 648; continue; // -> WindowBorderSize
            case make_condition(657, 't'): state = 658; continue; // -> WindowMenuButtonPosition
            case make_condition(674, 'e'): state = 675; continue; // -> WindowMinSize
            case make_condition(681, 'g'): state = 682; continue; // -> WindowPadding
            case make_condition(688, 'n'): state = 689; continue; // -> WindowRounding
            case make_condition(696, 'l'): state = 697; continue; // -> WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 13:
            switch (make_condition(state, key[i])) {
            case make_condition(17, 'l'): state = 18; continue; // -> AntiAliasedFill
            case make_condition(21, 'n'): state = 22; continue; // -> AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(43, 'g'): state = 44; continue; // -> ButtonTextAlign
            case make_condition(68, 'z'): state = 69; continue; // -> ChildBorderSize
            case make_condition(90, 'a'): state = 91; continue; // -> CircleTessellationMaxError
            case make_condition(115, 's'): state = 116; continue; // -> ColorButtonPosition
            case make_condition(132, 'c'): state = 133; continue; // -> ColumnsMinSpacing
            case make_condition(148, 't'): state = 149; continue; // -> CurveTessellationTol
            case make_condition(178, 'e'): state = 179; continue; // -> DisplaySafeAreaPadding
            case make_condition(193, 'P'): state = 194; continue; // -> DisplayWindowPadding
            case make_condition(213, 'z'): state = 214; continue; // -> FrameBorderSize
            case make_condition(262, 'm'): state = 263; continue; // -> HoverDelayNormal
            case make_condition(268, 'r'): state = 269; continue; // -> HoverDelayShort
            case make_condition(278, 'T'): state = 279; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(301, 'r'): state = 302; continue; // -> HoverStationaryDelay
            case make_condition(321, 'z'): state = 322; continue; // -> ImageBorderSize
            case make_condition(347, 'i'): state = 348; continue; // -> ItemInnerSpacing
            case make_condition(370, 'z'): state = 371; continue; // -> LogSliderDeadzone
            case make_condition(387, 'a'): state = 388; continue; // -> MouseCursorScale
            case make_condition(403, 'z'): state = 404; continue; // -> PopupBorderSize
            case make_condition(437, 'd'): state = 438; continue; // -> ScrollbarRounding
            case make_condition(457, 't'): state = 458; continue; // -> SelectableTextAlign
            case make_condition(474, 'A'): state = 475; continue; // -> SeparatorTextAlign
            case make_condition(474, 'B'): state = 480; continue; // -> SeparatorTextBorderSize
            case make_condition(474, 'P'): state = 490; continue; // -> SeparatorTextPadding
            case make_condition(509, 'i'): state = 510; continue; // -> TabBarBorderSize
            case make_condition(519, 'e'): state = 520; continue; // -> TabBarOverlineSize
            case make_condition(543, 'n'): state = 544; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(588, 'a'): state = 589; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(619, 'd'): state = 620; continue; // -> TouchExtraPadding
            case make_condition(636, 'o'): state = 637; continue; // -> WindowBorderHoverPadding
            case make_condition(648, 'i'): state = 649; continue; // -> WindowBorderSize
            case make_condition(658, 't'): state = 659; continue; // -> WindowMenuButtonPosition
            case make_condition(689, 'g'): state = 690; continue; // -> WindowRounding
            case make_condition(697, 'i'): state = 698; continue; // -> WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 14:
            switch (make_condition(state, key[i])) {
            case make_condition(18, 'l'): state = 19; continue; // -> AntiAliasedFill
            case make_condition(22, 'e'): state = 23; continue; // -> AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(44, 'n'): state = 45; continue; // -> ButtonTextAlign
            case make_condition(69, 'e'): state = 70; continue; // -> ChildBorderSize
            case make_condition(91, 't'): state = 92; continue; // -> CircleTessellationMaxError
            case make_condition(116, 'i'): state = 117; continue; // -> ColorButtonPosition
            case make_condition(133, 'i'): state = 134; continue; // -> ColumnsMinSpacing
            case make_condition(149, 'i'): state = 150; continue; // -> CurveTessellationTol
            case make_condition(179, 'a'): state = 180; continue; // -> DisplaySafeAreaPadding
            case make_condition(194, 'a'): state = 195; continue; // -> DisplayWindowPadding
            case make_condition(214, 'e'): state = 215; continue; // -> FrameBorderSize
            case make_condition(263, 'a'): state = 264; continue; // -> HoverDelayNormal
            case make_condition(269, 't'): state = 270; continue; // -> HoverDelayShort
            case make_condition(279, 'o'): state = 280; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(302, 'y'): state = 303; continue; // -> HoverStationaryDelay
            case make_condition(322, 'e'): state = 323; continue; // -> ImageBorderSize
            case make_condition(348, 'n'): state = 349; continue; // -> ItemInnerSpacing
            case make_condition(371, 'o'): state = 372; continue; // -> LogSliderDeadzone
            case make_condition(388, 'l'): state = 389; continue; // -> MouseCursorScale
            case make_condition(404, 'e'): state = 405; continue; // -> PopupBorderSize
            case make_condition(438, 'i'): state = 439; continue; // -> ScrollbarRounding
            case make_condition(458, 'A'): state = 459; continue; // -> SelectableTextAlign
            case make_condition(475, 'l'): state = 476; continue; // -> SeparatorTextAlign
            case make_condition(480, 'o'): state = 481; continue; // -> SeparatorTextBorderSize
            case make_condition(490, 'a'): state = 491; continue; // -> SeparatorTextPadding
            case make_condition(510, 'z'): state = 511; continue; // -> TabBarBorderSize
            case make_condition(520, 'S'): state = 521; continue; // -> TabBarOverlineSize
            case make_condition(544, 'M'): state = 545; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(589, 'd'): state = 590; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(620, 'i'): state = 621; continue; // -> TouchExtraPadding
            case make_condition(637, 'v'): state = 638; continue; // -> WindowBorderHoverPadding
            case make_condition(649, 'z'): state = 650; continue; // -> WindowBorderSize
            case make_condition(659, 'o'): state = 660; continue; // -> WindowMenuButtonPosition
            case make_condition(698, 'g'): state = 699; continue; // -> WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 15:
            switch (make_condition(state, key[i])) {
            case make_condition(23, 's'): state = 24; continue; // -> AntiAliasedLines, AntiAliasedLinesUseTex
            case make_condition(92, 'i'): state = 93; continue; // -> CircleTessellationMaxError
            case make_condition(117, 't'): state = 118; continue; // -> ColorButtonPosition
            case make_condition(134, 'n'): state = 135; continue; // -> ColumnsMinSpacing
            case make_condition(150, 'o'): state = 151; continue; // -> CurveTessellationTol
            case make_condition(180, 'P'): state = 181; continue; // -> DisplaySafeAreaPadding
            case make_condition(195, 'd'): state = 196; continue; // -> DisplayWindowPadding
            case make_condition(264, 'l'): state = 265; continue; // -> HoverDelayNormal
            case make_condition(280, 'o'): state = 281; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(303, 'D'): state = 304; continue; // -> HoverStationaryDelay
            case make_condition(349, 'g'): state = 350; continue; // -> ItemInnerSpacing
            case make_condition(372, 'n'): state = 373; continue; // -> LogSliderDeadzone
            case make_condition(389, 'e'): state = 390; continue; // -> MouseCursorScale
            case make_condition(439, 'n'): state = 440; continue; // -> ScrollbarRounding
            case make_condition(459, 'l'): state = 460; continue; // -> SelectableTextAlign
            case make_condition(476, 'i'): state = 477; continue; // -> SeparatorTextAlign
            case make_condition(481, 'r'): state = 482; continue; // -> SeparatorTextBorderSize
            case make_condition(491, 'd'): state = 492; continue; // -> SeparatorTextPadding
            case make_condition(511, 'e'): state = 512; continue; // -> TabBarBorderSize
            case make_condition(521, 'i'): state = 522; continue; // -> TabBarOverlineSize
            case make_condition(545, 'i'): state = 546; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(590, 'e'): state = 591; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(621, 'n'): state = 622; continue; // -> TouchExtraPadding
            case make_condition(638, 'e'): state = 639; continue; // -> WindowBorderHoverPadding
            case make_condition(650, 'e'): state = 651; continue; // -> WindowBorderSize
            case make_condition(660, 'n'): state = 661; continue; // -> WindowMenuButtonPosition
            case make_condition(699, 'n'): state = 700; continue; // -> WindowTitleAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 16:
            switch (make_condition(state, key[i])) {
            case make_condition(24, 'U'): state = 25; continue; // -> AntiAliasedLinesUseTex
            case make_condition(93, 'o'): state = 94; continue; // -> CircleTessellationMaxError
            case make_condition(118, 'i'): state = 119; continue; // -> ColorButtonPosition
            case make_condition(135, 'g'): state = 136; continue; // -> ColumnsMinSpacing
            case make_condition(151, 'n'): state = 152; continue; // -> CurveTessellationTol
            case make_condition(181, 'a'): state = 182; continue; // -> DisplaySafeAreaPadding
            case make_condition(196, 'd'): state = 197; continue; // -> DisplayWindowPadding
            case make_condition(281, 'l'): state = 282; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(304, 'e'): state = 305; continue; // -> HoverStationaryDelay
            case make_condition(373, 'e'): state = 374; continue; // -> LogSliderDeadzone
            case make_condition(440, 'g'): state = 441; continue; // -> ScrollbarRounding
            case make_condition(460, 'i'): state = 461; continue; // -> SelectableTextAlign
            case make_condition(477, 'g'): state = 478; continue; // -> SeparatorTextAlign
            case make_condition(482, 'd'): state = 483; continue; // -> SeparatorTextBorderSize
            case make_condition(492, 'd'): state = 493; continue; // -> SeparatorTextPadding
            case make_condition(522, 'z'): state = 523; continue; // -> TabBarOverlineSize
            case make_condition(546, 'n'): state = 547; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(591, 'r'): state = 592; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(622, 'g'): state = 623; continue; // -> TouchExtraPadding
            case make_condition(639, 'r'): state = 640; continue; // -> WindowBorderHoverPadding
            case make_condition(661, 'P'): state = 662; continue; // -> WindowMenuButtonPosition
            default: return ImGuiStyleMember::__unknown__;
            }
        case 17:
            switch (make_condition(state, key[i])) {
            case make_condition(25, 's'): state = 26; continue; // -> AntiAliasedLinesUseTex
            case make_condition(94, 'n'): state = 95; continue; // -> CircleTessellationMaxError
            case make_condition(119, 'o'): state = 120; continue; // -> ColorButtonPosition
            case make_condition(152, 'T'): state = 153; continue; // -> CurveTessellationTol
            case make_condition(182, 'd'): state = 183; continue; // -> DisplaySafeAreaPadding
            case make_condition(197, 'i'): state = 198; continue; // -> DisplayWindowPadding
            case make_condition(282, 't'): state = 283; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(305, 'l'): state = 306; continue; // -> HoverStationaryDelay
            case make_condition(461, 'g'): state = 462; continue; // -> SelectableTextAlign
            case make_condition(478, 'n'): state = 479; continue; // -> SeparatorTextAlign
            case make_condition(483, 'e'): state = 484; continue; // -> SeparatorTextBorderSize
            case make_condition(493, 'i'): state = 494; continue; // -> SeparatorTextPadding
            case make_condition(523, 'e'): state = 524; continue; // -> TabBarOverlineSize
            case make_condition(547, 'W'): state = 548; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(592, 's'): state = 593; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(640, 'P'): state = 641; continue; // -> WindowBorderHoverPadding
            case make_condition(662, 'o'): state = 663; continue; // -> WindowMenuButtonPosition
            default: return ImGuiStyleMember::__unknown__;
            }
        case 18:
            switch (make_condition(state, key[i])) {
            case make_condition(26, 'e'): state = 27; continue; // -> AntiAliasedLinesUseTex
            case make_condition(95, 'M'): state = 96; continue; // -> CircleTessellationMaxError
            case make_condition(120, 'n'): state = 121; continue; // -> ColorButtonPosition
            case make_condition(153, 'o'): state = 154; continue; // -> CurveTessellationTol
            case make_condition(183, 'd'): state = 184; continue; // -> DisplaySafeAreaPadding
            case make_condition(198, 'n'): state = 199; continue; // -> DisplayWindowPadding
            case make_condition(283, 'i'): state = 284; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(306, 'a'): state = 307; continue; // -> HoverStationaryDelay
            case make_condition(462, 'n'): state = 463; continue; // -> SelectableTextAlign
            case make_condition(484, 'r'): state = 485; continue; // -> SeparatorTextBorderSize
            case make_condition(494, 'n'): state = 495; continue; // -> SeparatorTextPadding
            case make_condition(548, 'i'): state = 549; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(593, 'A'): state = 594; continue; // -> TableAngledHeadersAngle
            case make_condition(593, 'T'): state = 599; continue; // -> TableAngledHeadersTextAlign
            case make_condition(641, 'a'): state = 642; continue; // -> WindowBorderHoverPadding
            case make_condition(663, 's'): state = 664; continue; // -> WindowMenuButtonPosition
            default: return ImGuiStyleMember::__unknown__;
            }
        case 19:
            switch (make_condition(state, key[i])) {
            case make_condition(27, 'T'): state = 28; continue; // -> AntiAliasedLinesUseTex
            case make_condition(96, 'a'): state = 97; continue; // -> CircleTessellationMaxError
            case make_condition(154, 'l'): state = 155; continue; // -> CurveTessellationTol
            case make_condition(184, 'i'): state = 185; continue; // -> DisplaySafeAreaPadding
            case make_condition(199, 'g'): state = 200; continue; // -> DisplayWindowPadding
            case make_condition(284, 'p'): state = 285; continue; // -> HoverFlagsForTooltipMouse, HoverFlagsForTooltipNav
            case make_condition(307, 'y'): state = 308; continue; // -> HoverStationaryDelay
            case make_condition(485, 'S'): state = 486; continue; // -> SeparatorTextBorderSize
            case make_condition(495, 'g'): state = 496; continue; // -> SeparatorTextPadding
            case make_condition(549, 'd'): state = 550; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(594, 'n'): state = 595; continue; // -> TableAngledHeadersAngle
            case make_condition(599, 'e'): state = 600; continue; // -> TableAngledHeadersTextAlign
            case make_condition(642, 'd'): state = 643; continue; // -> WindowBorderHoverPadding
            case make_condition(664, 'i'): state = 665; continue; // -> WindowMenuButtonPosition
            default: return ImGuiStyleMember::__unknown__;
            }
        case 20:
            switch (make_condition(state, key[i])) {
            case make_condition(28, 'e'): state = 29; continue; // -> AntiAliasedLinesUseTex
            case make_condition(97, 'x'): state = 98; continue; // -> CircleTessellationMaxError
            case make_condition(185, 'n'): state = 186; continue; // -> DisplaySafeAreaPadding
            case make_condition(285, 'M'): state = 286; continue; // -> HoverFlagsForTooltipMouse
            case make_condition(285, 'N'): state = 291; continue; // -> HoverFlagsForTooltipNav
            case make_condition(486, 'i'): state = 487; continue; // -> SeparatorTextBorderSize
            case make_condition(550, 't'): state = 551; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(595, 'g'): state = 596; continue; // -> TableAngledHeadersAngle
            case make_condition(600, 'x'): state = 601; continue; // -> TableAngledHeadersTextAlign
            case make_condition(643, 'd'): state = 644; continue; // -> WindowBorderHoverPadding
            case make_condition(665, 't'): state = 666; continue; // -> WindowMenuButtonPosition
            default: return ImGuiStyleMember::__unknown__;
            }
        case 21:
            switch (make_condition(state, key[i])) {
            case make_condition(29, 'x'): state = 30; continue; // -> AntiAliasedLinesUseTex
            case make_condition(98, 'E'): state = 99; continue; // -> CircleTessellationMaxError
            case make_condition(186, 'g'): state = 187; continue; // -> DisplaySafeAreaPadding
            case make_condition(286, 'o'): state = 287; continue; // -> HoverFlagsForTooltipMouse
            case make_condition(291, 'a'): state = 292; continue; // -> HoverFlagsForTooltipNav
            case make_condition(487, 'z'): state = 488; continue; // -> SeparatorTextBorderSize
            case make_condition(551, 'h'): state = 552; continue; // -> TabCloseButtonMinWidthSelected, TabCloseButtonMinWidthUnselected
            case make_condition(596, 'l'): state = 597; continue; // -> TableAngledHeadersAngle
            case make_condition(601, 't'): state = 602; continue; // -> TableAngledHeadersTextAlign
            case make_condition(644, 'i'): state = 645; continue; // -> WindowBorderHoverPadding
            case make_condition(666, 'i'): state = 667; continue; // -> WindowMenuButtonPosition
            default: return ImGuiStyleMember::__unknown__;
            }
        case 22:
            switch (make_condition(state, key[i])) {
            case make_condition(99, 'r'): state = 100; continue; // -> CircleTessellationMaxError
            case make_condition(287, 'u'): state = 288; continue; // -> HoverFlagsForTooltipMouse
            case make_condition(292, 'v'): state = 293; continue; // -> HoverFlagsForTooltipNav
            case make_condition(488, 'e'): state = 489; continue; // -> SeparatorTextBorderSize
            case make_condition(552, 'S'): state = 553; continue; // -> TabCloseButtonMinWidthSelected
            case make_condition(552, 'U'): state = 561; continue; // -> TabCloseButtonMinWidthUnselected
            case make_condition(597, 'e'): state = 598; continue; // -> TableAngledHeadersAngle
            case make_condition(602, 'A'): state = 603; continue; // -> TableAngledHeadersTextAlign
            case make_condition(645, 'n'): state = 646; continue; // -> WindowBorderHoverPadding
            case make_condition(667, 'o'): state = 668; continue; // -> WindowMenuButtonPosition
            default: return ImGuiStyleMember::__unknown__;
            }
        case 23:
            switch (make_condition(state, key[i])) {
            case make_condition(100, 'r'): state = 101; continue; // -> CircleTessellationMaxError
            case make_condition(288, 's'): state = 289; continue; // -> HoverFlagsForTooltipMouse
            case make_condition(553, 'e'): state = 554; continue; // -> TabCloseButtonMinWidthSelected
            case make_condition(561, 'n'): state = 562; continue; // -> TabCloseButtonMinWidthUnselected
            case make_condition(603, 'l'): state = 604; continue; // -> TableAngledHeadersTextAlign
            case make_condition(646, 'g'): state = 647; continue; // -> WindowBorderHoverPadding
            case make_condition(668, 'n'): state = 669; continue; // -> WindowMenuButtonPosition
            default: return ImGuiStyleMember::__unknown__;
            }
        case 24:
            switch (make_condition(state, key[i])) {
            case make_condition(101, 'o'): state = 102; continue; // -> CircleTessellationMaxError
            case make_condition(289, 'e'): state = 290; continue; // -> HoverFlagsForTooltipMouse
            case make_condition(554, 'l'): state = 555; continue; // -> TabCloseButtonMinWidthSelected
            case make_condition(562, 's'): state = 563; continue; // -> TabCloseButtonMinWidthUnselected
            case make_condition(604, 'i'): state = 605; continue; // -> TableAngledHeadersTextAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 25:
            switch (make_condition(state, key[i])) {
            case make_condition(102, 'r'): state = 103; continue; // -> CircleTessellationMaxError
            case make_condition(555, 'e'): state = 556; continue; // -> TabCloseButtonMinWidthSelected
            case make_condition(563, 'e'): state = 564; continue; // -> TabCloseButtonMinWidthUnselected
            case make_condition(605, 'g'): state = 606; continue; // -> TableAngledHeadersTextAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 26:
            switch (make_condition(state, key[i])) {
            case make_condition(556, 'c'): state = 557; continue; // -> TabCloseButtonMinWidthSelected
            case make_condition(564, 'l'): state = 565; continue; // -> TabCloseButtonMinWidthUnselected
            case make_condition(606, 'n'): state = 607; continue; // -> TableAngledHeadersTextAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 27:
            switch (make_condition(state, key[i])) {
            case make_condition(557, 't'): state = 558; continue; // -> TabCloseButtonMinWidthSelected
            case make_condition(565, 'e'): state = 566; continue; // -> TabCloseButtonMinWidthUnselected
            default: return ImGuiStyleMember::__unknown__;
            }
        case 28:
            switch (make_condition(state, key[i])) {
            case make_condition(558, 'e'): state = 559; continue; // -> TabCloseButtonMinWidthSelected
            case make_condition(566, 'c'): state = 567; continue; // -> TabCloseButtonMinWidthUnselected
            default: return ImGuiStyleMember::__unknown__;
            }
        case 29:
            switch (make_condition(state, key[i])) {
            case make_condition(559, 'd'): state = 560; continue; // -> TabCloseButtonMinWidthSelected
            case make_condition(567, 't'): state = 568; continue; // -> TabCloseButtonMinWidthUnselected
            default: return ImGuiStyleMember::__unknown__;
            }
        case 30:
            switch (make_condition(state, key[i])) {
            case make_condition(568, 'e'): state = 569; continue; // -> TabCloseButtonMinWidthUnselected
            default: return ImGuiStyleMember::__unknown__;
            }
        case 31:
            switch (make_condition(state, key[i])) {
            case make_condition(569, 'd'): state = 570; continue; // -> TabCloseButtonMinWidthUnselected
            default: return ImGuiStyleMember::__unknown__;
            }
        default: return ImGuiStyleMember::__unknown__;
        }
    }
    #undef make_condition
    switch (state) {
    case 5: return ImGuiStyleMember::Alpha;
    case 19: return ImGuiStyleMember::AntiAliasedFill;
    case 24: return ImGuiStyleMember::AntiAliasedLines;
    case 30: return ImGuiStyleMember::AntiAliasedLinesUseTex;
    case 45: return ImGuiStyleMember::ButtonTextAlign;
    case 56: return ImGuiStyleMember::CellPadding;
    case 70: return ImGuiStyleMember::ChildBorderSize;
    case 78: return ImGuiStyleMember::ChildRounding;
    case 103: return ImGuiStyleMember::CircleTessellationMaxError;
    case 121: return ImGuiStyleMember::ColorButtonPosition;
    case 122: return ImGuiStyleMember::Colors;
    case 136: return ImGuiStyleMember::ColumnsMinSpacing;
    case 155: return ImGuiStyleMember::CurveTessellationTol;
    case 168: return ImGuiStyleMember::DisabledAlpha;
    case 187: return ImGuiStyleMember::DisplaySafeAreaPadding;
    case 200: return ImGuiStyleMember::DisplayWindowPadding;
    case 215: return ImGuiStyleMember::FrameBorderSize;
    case 222: return ImGuiStyleMember::FramePadding;
    case 230: return ImGuiStyleMember::FrameRounding;
    case 241: return ImGuiStyleMember::GrabMinSize;
    case 249: return ImGuiStyleMember::GrabRounding;
    case 265: return ImGuiStyleMember::HoverDelayNormal;
    case 270: return ImGuiStyleMember::HoverDelayShort;
    case 290: return ImGuiStyleMember::HoverFlagsForTooltipMouse;
    case 293: return ImGuiStyleMember::HoverFlagsForTooltipNav;
    case 308: return ImGuiStyleMember::HoverStationaryDelay;
    case 323: return ImGuiStyleMember::ImageBorderSize;
    case 335: return ImGuiStyleMember::IndentSpacing;
    case 350: return ImGuiStyleMember::ItemInnerSpacing;
    case 357: return ImGuiStyleMember::ItemSpacing;
    case 374: return ImGuiStyleMember::LogSliderDeadzone;
    case 390: return ImGuiStyleMember::MouseCursorScale;
    case 405: return ImGuiStyleMember::PopupBorderSize;
    case 413: return ImGuiStyleMember::PopupRounding;
    case 426: return ImGuiStyleMember::ScaleAllSizes;
    case 441: return ImGuiStyleMember::ScrollbarRounding;
    case 445: return ImGuiStyleMember::ScrollbarSize;
    case 463: return ImGuiStyleMember::SelectableTextAlign;
    case 479: return ImGuiStyleMember::SeparatorTextAlign;
    case 489: return ImGuiStyleMember::SeparatorTextBorderSize;
    case 496: return ImGuiStyleMember::SeparatorTextPadding;
    case 512: return ImGuiStyleMember::TabBarBorderSize;
    case 524: return ImGuiStyleMember::TabBarOverlineSize;
    case 533: return ImGuiStyleMember::TabBorderSize;
    case 560: return ImGuiStyleMember::TabCloseButtonMinWidthSelected;
    case 570: return ImGuiStyleMember::TabCloseButtonMinWidthUnselected;
    case 578: return ImGuiStyleMember::TabRounding;
    case 598: return ImGuiStyleMember::TableAngledHeadersAngle;
    case 607: return ImGuiStyleMember::TableAngledHeadersTextAlign;
    case 623: return ImGuiStyleMember::TouchExtraPadding;
    case 647: return ImGuiStyleMember::WindowBorderHoverPadding;
    case 651: return ImGuiStyleMember::WindowBorderSize;
    case 669: return ImGuiStyleMember::WindowMenuButtonPosition;
    case 675: return ImGuiStyleMember::WindowMinSize;
    case 682: return ImGuiStyleMember::WindowPadding;
    case 690: return ImGuiStyleMember::WindowRounding;
    case 700: return ImGuiStyleMember::WindowTitleAlign;
    default: return ImGuiStyleMember::__unknown__;
    }
}

}
