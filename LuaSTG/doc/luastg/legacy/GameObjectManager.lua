--------------------------------------------------------------------------------
--- LuaSTG Sub 游戏对象管理器
--- 璀境石
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

local M = {}

--------------------------------------------------------------------------------
--- 游戏对象管理器

---获取申请的对象数
---@return number
function M.GetnObj()
end

---@alias lstg.ObjList.Next fun(group_id:number, object_id:number):number, lstg.GameObject

--- 游戏对象迭代器，如果填写的碰撞组不是有效的碰撞组，则对所有游戏对象进行迭代
---@param group_id number
---@return lstg.ObjList.Next, number, number
function M.ObjList(group_id)
end

---回收所有对象，并释放绑定的资源
function M.ResetPool()
end

--- 【禁止在协同程序中调用此方法】  
--- 更新所有游戏对象并触发游戏对象的frame回调函数  
--- 从 LuaSTG Sub v0.21.13（第二代游戏循环更新顺序）开始，可以传递版本参数 `version`：  
--- * 不传递 `version` 参数或参数为 1 时，遵循旧逻辑  
--- * `version` 参数或参数为 2 时，启用新逻辑  
---@param version integer?
function M.ObjFrame(version)
end

---【禁止在协同程序中调用此方法】
---绘制所有游戏对象并触发游戏对象的render回调函数
function M.ObjRender()
end

--- 【禁止在协同程序中调用此方法】  
---  对所有游戏对象进行出界判断，如果离开场景边界，将会触发对象的 del 回调函数  
--- 从 LuaSTG Sub v0.21.13（第二代游戏循环更新顺序）开始，可以传递版本参数 `version`：  
--- * 不传递 `version` 参数或参数为 1 时，遵循旧逻辑  
--- * `version` 参数或参数为 2 时，启用新逻辑  
---@param version integer?
function M.BoundCheck(version)
end

--- [LuaSTG Sub v0.17.0 修改]  
--- 更改场景边界，默认为-100, 100, -100, 100  
--- LuaSTG Sub v0.17.0 修复了设置场景边界时参数被取整为整数的问题  
---@param left number
---@param right number
---@param bottom number
---@param top number
function M.SetBound(left, right, bottom, top)
end

---【禁止在协同程序中调用此方法】  
--- 对两个碰撞组的对象进行碰撞检测  
--- 如果发生碰撞则触发groupidA内的对象的colli回调函数，并传入groupidB内的对象作为参数
---@param groupidA number @只能为0到15范围内的整数
---@param groupidB number @只能为0到15范围内的整数
function M.CollisionCheck(groupidA, groupidB)
end

--- 【禁止在协同程序中调用此方法】  
--- 保存游戏对象的x, y坐标并计算dx, dy  
--- 从 LuaSTG Sub v0.21.13（第二代游戏循环更新顺序）开始，如果启用新逻辑，  
--- 请勿调用 `lstg.UpdateXY` 方法，相关逻辑已合并到 `lstg.AfterFrame` 中  
function M.UpdateXY()
end

--- 【禁止在协同程序中调用此方法】  
--- 增加游戏对象的timer, ani计数器，如果对象被标记为kill或者del，则回收该对象  
--- 从 LuaSTG Sub v0.21.13（第二代游戏循环更新顺序）开始，可以传递版本参数 `version`：  
--- * 不传递 `version` 参数或参数为 1 时，遵循旧逻辑  
--- * `version` 参数或参数为 2 时，启用新逻辑  
---@param version integer?
function M.AfterFrame(version)
end

--------------------------------------------------------------------------------
--- 游戏对象

--- 申请游戏对象，并将游戏对象和指定的class绑定，剩余的参数将会传递给init回调函数并执行
---@param class lstg.Class
---@vararg any
---@return lstg.GameObject
function M.New(class, ...)
end

--- 触发指定游戏对象的del回调函数，并将该对象标记为del状态，剩余参数将传递给del回调函数
---@param unit lstg.GameObject
---@vararg any
function M.Del(unit, ...)
end

--- 触发指定游戏对象的kill回调函数，并将该对象标记为kill状态，剩余参数将传递给kill回调函数
---@param unit lstg.GameObject
---@vararg any
function M.Kill(unit, ...)
end

--- 检查指定游戏对象的引用是否有效，如果返回假，则该对象已经被对象池回收或不是 有效的lstg.GameObject对象；
--- unit参数可以是任何值，因此也可以用来判断传入的参数 是否是游戏对象
---@param unit any
---@return boolean
function M.IsValid(unit)
end

--------------------------------------------------------------------------------
--- 碰撞相关

--- 检查指定对象是否在指定的矩形区域内
---@param unit lstg.GameObject
---@param left number
---@param right number
---@param bottom number
---@param top number
---@return boolean
function M.BoxCheck(unit, left, right, bottom, top)
end

--- 检查两个对象是否发生碰撞
---@param unitA lstg.GameObject
---@param unitB lstg.GameObject
---@param ignoreworldmask boolean @如果该参数为true，则忽略world掩码
function M.ColliCheck(unitA, unitB, ignoreworldmask)
end

--------------------------------------------------------------------------------
--- 属性访问（用于游戏对象的 lua metatable）

--- 更改游戏对象上某些属性的值
---@param t lstg.GameObject
---@param k number|string
---@param v any
function M.SetAttr(t, k, v)
end

--- 获取游戏对象上某些属性的值
---@param t lstg.GameObject
---@param k number|string
function M.GetAttr(t, k)
end

--------------------------------------------------------------------------------
--- 帮助函数

---设置游戏对象的速度
---@param unit lstg.GameObject
---@param v number
---@param a number
---@param updaterot boolean @如果该参数为true，则同时设置对象的rot
function M.SetV(unit, v, a, updaterot)
end

