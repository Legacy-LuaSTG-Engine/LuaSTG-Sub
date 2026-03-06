--------------------------------------------------------------------------------
--- LuaSTG Sub 粒子系统（兼容 HGE 粒子系统）
--- 璀境石
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

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
--- API

--- rot 为角度制，可以让实际发射角度在发射角度 Direction 基础上进一步旋转  
--- 如果不填写 rot 参数，则不会（修改）更新 Rotation  
--- 如果不填写 x 和 y 参数，则不会（修改）更新 Center  
--- 时间增量 time_delta 默认为 1.0 / 60.0  
---@param dt number
---@param x number
---@param y number
---@param rot number
---@overload fun()
---@overload fun(dt:number)
---@overload fun(dt:number, x:number, y:number)
function C:Update(dt, x, y, rot)
end

---@param scale number
function C:Render(scale)
end

--- 默认情况下，开启旧行为模式  
--- 
--- 在旧行为模式下：  
---   * HGE 粒子特效文件 psi 定义的发射角度 Direction 被忽略  
---   * HGE 粒子特效文件 psi 定义的让发射角度与移动方向相关的开关 Relative 被忽略  
--- 和 LuaSTG Plus、LuaSTG Ex Plus、LuaSTG-x 一致  
--- 
--- 关闭旧行为模式后：  
---   * HGE 粒子特效文件 psi 定义的发射角度 Direction 不会忽略，最终粒子发射角度会和 Rotation 相加  
---   * HGE 粒子特效文件 psi 定义的让发射角度与移动方向相关的开关 Relative 不会忽略  
--- 
--- 和 HGE 粒子特效编辑器效果一致  
--- 但是，即使如此，仍然不推荐开启 Relative 功能，除非 Rotation 始终为 0  
---@param enable boolean
function C:SetOldBehavior(enable)
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

--- 【行为可变更】  
--- 默认情况下未使用该属性，请查看 `SetOldBehavior` 方法获得更多信息  
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

--- 【行为可变更】  
--- 默认情况下未使用该属性，请查看 `SetOldBehavior` 方法获得更多信息  
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

--- 【行为可变更】  
--- 默认情况下 Rotation 会覆盖 Direction 属性，请查看 `SetOldBehavior` 方法获得更多信息  
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

--- 【行为可变更】  
--- 默认情况下未使用该属性，请查看 `SetOldBehavior` 方法获得更多信息  
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

--- 【行为可变更】  
--- 默认情况下未使用该属性，请查看 `SetOldBehavior` 方法获得更多信息  
---@param b boolean
function C:setRelative(b)
end

--- 和 LuaSTG-x 不兼容，LuaSTG-x 传入的是 lstg.RenderMode 对象
---@param blend lstg.BlendMode
function C:setRenderMode(blend)
end

--- 【行为可变更】  
--- 默认情况下 Rotation 会覆盖 Direction 属性，请查看 `SetOldBehavior` 方法获得更多信息  
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

local M = {}

--- 从粒子系统资源 ps_name 创建粒子系统实例
---@param ps_name string
---@return lstg.ParticleSystem
function M.ParticleSystemData(ps_name)
end

return M
