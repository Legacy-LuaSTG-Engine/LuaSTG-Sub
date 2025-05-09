--------------------------------------------------------------------------------
--- lua coroutine wrapper: task manager
--- by chuikingshek (璀境石)
--------------------------------------------------------------------------------

---@class task.Manager
local Manager = {}

--- Note: "thread" in lua are actually coroutines
---@param f function
---@return thread
function Manager:add(f)
    local t = coroutine.create(f)
    self.size = self.size + 1
    self[self.size] = t
    return t
end

--- resume all not "dead" coroutines
function Manager:resume()
    local n = self.size
    for i = 1, n do
        if coroutine.status(self[i]) ~= "dead" then
            assert(coroutine.resume(self[i]))
        end
    end
end

--- remove all "dead" coroutines
function Manager:gc()
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
function Manager:clear()
    local n = self.size
    for i = n, 1, -1 do
        self[i] = nil
    end
    self.size = 0
end

---@return task.Manager
function Manager.create()
    local instance = {
        size = 0,
        add = Manager.add,
        resume = Manager.resume,
        gc = Manager.gc,
        clear = Manager.clear,
    }
    return instance
end

return Manager
