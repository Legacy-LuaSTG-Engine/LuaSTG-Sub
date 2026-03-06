local lstg = require("lstg")

local helpers = {}

---@param scope lstg.ResourcePoolType
---@param f fun()
function helpers.resourcesLoadingScope(scope, f)
    local last_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus(scope)
    f()
    lstg.SetResourceStatus(last_pool)
end

return helpers
