--------------------------------------------------------------------------------
--- LuaSTG Sub 游戏对象
--- 璀境石
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
--- 游戏对象的回调函数和其他特性

---@class lstg.Class
local game_object_class = {
	-------- 数组部分 --------

	--- init 回调函数  
	---@param self lstg.GameObject
	[1] = function(self)
	end,

	--- del 回调函数  
	---@param self lstg.GameObject
	[2] = function(self)
	end,

	--- frame 回调函数  
	---@param self lstg.GameObject
	[3] = function(self)
	end,

	--- render 回调函数  
	--- 推荐使用引擎提供的 lstg.DefaultRenderFunc  
	---@param self lstg.GameObject
	[4] = function(self)
	end,

	--- colli 回调函数  
	---@param self lstg.GameObject
	---@param other lstg.GameObject
	[5] = function(self, other)
	end,

	--- kill 回调函数  
	---@param self lstg.GameObject
	[6] = function(self)
	end;

	-------- 散列部分 --------

	--- 【警告】要通过 lstg.New 创建游戏对象实例，该属性是必须的，且必须为 true  
	--- 标记该 table 可用于 lstg.New 创建游戏对象实例  
	is_class = true,

	--- [LuaSTG Ex Plus 新增]  
	--- 【可选】  
	--- 启用 lstg.GameRenderObject 的功能
	[".render"] = false,

	--- [LuaSTG Ex Plus 新增]  
	--- 【可选】  
	--- 一组位掩码的组合，用来禁用某些 lua 回调函数，并使用引擎默认的方法，以此提高性能  
	--- 目前可用的值：  
	--- * 0x08 禁用 frame 回调函数，并使用引擎默认的更新方法  
	--- * 0x10 禁用 render 回调函数，并使用引擎默认的渲染方法  
	default_function = 0x0,
}

--------------------------------------------------------------------------------
--- 实例化的游戏对象

---@alias lstg.GameObject.Status '"normal"' | '"del"' | '"kill"'

---@class lstg.GameObject.Internal : lightuserdata

