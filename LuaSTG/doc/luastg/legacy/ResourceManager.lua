---@diagnostic disable: duplicate-set-field

--------------------------------------------------------------------------------
--- LuaSTG Sub 资源管理
--- 璀境石
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

local M = {}

--------------------------------------------------------------------------------
--- 迁移指南

-- 关于 lstg.SetTextureSamplerState：
-- 在之前的 LuaSTG 版本中，也存在 lstg.SetTextureSamplerState 方法，
-- 但是并未公开，且参数不一样，行为不一样，影响范围也不一样。
-- 旧 API 设置的采样器是全局的状态，修改后必须手动还原。
-- 旧 API 的函数签名为 lstg.SetTextureSamplerState(state:string, mode:string)
-- 参数组合为 state = "address", mode = "wrap" | "clamp"
-- 或 state = "filter", mode = "point" | "linear"
-- 旧 API 的行为已被移除，强烈建议替换为新行为。
-- 新的行为更加容易理解，将采样器与纹理绑定，这也是其他游戏引擎的设计方式

--------------------------------------------------------------------------------
--- 资源池管理

---@alias lstg.ResourcePoolType '"none"' | '"global"' | '"stage"'

--- [LuaSTG Sub 新增]
--- 加载资源时是否输出日志，引擎默认开启日志输出
---@param b boolean
function M.SetResLoadInfo(b)
end

--- 切换活跃资源池  
--- LuaSTG 提供了 2 个资源池用于存放游戏资源  
--- 活跃的资源池有最高优先级，查找资源优先从活跃的资源池内查找  
--- 否则才会去其他的资源池查找  
---@param respool lstg.ResourcePoolType
function M.SetResourceStatus(respool)
end

--- 返回当前激活的资源池  
--- 如果返回值为 "none" 代表**引擎还没有加载完成**  
---@return lstg.ResourcePoolType
function M.GetResourceStatus()
end

--- 清空指定资源池  
--- 如果提供额外的restype, resname参数，则清空指定资源池的指定资源  
--- 资源类型为整数，从 1 到 9 分别代表：  
--- 1. tex 纹理 textrue  
--- 2. img 图片精灵 image (sprite)  
--- 3. ani 动画 animation (sprite sequence)  
--- 4. bgm 音乐 music  
--- 5. snd 音效 sound effect  
--- 6. ps  HGE粒子 HGE particle system  
--- 7. fnt 纹理字体 HGE sprite font  
--- 8. ttf TTF字体 truetype font  
--- 9. fx  屏幕后处理 Direct3D 11 PixelShader  
---@param respool lstg.ResourcePoolType
---@overload fun(respool:lstg.ResourcePoolType, restype:number, resname:string)
function M.RemoveResource(respool)
end

--- 判断某个资源处于哪个资源池内，当资源不存在的时候返回值为空
---@param restype number @参考 lstg.RemoveResource
---@param resname string
---@return lstg.ResourcePoolType|nil
function M.CheckRes(restype, resname)
end

--- 枚举资源  
--- 返回 "global" 资源池和 "stage" 资源池的资源名 table  
---@param restype number @参考 lstg.RemoveResource
---@return string[], string[]
function M.EnumRes(restype)
end

--------------------------------------------------------------------------------
--- 纹理和渲染目标

--- 从文件加载纹理  
--- mipmap 默认为 false  
---@param texname string
---@param filepath string
---@param mipmap boolean
---@overload fun(texname:string, filepath:string)
function M.LoadTexture(texname, filepath, mipmap)
end

--- [LuaSTG Sub 更改]  
--- 创建渲染目标  
--- 不提供宽高参数时，创建的渲染目标大小与窗口大小一致，且会自动调整大小  
--- 提供宽高参数时，不会自动调整大小  
---@param rtname string
---@param width number
---@param height number
---@overload fun(rtname:string)
---@overload fun(rtname:string, width:number, height:number)
function M.CreateRenderTarget(rtname, width, height, depth_buffer)
end

--- 未来可能移除
---@param name string
function M.IsRenderTarget(name)
end

--- 返回纹理或渲染目标的宽度和高度
---@param texname string
---@return number, number
function M.GetTextureSize(texname)
end

--- [LuaSTG Ex Plus 新增]  
--- 保存纹理或渲染目标为 JPEG 图片  
---@param texname string
---@param saveto string
function M.SaveTexture(texname, saveto)
end

--- 截取程序画面，保存的文件格式为 JPEG
---@param saveto string
function M.Snapshot(saveto)
end

---@alias lstg.KnownSamplerState '""' | '"point+wrap"' | '"point+clamp"' | '"linear+wrap"' | '"linear+clamp"'

--- [LuaSTG Sub v0.18.5 新增]  
--- 设置纹理使用的采样器  
--- 当参数 known_sampler_state 为 "" 时，纹理使用 LuaSTG 默认的采样器  
--- LuaSTG 默认情况下使用 "linear+clamp"  
--- [LuaSTG Sub v0.18.5 Add]  
--- Set the sampler used by the texture  
--- When the parameter known_sampler_state is "",  
--- the texture uses the LuaSTG default sampler  
--- LuaSTG uses "linear+clamp" by default  
---@param texture_name string
---@param known_sampler_state lstg.KnownSamplerState
function M.SetTextureSamplerState(texture_name, known_sampler_state)
end

