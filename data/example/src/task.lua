--------------------------------------------------------------------------------
--- lua coroutine wrapper - task manager
--- by Kuanlan@outlook.com
--------------------------------------------------------------------------------

---@class task
local M = {}

---@class task.Manager
local I = {}

I.size = 0

--- Note: "thread" in lua are actually coroutines
---@param f function
---@return thread
function I:add(f)
    local co = coroutine.create(f)
    self.size = self.size + 1
    self[self.size] = co
    return co
end

--- resume all not "dead" coroutines
function I:resume_all()
    local n = self.size
    for i = 1, n do
        if coroutine.status(self[i]) ~= "dead" then
            assert(coroutine.resume(self[i]))
        end
    end
end

--- remove all "dead" coroutines
function I:remove_dead()
    local j = 1
    local n = self.size
    for i = 1, n do
        if coroutine.status(self[i]) ~= "dead" then
            if i > j then
                self[j] = self[i]
            end
            j = j + 1
        end
    end
    for k = j, n do
        self[k] = nil
    end
    self.size = j - 1
end

--- remove all coroutines
function I:clear()
    local n = self.size
    for i = 1, n do
        self[i] = nil
    end
    self.size = 0
end

---@return task.Manager
function M.Manager()
    ---@type task.Manager
    local C = { size = 0 }
    C.add = I.add
    C.resume_all = I.resume_all
    C.remove_dead = I.remove_dead
    C.clear = I.clear
    return C
end

---@param times number
function M.wait(times)
    for _ = 1, times do
        coroutine.yield()
    end
end

return M
