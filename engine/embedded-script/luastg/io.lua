local table = require("table")
local lstg = require("lstg")

local LOG_LEVEL_INFO = 2

function lstg.SystemLog(text)
    lstg.Log(LOG_LEVEL_INFO, text)
end

function lstg.Print(...)
    local args = {...}
    local argc = select('#', ...)
    for i = 1, argc do
        args[i] = tostring(args[i])
    end
    lstg.Log(LOG_LEVEL_INFO, table.concat(args, '\t'))
end

print = lstg.Print