--------------------------------------------------------------------------------
--- 图片精灵

--- 从纹理加载图片精灵  
--- 坐标单位和宽度单位为像素  
--- 类似纹理坐标，左上角为原点，向右为x轴正方向，向下为y轴正方向  
--- 额外的 a、b、rect 参数用于 lstg.GameObject  
--- 更改游戏对象的 img 属性时会同时将图片精灵资源上的 a、b、rect 应用到该游戏对象上
---@param imgname string
---@param texname string
---@param x number
---@param y number
---@param width number
---@param height number
---@param a number
---@param b number
---@param rect boolean
---@overload fun(resname:string, texres:string, x:number, y:number, width:number, height:number)
function M.LoadImage(imgname, texname, x, y, width, height, a, b, rect)
end

--- 更改图片精灵渲染时使用的混合模式和顶点颜色  
--- 如果提供 4 个颜色参数，则对图片精灵的 4 个顶点分别设置颜色  
---@param imgname string
---@param blendmode lstg.BlendMode
---@param color lstg.Color @顶点颜色
---@overload fun(imgname:string, blendmode:lstg.BlendMode, c1:lstg.Color, c2:lstg.Color, c3:lstg.Color, c4:lstg.Color)
function M.SetImageState(imgname, blendmode, color)
end

--- 更改图片精灵渲染中心，单位为像素，相对于加载时的 x 和 y  
--- 参考 lstg.LoadImage  
---@param imgname string
---@param x number
---@param y number
function M.SetImageCenter(imgname, x, y)
end

--- [LuaSTG Sub v0.15.1 更改]  
--- 设置图片精灵渲染缩放或者全局渲染缩放，默认为 1.0  
--- 类似某些游戏引擎的 Units per Pixel（每单位有多少像素）功能  
--- 设置图片精灵渲染缩放时：  
--- * 影响指定的图片精灵渲染时的缩放  
--- 
--- 设置全局渲染缩放时：  
--- * 影响图片精灵、图片精灵序列、HGE 粒子效果、HGE 纹理字体、矢量字体渲染时的缩放  
--- * 影响曲线激光宽度  
--- * 影响修改游戏对象的 img 时自动设置的碰撞盒大小  
--- 
--- [LuaSTG Sub v0.15.1 Change]  
--- Set sprite (image) rendering scaling or global rendering scaling, default to 1.0  
--- Similar to the Units per Pixel of some game engines  
--- When set the sprite rendering scaling:  
--- * Affects the rendering scaling of the specified sprite   
--- 
--- When set the global rendering scaling:  
--- * Affects the rendering scaling of sprites, sprite sequences (animation), HGE particle effects, HGE texture fonts, truetype font  
--- * Affect curve laser (bent laser) width  
--- * Affects the collision box size that is automatically set when modifying the game object's .img  
---@param imgname string
---@param scale number
---@overload fun(scale:number)
function M.SetImageScale(imgname, scale)
end

--- [LuaSTG Ex Plus 新增]  
--- [LuaSTG Sub v0.15.1 更改]  
--- 参考 `lstg.SetImageScale`  
--- [LuaSTG Ex Plus Add]  
--- [LuaSTG Sub v0.15.1 Change]  
--- See `lstg.SetImageScale`  
---@param imgname string
---@return number
---@overload fun():number
function M.GetImageScale(imgname)
end

--------------------------------------------------------------------------------
--- 图片序列  

--- 从纹理加载图片序列  
---@see lstg.LoadImage
---@param sprite_sequence_name string
---@param texture_name string
---@param x number
---@param y number
---@param width number 单张图片宽度
---@param height number 单张图片高度
---@param col number 列数
---@param row number 行数
---@param interval number 每张图片持续显示的帧数
---@param a number?
---@param b number?
---@param rect boolean?
function M.LoadAnimation(sprite_sequence_name, texture_name, x, y, width, height, col, row, interval, a, b, rect)
end

--- [LuaSTG Sub v0.21.101 新增]  
--- 从已有的图片精灵加载图片序列  
--- 提醒：该方法会复制图片精灵的副本来使用，而不是引用原图片精灵  
--- 注意：如果要设置渲染中心，必须在加载前将每张图片的渲染中心设置好  
--- 警告：通过该方法加载的图片序列不应该使用 `lstg.SetAnimationCenter` 方法修改渲染中心  
---@param sprite_sequence_name string
---@param sprite_names string[] 图片列表
---@param interval integer 每张图片持续显示的帧数
---@param a number?
---@param b number?
---@param rect boolean?
function M.LoadAnimation(sprite_sequence_name, sprite_names, interval, a, b, rect)
end

