param(
    [ValidateSet("vs2022", "vs2026-v143", "vs2026")]
    [string]$Toolchain = "vs2026-v143"
)

$ProjectRoot = [System.IO.Path]::GetFullPath([System.IO.Path]::Join($PSScriptRoot, ".."))
$ReleasesRoot = [System.IO.Path]::Join($ProjectRoot, "build", "releases")
$BinaryRootX86 = [System.IO.Path]::Join($ProjectRoot, "build", "x86", "bin")
$BinaryRootAMD64 = [System.IO.Path]::Join($ProjectRoot, "build", "amd64", "bin")
$ExampleRoot = [System.IO.Path]::Join($ProjectRoot, "data", "example")

Write-Output "Project Root       : $ProjectRoot"
Write-Output "Releases Root      : $ReleasesRoot"
Write-Output "Binary Root (x86)  : $BinaryRootX86"
Write-Output "Binary Root (amd64): $BinaryRootAMD64"
Write-Output "Example Root       : $ExampleRoot"

# build

Set-Location $ProjectRoot

$Presets = switch ($Toolchain) {
    "vs2022"     { @{ AMD64 = "windows-amd64-release"; X86 = "windows-x86-release" } }
    "vs2026-v143"{ @{ AMD64 = "windows-vs2026-v143-amd64-release"; X86 = "windows-vs2026-v143-x86-release" } }
    "vs2026"     { @{ AMD64 = "windows-vs2026-amd64-release"; X86 = "windows-vs2026-x86-release" } }
}

cmake --workflow --preset $Presets.AMD64
cmake --workflow --preset $Presets.X86

# read version info

$ConfigFilePath = [System.IO.Path]::Join($ProjectRoot, "LuaSTG", "LuaSTG", "LConfig.h")
$ConfigFile = [System.IO.File]::ReadAllText($ConfigFilePath, [System.Text.Encoding]::UTF8)
$VersionMajor = "0"
$VersionMinor = "0"
$VersionPatch = "0"
foreach ($Line in $ConfigFile.Split("`n")) {
    if ($Line.Contains("LUASTG_VERSION_MAJOR")) {
        $VersionMajor = $Line.Replace("#define", "").Replace("LUASTG_VERSION_MAJOR", "").Trim()
    }
    if ($Line.Contains("LUASTG_VERSION_MINOR")) {
        $VersionMinor = $Line.Replace("#define", "").Replace("LUASTG_VERSION_MINOR", "").Trim()
    }
    if ($Line.Contains("LUASTG_VERSION_PATCH")) {
        $VersionPatch = $Line.Replace("#define", "").Replace("LUASTG_VERSION_PATCH", "").Trim()
    }
}
$VersionFull = "$VersionMajor.$VersionMinor.$VersionPatch"
$ReleaseRoot = [System.IO.Path]::Join($ReleasesRoot, "LuaSTG-Sub-v$VersionFull")

Write-Output "Version            : $VersionFull"
Write-Output "Release Root       : $ReleaseRoot"

if (-not [System.IO.Directory]::Exists($ReleaseRoot)) {
    [System.IO.Directory]::CreateDirectory($ReleaseRoot)
}

# copy engine binary file

$BinaryFilesAMD64 = @(
    @{
        Source = [System.IO.Path]::Join($BinaryRootAMD64, "LuaSTGSub.exe")
        Destination = [System.IO.Path]::Join($ReleaseRoot, "LuaSTGSub.exe")
    },
    @{
        Source = [System.IO.Path]::Join($BinaryRootAMD64, "d3dcompiler_47.dll")
        Destination = [System.IO.Path]::Join($ReleaseRoot, "d3dcompiler_47.dll")
    },
    @{
        Source = [System.IO.Path]::Join($BinaryRootAMD64, "xaudio2_9redist.dll")
        Destination = [System.IO.Path]::Join($ReleaseRoot, "xaudio2_9redist.dll")
    }
)

