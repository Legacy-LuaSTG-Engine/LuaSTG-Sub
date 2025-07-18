$MetadataFileUri = "https://github.com/dearimgui/dear_bindings/releases/download/DearBindings_v0.16_ImGui_v1.92.1/dcimgui.json"
$MetadataFile = "$PSScriptRoot/dcimgui.json"
$MetadataFileHash = "5EE38CC7D175CCC5AEBB259B4BC970DE925E36FA7D3CBD03599E20A4195920CE"
$CurrentMetadataFileHash = ""

Write-Output "Expected metadata file hash: $MetadataFileHash"
if (Test-Path -Path $MetadataFile -PathType Leaf) {
	$CurrentMetadataFileHash = (Get-FileHash -Path $MetadataFile -Algorithm SHA256).Hash
}
Write-Output "Current metadata file hash: $CurrentMetadataFileHash"
if ($MetadataFileHash -ne $CurrentMetadataFileHash) {
	Invoke-WebRequest -Uri $MetadataFileUri -OutFile $MetadataFile
}

if (Test-Path -Path $MetadataFile -PathType Leaf) {
	$CurrentMetadataFileHash = (Get-FileHash -Path $MetadataFile -Algorithm SHA256).Hash
	if ($MetadataFileHash -eq $CurrentMetadataFileHash) {
		Write-Output "Metadata file downloaded"
	}
	else {
		Write-Output "Download metadata file failed: verify failed (hash: $CurrentMetadataFileHash)"
	}
}
else {
	Write-Output "Download metadata file failed: file not exists"
}

$MetadataText = Get-Content -Path $MetadataFile -Encoding utf8 -Raw
$Metadata = ConvertFrom-Json -InputObject $MetadataText

foreach ($Enum in $Metadata.enums) {
	Write-Output ("Name: " + $Enum.name)
	foreach ($Element in $Enum.elements) {
		Write-Output ("    Name: " + $Element.name + " = " + $Element.value)
	}
}

$OutputFile = "$PSScriptRoot/../modern/Constants.cpp"
$IncludeObsolete = $false

Set-Content -Path $OutputFile -Value "" -NoNewline -Encoding utf8

function Add-Cpp {
	param (
		[string] $Value
	)
	Add-Content -Path $OutputFile -Value $Value -NoNewline -Encoding utf8
}

function Find-Obsolete {
	param (
		$Element
	)
	if ($null -ne $Element.conditionals) {
		foreach ($Conditional in $Element.conditionals) {
			if ("ifndef" -eq $Conditional.condition -and "IMGUI_DISABLE_OBSOLETE_FUNCTIONS" -eq $Conditional.expression) {
				return $true
			}
		}
	}
	return $false
}

Add-Cpp '#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

void imgui::binding::registerConstants(lua_State* const vm) {
	lua::stack_t const ctx(vm);
	auto const m = ctx.push_module(module_name); // imgui
'

foreach ($Enum in $Metadata.enums) {
	Add-Cpp "	{`n"
	Add-Cpp ("		auto const e = ctx.create_map(" + $Enum.elements.Length + ");`n")
	Add-Cpp ("		ctx.set_map_value(m, `"" + $Enum.name.TrimEnd("_") + "`"sv, e);`n")
	foreach ($Element in $Enum.elements) {
		if (Find-Obsolete $Element) {
			if ($IncludeObsolete) {
				Add-Cpp "	#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS`n"
				Add-Cpp ("		ctx.set_map_value(e, `"" + $Element.name.Substring($Enum.name.Length) + "`"sv, " + $Element.name + ");`n")
				Add-Cpp "	#endif IMGUI_DISABLE_OBSOLETE_FUNCTIONS`n"
			}
		}
		else {
			Add-Cpp ("		ctx.set_map_value(e, `"" + $Element.name.Substring($Enum.name.Length) + "`"sv, " + $Element.name + ");`n")
		}
	}
	Add-Cpp "	}`n"
}

Add-Cpp '}
'
