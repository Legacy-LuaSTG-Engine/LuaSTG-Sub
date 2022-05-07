--------------------------------------------------------------------------------
--- LuaSTG Sub 杂项功能
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
--- 平台调用

--- 警告：该功能慎用，可能会被杀毒软件误杀
--- 注意：该 API 未来可能被移除，并作为平台拓展提供
--- wait 和 show 如果不填写，默认为 true
---@param path string
---@param arg string
---@param workingdir string
---@param wait boolean
---@param show boolean
---@return boolean
---@overload fun(path:string):boolean
---@overload fun(path:string, arg :string):boolean
---@overload fun(path:string, arg :string, workingdir:string):boolean
---@overload fun(path:string, arg :string, workingdir:string, wait:boolean):boolean
function lstg.Execute(path, arg, workingdir, wait, show)
end