foreach ($BinaryFile in $BinaryFilesAMD64) {
    if (Test-Path -Path $BinaryFile.Destination) {
        Remove-Item -Path $BinaryFile.Destination
    }
    Copy-Item -Path $BinaryFile.Source -Destination $BinaryFile.Destination
}

$Release32Root = [System.IO.Path]::Join($ReleaseRoot, "windows-32bit")
$BinaryFilesX86 = @(
    @{
        Source = [System.IO.Path]::Join($BinaryRootX86, "LuaSTGSub.exe")
        Destination = [System.IO.Path]::Join($Release32Root, "LuaSTGSub.exe")
    },
    @{
        Source = [System.IO.Path]::Join($BinaryRootX86, "d3dcompiler_47.dll")
        Destination = [System.IO.Path]::Join($Release32Root, "d3dcompiler_47.dll")
    },
    @{
        Source = [System.IO.Path]::Join($BinaryRootX86, "xaudio2_9redist.dll")
        Destination = [System.IO.Path]::Join($Release32Root, "xaudio2_9redist.dll")
    }
)

if (-not [System.IO.Directory]::Exists($Release32Root)) {
    [System.IO.Directory]::CreateDirectory($Release32Root)
}

foreach ($BinaryFile in $BinaryFilesX86) {
    if (Test-Path -Path $BinaryFile.Destination) {
        Remove-Item -Path $BinaryFile.Destination
    }
    Copy-Item -Path $BinaryFile.Source -Destination $BinaryFile.Destination
}

# copy example file

$ExampleAssets = [System.IO.Path]::Join($ExampleRoot, "assets")
$ReleaseAssets = [System.IO.Path]::Join($ReleaseRoot, "assets")
$ExampleScripts = [System.IO.Path]::Join($ExampleRoot, "scripts")
$ReleaseScripts = [System.IO.Path]::Join($ReleaseRoot, "scripts")
$DocRoot = [System.IO.Path]::Join($ProjectRoot, "LuaSTG", "doc")
$ReleaseDocRoot = [System.IO.Path]::Join($ReleaseRoot, "doc")
$LicenseRoot = [System.IO.Path]::Join($ProjectRoot, "data", "license")
$ReleaseLicenseRoot = [System.IO.Path]::Join($ReleaseRoot, "license")

if (Test-Path -Path $ReleaseAssets) {
    Remove-Item -Path $ReleaseAssets -Recurse
}
if (Test-Path -Path $ReleaseScripts) {
    Remove-Item -Path $ReleaseScripts -Recurse
}
if (Test-Path -Path $ReleaseDocRoot) {
    Remove-Item -Path $ReleaseDocRoot -Recurse
}
if (Test-Path -Path $ReleaseLicenseRoot) {
    Remove-Item -Path $ReleaseLicenseRoot -Recurse
}
Copy-Item -Path $ExampleAssets -Destination $ReleaseAssets -Recurse
Copy-Item -Path $ExampleScripts -Destination $ReleaseScripts -Recurse
Copy-Item -Path $DocRoot -Destination $ReleaseDocRoot -Recurse -Exclude ".git"
Copy-Item -Path $LicenseRoot -Destination $ReleaseLicenseRoot -Recurse
[System.IO.File]::Copy([System.IO.Path]::Join($ExampleRoot, "config.json"), [System.IO.Path]::Join($ReleaseRoot, "config.json"), $true)
[System.IO.File]::Copy([System.IO.Path]::Join($ExampleRoot, "使用说明.txt"), [System.IO.Path]::Join($ReleaseRoot, "使用说明.txt"), $true)

# archive

$ArchivePath = [System.IO.Path]::Join($ReleasesRoot, "LuaSTG-Sub-v$VersionFull.zip")
Compress-Archive -Path $ReleaseRoot -DestinationPath $ArchivePath -CompressionLevel Optimal -Force
