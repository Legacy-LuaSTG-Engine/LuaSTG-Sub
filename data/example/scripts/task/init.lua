--------------------------------------------------------------------------------
--- lua coroutine wrapper
--- by 璀境石
--------------------------------------------------------------------------------

---@class task
local task = {}

---@param frames integer
function task.wait(frames)
    for _ = 1, frames do
        coroutine.yield()
    end
end

return task
