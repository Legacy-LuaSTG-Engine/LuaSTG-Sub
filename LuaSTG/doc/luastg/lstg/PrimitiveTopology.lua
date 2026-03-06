---@enum lstg.PrimitiveTopology
local PrimitiveTopology = {
    --- 三角形列表，每三个顶点组成一个三角形  
    --- ```
    ---      [1]  [3]-----[5]  
    ---     /   \   \     /  
    ---    /     \   \   /  
    ---   /       \   \ /  
    --- [0]-------[2] [4]  
    --- ```
    triangle_list = 4,

    --- 三角形条带，一系列顶点组成带状  
    --- ```
    ---      [1]------[3]  
    ---     /   \     /  
    ---    /     \   /  
    ---   /       \ /  
    --- [0]-------[2]  
    --- ```
    triangle_strip = 5,
}

return PrimitiveTopology
