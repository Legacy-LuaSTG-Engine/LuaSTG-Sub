// This file was generated through a code generator, DO NOT edit it, please edit the code generator

#include "ImGuiStyleMember.hpp"

namespace imgui_binding_lua {

ImGuiStyleMember mapImGuiStyleMember(char const* const key, size_t const len) noexcept {
    if (key == nullptr || len == 0 || len > 27) {
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
            case make_condition(0, 'I'): state = 309; continue; // -> IndentSpacing, ItemInnerSpacing, ItemSpacing
            case make_condition(0, 'L'): state = 344; continue; // -> LogSliderDeadzone
            case make_condition(0, 'M'): state = 361; continue; // -> MouseCursorScale
            case make_condition(0, 'P'): state = 377; continue; // -> PopupBorderSize, PopupRounding
            case make_condition(0, 'S'): state = 400; continue; // -> ScaleAllSizes, ScrollbarRounding, ScrollbarSize, SelectableTextAlign, SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(0, 'T'): state = 483; continue; // -> TabBarBorderSize, TabBorderSize, TabMinWidthForCloseButton, TabRounding, TableAngledHeadersAngle, TableAngledHeadersTextAlign, TouchExtraPadding
            case make_condition(0, 'W'): state = 583; continue; // -> WindowBorderSize, WindowMenuButtonPosition, WindowMinSize, WindowPadding, WindowRounding, WindowTitleAlign
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
            case make_condition(309, 'n'): state = 310; continue; // -> IndentSpacing
            case make_condition(309, 't'): state = 322; continue; // -> ItemInnerSpacing, ItemSpacing
            case make_condition(344, 'o'): state = 345; continue; // -> LogSliderDeadzone
            case make_condition(361, 'o'): state = 362; continue; // -> MouseCursorScale
            case make_condition(377, 'o'): state = 378; continue; // -> PopupBorderSize, PopupRounding
            case make_condition(400, 'c'): state = 401; continue; // -> ScaleAllSizes, ScrollbarRounding, ScrollbarSize
            case make_condition(400, 'e'): state = 432; continue; // -> SelectableTextAlign, SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(483, 'a'): state = 484; continue; // -> TabBarBorderSize, TabBorderSize, TabMinWidthForCloseButton, TabRounding, TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(483, 'o'): state = 567; continue; // -> TouchExtraPadding
            case make_condition(583, 'i'): state = 584; continue; // -> WindowBorderSize, WindowMenuButtonPosition, WindowMinSize, WindowPadding, WindowRounding, WindowTitleAlign
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
            case make_condition(310, 'd'): state = 311; continue; // -> IndentSpacing
            case make_condition(322, 'e'): state = 323; continue; // -> ItemInnerSpacing, ItemSpacing
            case make_condition(345, 'g'): state = 346; continue; // -> LogSliderDeadzone
            case make_condition(362, 'u'): state = 363; continue; // -> MouseCursorScale
            case make_condition(378, 'p'): state = 379; continue; // -> PopupBorderSize, PopupRounding
            case make_condition(401, 'a'): state = 402; continue; // -> ScaleAllSizes
            case make_condition(401, 'r'): state = 413; continue; // -> ScrollbarRounding, ScrollbarSize
            case make_condition(432, 'l'): state = 433; continue; // -> SelectableTextAlign
            case make_condition(432, 'p'): state = 450; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(484, 'b'): state = 485; continue; // -> TabBarBorderSize, TabBorderSize, TabMinWidthForCloseButton, TabRounding, TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(567, 'u'): state = 568; continue; // -> TouchExtraPadding
            case make_condition(584, 'n'): state = 585; continue; // -> WindowBorderSize, WindowMenuButtonPosition, WindowMinSize, WindowPadding, WindowRounding, WindowTitleAlign
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
            case make_condition(311, 'e'): state = 312; continue; // -> IndentSpacing
            case make_condition(323, 'm'): state = 324; continue; // -> ItemInnerSpacing, ItemSpacing
            case make_condition(346, 'S'): state = 347; continue; // -> LogSliderDeadzone
            case make_condition(363, 's'): state = 364; continue; // -> MouseCursorScale
            case make_condition(379, 'u'): state = 380; continue; // -> PopupBorderSize, PopupRounding
            case make_condition(402, 'l'): state = 403; continue; // -> ScaleAllSizes
            case make_condition(413, 'o'): state = 414; continue; // -> ScrollbarRounding, ScrollbarSize
            case make_condition(433, 'e'): state = 434; continue; // -> SelectableTextAlign
            case make_condition(450, 'a'): state = 451; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(485, 'B'): state = 486; continue; // -> TabBarBorderSize, TabBorderSize
            case make_condition(485, 'M'): state = 508; continue; // -> TabMinWidthForCloseButton
            case make_condition(485, 'R'): state = 530; continue; // -> TabRounding
            case make_condition(485, 'l'): state = 538; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(568, 'c'): state = 569; continue; // -> TouchExtraPadding
            case make_condition(585, 'd'): state = 586; continue; // -> WindowBorderSize, WindowMenuButtonPosition, WindowMinSize, WindowPadding, WindowRounding, WindowTitleAlign
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
            case make_condition(312, 'n'): state = 313; continue; // -> IndentSpacing
            case make_condition(324, 'I'): state = 325; continue; // -> ItemInnerSpacing
            case make_condition(324, 'S'): state = 337; continue; // -> ItemSpacing
            case make_condition(347, 'l'): state = 348; continue; // -> LogSliderDeadzone
            case make_condition(364, 'e'): state = 365; continue; // -> MouseCursorScale
            case make_condition(380, 'p'): state = 381; continue; // -> PopupBorderSize, PopupRounding
            case make_condition(403, 'e'): state = 404; continue; // -> ScaleAllSizes
            case make_condition(414, 'l'): state = 415; continue; // -> ScrollbarRounding, ScrollbarSize
            case make_condition(434, 'c'): state = 435; continue; // -> SelectableTextAlign
            case make_condition(451, 'r'): state = 452; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(486, 'a'): state = 487; continue; // -> TabBarBorderSize
            case make_condition(486, 'o'): state = 499; continue; // -> TabBorderSize
            case make_condition(508, 'i'): state = 509; continue; // -> TabMinWidthForCloseButton
            case make_condition(530, 'o'): state = 531; continue; // -> TabRounding
            case make_condition(538, 'e'): state = 539; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(569, 'h'): state = 570; continue; // -> TouchExtraPadding
            case make_condition(586, 'o'): state = 587; continue; // -> WindowBorderSize, WindowMenuButtonPosition, WindowMinSize, WindowPadding, WindowRounding, WindowTitleAlign
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
            case make_condition(313, 't'): state = 314; continue; // -> IndentSpacing
            case make_condition(325, 'n'): state = 326; continue; // -> ItemInnerSpacing
            case make_condition(337, 'p'): state = 338; continue; // -> ItemSpacing
            case make_condition(348, 'i'): state = 349; continue; // -> LogSliderDeadzone
            case make_condition(365, 'C'): state = 366; continue; // -> MouseCursorScale
            case make_condition(381, 'B'): state = 382; continue; // -> PopupBorderSize
            case make_condition(381, 'R'): state = 392; continue; // -> PopupRounding
            case make_condition(404, 'A'): state = 405; continue; // -> ScaleAllSizes
            case make_condition(415, 'l'): state = 416; continue; // -> ScrollbarRounding, ScrollbarSize
            case make_condition(435, 't'): state = 436; continue; // -> SelectableTextAlign
            case make_condition(452, 'a'): state = 453; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(487, 'r'): state = 488; continue; // -> TabBarBorderSize
            case make_condition(499, 'r'): state = 500; continue; // -> TabBorderSize
            case make_condition(509, 'n'): state = 510; continue; // -> TabMinWidthForCloseButton
            case make_condition(531, 'u'): state = 532; continue; // -> TabRounding
            case make_condition(539, 'A'): state = 540; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(570, 'E'): state = 571; continue; // -> TouchExtraPadding
            case make_condition(587, 'w'): state = 588; continue; // -> WindowBorderSize, WindowMenuButtonPosition, WindowMinSize, WindowPadding, WindowRounding, WindowTitleAlign
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
            case make_condition(314, 'S'): state = 315; continue; // -> IndentSpacing
            case make_condition(326, 'n'): state = 327; continue; // -> ItemInnerSpacing
            case make_condition(338, 'a'): state = 339; continue; // -> ItemSpacing
            case make_condition(349, 'd'): state = 350; continue; // -> LogSliderDeadzone
            case make_condition(366, 'u'): state = 367; continue; // -> MouseCursorScale
            case make_condition(382, 'o'): state = 383; continue; // -> PopupBorderSize
            case make_condition(392, 'o'): state = 393; continue; // -> PopupRounding
            case make_condition(405, 'l'): state = 406; continue; // -> ScaleAllSizes
            case make_condition(416, 'b'): state = 417; continue; // -> ScrollbarRounding, ScrollbarSize
            case make_condition(436, 'a'): state = 437; continue; // -> SelectableTextAlign
            case make_condition(453, 't'): state = 454; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(488, 'B'): state = 489; continue; // -> TabBarBorderSize
            case make_condition(500, 'd'): state = 501; continue; // -> TabBorderSize
            case make_condition(510, 'W'): state = 511; continue; // -> TabMinWidthForCloseButton
            case make_condition(532, 'n'): state = 533; continue; // -> TabRounding
            case make_condition(540, 'n'): state = 541; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(571, 'x'): state = 572; continue; // -> TouchExtraPadding
            case make_condition(588, 'B'): state = 589; continue; // -> WindowBorderSize
            case make_condition(588, 'M'): state = 599; continue; // -> WindowMenuButtonPosition, WindowMinSize
            case make_condition(588, 'P'): state = 623; continue; // -> WindowPadding
            case make_condition(588, 'R'): state = 630; continue; // -> WindowRounding
            case make_condition(588, 'T'): state = 638; continue; // -> WindowTitleAlign
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
            case make_condition(315, 'p'): state = 316; continue; // -> IndentSpacing
            case make_condition(327, 'e'): state = 328; continue; // -> ItemInnerSpacing
            case make_condition(339, 'c'): state = 340; continue; // -> ItemSpacing
            case make_condition(350, 'e'): state = 351; continue; // -> LogSliderDeadzone
            case make_condition(367, 'r'): state = 368; continue; // -> MouseCursorScale
            case make_condition(383, 'r'): state = 384; continue; // -> PopupBorderSize
            case make_condition(393, 'u'): state = 394; continue; // -> PopupRounding
            case make_condition(406, 'l'): state = 407; continue; // -> ScaleAllSizes
            case make_condition(417, 'a'): state = 418; continue; // -> ScrollbarRounding, ScrollbarSize
            case make_condition(437, 'b'): state = 438; continue; // -> SelectableTextAlign
            case make_condition(454, 'o'): state = 455; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(489, 'o'): state = 490; continue; // -> TabBarBorderSize
            case make_condition(501, 'e'): state = 502; continue; // -> TabBorderSize
            case make_condition(511, 'i'): state = 512; continue; // -> TabMinWidthForCloseButton
            case make_condition(533, 'd'): state = 534; continue; // -> TabRounding
            case make_condition(541, 'g'): state = 542; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(572, 't'): state = 573; continue; // -> TouchExtraPadding
            case make_condition(589, 'o'): state = 590; continue; // -> WindowBorderSize
            case make_condition(599, 'e'): state = 600; continue; // -> WindowMenuButtonPosition
            case make_condition(599, 'i'): state = 617; continue; // -> WindowMinSize
            case make_condition(623, 'a'): state = 624; continue; // -> WindowPadding
            case make_condition(630, 'o'): state = 631; continue; // -> WindowRounding
            case make_condition(638, 'i'): state = 639; continue; // -> WindowTitleAlign
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
            case make_condition(316, 'a'): state = 317; continue; // -> IndentSpacing
            case make_condition(328, 'r'): state = 329; continue; // -> ItemInnerSpacing
            case make_condition(340, 'i'): state = 341; continue; // -> ItemSpacing
            case make_condition(351, 'r'): state = 352; continue; // -> LogSliderDeadzone
            case make_condition(368, 's'): state = 369; continue; // -> MouseCursorScale
            case make_condition(384, 'd'): state = 385; continue; // -> PopupBorderSize
            case make_condition(394, 'n'): state = 395; continue; // -> PopupRounding
            case make_condition(407, 'S'): state = 408; continue; // -> ScaleAllSizes
            case make_condition(418, 'r'): state = 419; continue; // -> ScrollbarRounding, ScrollbarSize
            case make_condition(438, 'l'): state = 439; continue; // -> SelectableTextAlign
            case make_condition(455, 'r'): state = 456; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(490, 'r'): state = 491; continue; // -> TabBarBorderSize
            case make_condition(502, 'r'): state = 503; continue; // -> TabBorderSize
            case make_condition(512, 'd'): state = 513; continue; // -> TabMinWidthForCloseButton
            case make_condition(534, 'i'): state = 535; continue; // -> TabRounding
            case make_condition(542, 'l'): state = 543; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(573, 'r'): state = 574; continue; // -> TouchExtraPadding
            case make_condition(590, 'r'): state = 591; continue; // -> WindowBorderSize
            case make_condition(600, 'n'): state = 601; continue; // -> WindowMenuButtonPosition
            case make_condition(617, 'n'): state = 618; continue; // -> WindowMinSize
            case make_condition(624, 'd'): state = 625; continue; // -> WindowPadding
            case make_condition(631, 'u'): state = 632; continue; // -> WindowRounding
            case make_condition(639, 't'): state = 640; continue; // -> WindowTitleAlign
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
            case make_condition(317, 'c'): state = 318; continue; // -> IndentSpacing
            case make_condition(329, 'S'): state = 330; continue; // -> ItemInnerSpacing
            case make_condition(341, 'n'): state = 342; continue; // -> ItemSpacing
            case make_condition(352, 'D'): state = 353; continue; // -> LogSliderDeadzone
            case make_condition(369, 'o'): state = 370; continue; // -> MouseCursorScale
            case make_condition(385, 'e'): state = 386; continue; // -> PopupBorderSize
            case make_condition(395, 'd'): state = 396; continue; // -> PopupRounding
            case make_condition(408, 'i'): state = 409; continue; // -> ScaleAllSizes
            case make_condition(419, 'R'): state = 420; continue; // -> ScrollbarRounding
            case make_condition(419, 'S'): state = 428; continue; // -> ScrollbarSize
            case make_condition(439, 'e'): state = 440; continue; // -> SelectableTextAlign
            case make_condition(456, 'T'): state = 457; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(491, 'd'): state = 492; continue; // -> TabBarBorderSize
            case make_condition(503, 'S'): state = 504; continue; // -> TabBorderSize
            case make_condition(513, 't'): state = 514; continue; // -> TabMinWidthForCloseButton
            case make_condition(535, 'n'): state = 536; continue; // -> TabRounding
            case make_condition(543, 'e'): state = 544; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(574, 'a'): state = 575; continue; // -> TouchExtraPadding
            case make_condition(591, 'd'): state = 592; continue; // -> WindowBorderSize
            case make_condition(601, 'u'): state = 602; continue; // -> WindowMenuButtonPosition
            case make_condition(618, 'S'): state = 619; continue; // -> WindowMinSize
            case make_condition(625, 'd'): state = 626; continue; // -> WindowPadding
            case make_condition(632, 'n'): state = 633; continue; // -> WindowRounding
            case make_condition(640, 'l'): state = 641; continue; // -> WindowTitleAlign
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
            case make_condition(318, 'i'): state = 319; continue; // -> IndentSpacing
            case make_condition(330, 'p'): state = 331; continue; // -> ItemInnerSpacing
            case make_condition(342, 'g'): state = 343; continue; // -> ItemSpacing
            case make_condition(353, 'e'): state = 354; continue; // -> LogSliderDeadzone
            case make_condition(370, 'r'): state = 371; continue; // -> MouseCursorScale
            case make_condition(386, 'r'): state = 387; continue; // -> PopupBorderSize
            case make_condition(396, 'i'): state = 397; continue; // -> PopupRounding
            case make_condition(409, 'z'): state = 410; continue; // -> ScaleAllSizes
            case make_condition(420, 'o'): state = 421; continue; // -> ScrollbarRounding
            case make_condition(428, 'i'): state = 429; continue; // -> ScrollbarSize
            case make_condition(440, 'T'): state = 441; continue; // -> SelectableTextAlign
            case make_condition(457, 'e'): state = 458; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(492, 'e'): state = 493; continue; // -> TabBarBorderSize
            case make_condition(504, 'i'): state = 505; continue; // -> TabBorderSize
            case make_condition(514, 'h'): state = 515; continue; // -> TabMinWidthForCloseButton
            case make_condition(536, 'g'): state = 537; continue; // -> TabRounding
            case make_condition(544, 'd'): state = 545; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(575, 'P'): state = 576; continue; // -> TouchExtraPadding
            case make_condition(592, 'e'): state = 593; continue; // -> WindowBorderSize
            case make_condition(602, 'B'): state = 603; continue; // -> WindowMenuButtonPosition
            case make_condition(619, 'i'): state = 620; continue; // -> WindowMinSize
            case make_condition(626, 'i'): state = 627; continue; // -> WindowPadding
            case make_condition(633, 'd'): state = 634; continue; // -> WindowRounding
            case make_condition(641, 'e'): state = 642; continue; // -> WindowTitleAlign
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
            case make_condition(319, 'n'): state = 320; continue; // -> IndentSpacing
            case make_condition(331, 'a'): state = 332; continue; // -> ItemInnerSpacing
            case make_condition(354, 'a'): state = 355; continue; // -> LogSliderDeadzone
            case make_condition(371, 'S'): state = 372; continue; // -> MouseCursorScale
            case make_condition(387, 'S'): state = 388; continue; // -> PopupBorderSize
            case make_condition(397, 'n'): state = 398; continue; // -> PopupRounding
            case make_condition(410, 'e'): state = 411; continue; // -> ScaleAllSizes
            case make_condition(421, 'u'): state = 422; continue; // -> ScrollbarRounding
            case make_condition(429, 'z'): state = 430; continue; // -> ScrollbarSize
            case make_condition(441, 'e'): state = 442; continue; // -> SelectableTextAlign
            case make_condition(458, 'x'): state = 459; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(493, 'r'): state = 494; continue; // -> TabBarBorderSize
            case make_condition(505, 'z'): state = 506; continue; // -> TabBorderSize
            case make_condition(515, 'F'): state = 516; continue; // -> TabMinWidthForCloseButton
            case make_condition(545, 'H'): state = 546; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(576, 'a'): state = 577; continue; // -> TouchExtraPadding
            case make_condition(593, 'r'): state = 594; continue; // -> WindowBorderSize
            case make_condition(603, 'u'): state = 604; continue; // -> WindowMenuButtonPosition
            case make_condition(620, 'z'): state = 621; continue; // -> WindowMinSize
            case make_condition(627, 'n'): state = 628; continue; // -> WindowPadding
            case make_condition(634, 'i'): state = 635; continue; // -> WindowRounding
            case make_condition(642, 'A'): state = 643; continue; // -> WindowTitleAlign
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
            case make_condition(320, 'g'): state = 321; continue; // -> IndentSpacing
            case make_condition(332, 'c'): state = 333; continue; // -> ItemInnerSpacing
            case make_condition(355, 'd'): state = 356; continue; // -> LogSliderDeadzone
            case make_condition(372, 'c'): state = 373; continue; // -> MouseCursorScale
            case make_condition(388, 'i'): state = 389; continue; // -> PopupBorderSize
            case make_condition(398, 'g'): state = 399; continue; // -> PopupRounding
            case make_condition(411, 's'): state = 412; continue; // -> ScaleAllSizes
            case make_condition(422, 'n'): state = 423; continue; // -> ScrollbarRounding
            case make_condition(430, 'e'): state = 431; continue; // -> ScrollbarSize
            case make_condition(442, 'x'): state = 443; continue; // -> SelectableTextAlign
            case make_condition(459, 't'): state = 460; continue; // -> SeparatorTextAlign, SeparatorTextBorderSize, SeparatorTextPadding
            case make_condition(494, 'S'): state = 495; continue; // -> TabBarBorderSize
            case make_condition(506, 'e'): state = 507; continue; // -> TabBorderSize
            case make_condition(516, 'o'): state = 517; continue; // -> TabMinWidthForCloseButton
            case make_condition(546, 'e'): state = 547; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(577, 'd'): state = 578; continue; // -> TouchExtraPadding
            case make_condition(594, 'S'): state = 595; continue; // -> WindowBorderSize
            case make_condition(604, 't'): state = 605; continue; // -> WindowMenuButtonPosition
            case make_condition(621, 'e'): state = 622; continue; // -> WindowMinSize
            case make_condition(628, 'g'): state = 629; continue; // -> WindowPadding
            case make_condition(635, 'n'): state = 636; continue; // -> WindowRounding
            case make_condition(643, 'l'): state = 644; continue; // -> WindowTitleAlign
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
            case make_condition(333, 'i'): state = 334; continue; // -> ItemInnerSpacing
            case make_condition(356, 'z'): state = 357; continue; // -> LogSliderDeadzone
            case make_condition(373, 'a'): state = 374; continue; // -> MouseCursorScale
            case make_condition(389, 'z'): state = 390; continue; // -> PopupBorderSize
            case make_condition(423, 'd'): state = 424; continue; // -> ScrollbarRounding
            case make_condition(443, 't'): state = 444; continue; // -> SelectableTextAlign
            case make_condition(460, 'A'): state = 461; continue; // -> SeparatorTextAlign
            case make_condition(460, 'B'): state = 466; continue; // -> SeparatorTextBorderSize
            case make_condition(460, 'P'): state = 476; continue; // -> SeparatorTextPadding
            case make_condition(495, 'i'): state = 496; continue; // -> TabBarBorderSize
            case make_condition(517, 'r'): state = 518; continue; // -> TabMinWidthForCloseButton
            case make_condition(547, 'a'): state = 548; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(578, 'd'): state = 579; continue; // -> TouchExtraPadding
            case make_condition(595, 'i'): state = 596; continue; // -> WindowBorderSize
            case make_condition(605, 't'): state = 606; continue; // -> WindowMenuButtonPosition
            case make_condition(636, 'g'): state = 637; continue; // -> WindowRounding
            case make_condition(644, 'i'): state = 645; continue; // -> WindowTitleAlign
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
            case make_condition(334, 'n'): state = 335; continue; // -> ItemInnerSpacing
            case make_condition(357, 'o'): state = 358; continue; // -> LogSliderDeadzone
            case make_condition(374, 'l'): state = 375; continue; // -> MouseCursorScale
            case make_condition(390, 'e'): state = 391; continue; // -> PopupBorderSize
            case make_condition(424, 'i'): state = 425; continue; // -> ScrollbarRounding
            case make_condition(444, 'A'): state = 445; continue; // -> SelectableTextAlign
            case make_condition(461, 'l'): state = 462; continue; // -> SeparatorTextAlign
            case make_condition(466, 'o'): state = 467; continue; // -> SeparatorTextBorderSize
            case make_condition(476, 'a'): state = 477; continue; // -> SeparatorTextPadding
            case make_condition(496, 'z'): state = 497; continue; // -> TabBarBorderSize
            case make_condition(518, 'C'): state = 519; continue; // -> TabMinWidthForCloseButton
            case make_condition(548, 'd'): state = 549; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(579, 'i'): state = 580; continue; // -> TouchExtraPadding
            case make_condition(596, 'z'): state = 597; continue; // -> WindowBorderSize
            case make_condition(606, 'o'): state = 607; continue; // -> WindowMenuButtonPosition
            case make_condition(645, 'g'): state = 646; continue; // -> WindowTitleAlign
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
            case make_condition(335, 'g'): state = 336; continue; // -> ItemInnerSpacing
            case make_condition(358, 'n'): state = 359; continue; // -> LogSliderDeadzone
            case make_condition(375, 'e'): state = 376; continue; // -> MouseCursorScale
            case make_condition(425, 'n'): state = 426; continue; // -> ScrollbarRounding
            case make_condition(445, 'l'): state = 446; continue; // -> SelectableTextAlign
            case make_condition(462, 'i'): state = 463; continue; // -> SeparatorTextAlign
            case make_condition(467, 'r'): state = 468; continue; // -> SeparatorTextBorderSize
            case make_condition(477, 'd'): state = 478; continue; // -> SeparatorTextPadding
            case make_condition(497, 'e'): state = 498; continue; // -> TabBarBorderSize
            case make_condition(519, 'l'): state = 520; continue; // -> TabMinWidthForCloseButton
            case make_condition(549, 'e'): state = 550; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(580, 'n'): state = 581; continue; // -> TouchExtraPadding
            case make_condition(597, 'e'): state = 598; continue; // -> WindowBorderSize
            case make_condition(607, 'n'): state = 608; continue; // -> WindowMenuButtonPosition
            case make_condition(646, 'n'): state = 647; continue; // -> WindowTitleAlign
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
            case make_condition(359, 'e'): state = 360; continue; // -> LogSliderDeadzone
            case make_condition(426, 'g'): state = 427; continue; // -> ScrollbarRounding
            case make_condition(446, 'i'): state = 447; continue; // -> SelectableTextAlign
            case make_condition(463, 'g'): state = 464; continue; // -> SeparatorTextAlign
            case make_condition(468, 'd'): state = 469; continue; // -> SeparatorTextBorderSize
            case make_condition(478, 'd'): state = 479; continue; // -> SeparatorTextPadding
            case make_condition(520, 'o'): state = 521; continue; // -> TabMinWidthForCloseButton
            case make_condition(550, 'r'): state = 551; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(581, 'g'): state = 582; continue; // -> TouchExtraPadding
            case make_condition(608, 'P'): state = 609; continue; // -> WindowMenuButtonPosition
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
            case make_condition(447, 'g'): state = 448; continue; // -> SelectableTextAlign
            case make_condition(464, 'n'): state = 465; continue; // -> SeparatorTextAlign
            case make_condition(469, 'e'): state = 470; continue; // -> SeparatorTextBorderSize
            case make_condition(479, 'i'): state = 480; continue; // -> SeparatorTextPadding
            case make_condition(521, 's'): state = 522; continue; // -> TabMinWidthForCloseButton
            case make_condition(551, 's'): state = 552; continue; // -> TableAngledHeadersAngle, TableAngledHeadersTextAlign
            case make_condition(609, 'o'): state = 610; continue; // -> WindowMenuButtonPosition
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
            case make_condition(448, 'n'): state = 449; continue; // -> SelectableTextAlign
            case make_condition(470, 'r'): state = 471; continue; // -> SeparatorTextBorderSize
            case make_condition(480, 'n'): state = 481; continue; // -> SeparatorTextPadding
            case make_condition(522, 'e'): state = 523; continue; // -> TabMinWidthForCloseButton
            case make_condition(552, 'A'): state = 553; continue; // -> TableAngledHeadersAngle
            case make_condition(552, 'T'): state = 558; continue; // -> TableAngledHeadersTextAlign
            case make_condition(610, 's'): state = 611; continue; // -> WindowMenuButtonPosition
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
            case make_condition(471, 'S'): state = 472; continue; // -> SeparatorTextBorderSize
            case make_condition(481, 'g'): state = 482; continue; // -> SeparatorTextPadding
            case make_condition(523, 'B'): state = 524; continue; // -> TabMinWidthForCloseButton
            case make_condition(553, 'n'): state = 554; continue; // -> TableAngledHeadersAngle
            case make_condition(558, 'e'): state = 559; continue; // -> TableAngledHeadersTextAlign
            case make_condition(611, 'i'): state = 612; continue; // -> WindowMenuButtonPosition
            default: return ImGuiStyleMember::__unknown__;
            }
        case 20:
            switch (make_condition(state, key[i])) {
            case make_condition(28, 'e'): state = 29; continue; // -> AntiAliasedLinesUseTex
            case make_condition(97, 'x'): state = 98; continue; // -> CircleTessellationMaxError
            case make_condition(185, 'n'): state = 186; continue; // -> DisplaySafeAreaPadding
            case make_condition(285, 'M'): state = 286; continue; // -> HoverFlagsForTooltipMouse
            case make_condition(285, 'N'): state = 291; continue; // -> HoverFlagsForTooltipNav
            case make_condition(472, 'i'): state = 473; continue; // -> SeparatorTextBorderSize
            case make_condition(524, 'u'): state = 525; continue; // -> TabMinWidthForCloseButton
            case make_condition(554, 'g'): state = 555; continue; // -> TableAngledHeadersAngle
            case make_condition(559, 'x'): state = 560; continue; // -> TableAngledHeadersTextAlign
            case make_condition(612, 't'): state = 613; continue; // -> WindowMenuButtonPosition
            default: return ImGuiStyleMember::__unknown__;
            }
        case 21:
            switch (make_condition(state, key[i])) {
            case make_condition(29, 'x'): state = 30; continue; // -> AntiAliasedLinesUseTex
            case make_condition(98, 'E'): state = 99; continue; // -> CircleTessellationMaxError
            case make_condition(186, 'g'): state = 187; continue; // -> DisplaySafeAreaPadding
            case make_condition(286, 'o'): state = 287; continue; // -> HoverFlagsForTooltipMouse
            case make_condition(291, 'a'): state = 292; continue; // -> HoverFlagsForTooltipNav
            case make_condition(473, 'z'): state = 474; continue; // -> SeparatorTextBorderSize
            case make_condition(525, 't'): state = 526; continue; // -> TabMinWidthForCloseButton
            case make_condition(555, 'l'): state = 556; continue; // -> TableAngledHeadersAngle
            case make_condition(560, 't'): state = 561; continue; // -> TableAngledHeadersTextAlign
            case make_condition(613, 'i'): state = 614; continue; // -> WindowMenuButtonPosition
            default: return ImGuiStyleMember::__unknown__;
            }
        case 22:
            switch (make_condition(state, key[i])) {
            case make_condition(99, 'r'): state = 100; continue; // -> CircleTessellationMaxError
            case make_condition(287, 'u'): state = 288; continue; // -> HoverFlagsForTooltipMouse
            case make_condition(292, 'v'): state = 293; continue; // -> HoverFlagsForTooltipNav
            case make_condition(474, 'e'): state = 475; continue; // -> SeparatorTextBorderSize
            case make_condition(526, 't'): state = 527; continue; // -> TabMinWidthForCloseButton
            case make_condition(556, 'e'): state = 557; continue; // -> TableAngledHeadersAngle
            case make_condition(561, 'A'): state = 562; continue; // -> TableAngledHeadersTextAlign
            case make_condition(614, 'o'): state = 615; continue; // -> WindowMenuButtonPosition
            default: return ImGuiStyleMember::__unknown__;
            }
        case 23:
            switch (make_condition(state, key[i])) {
            case make_condition(100, 'r'): state = 101; continue; // -> CircleTessellationMaxError
            case make_condition(288, 's'): state = 289; continue; // -> HoverFlagsForTooltipMouse
            case make_condition(527, 'o'): state = 528; continue; // -> TabMinWidthForCloseButton
            case make_condition(562, 'l'): state = 563; continue; // -> TableAngledHeadersTextAlign
            case make_condition(615, 'n'): state = 616; continue; // -> WindowMenuButtonPosition
            default: return ImGuiStyleMember::__unknown__;
            }
        case 24:
            switch (make_condition(state, key[i])) {
            case make_condition(101, 'o'): state = 102; continue; // -> CircleTessellationMaxError
            case make_condition(289, 'e'): state = 290; continue; // -> HoverFlagsForTooltipMouse
            case make_condition(528, 'n'): state = 529; continue; // -> TabMinWidthForCloseButton
            case make_condition(563, 'i'): state = 564; continue; // -> TableAngledHeadersTextAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 25:
            switch (make_condition(state, key[i])) {
            case make_condition(102, 'r'): state = 103; continue; // -> CircleTessellationMaxError
            case make_condition(564, 'g'): state = 565; continue; // -> TableAngledHeadersTextAlign
            default: return ImGuiStyleMember::__unknown__;
            }
        case 26:
            switch (make_condition(state, key[i])) {
            case make_condition(565, 'n'): state = 566; continue; // -> TableAngledHeadersTextAlign
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
    case 321: return ImGuiStyleMember::IndentSpacing;
    case 336: return ImGuiStyleMember::ItemInnerSpacing;
    case 343: return ImGuiStyleMember::ItemSpacing;
    case 360: return ImGuiStyleMember::LogSliderDeadzone;
    case 376: return ImGuiStyleMember::MouseCursorScale;
    case 391: return ImGuiStyleMember::PopupBorderSize;
    case 399: return ImGuiStyleMember::PopupRounding;
    case 412: return ImGuiStyleMember::ScaleAllSizes;
    case 427: return ImGuiStyleMember::ScrollbarRounding;
    case 431: return ImGuiStyleMember::ScrollbarSize;
    case 449: return ImGuiStyleMember::SelectableTextAlign;
    case 465: return ImGuiStyleMember::SeparatorTextAlign;
    case 475: return ImGuiStyleMember::SeparatorTextBorderSize;
    case 482: return ImGuiStyleMember::SeparatorTextPadding;
    case 498: return ImGuiStyleMember::TabBarBorderSize;
    case 507: return ImGuiStyleMember::TabBorderSize;
    case 529: return ImGuiStyleMember::TabMinWidthForCloseButton;
    case 537: return ImGuiStyleMember::TabRounding;
    case 557: return ImGuiStyleMember::TableAngledHeadersAngle;
    case 566: return ImGuiStyleMember::TableAngledHeadersTextAlign;
    case 582: return ImGuiStyleMember::TouchExtraPadding;
    case 598: return ImGuiStyleMember::WindowBorderSize;
    case 616: return ImGuiStyleMember::WindowMenuButtonPosition;
    case 622: return ImGuiStyleMember::WindowMinSize;
    case 629: return ImGuiStyleMember::WindowPadding;
    case 637: return ImGuiStyleMember::WindowRounding;
    case 647: return ImGuiStyleMember::WindowTitleAlign;
    default: return ImGuiStyleMember::__unknown__;
    }
}

}
