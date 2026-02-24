---@diagnostic disable: missing-return, duplicate-set-field

--------------------------------------------------------------------------------
--- LuaSTG Sub 音频命令
--- 璀境石
--------------------------------------------------------------------------------

local M = {}

--------------------------------------------------------------------------------
--- 状态
--- Status

---@alias lstg.AudioStatus '"playing"' | '"paused"' | '"stopped"'

--------------------------------------------------------------------------------
--- 音效
--- Sound effect

--- 播放音效  
--- 参数 `volume` 指定音量大小，范围是 0.0 到 1.0 的浮点数，默认为 1.0  
--- 
--- Play sound effect  
--- Parameter `volume` specifies the volume level,
--- accepts number from 0.0 to 1.0, default to 1.0.  
---@param se_resource_name string
---@param volume number?
---@param pan number?
function M.PlaySound(se_resource_name, volume, pan)
end

---@param se_resource_name string
function M.StopSound(se_resource_name)
end

---@param se_resource_name string
function M.PauseSound(se_resource_name)
end

---@param se_resource_name string
function M.ResumeSound(se_resource_name)
end

---@param se_resource_name string
---@return lstg.AudioStatus
function M.GetSoundState(se_resource_name)
end

--------------------------------------------------------------------------------
--- 音乐
--- Music

--- 播放音乐  
--- 参数 `volume` 指定音量大小，范围是 0.0 到 1.0 的浮点数，默认为 1.0  
--- 参数 `position` 指定起始位置，单位是秒，默认为 0.0 秒  
--- 
--- Play music  
--- Parameter `volume` specifies the volume level,
--- accepts number from 0.0 to 1.0, default to 1.0.  
--- Parameter `position` specifies the start position in seconds, default to 0.0s.  
---@param music_resource_name string
---@param volume number?
---@param position number?
function M.PlayMusic(music_resource_name, volume, position)
end

---@param music_resource_name string
function M.StopMusic(music_resource_name)
end

---@param music_resource_name string
function M.PauseMusic(music_resource_name)
end

---@param music_resource_name string
function M.ResumeMusic(music_resource_name)
end

---@param music_resource_name string
---@return lstg.AudioStatus
function M.GetMusicState(music_resource_name)
end

--- 单独设置音乐音量  
--- 
--- Setting the music volume individually  
---@param music_resource_name string
---@param volume number
function M.SetBGMVolume(music_resource_name, volume)
end

--------------------------------------------------------------------------------
--- 混音器
--- Mixer

--- 设置音效通道混音器音量  
--- 
--- Setting the sound effect channel mixer volume  
--- 
--- ```
--- +------+
--- | SE 1 |--------------+
--- +------+              |
---                       v
--- +------+     +------------------+
--- | SE 2 |---->| SE Channel Mixer |----> Audio Output
--- +------+     +------------------+
---                       ^
--- +------+              |
--- | SE...|--------------+
--- +------+
--- ```
---@param volume number
function M.SetSEVolume(volume)
end

--- 设置音乐通道混音器音量  
--- 
--- Setting the music channel mixer volume  
--- 
--- ```
--- +---------+
--- | Music 1 |----------------+
--- +---------+                |
---                            v
--- +---------+     +---------------------+
--- | Music 2 |---->| Music Channel Mixer |----> Audio Output
--- +---------+     +---------------------+
---                            ^
--- +---------+                |
--- | Music...|----------------+
--- +---------+
--- ```
---@param volume number
function M.SetBGMVolume(volume)
end

return M
