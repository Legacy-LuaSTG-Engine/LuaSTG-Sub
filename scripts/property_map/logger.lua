---@class logger
local logger = {}

local function date()
    local os_date = os.date("*t")
    return string.format(
        "%04d-%02d-%02d %02d:%02d:%02d",
        os_date.year, os_date.month, os_date.day,
        os_date.hour, os_date.min, os_date.sec)
end

function logger.debug(fmt, ...)
    print(string.format("[%s] [D] " .. fmt, date(), ...))
end

function logger.info(fmt, ...)
    print(string.format("[%s] [I] " .. fmt, date(), ...))
end

function logger.warn(fmt, ...)
    print(string.format("[%s] [W] " .. fmt, date(), ...))
end

function logger.error(fmt, ...)
    print(string.format("[%s] [E] " .. fmt, date(), ...))
end

return logger
