# 配置工作目录

Set-Location -Path ($PSScriptRoot + "/..")
$CurrentLocation = Get-Location
Write-Output ("当前工作目录是：" + $CurrentLocation)

# 构建主要目标

#cmake --build "build/x86"   --target LuaSTG --config Release --clean-first
cmake --build "build/amd64" --target LuaSTG --config Release --clean-first

# 复原

Set-Location $PSScriptRoot
