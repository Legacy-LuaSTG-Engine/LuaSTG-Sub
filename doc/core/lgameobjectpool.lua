--------------------------------------------------------------------------------
--- LuaSTG Sub 对象池与游戏对象
--- 璀境石
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
--- 游戏对象的回调函数集合

---@class lstg.Class
local game_object_class = {
	--数组部分
	function(self)
	end, --init
	function(self, ...)
	end, --del
	function(self)
	end, --frame
	lstg.DefaultRenderFunc, --render
	function(self, other)
	end, --colli
	function(self, ...)
	end; --kill

	--散列部分
	---标识一个table是否为合法的lstg.Class
	is_class = true,
	---[不稳定的特性，可能会在日后更改] 标识这个lstg.Class是否可以创建一个lstg.RenderObject对象
	--[".render"] = false,
}

---@param self lstg.GameObject
---@vararg any @随着lstg.New传入的参数
function game_object_class.init(self, ...)
end

---@param self lstg.GameObject
function game_object_class.frame(self)
end

---@param self lstg.GameObject
function game_object_class.render(self)
end

---@param self lstg.GameObject
---@vararg any @随着lstg.Del传入的参数
function game_object_class.del(self, ...)
end

---@param self lstg.GameObject
---@vararg any @随着lstg.Kill传入的参数
function game_object_class.kill(self, ...)
end

---@param self lstg.GameObject
---@param other lstg.GameObject
function game_object_class.colli(self, other)
end

--------------------------------------------------------------------------------
--- 实例化的游戏对象

---@alias lstg.GameObject.Status '"normal"' | '"del"' | '"kill"'

---luastg中实例化的游戏对象
---@class lstg.GameObject
local game_object = {
	--数组部分
	---class
	---@type lstg.Class
	game_object_class,
	---id
	0;

	--散列部分
	--========object========
	---状态，可以为"normal"、"kill"、"del"，分别代表正常状态、被标记为kill、被标记为del
	---@type lstg.GameObject.Status
	status = "normal",
	---class，实际上映射到数组部分索引为1的位置
	---@type lstg.Class
	class = game_object_class,
	--========user========
	---计数器，会每帧自增
	timer = 0,
	--========position========
	---x坐标
	x = 0,
	---y坐标
	y = 0,
	---x坐标相对上一帧的变化量
	dx = 0,
	---y坐标相对上一帧的变化量
	dy = 0,
	--========movement========
	---x轴加速度分量
	ax = 0,
	---y轴加速度分量
	ay = 0,
	---重力加速度，方向永远朝向y轴负方向
	ag = 0,
	---x轴速度分量
	vx = 0,
	---y轴速度分量
	vy = 0,
	---最大速度
	maxv = 1e100,
	---x轴最大速度分量
	maxvx = 1e100,
	---y轴最大速度分量
	maxvy = 1e100,
	--========render========
	---动画图片精灵计数器，只读
	ani = 0,
	---渲染图层（和游戏对象的渲染顺序有关）
	layer = 0,
	---不参与渲染
	hide = false,
	---图片精灵朝向
	rot = 0,
	---朝向自增量，和navi属性冲突
	omiga = 0,
	---自动根据坐标变换计算朝向，和omiga属性冲突
	navi = false,
	---图片精灵横向缩放
	hscale = 1,
	---图片精灵纵向缩放
	vscale = 1,
	---可应用图片精灵资源、动画资源、HGE粒子资源，赋值为nil时将会释放绑定在对象上的资源
	img = "unkown",
	--========collision========
	---碰撞组id，取值范围 0 到 15
	group = 0,
	---是否出界自动删除，参考lstg.SetBound
	bound = true,
	---是否参与碰撞
	colli = true,
	---如果rect属性为true，则为矩形半宽，否则为椭圆半长轴，如果a严格等于b，则为圆的半径
	a = 0,
	---如果rect属性为true，则为矩形半高，否则为椭圆半短轴，如果a严格等于b，则为圆的半径
	b = 0,
	---碰撞体类型，false时为圆或椭圆，true时为有向矩形
	rect = false,
	--[[
	--========ex+========
	---自动根据当前坐标和上一帧坐标计算vx,vy
	rmove = false,
	---计算出的朝向
	_angle = 0,
	---计算出的速度
	_speed = 0,
	--]]
	---剩余暂停时间，影响frame回调的执行、速度与坐标的计算、timer和ani等的更新
	pause = 0,
	---不受暂停影响
	nopause = false,
	---世界掩码，与渲染、碰撞检测有关
	world = 1,
}

--[[
---@class lstg.RenderObject : lstg.GameObject
local render_object = {
	--========.render========
	---渲染时使用的混合模式
	_blend = "",
	---顶点色
	---@type lstg.Color
	_color = {},
	---顶点色alpha分量
	_a = 255,
	---顶点色红色分量
	_r = 255,
	---顶点色绿色分量
	_g = 255,
	---顶点色蓝色分量
	_b = 255,
}
--]]

---@class object : lstg.GameObject
local _ = game_object

--------------------------------------------------------------------------------
--- 游戏对象池

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
