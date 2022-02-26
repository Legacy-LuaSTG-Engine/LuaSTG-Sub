--------------------------------------------------------------------------------
--- LuaSTG Sub 音频命令
--- 璀境石
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
--- 迁移指南

-- 关于全局音量：
-- lstg.SetSEVolume 和 lstg.SetBGMVolume 现在对播放中的音频也会生效

-- 关于 lstg.UpdateSound
-- 已经移除

--------------------------------------------------------------------------------
--- 状态

---@alias lstg.AudioStatus '"playing"' | '"paused"' | '"stopped"'

--------------------------------------------------------------------------------
--- 音效

---@param sndname string
---@param volume number
---@param pan number
function lstg.PlaySound(sndname, volume, pan)
end

---@param sndname string
function lstg.StopSound(sndname)
end

---@param sndname string
function lstg.PauseSound(sndname)
end

---@param sndname string
function lstg.ResumeSound(sndname)
end

---@param sndname string
---@return lstg.AudioStatus
function lstg.GetSoundState(sndname)
end

--- [LuaSTG Sub 变更]
--- 设置全局音效音量
---@param volume number
function lstg.SetSEVolume(volume)
end

--------------------------------------------------------------------------------
--- 音乐

--- position 以秒为单位
---@param bgmname string
---@param volume number
---@param position number
function lstg.PlayMusic(bgmname, volume, position)
end

---@param bgmname string
function lstg.StopMusic(bgmname)
end

---@param bgmname string
function lstg.PauseMusic(bgmname)
end

---@param bgmname string
function lstg.ResumeMusic(bgmname)
end

---@param bgmname string
---@return lstg.AudioStatus
function lstg.GetMusicState(bgmname)
end

--- [LuaSTG Sub 变更]
--- 设置全局音乐音量
--- 当参数为 2 个时，设置指定音乐的音量
---@param volume number
---@overload fun(bgmname:string, volume:number)
function lstg.SetBGMVolume(volume)
end
