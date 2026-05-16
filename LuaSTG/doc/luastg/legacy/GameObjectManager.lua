---@diagnostic disable: duplicate-set-field, unused-local

--------------------------------------------------------------------------------
--- LuaSTG Sub 游戏对象管理器
--- 璀境石
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

local M = {}

--------------------------------------------------------------------------------
--- 游戏对象管理器

--- 获取申请的对象数
---@return number
function M.GetnObj()
end

---@alias lstg.ObjList.Next fun(group_id:number, object_id:number):number, lstg.GameObject

--- 游戏对象迭代器，如果填写的碰撞组不是有效的碰撞组，则对所有游戏对象进行迭代
---@param group_id number
---@return lstg.ObjList.Next, number, number
function M.ObjList(group_id)
end

--- 回收所有对象，并释放绑定的资源
function M.ResetPool()
end

--- **禁止在协同程序（continue）中调用此方法**  
--- 
--- 更新所有游戏对象并触发游戏对象的frame回调函数  
--- 从 LuaSTG Sub v0.21.13（第二代游戏循环更新顺序）开始，可以传递版本参数 `version`
--- 
--- * 不传递 `version` 参数或参数值为 `1` 时，逻辑伪代码  
---     ```lua
---     for object in lstg.ObjList() do
---         -- 执行 frame 回调函数
---         object:frame()
---         -- 根据 ax、ay、ag 更新 vx、vy
---         object.vx = object.vx + object.ax
---         object.vy = object.vy + object.ay - object.ag
---         -- 根据 maxv 限制 vx、vy
---         local speed = sqrt(object.vx * object.vx + object.vy * object.vy)
---         if speed > maxv then
---             local scale = maxv / speed
---             object.vx = object.vx * scale
---             object.vy = object.vy * scale
---         end
---         -- 根据 maxvx、maxvy 限制 vx、vy 范围
---         object.vx = clamp(object.vx, -object.maxvx, object.maxvx)
---         object.vy = clamp(object.vy, -object.maxvy, object.maxvy)
---         -- 根据 vx、vy 更新 x、y
---         object.x = object.x + object.vx
---         object.y = object.y + object.vy
---         -- 根据 omega（omiga）更新 rot
---         object.rot = object.rot + object.omega
---         -- 更新粒子系统（若有）
---         updateParticleSystem(object)
---     end
---     ```
--- * `version` 参数值为 `2` 时，逻辑伪代码  
---     ```lua
---     for object in lstg.ObjList() do
---         -- 执行 frame 回调函数
---         object:frame()
---     end
---     for object in lstg.ObjList() do
---         -- 根据 ax、ay、ag 更新 vx、vy
---         object.vx = object.vx + object.ax
---         object.vy = object.vy + object.ay - object.ag
---         -- 根据 maxv 限制 vx、vy
---         local speed = sqrt(object.vx * object.vx + object.vy * object.vy)
---         if speed > maxv then
---             local scale = maxv / speed
---             object.vx = object.vx * scale
---             object.vy = object.vy * scale
---         end
---         -- 根据 maxvx、maxvy 限制 vx、vy 范围
---         object.vx = clamp(object.vx, -object.maxvx, object.maxvx)
---         object.vy = clamp(object.vy, -object.maxvy, object.maxvy)
---         -- 根据 vx、vy 更新 x、y
---         object.x = object.x + object.vx
---         object.y = object.y + object.vy
---         -- 根据 omega（omiga）更新 rot
---         object.rot = object.rot + object.omega
---         -- 根据 navi 更新 rot
---         if object.navi then
---             object.rot = atan2(object.dy, object.dx)
---         end
---         -- 更新粒子系统（若有）
---         updateParticleSystem(object)
---     end
---     ````
--- 
---@param version integer?
function M.ObjFrame(version)
end

--- **禁止在协同程序（continue）中调用此方法**  
---
--- 绘制所有游戏对象并触发游戏对象的render回调函数
function M.ObjRender()
end

--- **禁止在协同程序（continue）中调用此方法**  
--- 
---  对所有游戏对象进行出界判断，如果离开场景边界，将会触发对象的 del 回调函数  
--- 从 LuaSTG Sub v0.21.13（第二代游戏循环更新顺序）开始，可以传递版本参数 `version`  
--- 
--- * 不传递 `version` 参数或参数值为 `1` 时，逻辑伪代码  
---     ```lua
---     for object in lstg.ObjList() do
---         -- 判断游戏对象中心位置是否离开世界边界
---         if not isInWorldBoundary(object) then
---             -- 标记删除游戏对象
---             lstg.Del(object)
---         end
---     end
---     ```
--- * `version` 参数值为 `2` 时，逻辑伪代码  
---     ```lua
---     -- 第一步
---     local results = {}
---     for object in lstg.ObjList() do
---         -- 判断游戏对象中心位置是否离开世界边界
---         if not isInWorldBoundary(object) then
---             table.insert(results, object)
---         end
---     end
---     -- 第二步
---     for _, object in ipairs(results) do
---         -- 标记删除游戏对象
---         lstg.Del(object)
---     end
---     ```
--- 
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

--- **禁止在协同程序（continue）中调用此方法**  
---
--- 对两个碰撞组的对象进行相交检测  
--- `group1` 和 `group2` 的取值范围是 0 到 15 的整数  
--- 
--- * 逻辑伪代码
---     ```lua
---     for object1 in lstg.ObjList(group1) do
---         for object2 in lstg.ObjList(group2) do
---             if hasIntersection(object1, object2) then
---                 object1:colli(object2)
---             end
---         end
---     end
---     ```
--- 
---@param group1 number
---@param group2 number
function M.CollisionCheck(group1, group2)
end

--- **禁止在协同程序（continue）中调用此方法**  
--- 
--- 从 LuaSTG Sub v0.21.13（第二代游戏循环更新顺序）开始，该方法重载可用  
--- 一次性对多个碰撞组对的对象进行相交检测  
--- `group_pairs` 参数是碰撞组对列表，每个碰撞组对是一个长度为 2 的数组，分别代表 `group1` 和 `group2`  
--- 
--- * 逻辑伪代码
---     ```lua
---     -- 第一步
---     local results = {}
---     for group_pair in ipairs(group_pairs) do
---         for object1 in lstg.ObjList(group_pair[1]) do
---             for object2 in lstg.ObjList(group_pair[2]) do
---                 if hasIntersection(object1, object2) then
---                     table.insert(results, { object1, object2 })
---                 end
---             end
---         end
---     end
---     -- 第二步
---     for _, result in ipairs(results) do
---         result[1]:colli(result[2])
---     end
---     ```
--- 
---@param group_pairs { [1]: number, [2]: number }[]
function M.CollisionCheck(group_pairs)
end

--- **禁止在协同程序（continue）中调用此方法**  
--- 
--- 从 LuaSTG Sub v0.21.13（第二代游戏循环更新顺序）开始，可以传递版本参数 `version`  
--- 
--- * 不传递 `version` 参数或参数值为 `1` 时，逻辑伪代码  
---     ```lua
---     for object in lstg.ObjList() do
---         -- 更新 dx、dy（注意 lastx、lasty 在 lua 层不可访问）
---         object.dx = object.x - object.lastx
---         object.dy = object.y - object.lasty
---         object.lastx = object.x
---         object.lasty = object.y
---         -- 根据 navi 更新 rot
---         if object.navi then
---             object.rot = atan2(object.dy, object.dx)
---         end
---     end
---     ```
--- * `version` 参数值为 `2` 时，逻辑伪代码  
---     ```lua
---     for object in lstg.ObjList() do
---         -- 更新 dx、dy，注意 lastx、lasty 在 lua 层不可访问
---         object.dx = object.x - object.lastx
---         object.dy = object.y - object.lasty
---         object.lastx = object.x
---         object.lasty = object.y
---         -- 更新计时器
---         object.timer = object.timer + 1
---         object.ani = object.ani + 1
---     end
---     ```
--- 
---@param version integer?
function M.UpdateXY(version)
end

--- **禁止在协同程序（continue）中调用此方法**  
--- 
--- 从 LuaSTG Sub v0.21.13（第二代游戏循环更新顺序）开始，可以传递版本参数 `version`：  
--- 
--- * 不传递 `version` 参数或参数值为 `1` 时，逻辑伪代码  
---     ```lua
---     for object in lstg.ObjList() do
---         -- 更新计时器
---         object.timer = object.timer + 1
---         object.ani = object.ani + 1
---         -- 如果对象标记为删除（不是正常状态），回收对象
---         if object.status ~= "normal" then
---             freeGameObject(object)
---         end
---     end
---     ```
--- * `version` 参数值为 `2` 时，逻辑伪代码  
---     ```lua
---     for object in lstg.ObjList() do
---         -- 如果游戏对象是正常状态，执行更新，否则回收对象
---         if object.status == "normal" then
---             -- 更新 dx、dy，注意 lastx、lasty 在 lua 层不可访问
---             object.dx = object.x - object.lastx
---             object.dy = object.y - object.lasty
---             object.lastx = object.x
---             object.lasty = object.y
---             -- 更新计时器
---             object.timer = object.timer + 1
---             object.ani = object.ani + 1
---         else
---             -- 回收对象
---             freeGameObject(object)
---         end
---     end
---     ```
--- * `version` 参数值为 `3` 时，逻辑伪代码  
---     ```lua
---     for object in lstg.ObjList() do
---         if object.status ~= "normal" then
---             freeGameObject(object)
---         end
---     end
---     ```
--- 
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
