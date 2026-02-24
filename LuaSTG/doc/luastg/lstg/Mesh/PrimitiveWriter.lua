---@diagnostic disable: unused-local, missing-return, duplicate-set-field

---@class lstg.Mesh.PrimitiveWriter
local PrimitiveWriter = {}

--------------------------------------------------------------------------------
--- 写入顶点数据

--- 开始一个新的顶点  
--- 后续调用 `position`、`uv`、`color` 方法都会修改同一个顶点  
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:vertex()
end

--- 直接写入一个新的顶点  
--- 后续调用 `position`、`uv`、`color` 方法仍然可以修改同一个顶点  
---@param x number
---@param y number
---@param z number
---@param u number
---@param v number
---@param r number
---@param g number
---@param b number
---@param a number?
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:vertex(x, y, z, u, v, r, g, b, a)
end

--- 直接写入一个新的顶点  
--- 后续调用 `position`、`uv`、`color` 方法仍然可以修改同一个顶点  
---@param x number
---@param y number
---@param u number
---@param v number
---@param r number
---@param g number
---@param b number
---@param a number?
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:vertex(x, y, u, v, r, g, b, a)
end

--- 直接写入一个新的顶点  
--- 后续调用 `position`、`uv`、`color` 方法仍然可以修改同一个顶点  
---@param x number
---@param y number
---@param z number
---@param u number
---@param v number
---@param color lstg.Color
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:vertex(x, y, z, u, v, color)
end

--- 直接写入一个新的顶点  
--- 后续调用 `position`、`uv`、`color` 方法仍然可以修改同一个顶点  
---@param x number
---@param y number
---@param u number
---@param v number
---@param color lstg.Color
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:vertex(x, y, u, v, color)
end

--- 设置当前顶点位置坐标
---@param x number
---@param y number
---@param z number?
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:position(x, y, z)
end

--- 设置当前顶点纹理坐标，纹理坐标为 [0.0, 1.0] 归一化浮点数  
---@param u number
---@param v number
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:uv(u, v)
end

--- 设置当前顶点颜色，所有通道为 [0.0, 1.0] 归一化浮点数  
---@param r number
---@param g number
---@param b number
---@param a number?
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:color(r, g, b, a)
end

--- 设置当前顶点颜色  
---@param color lstg.Color
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:color(color)
end

--------------------------------------------------------------------------------
--- 自定义装配

--- 自定义装配，开始组装新的图形  
--- 该方式可以处理顶点被大量的面共享的情况  
--- 局部顶点索引会被重置，从 0 开始  
--- 
--- 示例代码：  
--- 
--- ```lua
--- local mesh = ...
--- local white = lstg.Color(255, 255, 255, 255)
--- local writer = mesh:createPrimitiveWriter()
--- writer
---     :begin()
---         :vertex():position(0.0， 1.0):uv(0.0, 0.0):color(white)
---         :vertex():position(1.0， 1.0):uv(1.0, 0.0):color(white)
---         :vertex():position(1.0， 0.0):uv(1.0, 1.0):color(white)
---         :vertex():position(0.0， 0.0):uv(0.0, 1.0):color(white)
---         :triangle(0, 1, 3)
---         :triangle(1, 2, 3)
---     :begin()
---         :vertex():position(0.0， 0.0):uv(0.0, 0.0):color(white)
---         :vertex():position(1.0， 0.0):uv(1.0, 0.0):color(white)
---         :vertex():position(1.0，-1.0):uv(1.0, 1.0):color(white)
---         :vertex():position(0.0，-1.0):uv(0.0, 1.0):color(white)
---         :quad(0, 1, 2, 3)
--- ```
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:begin()
end

--- 根据局部顶点索引装配三角形  
---@see lstg.Mesh.PrimitiveWriter.begin
---@param i1 integer
---@param i2 integer
---@param i3 integer
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:triangle(i1, i2, i3)
end

--- 根据局部顶点索引装配四边形  
---@see lstg.Mesh.PrimitiveWriter.begin
---@param i1 integer
---@param i2 integer
---@param i3 integer
---@param i4 integer
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:quad(i1, i2, i3, i4)
end

--- 写入一个或多个局部顶点索引  
---@param i integer
---@param ... integer
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:index(i, ...)
end

--------------------------------------------------------------------------------
--- 三角形和矩形

--- 开始一个新的三角形  
--- 写入三个顶点后，图元写入器会自动将顶点和顶点索引（如果网格开启了顶点索引）数据写入网格对象  
--- 
--- 示例代码：  
--- 
--- ```lua
--- local mesh = ...
--- local white = lstg.Color(255, 255, 255, 255)
--- local writer = mesh:createPrimitiveWriter()
--- writer
---     :beginTriangle()
---         :vertex():position(0.0， 1.0):uv(0.0, 0.0):color(white)
---         :vertex():position(1.0， 1.0):uv(1.0, 0.0):color(white)
---         :vertex():position(0.0， 0.0):uv(0.0, 1.0):color(white)
---     :beginTriangle()
---         :vertex():position(0.0， 0.0):uv(0.0, 1.0):color(white)
---         :vertex():position(1.0， 1.0):uv(1.0, 0.0):color(white)
---         :vertex():position(1.0， 0.0):uv(1.0, 1.0):color(white)
--- ```
--- 
--- 示例代码生成的图形：  
--- 
--- ```
---            ^ Y
---            |
---            +-----+ (1.0， 1.0)
---            |     |
---            |     |
--- (0.0, 0.0) +-----+--> X
--- ```
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:beginTriangle()
end

