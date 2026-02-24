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
	[1] = function(self, ...)
	end,

	--- del 回调函数  
	---@param self lstg.GameObject
	[2] = function(self, ...)
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
	[6] = function(self, ...)
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
	[3] = nil,

	--- [LuaSTG Sub v0.17.0 新增]  
	--- 数组部分 [4] 是引擎内部用途的类实例，严禁访问或修改  
	---@type lstg.ParticleSystem
	[4] = nil;

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
	omega = 0.0,
	--- 和 omega 相同，但 omiga 是一个历史遗留错误，现在通过两个变量都可以访问到同一个值
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
	--- [LuaSTG Sub v0.20.7 移除]  
	--- 【警告】不建议使用  
	--- 自动根据当前坐标和上一帧坐标计算 vx 和 vy  
	--rmove = false,

	--- [LuaSTG Ex Plus 新增]  
	--- [LuaSTG Sub v0.20.7 移除]  
	--- 【警告】这个叼毛东西可能会坑人，给变量起名字时稍不注意就会重名，并导致奇怪的 bug  
	--- 剩余暂停时间，影响：frame 回调的执行，速度与坐标的计算，HGE 粒子特效的更新，timer 和 ani 的更新  
	--pause = 0,

	--- [LuaSTG Ex Plus 新增]  
	--- 不受超级暂停影响  
	nopause = false,
}

---@alias object lstg.GameObject

--------------------------------------------------------------------------------
--- 实例化的游戏对象（已启用渲染拓展功能）

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
	_color = {},

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

	--- [LuaSTG Sub v0.17.0 新增]  
	--- 【不可赋值】  
	--- 绑定在游戏对象上的粒子系统  
	---@type lstg.ParticleSystem
	rc = {},
}
