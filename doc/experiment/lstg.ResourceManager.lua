
---@alias lstg.KnownSamplerState '""' | '"point+wrap"' | '"point+clamp"' | '"linear+wrap"' | '"linear+clamp"'

--- 警告：在之前的 LuaSTG 版本中，也存在 lstg.SetTextureSamplerState 方法，
--- 但是并未公开，且参数不一样，行为不一样，影响范围也不一样。
--- 旧 API 设置的采样器是全局的状态，用完后必须手动还原。  
--- 警告：旧 API 的函数签名为 lstg.SetTextureSamplerState(state:string, mode:string)
--- 且参数组合为 state = "address", mode = "wrap" | "clamp"
--- 或 state = "filter", mode = "point" | "linear"  
--- 警告：旧 API 的行为未来将会移除，强烈建议替换为新行为。  
--- 
--- 设置纹理使用的采样器  
--- 当参数 known_sampler_state 为 "" 时，纹理使用 LuaSTG 默认的采样器  
--- LuaSTG 默认情况下使用 "linear+clamp"  
---@param texture_name string
---@param known_sampler_state lstg.KnownSamplerState
function lstg.SetTextureSamplerState(texture_name, known_sampler_state)
end