--- 实例化的游戏对象
---@class lstg.GameObject
local game_object = {
	-------- 数组部分 --------

	--- 数组部分 [1] 是创建对象时传入的 class 参数，可以通过属性 class 访问
	---@type lstg.Class
	[1] = game_object_class,

	--- 数组部分 [2] 是引擎内部用途的对象索引，严禁访问或修改
	[2] = 0,

	--- [LuaSTG Sub v0.16.1 新增]  
	--- 数组部分 [3] 是引擎内部用途的指针，严禁访问或修改  
	---@type lstg.GameObject.Internal
	[3] = {};

	-------- 散列部分 --------

	-------- 基本信息

	--- 对象状态，"del" 和 "kill" 均代表对象即将被回收
	---@type lstg.GameObject.Status
	status = "normal",

	--- 创建对象时传入的 class 参数，等效于数组部分 [1]
	---@type lstg.Class
	class = game_object_class,

	-------- 分组

	--- [LuaSTG Ex Plus 新增]  
	--- 世界掩码，与更新、渲染、碰撞检测有关  
	world = 0xF,

	-------- 位置

	--- x 坐标
	x = 0.0,

	--- y 坐标
	y = 0.0,

	--- 【只读】  
	--- x 坐标相对上一帧的变化量
	dx = 0.0,

	--- 【只读】  
	--- y 坐标相对上一帧的变化量
	dy = 0.0,

	-------- 运动学

	--- x 轴速度分量
	vx = 0.0,

	--- y 轴速度分量
	vy = 0.0,

	--- x 轴加速度分量
	ax = 0.0,

	--- y 轴加速度分量
	ay = 0.0,

	--- [LuaSTG Ex Plus 新增]  
	--- 原始代码在 THlib 中，由于使用频率较高，处于性能考虑，将 lua 代码移植到引擎内用 C++ 实现  
	--- x 轴最大速度分量  
	maxvx = 1e100,

	--- [LuaSTG Ex Plus 新增]  
	--- 原始代码在 THlib 中，由于使用频率较高，处于性能考虑，将 lua 代码移植到引擎内用 C++ 实现  
	--- y 轴最大速度分量  
	maxvy = 1e100,

	--- [LuaSTG Ex Plus 新增]  
	--- 原始代码在 THlib 中，由于使用频率较高，处于性能考虑，将 lua 代码移植到引擎内用 C++ 实现  
	--- 最大速度  
	maxv = 1e100,

	--- [LuaSTG Ex Plus 新增]  
	--- 原始代码在 THlib 中，由于使用频率较高，处于性能考虑，将 lua 代码移植到引擎内用 C++ 实现  
	--- 重力加速度，方向永远朝向 y 轴负方向  
	ag = 0.0,

	--- [LuaSTG Ex Plus 新增]  
	--- 【警告】该方法在速度大小较小时误差很大  
	--- 【警告】不建议使用  
	--- 读取时，计算速度朝向（相当于调用 lstg.GetV）  
	--- 写入时，修改速度朝向  
	_angle = 0.0,

	--- [LuaSTG Ex Plus 新增]  
	--- 【警告】该方法在速度大小较小时误差很大  
	--- 【警告】不建议使用  
	--- 读取时，计算速度大小（相当于调用 lstg.GetV）  
	--- 写入时，修改速度大小  
	_speed = 0.0,

	-------- 碰撞体

	--- 【警告】严禁在使用 lstg.ObjList 遍历对象时直接修改碰撞组，应该缓存下来等遍历结束后修改  
	--- 【警告】尽量避免在 colli 回调函数中（比如每帧调用的 lstg.CollisionCheck 触发）修改碰撞组  
	--- 【提示】LuaSTG 引擎会尽可能地包容这些行为，并尝试让对象遍历、碰撞检测的过程不被意外中止，但是这并不保证在一些极端情况下仍然会导致游戏崩溃  
	--- 碰撞组，取值范围 0 到 15，一共 16 个碰撞组  
	group = 0,

	--- 是否离开边界（只判断中心坐标）自动删除，参考 lstg.SetBound
	bound = true,

	--- 是否参与碰撞检测
	colli = true,

	--- 碰撞体类型，false 时为圆或椭圆，true 时为有向矩形
	rect = false,

	--- 如果 rect 为 true，代表矩形横向长度一半  
	--- 如果 rect 为 false，代表椭圆横向长度一半  
	--- 如果 a 严格等于 b，代表圆的半径  
	a = 0.0,

	--- 如果 rect 为 true，代表矩形纵向长度一半  
	--- 如果 rect 为 false，代表椭圆纵向长度一半  
	--- 如果 a 严格等于 b，代表圆的半径  
	b = 0.0,

	-------- 渲染

	--- 【警告】严禁在 render 回调函数中（比如由每帧调用的 lstg.ObjRender 触发）修改图层  
	--- 渲染图层，影响游戏对象渲染的先后顺序  
	layer = 0.0,

	--- 横向渲染缩放
	hscale = 1.0,

	--- 纵向渲染缩放
	vscale = 1.0,

	--- 渲染时平面旋转角度（角度制）
	rot = 0.0,

	--- 平面旋转角度自增量（角度制），和 navi 冲突
	omiga = 0.0,

	--- 【只读】  
	--- 连续自增动画计数器  
	ani = 0,

	--- 不渲染
	hide = false,

	--- 自动根据对象运动计算渲染时平面旋转角度，和 omiga 冲突
	navi = false,

	--- 游戏对象的渲染资源，可以是图片精灵、动画、HGE 粒子特效，赋值为 nil 时将会释放资源  
	--- 读取时：获取资源名称  
	--- 写入时：按 图片精灵 -> 动画 -> HGE 粒子特效 的顺序搜素资源，并应用到游戏对象上  
	img = "unkown",

	-------- 杂项

	--- 自增计数器，可能被自由修改
	timer = 0,

	--- [LuaSTG Ex Plus 新增]  
	--- 【警告】不建议使用  
	--- 自动根据当前坐标和上一帧坐标计算 vx 和 vy  
	rmove = false,

	--- [LuaSTG Ex Plus 新增]  
	--- 【警告】这个叼毛东西可能会坑人，给变量起名字时稍不注意就会重名，并导致奇怪的 bug  
	--- 剩余暂停时间，影响：frame 回调的执行，速度与坐标的计算，HGE 粒子特效的更新，timer 和 ani 的更新  
	pause = 0,

	--- [LuaSTG Ex Plus 新增]  
	--- 不受超级暂停影响  
	nopause = false,
}

---@alias object lstg.GameObject

---@class lstg.GameRenderObject : lstg.GameObject
local render_object = {
	-------- 散列部分 --------

	-------- 渲染

	--- [LuaSTG Ex Plus 新增]  
	--- 绑定在游戏对象上的混合模式  
	---@type lstg.BlendMode
	_blend = "",

	--- [LuaSTG Ex Plus 新增]  
	--- 【警告】该属性以值的方式交互  
	--- 绑定在游戏对象上的顶点色  
	--- 读取时：获取顶点色的 lstg.Color 副本，对该副本的修改不会影响游戏对象  
	--- 写入时：复制顶点色到游戏对象，不引用 lstg.Color 对象
	---@type lstg.Color
	_color = lstg.Color(255, 255, 255, 255),

	--- [LuaSTG Ex Plus 新增]  
	--- 绑定在游戏对象上的顶点色 alpha 分量  
	_a = 255,

	--- [LuaSTG Ex Plus 新增]  
	--- 绑定在游戏对象上的顶点色红色分量  
	_r = 255,

	--- [LuaSTG Ex Plus 新增]  
	--- 绑定在游戏对象上的顶点色绿色分量  
	_g = 255,

	--- [LuaSTG Ex Plus 新增]  
	--- 绑定在游戏对象上的顶点色蓝色分量  
	_b = 255,
}

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
---对所有游戏对象进行出界判断，如果离开场景边界，将会触发对象的del回调函数
function lstg.BoundCheck()
end

---更改场景边界，默认为-100, 100, -100, 100
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
