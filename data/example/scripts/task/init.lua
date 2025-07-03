--------------------------------------------------------------------------------
--- lua coroutine wrapper
--- by 璀境石
--------------------------------------------------------------------------------

---@class task
local task = {}

---@param times integer?
function task.wait(times)
    for _ = 1, times or 1 do
        coroutine.yield()
    end
end

return task
