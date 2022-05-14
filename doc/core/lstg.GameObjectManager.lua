--------------------------------------------------------------------------------
--- LuaSTG Sub 游戏对象管理器
--- 璀境石
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
--- 游戏对象管理器

---获取申请的对象数
---@return number
function lstg.GetnObj()
end

---回收所有对象，并释放绑定的资源
function lstg.ResetPool()
end

---【禁止在协同程序中调用此方法】
---更新所有游戏对象并触发游戏对象的frame回调函数
function lstg.ObjFrame()
end

---【禁止在协同程序中调用此方法】
---绘制所有游戏对象并触发游戏对象的render回调函数
function lstg.ObjRender()
end

---【禁止在协同程序中调用此方法】  
--- 对所有游戏对象进行出界判断，如果离开场景边界，将会触发对象的 del 回调函数  
function lstg.BoundCheck()
end

--- [LuaSTG Sub v0.17.0 修改]  
--- 更改场景边界，默认为-100, 100, -100, 100  
--- LuaSTG Sub v0.17.0 修复了设置场景边界时参数被取整为整数的问题  
---@param left number
---@param right number
---@param bottom number
---@param top number
function lstg.SetBound(left, right, bottom, top)
end

---【禁止在协同程序中调用此方法】
---对两个碰撞组的对象进行碰撞检测，如果发生碰撞则触发groupidA内的对象的colli回调函数，并传入groupidB内的对象作为参数
---@param groupidA number @只能为0到15范围内的整数
---@param groupidB number @只能为0到15范围内的整数
function lstg.CollisionCheck(groupidA, groupidB)
end

---【禁止在协同程序中调用此方法】
---保存游戏对象的x, y坐标并计算dx, dy
function lstg.UpdateXY()
end

---【禁止在协同程序中调用此方法】
---增加游戏对象的timer, ani计数器，如果对象被标记为kill或者del，则回收该对象
function lstg.AfterFrame()
end

--------------------------------------------------------------------------------
--- 游戏对象

--- 申请游戏对象，并将游戏对象和指定的class绑定，剩余的参数将会传递给init回调函数并执行
---@param class lstg.Class
---@vararg any
---@return lstg.GameObject
function lstg.New(class, ...)
	---@type lstg.GameObject
	local ret = {}
	return ret
end

--- 触发指定游戏对象的del回调函数，并将该对象标记为del状态，剩余参数将传递给del回调函数
---@param unit lstg.GameObject
---@vararg any
function lstg.Del(unit, ...)
end

--- 触发指定游戏对象的kill回调函数，并将该对象标记为kill状态，剩余参数将传递给kill回调函数
---@param unit lstg.GameObject
---@vararg any
function lstg.Kill(unit, ...)
end

--- 检查指定游戏对象的引用是否有效，如果返回假，则该对象已经被对象池回收或不是 有效的lstg.GameObject对象；
--- unit参数可以是任何值，因此也可以用来判断传入的参数 是否是游戏对象
---@param unit any
---@return boolean
function lstg.IsValid(unit)
	return false
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
function lstg.BoxCheck(unit, left, right, bottom, top)
	return true
end

--- 检查两个对象是否发生碰撞
---@param unitA lstg.GameObject
---@param unitB lstg.GameObject
---@param ignoreworldmask boolean @如果该参数为true，则忽略world掩码
function lstg.ColliCheck(unitA, unitB, ignoreworldmask)
	return false
end

---@alias lstg.ObjList.Next fun(groupid:number, objid:number):number, lstg.GameObject

--- 碰撞组迭代器，如果填写的碰撞组不是有效的碰撞组，则对所有游戏对象进行迭代
---@param groupid number
---@return lstg.ObjList.Next, number, number
function lstg.ObjList(groupid)
end

--------------------------------------------------------------------------------
--- 属性访问（用于游戏对象的 lua metatable）

--- 更改游戏对象上某些属性的值
---@param t lstg.GameObject
---@param k number|string
---@param v any
function lstg.SetAttr(t, k, v)
end

--- 获取游戏对象上某些属性的值
---@param t lstg.GameObject
---@param k number|string
function lstg.GetAttr(t, k)
end

--------------------------------------------------------------------------------
--- 帮助函数

---设置游戏对象的速度
---@param unit lstg.GameObject
---@param v number
---@param a number
---@param updaterot boolean @如果该参数为true，则同时设置对象的rot
function lstg.SetV(unit, v, a, updaterot)
end

---@param unit lstg.GameObject
---@return number, number @速度大小，速度朝向
function lstg.GetV(unit)
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
function lstg.Angle(x1, y1, x2, y2)
	return 0
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
function lstg.Dist(x1, y1, x2, y2)
	return 0
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
function lstg.SetImgState(unit, blend, a, r, g, b)
end

--- 执行游戏对象默认渲染方法
---@param unit lstg.GameObject
function lstg.DefaultRenderFunc(unit)
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
function lstg.SetParState(unit, blend, a, r, g, b)
end

--- 停止游戏对象上的粒子发射器
---@param unit lstg.GameObject
function lstg.ParticleStop(unit)
end

--- 启动游戏对象上的粒子发射器
---@param unit lstg.GameObject
function lstg.ParticleFire(unit)
end

--- 获取游戏对象上的粒子发射器的粒子数量
---@param unit lstg.GameObject
---@return number
function lstg.ParticleGetn(unit)
end

--- 设置粒子发射器的粒子发射密度
---@param unit lstg.GameObject
---@param emission number @每秒发射的粒子数量
function lstg.ParticleSetEmission(unit, emission)
end

--- 获取粒子发射器的粒子发射密度
---@param unit lstg.GameObject
---@return number @每秒发射的粒子数量
function lstg.ParticleGetEmission(unit)
end

--------------------------------------------------------------------------------
--- 游戏对象池更新暂停（高级功能）


--- 设置游戏对象池下一帧开始暂停更新的时间（帧）
---@param t number
function lstg.SetSuperPause(t)
end

--- 更改游戏对象池下一帧开始暂停更新的时间（帧），等效于GetSuperPause并加上t，然后SetSuperPause
---@param t number
function lstg.AddSuperPause(t)
end

--- 获取游戏对象池暂停更新的时间（帧），获取的是下一帧的
---@return number
function lstg.GetSuperPause()
end

--- 获取当前帧游戏对象池暂停更新的时间（帧）
function lstg.GetCurrentSuperPause()
end

--------------------------------------------------------------------------------
--- 游戏对象world掩码（高级功能）

--- 设置当前激活的world掩码
---@param mask number
function lstg.SetWorldFlag(mask)
end

--- 获取当前激活的 world 掩码
---@return number
function lstg.GetWorldFlag()
end

--- 判断两个对象是否在同一个 world 内，当两个游戏对象的 world 掩码相同或者按位与不为 0 时返回 true
---@param unitA lstg.GameObject
---@param unitB lstg.GameObject
---@return boolean
function lstg.IsSameWorld(unitA, unitB)
end

--- 设置多 world 的掩码，最多可支持 4 个不同的掩码，将会在进行碰撞检测的时候用到
---@param maskA number
---@param maskB number
---@param maskC number
---@param maskD number
function lstg.ActiveWorlds(maskA, maskB, maskC, maskD)
end

--- 检查两个对象是否存在于相同的 world 内，参考ActiveWorlds
---@param unitA lstg.GameObject
---@param unitB lstg.GameObject
---@return boolean
function lstg.CheckWorlds(unitA, unitB)
end
