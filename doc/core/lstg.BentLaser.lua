---=====================================
---luastg 曲线激光对象包装
---作者:Xiliusha
---邮箱:Xiliusha@outlook.com
---=====================================

----------------------------------------
---lstg.BentLaser

---@class lstg.BentLaser @曲线激光
local bentlaser = {}

---更新曲线激光的节点
---@param unit lstg.GameObject
---@param length number @曲线激光的长度（最大节点数量），整数，不能小于1，且曲线激光最大节点数量为512
---@param width number @宽度，不是半宽
function bentlaser:Update(unit, length, width)
end

--[[
---更新指定节点
---@param unit lstg.GameObject
---@param index number @节点索引，整数，不同于lua，这个索引是c/c++风格索引，从0开始
---@param length number @未使用的参数，但是需要填写大于1的整数
---@param width number @未使用的参数
---@param active boolean @该节点是否是激活的
function bentlaser:UpdateNode(unit, index, length, width, active)
end
--]]

---释放曲线激光对象，这将导致对象不再有效
function bentlaser:Release()
end

---渲染曲线激光，曲线激光纹理来自纹理资源的一部分，uv_left，uv_top指定矩形左上角，uv_width，uv_height指定矩形大小
---@param texture string @纹理资源名
---@param blendmode string @混合模式名
---@param color lstg.Color
---@param uv_left number
---@param uv_top number
---@param uv_width number
---@param uv_height number
---@param scale number @曲线激光宽度的缩放默认为1.0
function bentlaser:Render(texture, blendmode, color, uv_left, uv_top, uv_width, uv_height, scale)
end

---将曲线激光与给定的数据进行碰撞检测
---@param x number
---@param y number
---@param rot number
---@param a number
---@param b number
---@param rect boolean
---@return boolean
function bentlaser:CollisionCheck(x, y, rot, a, b, rect)
end

--[[
---渲染曲线激光的碰撞体
---@param color lstg.Color
function bentlaser:RenderCollider(color)
end

---将曲线激光与给定的数据进行碰撞检测，曲线激光的宽度将被替换成参数width
---@param x number
---@param y number
---@param rot number
---@param a number
---@param b number
---@param rect boolean
---@param width number @宽度，不是半宽
---@return boolean
function bentlaser:CollisionCheckWidth(x, y, rot, a, b, rect, width)
	return false
end
--]]

---检查曲线激光是否离开场景边界
---@return boolean
function bentlaser:BoundCheck()
end

--[==[
---对曲线激光进行等长采样，返回采样数据
---@param length number
---@return table[] @带有x、y、rot成员的table
function bentlaser:SampleByLength(length)
	return { { x = 0, y = 0, rot = 0 }, }
end

---对曲线激光进行等长时间采样（单位为帧），返回采样数据
---@param time number @整数
---@return table[] @带有x、y、rot成员的table
function bentlaser:SampleByTime(time)
	return { { x = 0, y = 0, rot = 0 }, }
end

---根据一个对象表更新曲线激光的位置
---@param units lstg.GameObject[]|table[] @对象组，可以是带有x、y成员的table，也可以是lstg.GameObject，最大数量不超过512
---@param length number @第一个参数里面含有的对象数
---@param width number @宽度，不是半宽
---@param index number @索引初始位置
---@param revert boolean @反向索引
function bentlaser:UpdatePositionByList(units, length, width, index, revert)
end

---更改曲线激光的宽度
---@param width number @宽度，不是半宽
function bentlaser:SetAllWidth(width)
end
--]==]

---构造曲线激光对象
---@return lstg.BentLaser
function lstg.BentLaserData()
end