---@param unit lstg.GameObject
---@return number, number @速度大小，速度朝向
function M.GetV(unit)
end

--- 计算向量的朝向，可以以以下的组合方式填写参数：
--- ```txt
--- lstg.GameObject, lstg.GameObject
--- lstg.GameObject, x, y
--- x, y, lstg.GameObject
--- x1, y1, x2, y2
--- ```
---@param x1 lstg.GameObject|number
---@param y1 lstg.GameObject|number
---@param x2 lstg.GameObject|number|nil
---@param y2 number|nil
---@return number
function M.Angle(x1, y1, x2, y2)
end

--- 计算向量的模，可以以以下的组合方式填写参数：
--- ```txt
--- lstg.GameObject, lstg.GameObject
--- lstg.GameObject, x, y
--- x, y, lstg.GameObject
--- x1, y1, x2, y2
--- ```
---@param x1 lstg.GameObject|number
---@param y1 lstg.GameObject|number
---@param x2 lstg.GameObject|number|nil
---@param y2 number|nil
---@return number
function M.Dist(x1, y1, x2, y2)
end

--------------------------------------------------------------------------------
--- 渲染

--- 设置绑定在游戏对象上的资源的混合模式和顶点颜色
---@param unit lstg.GameObject
---@param blend string
---@param a number @[0~255]
---@param r number @[0~255]
---@param g number @[0~255]
---@param b number @[0~255]
function M.SetImgState(unit, blend, a, r, g, b)
end

--- 执行游戏对象默认渲染方法
---@param unit lstg.GameObject
function M.DefaultRenderFunc(unit)
end

--------------------------------------------------------------------------------
--- 游戏对象上的粒子对象

--- 设置绑定在游戏对象上的粒子特效的混合模式和顶点颜色
---@param unit lstg.GameObject
---@param blend string
---@param a number @[0~255]
---@param r number @[0~255]
---@param g number @[0~255]
---@param b number @[0~255]
function M.SetParState(unit, blend, a, r, g, b)
end

--- 停止游戏对象上的粒子发射器
---@param unit lstg.GameObject
function M.ParticleStop(unit)
end

--- 启动游戏对象上的粒子发射器
---@param unit lstg.GameObject
function M.ParticleFire(unit)
end

--- 获取游戏对象上的粒子发射器的粒子数量
---@param unit lstg.GameObject
---@return number
function M.ParticleGetn(unit)
end

--- 设置粒子发射器的粒子发射密度
---@param unit lstg.GameObject
---@param emission number @每秒发射的粒子数量
function M.ParticleSetEmission(unit, emission)
end

--- 获取粒子发射器的粒子发射密度
---@param unit lstg.GameObject
---@return number @每秒发射的粒子数量
function M.ParticleGetEmission(unit)
end

--------------------------------------------------------------------------------
--- 游戏对象池更新暂停（高级功能）


--- 设置游戏对象池下一帧开始暂停更新的时间（帧）
---@param t number
function M.SetSuperPause(t)
end

--- 更改游戏对象池下一帧开始暂停更新的时间（帧），等效于GetSuperPause并加上t，然后SetSuperPause
---@param t number
function M.AddSuperPause(t)
end

--- 获取游戏对象池暂停更新的时间（帧），获取的是下一帧的
---@return number
function M.GetSuperPause()
end

--- 获取当前帧游戏对象池暂停更新的时间（帧）
function M.GetCurrentSuperPause()
end

--------------------------------------------------------------------------------
--- 游戏对象world掩码（高级功能）

--- 关于多 world 掩码对碰撞检测的影响
--- 假如设置了如下的 4 个 world 掩码
---   * Mask1: 0x01 (0001)
---   * Mask2: 0x02 (0010)
---   * Mask3: 0x04 (0100)
---   * Mask4: 0x07 (0111)
--- 现在有 3 个游戏对象，它们的 world 掩码分别为
---   * ObjectA: 0x01 (0001)
---   * ObjectB: 0x02 (0010)
---   * ObjectC: 0x07 (0111)
--- 那么调用 CollisionCheck 进行碰撞检测时
---   * ObjectA 和 ObjectB 之间不会进行碰撞检测
---   * ObjectA 和 ObjectC 之间以及 ObjectB 和 ObjectC 之间会进行碰撞检测
--- 判断方式相当于调用 IsSameWorld

--- 调用 SetWorldFlag 设置当前 world 掩码会影响以下的功能
--- 当游戏对象的 world 掩码与当前 world 掩码匹配时，才会
---   * DoRender: 渲染游戏对象
---   * BoundCheck: 进行出界检测
---   * DrawGroupCollider: 绘制游戏对象的碰撞体
--- 判断方式相当于调用 IsInWorld

--- 设置当前激活的world掩码
---@param mask number
function M.SetWorldFlag(mask)
end

--- 获取当前激活的 world 掩码
---@return number
function M.GetWorldFlag()
end

--- 检查两个 world 掩码是否存在交叠的部分
---@param maskA number
---@param maskB number
---@return boolean
function M.IsInWorld(maskA, maskB)
end

--- 根据 ActiveWorlds 设置的多 world 掩码，判断两个对象是否在同一个 world 内
---@param maskA number
---@param maskB number
---@return boolean
function M.IsSameWorld(maskA, maskB)
end

--- 设置多 world 的掩码，最多可支持 4 个不同的掩码，将会在进行碰撞检测的时候用到
---@param maskA number
---@param maskB number
---@param maskC number
---@param maskD number
function M.ActiveWorlds(maskA, maskB, maskC, maskD)
end

return M
