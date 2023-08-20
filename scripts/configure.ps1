# 配置工作目录

Set-Location -Path ($PSScriptRoot + "/..")
$CurrentLocation = Get-Location
Write-Output ("当前工作目录是：" + $CurrentLocation)

# 创建构建目录

function New-Build-Directory {
    param (
        [string] $Path
    )
    if (-not (Test-Path -Path $Path)) {
        New-Item -Path $Path -ItemType Directory
    }
}

New-Build-Directory -Path "build"
New-Build-Directory -Path "build/x86"
New-Build-Directory -Path "build/amd64"

# 配置 CMake

cmake -S "." -B "build/x86"   -G "Visual Studio 17 2022" -A Win32 ("-DCPM_SOURCE_CACHE=" + $CurrentLocation + "/build/packages")
cmake -S "." -B "build/amd64" -G "Visual Studio 17 2022" -A x64   ("-DCPM_SOURCE_CACHE=" + $CurrentLocation + "/build/packages")
