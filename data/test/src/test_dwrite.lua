local test = require("test")

---@class test.Module.DirectWrite : test.Base
local M = {}

local text = [[People communicate with text all the time in their daily lives. It is the primary way for people to consume an increasing volume of information. In the past, it used to be through printed content, primarily documents, newspapers, books, and so on. Increasingly, it is online content on their Windows PC. A typical Windows user spends a lot of time reading from their computer screen. They might be surfing the Web, scanning e-mail, composing a report, filling in a spreadsheet, or writing software, but what they're really doing is reading. Even though text and fonts permeate nearly every part of the user experience in Windows, for most users, reading on the screen is not as enjoyable as reading printed output.

统一的 App Store 让你的应用在 Windows 设备（如电脑、平板电脑、Xbox、HoloLens、Surface Hub 和物联网 (IoT) 设备）上可用。 可以向 Microsoft Store 提交应用，并使其对所有类型的设备或仅对所选设备类型可用。 你将在一个位置上提交和管理适用于 Windows 设备的所有应用。 想要使用 UWP 功能实现 C++ 桌面应用的现代化并将其在 Microsoft store 内出售？ 这同样可以实现。]]

function M:onCreate()
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
    DirectWrite.CreateTextureFromTextLayout(text_layout, "global", "text-texture", 4)

    local old_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")

    local w, h = lstg.GetTextureSize("text-texture")
    lstg.LoadImage("text-image", "text-texture", 0, 0, w, h)

    lstg.SetResourceStatus(old_pool)
end

function M:onDestroy()
    lstg.RemoveResource("global", 2, "text-image")
    lstg.RemoveResource("global", 1, "text-texture")
end

function M:onUpdate()
end

function M:onRender()
    window:applyCameraV()
    lstg.Render("text-image", window.width / 2, window.height / 2, 0, 1)
end

test.registerTest("test.Module.DirectWrite", M)
