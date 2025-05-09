$LastLocation = Get-Location
Set-Location -Path $PSScriptRoot
& ($PSScriptRoot + "/../lua/lua54.exe") "main.lua"
Set-Location $LastLocation