--- 更改动画渲染时使用的混合模式和顶点颜色  
--- 参考 lstg.SetImageState  
---@param aniname string
---@param blendmode lstg.BlendMode
---@param color lstg.Color @顶点颜色
---@overload fun(aniname:string, blendmode:lstg.BlendMode, c1:lstg.Color, c2:lstg.Color, c3:lstg.Color, c4:lstg.Color)
function M.SetAnimationState(aniname, blendmode, color)
end

--- 更改图片序列渲染中心  
--- 参考 lstg.SetImageCenter  
---@param aniname string
---@param x number
---@param y number
function M.SetAnimationCenter(aniname, x, y)
end

--- [LuaSTG Sub v0.15.1 新增]  
--- 设置图片精灵序列（动画）渲染缩放，默认为 1.0  
--- 类似某些游戏引擎的 Units per Pixel（每单位有多少像素）功能  
--- [LuaSTG Sub v0.15.1 Add]  
--- Set the rendering scaling of the specified sprite sequences (animation)  
--- Similar to the Units per Pixel of some game engines  
---@param aniname string
---@param scale number
function M.SetAnimationScale(aniname, scale)
end

--- [LuaSTG Sub v0.15.1 新增]  
--- 参考 `lstg.SetAnimationScale`  
--- [LuaSTG Sub v0.15.1 Add]  
--- See `lstg.SetAnimationScale`  
---@return number
function M.GetAnimationScale(aniname)
end

--------------------------------------------------------------------------------
--- 粒子特效

--- 从文件加载 HGE 粒子特效
--- 额外的 a、b、rect 参数只影响绑定的游戏对象，且不应该使用
---@param psname string
---@param filepath string
---@param imgname string @粒子使用的图片精灵
---@overload fun(psname:string, filepath:string, imgname:string, a:number, b:number, rect:number)
function M.LoadPS(psname, filepath, imgname)
end

--------------------------------------------------------------------------------
--- 纹理字体

--- 从文件加载 HGE 纹理字体  
--- 该字体纹理图片存放位置必须和字体定义文件在同一个文件夹下  
---@param fntname string
---@param filepath string
---@param mipmap boolean @若不提供该参数，则默认为 true
---@overload fun(fntname:string, filepath:string)
function M.LoadFont(fntname, filepath, mipmap)
end

--- 更改 HGE 纹理字体渲染时使用的混合模式和顶点颜色
---@param fntname string
---@param blendmode lstg.BlendMode
---@param color lstg.Color
function M.SetFontState(fntname, blendmode, color)
end

--------------------------------------------------------------------------------
--- 矢量字体

--- 从文件加载矢量字体，宽度和高度单位为像素
---@param ttfname string
---@param filepath string
---@param width number
---@param height number
function M.LoadTTF(ttfname, filepath, width, height)
end

--- 仅用于矢量字体  
--- 预加载指定字符串内的所有字符的字形  
---@param ttfname string
---@param cachestring string
function M.CacheTTFString(ttfname, cachestring)
end

--------------------------------------------------------------------------------
--- 音效

--- 从文件加载音效，支持 wav 和 ogg 格式的双声道、44100Hz 采样率、16 位深音频文件  
---@param sndname string
---@param filepath string
function M.LoadSound(sndname, filepath)
end

--------------------------------------------------------------------------------
--- 背景音乐

--- 从文件加载音效，支持 wav 和 ogg 格式的双声道、44100Hz 采样率、16 位深音频文件  
--- 支持循环节功能  
---@param bgmname string
---@param filepath string
---@param loopend number @循环区间的结束位置(秒)
---@param looplength number @循环区间的长度(秒)
function M.LoadMusic(bgmname, filepath, loopend, looplength)
end

--------------------------------------------------------------------------------
--- 屏幕后处理

--- 从文件编译并加载 Direct3D 11 Pixel Shader HSLS 文件
---@param fxname string
---@param filepath string
function M.LoadFX(fxname, filepath)
end

--------------------------------------------------------------------------------
--- 模型
--- Model

--- [LuaSTG Ex Plus 新增]  
--- [LuaSTG Sub v0.1.0 移除]  
--- [LuaSTG Sub v0.15.0 重新添加（有更改）]  
--- 从文件编译并加载 glTF 模型文件  
--- [LuaSTG Ex Plus Add]  
--- [LuaSTG Sub v0.1.0 Remove]  
--- [LuaSTG Sub v0.15.0 Re-add (with changes)]  
--- Load glTF model from file  
---@param modname string
---@param gltfpath string
function M.LoadModel(modname, gltfpath)
end

--------------------------------------------------------------------------------
--- 实验性 API

---@class lstg.ResourceManager
local ResourceManager = {}

M.ResourceManager = ResourceManager

---@param name lstg.ResourcePoolType
---@return lstg.ResourceCollection
function ResourceManager.getResourceCollection(name)
    ---@diagnostic disable-next-line: missing-return
end

---@param name lstg.ResourcePoolType
function ResourceManager.setCurrentResourceCollection(name)
end

---@return lstg.ResourcePoolType
function ResourceManager.getCurrentResourceCollection()
    ---@diagnostic disable-next-line: missing-return
end

return M
