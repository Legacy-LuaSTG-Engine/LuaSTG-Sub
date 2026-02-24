---@diagnostic disable: missing-return

--------------------------------------------------------------------------------
--- 接口
--- Interface

---@class lstg.Unknown
local M = {}

---@generic T
---@param interface_name `T`
---@return T? result
---@return string? error_message
function M:queryInterface(interface_name)
end
