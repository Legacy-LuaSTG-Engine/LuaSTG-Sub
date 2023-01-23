# 这个脚本用于一次性推送给 3 个远程仓库
# 这并非是给所有人用的

# 如果要更新子模块，请使用
# git submodule update --init --recursive

# 配置工作目录

Set-Location -Path ($PSScriptRoot + "/..")
$CurrentLocation = Get-Location
Write-Output ("当前工作目录是：" + $CurrentLocation)

# 推送

[string] $Git = "git"
[string] $SSH = "ssh"

Write-Output "---------- push ----------"

Write-Output "git push"
& $Git push github
Write-Output "git push gitlab"
& $Git push gitlab
Write-Output "git push gitee"
& $SSH -T Gitee
& $Git push gitee

Write-Output "---------- exit ----------"