--- 开始一个新的四边形  
--- 写入四个顶点后，图元写入器会自动将顶点和顶点索引（如果网格开启了顶点索引）数据写入网格对象  
--- 
--- 示例代码：  
--- 
--- ```lua
--- local mesh = ...
--- local white = lstg.Color(255, 255, 255, 255)
--- local writer = mesh:createPrimitiveWriter()
--- writer
---     :beginQuad()
---         :vertex():position(0.0， 1.0):uv(0.0, 0.0):color(white)
---         :vertex():position(1.0， 1.0):uv(1.0, 0.0):color(white)
---         :vertex():position(1.0， 0.0):uv(1.0, 1.0):color(white)
---         :vertex():position(0.0， 0.0):uv(0.0, 1.0):color(white)
--- ```
--- 
--- 示例代码生成的图形：  
--- 
--- ```
---            ^ Y
---            |
---            +-----+ (1.0， 1.0)
---            |     |
---            |     |
--- (0.0, 0.0) +-----+--> X
--- ```
--- 
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:beginQuad()
end

--------------------------------------------------------------------------------
--- 扇形（有待完善）

--- 仅限上下文：扇形  
--- 按球坐标系相对圆心/球心写入平面/空间中的点  
--- `phi` 和 `theta` 均为弧度制，如果需要按角度制写入，请通过 `math.rad` 转换  
--- 为了便于 2D 图形绘制，`theta` 参数可选，且默认情况下为 `math.pi / 2.0`   
--- 
--- LuaSTG 默认情况下坐标系为左手系：  
--- 
--- ```
---     ^ Y
---     |
---     |  ^ Z
---     | /
---     |/
---     *--------> X
--- ```
--- 
--- 在 X-Y 上观察 `phi` 角：  
--- 
--- ```
---                    ^ Y (phi = pi / 2)
---                    |   *
---                    |  / 
---                    | / \  phi
---                    |/   |
--- (phi = pi) --------*--------> X (phi = 0)
---                    |
---                    |
---                    |
---                    |
---             (phi = pi * 1.5)
--- ```
--- 
--- 正交于 Z 轴观察 `theta` 角：  
--- 
--- ```
---         ^ Z (theta = 0)
---         |   *
---         |-_/ theta
---         | / 
---         |/ 
--- --------*-------- X-Y 平面 (theta = pi / 2)
---         |
---         |
---         |
---         |
---    (theta = pi)
--- ```
--@param r number
--@param phi number
--@param theta number?
--@return lstg.Mesh.PrimitiveWriter
--function PrimitiveWriter:sphericalPosition(r, phi, theta)
--end

--- 仅限上下文：扇形  
--- 按极坐标系相对圆心写入纹理坐标  
--- `theta` 均弧度制，如果需要按角度制写入，请通过 `math.rad` 转换  
--- 
--- 纹理坐标系的 U 轴朝右 V 轴朝下：  
--- 
--- ```
---     *--------> U (theta = 0)
---     |\   |
---     | \_/ theta
---     |  \
---     |   *
---     v V (theta = pi / 2)
--- ```
--- 
--- 这与 2D 图形渲染坐标系有所差异，可以将 `theta` 值取负值后再传入  
--@param r number
--@param theta number
--function PrimitiveWriter:polarUv(r, theta)
--end

--- 开始一个新的扇形  
--- 第一个顶点视为为圆心/球心，后续的顶点视为圆弧上的点  
--- `spherical` 方法可以在第一个顶点的基础上计算接下来的顶点的相对坐标  
--- 
--- 示例代码：  
--- 
--- ```lua
--- local mesh = ...
--- local rad = math.rad
--- local white = lstg.Color(255, 255, 255, 255)
--- local writer = mesh:createPrimitiveWriter()
--- writer
---     :sector()
---         :vertex():position(0.0， 1.0):uv(0.5, 0.5):color(white)
---         :execute(function()
---             for a = 90, 0, -1 do
---                 writer:vertex()
---                     :sphericalPosition(1.0, rad(a))
---                     :polarUv(0.5, -rad(a))
---                     :color(white)
---             end
---         end)
--- ```
--- 
--- 示例代码生成的图形（ASCII 绘图就长这吊样，将就一下）：  
--- 
--- ```
--- ^ Y
--- |    |-_.
--- |    |   \
--- |    +----
--- |    (0.5, 0.5)
--- |
--- +-------------> X
--- ```
--@return lstg.Mesh.PrimitiveWriter
--function PrimitiveWriter:sector()
--end

--------------------------------------------------------------------------------
--- 辅助方法

--- 可用于嵌套执行复杂逻辑  
--- 示例代码：  
--- 
--- ```lua
--- local writer = ...
--- writer
---     :beginTriangle()
---         :vertex():position(0.0， 1.0):uv(0.0, 0.0):color(white)
---         :vertex():position(1.0， 1.0):uv(1.0, 0.0):color(white)
---         :vertex():position(0.0， 0.0):uv(0.0, 1.0):color(white)
---     :execute(function()
---         for x = 2, 10 do
---             writer:beginTriangle()
---                 :vertex():position(0.0 + x， 1.0):uv(0.0, 0.0):color(white)
---                 :vertex():position(1.0 + x， 1.0):uv(1.0, 0.0):color(white)
---                 :vertex():position(0.0 + x， 0.0):uv(0.0, 1.0):color(white)
---         end
---     end)
---     :beginTriangle()
---         :vertex():position(0.0， -1.0):uv(0.0, 0.0):color(white)
---         :vertex():position(1.0， -1.0):uv(1.0, 0.0):color(white)
---         :vertex():position(0.0， -0.0):uv(0.0, 1.0):color(white)
--- ```
---@param callback fun(writer:lstg.Mesh.PrimitiveWriter)
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:execute(callback)
end

--------------------------------------------------------------------------------
--- 其他方法

---@see lstg.Mesh.commit
---@return lstg.Mesh.PrimitiveWriter
function PrimitiveWriter:commit()
end
