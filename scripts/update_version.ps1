# 配置工作目录

Set-Location -Path ($PSScriptRoot + "/..")
[string] $CurrentLocation = Get-Location
[string] $EngineLocation  = $CurrentLocation + "/LuaSTG"
[string] $Lua             = $CurrentLocation + "/scripts/lua54.exe"
Write-Output ("当前工作目录是：" + $CurrentLocation)

# 运行

Set-Location -Path $EngineLocation
Write-Output ("进入目录是：" + $EngineLocation)

& $Lua "write-version.lua"
