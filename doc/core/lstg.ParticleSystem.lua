--------------------------------------------------------------------------------
--- LuaSTG Sub 粒子系统（兼容 HGE 粒子系统）
--- 璀境石
--------------------------------------------------------------------------------

--- [LuaSTG Sub v0.17.0 新增]  
---@class lstg.ParticleSystem.Vector2
local T_vec2 = {
    x = 0.0,
    y = 0.0,
}

--- [LuaSTG Sub v0.17.0 新增]  
---@class lstg.ParticleSystem.Color4F
local T_col4 = {
    r = 0.0,
    g = 0.0,
    b = 0.0,
    a = 0.0,
}

--- [LuaSTG Sub v0.17.0 新增]  
---@class lstg.ParticleSystem
local C = {}

--------------------------------------------------------------------------------
--- 默认行为

--- 时间增量 time_delta 默认为 1.0 / 60.0
---@param x number
---@param y number
---@param rot number
---@param time_delta number
---@overload fun(x:number, y:number, rot:number)
function C:Update(x, y, rot, time_delta)
end

---@param scale number
function C:Render(scale)
end

--------------------------------------------------------------------------------
--- 拓展 API，支持动态修改粒子系统参数，并部分兼容 LuaSTG-x

---@return number
function C:getAliveCount()
end

---@return number
function C:getAlphaVar()
end

---@return lstg.ParticleSystem.Vector2
function C:getCenter()
end

--- 和 LuaSTG-x 不兼容，LuaSTG-x 返回的是包装过的 lstg.Color
---@return lstg.ParticleSystem.Color4F
function C:getColorEnd()
end

--- 和 LuaSTG-x 不兼容，LuaSTG-x 返回的是包装过的 lstg.Color
---@return lstg.ParticleSystem.Color4F
function C:getColorStart()
end

---@return number
function C:getColorVar()
end

--- 和 LuaSTG-x 不兼容，LuaSTG-x 没有用上 Direction  
--- 实际上相当于 Rotation - PI / 2  
---@return number
function C:getDirection()
end

---@return number
function C:getEmissionFreq()
end

---@return number
function C:getGravityMax()
end

---@return number
function C:getGravityMin()
end

---@return number
function C:getLifeMax()
end

---@return number
function C:getLifeMin()
end

---@return number
function C:getLifetime()
end

---@return number
function C:getRadialAccelMax()
end

---@return number
function C:getRadialAccelMin()
end

--- 和 LuaSTG-x 不兼容，LuaSTG-x 没有用上 Relative
---@return boolean
function C:getRelative()
end

--- 和 LuaSTG-x 不兼容，LuaSTG-x 返回的是 lstg.RenderMode 对象
---@return lstg.BlendMode
function C:getRenderMode()
end

--- 和 LuaSTG-x 不兼容，LuaSTG-x 返回的是 lstg.ResParticle 对象  
--- LuaSTG Sub 还未将资源对象化，因此只能返回资源名称  
---@return string
function C:getResource()
end

--- 实际上相当于 Direction + PI / 2
---@return number
function C:getRotation()
end

--- 32 位无符号整数随机数种子
---@return number
function C:getSeed()
end

---@return number
function C:getSizeEnd()
end

---@return number
function C:getSizeStart()
end

---@return number
function C:getSizeVar()
end

---@return number
function C:getSpeedMax()
end

---@return number
function C:getSpeedMin()
end

---@return number
function C:getSpinEnd()
end

---@return number
function C:getSpinStart()
end

---@return number
function C:getSpinVar()
end

---@return number
function C:getSpread()
end

---@return number
function C:getTangentialAccelMax()
end

---@return number
function C:getTangentialAccelMin()
end

---@return boolean
function C:isActive()
end

---@param b boolean
function C:setActive(b)
end

---@param v number
function C:setAlphaVar(v)
end

---@param vec2 lstg.ParticleSystem.Vector2
function C:setCenter(vec2)
end

--- 和 LuaSTG-x 不兼容，LuaSTG-x 传入的是包装过的 lstg.Color
---@param col4f lstg.ParticleSystem.Color4F
function C:setColorEnd(col4f)
end

--- 和 LuaSTG-x 不兼容，LuaSTG-x 传入的是包装过的 lstg.Color
---@param col4f lstg.ParticleSystem.Color4F
function C:setColorStart(col4f)
end

---@param v number
function C:setColorVar(v)
end

--- 和 LuaSTG-x 不兼容，LuaSTG-x 没有用上 Direction  
--- 实际上相当于 Rotation - PI / 2  
---@param v number
function C:setDirection(v)
end

--- 发射密度（个/秒），参数 n 为整数
---@param n number
function C:setEmissionFreq(n)
end

---@param v number
function C:setGravityMax(v)
end

---@param v number
function C:setGravityMin(v)
end

---@param v number
function C:setLifeMax(v)
end

---@param v number
function C:setLifeMin(v)
end

--- 粒子系统单次运作时间（秒）
---@param v number
function C:setLifetime(v)
end

---@param v number
function C:setRadialAccelMax(v)
end

---@param v number
function C:setRadialAccelMin(v)
end

--- 和 LuaSTG-x 不兼容，LuaSTG-x 没有用上 Relative
---@param b boolean
function C:setRelative(b)
end

--- 和 LuaSTG-x 不兼容，LuaSTG-x 传入的是 lstg.RenderMode 对象
---@param blend lstg.BlendMode
function C:setRenderMode(blend)
end

--- 实际上相当于 Direction + PI / 2
---@param v number
function C:setRotation(v)
end

--- 32 位无符号整数随机数种子
---@param seed number
function C:setSeed(seed)
end

---@param v number
function C:setSizeEnd(v)
end

---@param v number
function C:setSizeStart(v)
end

---@param v number
function C:setSizeVar(v)
end

---@param v number
function C:setSpeedMax(v)
end

---@param v number
function C:setSpeedMin(v)
end

---@param v number
function C:setSpinEnd(v)
end

---@param v number
function C:setSpinStart(v)
end

---@param v number
function C:setSpinVar(v)
end

---@param v number
function C:setSpread(v)
end

---@param v number
function C:setTangentialAccelMax(v)
end

---@param v number
function C:setTangentialAccelMin(v)
end

--------------------------------------------------------------------------------
--- 创建

--- 从粒子系统资源 ps_name 创建粒子系统实例
---@param ps_name string
---@return lstg.ParticleSystem
function lstg.ParticleSystemData(ps_name)
end
