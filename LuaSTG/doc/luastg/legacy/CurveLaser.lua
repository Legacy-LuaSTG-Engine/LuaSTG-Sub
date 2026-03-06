--------------------------------------------------------------------------------
--- LuaSTG Sub 曲线激光对象
--- 璀境石
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

---@class lstg.CurveLaser
local CurveLaser = {}

--------------------------------------------------------------------------------
--- 更新与检测

--- [LuaSTG Sub v0.17.4 更改]  
--- 更新曲线激光，为曲线激光添加新节点、删除旧节点  
--- 参数 node_count 为曲线激光最大节点数量，该值的取值范围为 2 到 512  
--- 参数 width 为新增的节点的宽度  
--- 更新曲线激光节点时，可以传入新位置的 x 和 y 坐标，以及节点 rot 朝向  
--- 也可以传入一个游戏对象  
---@param x number
---@param y number
---@param rot number
---@param node_count number
---@param width number
---@overload fun(self:lstg.CurveLaser, unit:lstg.GameObject, node_count:number, width:number)
function CurveLaser:Update(x, y, rot, node_count, width)
end

--- [LuaSTG Ex Plus 新增]  
--- [LuaSTG Sub v0.17.4 更改]  
--- 修改曲线激光指定节点的数据，参数 node_index 为 lua 风格的索引，从 1 开始  
--- 一般情况下不会使用这个方法，仅在将曲线激光对象当成带状渲染器的时候才会用到  
--- 注意：该方法会重新计算相邻节点的数据，如果你需要修改整个曲线激光，建议使用 UpdateAllNode 方法，否则会影响性能  
---@param node_index number
---@param x number
---@param y number
---@param width number
function CurveLaser:UpdateNode(node_index, x, y, width)
end

--- [LuaSTG Sub v0.17.4 新增]  
--- 设置曲线激光节点数量为 node_count  
--- 并按顺序分别将 x、y 坐标和宽度 width 设置到节点上  
--- 如果 width 是一个数字，则所有节点都被设置一样的宽度  
--- 这样传参的目的是降低性能占用  
--- 一般情况下不会使用这个方法，仅在将曲线激光对象当成带状渲染器的时候才会用到  
---@param node_count number
---@param x number[]
---@param y number[]
---@param width number|number[]
function CurveLaser:UpdateAllNode(node_count, x, y, width)
end

--- 检查曲线激光是否还有节点处于场景范围内
---@return boolean
function CurveLaser:BoundCheck()
end

--- 将曲线激光与给定数据的碰撞体进行碰撞检测
---@param x number
---@param y number
---@param rot number
---@param a number
---@param b number
---@param rect boolean
---@return boolean
function CurveLaser:CollisionCheck(x, y, rot, a, b, rect)
end

--- [LuaSTG Ex Plus 新增]  
--- [LuaSTG Sub v0.20.11 废弃]  
--- 将曲线激光与给定数据的碰撞体进行碰撞检测
---@deprecated
---@param x number
---@param y number
---@param width number
---@param rot number
---@param a number
---@param b number
---@param rect boolean
---@return boolean
function CurveLaser:CollisionCheckWidth(x, y, width, rot, a, b, rect)
end

--- [LuaSTG Sub v0.20.11 新增]  
---@param width number
---@param x number
---@param y number
---@param rot number
---@param a number
---@param b number
---@param rect boolean
---@return boolean
---@overload fun(self:lstg.CurveLaser, width:number, game_object:lstg.GameObject)
function CurveLaser:CollisionCheckWithWidth(width, x, y, rot, a, b, rect)
end

--- [LuaSTG Ex Plus 新增]  
--- 更改曲线激光所有节点的宽度
---@param width number
function CurveLaser:SetAllWidth(width)
end

--- 释放曲线激光对象，这将导致对象不再有效
function CurveLaser:Release()
end

--------------------------------------------------------------------------------
--- 渲染

--- 渲染曲线激光，四个 uv_* 参数指定要渲染的纹理范围  
---@param texture string
---@param blendmode lstg.BlendMode
---@param color lstg.Color
---@param uv_left number
---@param uv_top number
---@param uv_width number
---@param uv_height number
---@param scale number
function CurveLaser:Render(texture, blendmode, color, uv_left, uv_top, uv_width, uv_height, scale)
end

--- [LuaSTG Ex Plus 新增]  
--- [LuaSTG Sub v0.1.0 移除]  
--- [LuaSTG Sub v0.17.4 重新添加]  
--- 渲染曲线激光的碰撞体（一般仅用于调试）  
---@param color lstg.Color
function CurveLaser:RenderCollider(color)
end

--------------------------------------------------------------------------------
--- 一些仍然存在于引擎中，但是难以使用的或者非常不推荐的 API

--[==[

---对曲线激光进行等长采样，返回采样数据
---@param length number
---@return table[] @带有x、y、rot成员的table
function CurveLaser:SampleByLength(length)
	return { { x = 0, y = 0, rot = 0 }, }
end

---对曲线激光进行等长时间采样（单位为帧），返回采样数据
---@param time number @整数
---@return table[] @带有x、y、rot成员的table
function CurveLaser:SampleByTime(time)
	return { { x = 0, y = 0, rot = 0 }, }
end

---更新指定节点
---@param unit lstg.GameObject
---@param index number @节点索引，整数，不同于lua，这个索引是c/c++风格索引，从0开始
---@param length number @未使用的参数，但是需要填写大于1的整数
---@param width number @未使用的参数
---@param active boolean @该节点是否是激活的
function CurveLaser:UpdateNode(unit, index, length, width, active)
end

---根据一个对象表更新曲线激光的位置
---@param units lstg.GameObject[]|table[] @对象组，可以是带有x、y成员的table，也可以是lstg.GameObject，最大数量不超过512
---@param length number @第一个参数里面含有的对象数
---@param width number @宽度，不是半宽
---@param index number @索引初始位置
---@param revert boolean @反向索引
function CurveLaser:UpdatePositionByList(units, length, width, index, revert)
end

--]==]

local M = {}

--- 创建曲线激光对象  
--- 不要在意函数名称，这里有着很长的故事  
---@return lstg.CurveLaser
function M.BentLaserData()
end

return M
