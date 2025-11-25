$MetadataFileUri = "https://github.com/dearimgui/dear_bindings/releases/download/DearBindings_v0.17_ImGui_v1.92.4/dcimgui.json"
$MetadataFile = "$PSScriptRoot/dcimgui.json"
$MetadataFileHash = "F7D596BC97B8500838FF0AB64F5A68F1A617CFE9F36DE0940626573D91BB8B41"
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

$OutputFile = "$PSScriptRoot/../src/Constants.cpp"
$IncludeObsolete = $false
$EnumValueDebug = $false

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
	lua::stack_balancer_t const sb(vm);
	lua::stack_t const ctx(vm);
	auto const m = ctx.push_module(module_name); // imgui
'

foreach ($Enum in $Metadata.enums) {
	$EnumName = $Enum.name.TrimEnd("_")
	Add-Cpp "	{`n"
	Add-Cpp ("		auto const e = ctx.create_map(" + $Enum.elements.Length + ");`n")
	Add-Cpp ("		ctx.set_map_value(m, `"$EnumName`"sv, e);`n")
	foreach ($Element in $Enum.elements) {
		$ElementName = $Element.name.Substring($Element.name.IndexOf("_") + 1)
		$ElementValue = $Element.name
		$DebugName = ""
		if ($EnumValueDebug -eq $true) {
			$DebugName = " " + $EnumName + "_" + $ElementName + ";"
		}
		if (Find-Obsolete $Element) {
			if ($IncludeObsolete) {
				Add-Cpp "	#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS`n"
				Add-Cpp ("		ctx.set_map_value(e, `"$ElementName`"sv, $ElementValue);$DebugName`n")
				Add-Cpp "	#endif IMGUI_DISABLE_OBSOLETE_FUNCTIONS`n"
			}
		}
		else {
			Add-Cpp ("		ctx.set_map_value(e, `"$ElementName`"sv, $ElementValue);$DebugName`n")
		}
	}
	Add-Cpp "	}`n"
}

Add-Cpp "}`n"
