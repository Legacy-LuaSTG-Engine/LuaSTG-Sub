---@diagnostic disable: missing-return, unused-local

---@class lstg.Clipboard
local Clipboard = {}

---@return boolean
function Clipboard.hasText()
end

---@return string?
function Clipboard.getText()
end

---@param text string
---@return boolean
function Clipboard.setText(text)
end

return Clipboard
