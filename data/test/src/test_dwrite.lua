local test = require("test")

---@class test.Module.DirectWrite : test.Base
local M = {}

local text = [[People communicate with text all the time in their daily lives. It is the primary way for people to consume an increasing volume of information. In the past, it used to be through printed content, primarily documents, newspapers, books, and so on. Increasingly, it is online content on their Windows PC. A typical Windows user spends a lot of time reading from their computer screen. They might be surfing the Web, scanning e-mail, composing a report, filling in a spreadsheet, or writing software, but what they're really doing is reading. Even though text and fonts permeate nearly every part of the user experience in Windows, for most users, reading on the screen is not as enjoyable as reading printed output.

统一的 App Store 让你的应用在 Windows 设备（如电脑、平板电脑、Xbox、HoloLens、Surface Hub 和物联网 (IoT) 设备）上可用。 可以向 Microsoft Store 提交应用，并使其对所有类型的设备或仅对所选设备类型可用。 你将在一个位置上提交和管理适用于 Windows 设备的所有应用。 想要使用 UWP 功能实现 C++ 桌面应用的现代化并将其在 Microsoft store 内出售？ 这同样可以实现。]]

function M:onCreate()
    ---@type DirectWrite
    local DirectWrite = require("DirectWrite")
    local font_collection = DirectWrite.CreateFontCollection({
        --"C:/Windows/Fonts/msyh.ttc",
        --"C:/Windows/Fonts/msyhbd.ttc",
        --"C:/Windows/Fonts/msyhl.ttc",
        "C:/Windows/Fonts/SourceHanSansCN-Bold.otf",
        "C:/Windows/Fonts/SourceHanSerifCN-Bold.otf",
    })
    lstg.Log(2, string.format("DirectWriteFontCollection:\n%s", font_collection:GetDebugInformation()))
    local text_format = DirectWrite.CreateTextFormat(
        "思源黑体 CN",
        font_collection,
        DirectWrite.FontWeight.Regular,
        DirectWrite.FontStyle.Normal,
        DirectWrite.FontStretch.Normal,
        24.0,
        ""
    )
    local text_layout = DirectWrite.CreateTextLayout(
        "测试文本：\n" .. text,
        text_format,
        1024, -- width
        640 -- height
    )
    text_layout:SetFontFamilyName("思源宋体 CN", 0, 5)
    --text_layout:SetStrikethrough(true, 0, 5)
    --text_layout:SetUnderline(true, 0, 5)
    text_layout:SetFontSize(48, 0, 5)
    text_layout:SetFontStyle(DirectWrite.FontStyle.Oblique, 30, 4)
    text_layout:SetFontFamilyName("思源宋体 CN", 57, 5)
    text_layout:SetFontFamilyName("思源宋体 CN", 63, 5)
    text_layout:SetTextAlignment(DirectWrite.TextAlignment.Justified)
    DirectWrite.CreateTextureFromTextLayout(text_layout, "global", "text-texture", 2)

    local text_layout_2 = DirectWrite.CreateTextLayout(
        "城阙辅三秦，风烟望五津。\n与君离别意，同是宦游人。\n海内存知己，天涯若比邻。\n无为在歧路，儿女共沾巾。\nLet life be beautiful like summer flowers and death like autumn leaves.\nThe world has kissed my soul with its pain, asking for its return in songs.\nあなたが帰ってきたのは詩です。\n立ち去るのは言葉です。\n風塵を見ても、次を作ることはできません。",
        text_format,
        512, -- width
        512 -- height
    )
    text_layout_2:SetFlowDirection(DirectWrite.FlowDirection.RightToLeft)
    text_layout_2:SetReadingDirection(2)
    --text_layout_2:SetStrikethrough(true, 0, 5)
    --text_layout_2:SetUnderline(true, 0, 5)
    DirectWrite.CreateTextureFromTextLayout(text_layout_2, "global", "text-texture-2", 2)
    --DirectWrite.SaveTextLayoutToFile(text_layout_2, "res/text-texture-2.png")

    local old_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")

    local w, h = lstg.GetTextureSize("text-texture")
    lstg.LoadImage("text-image", "text-texture", 0, 0, w, h)
    local w, h = lstg.GetTextureSize("text-texture-2")
    lstg.LoadImage("text-image-2", "text-texture-2", 0, 0, w, h)

    lstg.SetResourceStatus(old_pool)
end

function M:onDestroy()
    lstg.RemoveResource("global", 2, "text-image")
    lstg.RemoveResource("global", 1, "text-texture")
    lstg.RemoveResource("global", 2, "text-image-2")
    lstg.RemoveResource("global", 1, "text-texture-2")
end

function M:onUpdate()
end

function M:onRender()
    window:applyCameraV()
    lstg.Render("text-image", 64 + 512, window.height / 2, 0, 1)
    lstg.Render("text-image-2", 64 + 1024 + 256, window.height / 2, 0, 1)
end

test.registerTest("test.Module.DirectWrite", M)
