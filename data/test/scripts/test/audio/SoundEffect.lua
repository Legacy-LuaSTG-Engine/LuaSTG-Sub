local test = require("test")
local imgui = require("imgui")

local SE_NAME = "啊！"

---@class test.audio.SoundEffect : test.Base
local M = {}

function M:onCreate()
    local old = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    lstg.LoadSound(SE_NAME, "res/audio/啊！.wav")
    lstg.SetResourceStatus(old)
    self.vol = 1.0
    self.pan = 0.0
end

function M:onDestroy()
    lstg.RemoveResource("global", 5, SE_NAME)
end

function M:onUpdate()
    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin("Audio: Sound Effect") then
        ImGui.Text(("SE State: %s"):format(lstg.GetSoundState(SE_NAME)))
        _, self.vol = ImGui.SliderFloat("Volume", self.vol, 0.0, 1.0)
        _, self.pan = ImGui.SliderFloat("Pan", self.pan, -1.0, 1.0)
        if ImGui.Button("Play") then
            lstg.PlaySound(SE_NAME, self.vol, self.pan)
        end
        if ImGui.Button("Pause") then
            lstg.PauseSound(SE_NAME)
        end
        if ImGui.Button("Resume") then
            lstg.ResumeSound(SE_NAME)
        end
        if ImGui.Button("Stop") then
            lstg.StopSound(SE_NAME)
        end
    end
    ImGui.End()
end

function M:onRender()
end

test.registerTest("test.audio.SoundEffect", M, "Audio: Sound Effect")
