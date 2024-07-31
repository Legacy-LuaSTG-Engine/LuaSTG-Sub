---@diagnostic disable: duplicate-set-field, missing-return, unused-local

---@class imgui
local imgui = {}

---@class imgui.ImVec2
---@field x number
---@field y number

---@return imgui.ImVec2
function imgui.ImVec2()
end

---@param x number
---@param y number
---@return imgui.ImVec2
function imgui.ImVec2(x, y)
end

---@class imgui.ImVec4
---@field x number
---@field y number
---@field z number
---@field w number

---@return imgui.ImVec4
function imgui.ImVec4()
end

---@param x number
---@param y number
---@param z number
---@param w number
---@return imgui.ImVec4
function imgui.ImVec4(x, y, z, w)
end

return imgui
