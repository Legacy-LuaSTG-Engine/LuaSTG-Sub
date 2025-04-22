local t = coroutine.create(function()
    --lstg.DoFile("src/scripts/bad.lua.txt")
    dofile("src/scripts/bad.lua.txt")
end)
local b, e = coroutine.resume(t)
if not b then
    error(e .. debug.traceback(t))
end
