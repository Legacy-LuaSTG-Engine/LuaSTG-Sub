local test = require("test")
local imgui = require("imgui")

---@class test.audio.SoundEffect : test.Base
local M = {}

function M:onCreate()
    local old = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    lstg.LoadSound("啊！", "res/audio/啊！.wav")
    lstg.SetResourceStatus(old)
end

function M:onDestroy()
    lstg.RemoveResource("global", 5, "啊！")
end

function M:onUpdate()
    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin("Audio: Sound Effect") then
        ImGui.Text(("SE State: %s"):format(lstg.GetSoundState("啊！")))
        if ImGui.Button("Play") then
            lstg.PlaySound("啊！")
        end
        if ImGui.Button("Pause") then
            lstg.PauseSound("啊！")
        end
        if ImGui.Button("Resume") then
            lstg.ResumeSound("啊！")
        end
        if ImGui.Button("Stop") then
            lstg.StopSound("啊！")
        end
    end
    ImGui.End()
end

function M:onRender()
end

test.registerTest("test.audio.SoundEffect", M, "Audio: Sound Effect")
