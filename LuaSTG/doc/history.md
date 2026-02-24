# LuaSTG Sub 更新历史记录

* LuaSTG Sub v0.21.127
    * 图形/渲染
        * 修复：修复 `lstg.Sprite:setCenter` 与 `lstg.SetImageCenter` 行为不一致，现在 `center_x`、`center_y` 统一从 Sprite 矩形左上角开始  
        * 新增：补充缺失的 `lstg.Texture2D:setDefaultSampler` 方法，行为和 `lstg.SetTextureSamplerState` 一致  
* LuaSTG Sub v0.21.126（GPU API 分支）  
    * 图形/渲染
        * 移除：Windows 自带的图像编解码库 Windows Imaging Component 支持的容器格式和编码格式
            会受到  Windows 版本、编码器/解码器拓展安装数量的影响，并导致开发者和用户的体验出现不支持或出现难以排查的问题，
            为了避免各种外部因素干扰，从该版本开始不再通过 Windows Imaging Component 解码图像  
        * 修改：加载图像和纹理时，使用开源第三方图像解码库 libjpeg-turbo、libpng、libwebp、libqoi、stb_image、DDSTextureLoader11 (DirectXTK)  
        * 修改：现在加载纹理时不再自动缩放超过尺寸上限（8192x8192）的纹理，而是直接报错引起开发者注意  
        * 修复：修复 `lstg.Sprite` 创建时 `x`、`y`、`width`、`height` 参数未正确处理导致引用的纹理区域出错的问题  
            > 备注：在之前的版本中，如果 `x`、`y` 参数为 0，将不会遇到该问题  
* LuaSTG Sub v0.21.125（GPU API 分支）  
    * 引擎基础
        * 修复：修复引擎启动后的 10 帧内统计得到的平均帧率异常的问题
* LuaSTG Sub v0.21.124（GPU API 分支）  
    * 图形/渲染
        * 修改：重写交换链内部实现
        * 修改：通过修改渲染缓冲区获取逻辑，修复了从 v0.20 画布模式更新开始，窗口分辨率和画布分辨率一致时仍然需要复制整个渲染画面的问题
        * 修复：修复进入独占全屏前自动挑选可用的显示模式时，由于判断条件写错导致无法选到最优的显示模式的问题
* LuaSTG Sub v0.21.123（GPU API 分支）  
    * 图形/渲染
        * 修复：进入独占全屏时窗口消息递归调用导致引擎崩溃
        * 修改：调整交换链计时系统超时，缓解启动后初次拖动窗口时1秒的卡顿
* LuaSTG Sub v0.21.122（GPU API 分支）  
    * 图形/渲染
        * 修复：PostEffect 时顶点缓冲区可能溢出并导致报错
        * 修复：渲染曲线激光的碰撞判定时访问空指针导致引擎无报错崩溃
* LuaSTG Sub v0.21.121（GPU API 分支）  
    * 图形/渲染
        * 修改：重写图形渲染模块的内部实现，实现渲染 API 抽象层（暂未导出 Lua API）
        * 修改：重写主渲染器的内部实现，减少一次顶点/索引缓冲区复制
* LuaSTG Sub v0.21.120
    * 图形/渲染
        * 新增：交换链新增异步三重缓冲模式
        * 修改：在开启垂直同步的情况下，如果条件允许，将通过异步三重缓冲降低延迟
            > 异步三重缓冲的延迟介于传统垂直同步和禁用垂直同步之间，但不会造成画面撕裂。  
            > 部分用户对画面撕裂敏感，因此选择开启垂直同步，但要忍受更高的延迟，本次修改将进一步改善这些用户的体验。  
            > 引擎自动启用该功能的条件：
            > 1. 系统版本至少为 Windows 10 1809 / Windows 10 LTSB 2019
            > 2. 支持 `DXGI_FEATURE_PRESENT_ALLOW_TEARING`
            > 3. 支持 `DirectFlip`
            > 4. 支持 `IndependentFlip`
            > 5. 未通过引擎配置文件或命令行选项禁用现代交换链模型（`allow_modern_swap_chain`）
* LuaSTG Sub v0.21.119
    * 引擎基础
        * 新增：允许通过命令行参数修改引擎初始化配置，请参考文档[命令行参数.md](./specification/命令行参数.md)
        * 移除：`--allow-soft-adapter` 命令行参数已废弃并移除，请改为 `--graphics_system.allow_software_device=true`
        * 新增：现在 LuaSTG Sub 支持将日志输出到标准输出（`stdout`），以便其他软件（比如编辑器）实时展示日志。
* LuaSTG Sub v0.21.118
    * 引擎基础
        * 新增：LuaSTG Sub 已实验性支持 Windows on ARM64
* LuaSTG Sub v0.21.117（资源对象化分支）  
    * 图形/渲染  
        * 新增：`lstg.FontCollection` 字体库，用于加载并管理一个或多个字体  
        * 新增：`lstg.TextLayout` 文本布局，用于文本布局、排版、光栅化  
        * 新增：`lstg.TextRenderer` 文本布局渲染器，API 设计上类似 `lstg.SpriteRenderer`  
        * 新增：字体和文本渲染功能配套的枚举值类：
            - `lstg.FontWeight`  
            - `lstg.FontStyle`  
            - `lstg.FontWidth`  
            - `lstg.TextAlignment`  
            - `lstg.ParagraphAlignment`  
* LuaSTG Sub v0.21.116  
    * 引擎基础
        * 修复：修复引擎不读取配置文件中的 `logging.consolepreserve` 配置项的问题  
    * 图形/渲染  
        * 修复：修复字体文件光栅化返回错误的结果值，导致某些 API 行为首次调用和后续调用不一致的问题  
        * 移除：现在 `lstg.LoadTTF` 不再支持从注册表中匹配系统字体文件  
            > 该行为最初由 BakaChu 在 LuaSTG Plus 中引入，本意是方便直接使用系统字体，不幸的是实现方式存在很大的问题。
            > 系统注册表中储存的字体名称可能是中文，可能是英文，可能是其他语言，微软没有提供官方文档说明具体的逻辑。
            > 这很大概率会导致：在开发者电脑上能正常读取的字体，在其他用户的电脑上就无法找到。
            > 为了避免出现项目上线后大规模爆炸的惨剧，请**自行携带字体文件**。
* LuaSTG Sub v0.21.115  
    * 图形/渲染  
        * 修复：修复 `lstg.MeshRenderer` `draw` 后会导致后续内容无法渲染出来的问题  
* LuaSTG Sub v0.21.114（资源对象化分支）  
    注意：从该版本开始一些依赖外部命令行工具的插件（比如 GIF 录制插件）会失效，我们将在后续更新中提供替代方法
    * 平台拓展
        * 移除：默认情况下移除 `lstg.Execute`  
        * 新增：`lstg.ShellIntegration` 类，用于安全地打开文件、文件夹、网站（链接）  
            * 静态方法 `lstg.ShellIntegration.openFile` 拉起文件打开方式列表用于打开指定文件  
            * 静态方法 `lstg.ShellIntegration.openDirectory` 在文件资源管理器（explorer）中打开一个文件夹  
            * 静态方法 `lstg.ShellIntegration.openUrl` 用系统默认浏览器打开一个网站（链接）  
    * 图形/渲染  
        * 新增：新增 `lstg.RenderTarget` 对象，可以通过 `getTexture` 方法获取其 `lstg.Texture2D`  
        * 新增：新增 `lstg.DepthStencilBuffer` 对象  
        * 修改：`lstg.PushRenderTarget` 支持将 `lstg.RenderTarget` 对象作为第一个参数，以及可选的 `lstg.DepthStencilBuffer` 作为第二个参数  
        * 修改：`lstg.PostEffect` 额外支持 `lstg.Vector2`、`lstg.Vector3`、`lstg.Vector4`
    * Lua 虚拟机  
        * 移除：默认情况下移除 `os.execute`、`io.popen`  
* LuaSTG Sub v0.21.113
    * 引擎基础
        * 更新：更新 `luajit` 库，修复安全漏洞
        * 更新：更新 `dear-imgui` 库到 v1.92.3
    * 游戏对象
        * 修复：游戏对象通过 `.render` 启用渲染对象功能后，引擎崩溃  
        * 修复：曲线激光的 `BoundCheck` 方法的内部判断条件写反（由 v0.21.106 引入）  
    * 音频
        * 修改：限制输入的音量范围，不能超过 1.0，且不小于 0.0，涉及的 API：
            * API：`lstg.PlaySound`
            * API：`lstg.SetSEVolume`
            * API：`lstg.PlayMusic`
            * API：`lstg.SetBGMVolume`
* LuaSTG Sub v0.21.112
    * 游戏对象
        一些内部实现的修改
* LuaSTG Sub v0.21.111
    * 音频
        * 优化：优化 flac 解码器的代码逻辑，改善寻道缓存提升解码性能
        * 修复：修改 flac 解码器初始化流程，现在可以正确处理 ogg 容器的 flac 音频文件
        * 修复：修复 v0.21.106（该版本重写了音频系统）导致的音频播放到结尾时造成崩溃的问题
* LuaSTG Sub v0.21.110
    * 引擎基础
        * 修复：修复 v0.21.108 导致的 `dear-imgui` 库 lua 绑定缺少枚举值的绑定
    * 图形/渲染
        * 修改：为 `lstg.SetPerspective` 的 fov 参数添加取值范围校验，小于 0 或大于等于 pi（180度）将导致报错
* LuaSTG Sub v0.21.109
    * 窗口
        * 修复：修复引擎启动后窗口失去焦点、窗口不在顶层等问题
* LuaSTG Sub v0.21.108
    * 引擎基础
        * 更新：更新 `lua-cjson` 库到 v2.1.0.14，修复高危漏洞
        * 更新：更新 `dear-imgui` 库到 v1.92.1，支持最新的字体渲染逻辑
* LuaSTG Sub v0.21.107
    * 游戏对象
        * 修复：修复由上一个版本的修改导致的崩溃
    * 音频
        * 修复：修复同一帧多次播放、暂停、恢复、停止同一个音效的处理逻辑
* LuaSTG Sub v0.21.106
    * 游戏对象
        * 修改：优化游戏对象内存布局，减少内存占用
        * 修改：优化游戏对象碰撞体包围盒的更新计算
        * 修改：优化游戏对象碰撞检测的逻辑
        * 修改：优化 `lstg.ObjList` 遍历游戏对象的速度，接入 luajit ffi 加速
        * 修改：使 `lstg.Dist` 和 `lstg.Angle` 更加通用，只要传入的对象有 `x`、`y` 属性即可参与计算，不再局限于游戏对象
        * 修改：优化 `lstg.Dist` 和 `lstg.Angle`，接入 luajit ffi 加速
        * 修改：使 `lstg.SetV` 和 `lstg.GetV` 更加通用，只要传入的对象有 `vx`、`vy`、`rot` 属性即可参与计算，不再局限于游戏对象
        * 修改：优化 `lstg.SetV` 和 `lstg.GetV`，接入 luajit ffi 加速
        * 移除：由于设计上存在各种问题且难以维护，因此移除多 world 功能，涉及的 API：
            * 游戏对象属性：`world`
            * API：`lstg.GetWorldFlag`
            * API：`lstg.SetWorldFlag`
            * API：`lstg.IsInWorld`
            * API：`lstg.IsSameWorld`
            * API：`lstg.ActiveWorlds`
            * API：`lstg.GetCurrentObject`
    * 数学
        * 修改：随机数发生器 `lstg.Rand` 的 `Int` 方法加入参数校验，限制随机区间不得大于 2147483647，避免除以 0 导致引擎崩溃
* LuaSTG Sub v0.21.105（资源对象化分支）
    * 文件系统
        * 修复：修复带密码的压缩包加载后无法读取其中的文件的问题
* LuaSTG Sub v0.21.104（资源对象化分支）
    * 文件系统
        * 新增：新增实验性对象 `lstg.FileSystemWatcher` 用于监听文件夹下的文件修改
* LuaSTG Sub v0.21.103（资源对象化分支）
    * 窗口
        * 新增：新增 `lstg.Clipboard` 对象，用于读取或写入剪贴板（有可能会被杀毒软件误报）
    * 图形/渲染
        * 修复：（可能包含破坏性更改）修复 `lstg.RenderClear` 的颜色没有正确处理预乘 alpha 的问题
* LuaSTG Sub v0.21.102（资源对象化分支）
    * 图形/渲染
        * 新增：新增 `lstg.Sprite` 对象
        * 新增：新增 `lstg.SpriteRenderer` 对象
* LuaSTG Sub v0.21.101（资源对象化分支）
    * 图形/渲染
        * 新增：新增 `lstg.LoadAnimation` 方法重载，允许使用已有的图片精灵 
    * 数学
        * 新增：新增 `lstg.Vector2` 对象
        * 新增：新增 `lstg.Vector3` 对象
        * 新增：新增 `lstg.Vector4` 对象
    * 文件系统
        * 修改：通过搜索路径定位文件时，按搜索路径添加顺序的倒序遍历搜索路径
        * 修改：添加搜索路径时，如果重复，不再移除后重新添加，避免影响顺序
* LuaSTG Sub v0.21.100（资源对象化分支）
    * 图形/渲染
        * 新增：新增 `lstg.Texture2D` 对象
        * 修改：在 0.18.4-beta 中引入的实验性对象 `lstg.Mesh` 现已转为正式版本
        * 废弃：废弃 0.18.4-beta 中的 `lstg.Mesh` 实验性对象构造方法 `lstg.MeshData`
        * 新增：新增 `lstg.MeshRenderer` 对象
        * 废弃：废弃 0.18.4-beta 中引入的实验性渲染 API `lstg.RenderMesh`

* LuaSTG Sub v0.21.24
    * 图形/渲染
        * 修复：修复加载 truetype 字体时的内存访问越界错误
* LuaSTG Sub v0.21.23
    * 图形/渲染
        * 修复：在引擎初始化时创建的 RenderTarget 资源可能不会跟随窗口大小
* LuaSTG Sub v0.21.22
    * Lua 拓展库
        * 修复：适配 Steam API 1.61，修复编译失败问题
    * 窗口
        * 修复：修复可自动隐藏的窗口标题栏最大化/最小化按钮有时会失效的问题
        * 新增：`lstg.Display` 新增 `getFriendlyName` 方法
    * 图形/渲染
        * 新增：`lstg.SwapChain` 新增 `setWindowed` 方法，用于完全重新创建交换链
        * 修改：`lstg.SwapChain:setSize` 不再重新创建交换链，而是修改渲染缓冲区尺寸
* LuaSTG Sub v0.21.21（文本输入和输入法支持分支）
    * 游戏对象
        * 修复：游戏对象管理器判断碰撞组索引范围使用了错误的条件
    * 窗口
        * 新增：文本输入拓展 `lstg.Window.TextInputExtension` 用于接收窗口文本输入事件
        * 新增：输入法支持拓展 `lstg.Window.InputMethodExtension` 用于动态开启或关闭输入法支持
        * 修改：窗口居中时避开任务栏
* LuaSTG Sub v0.21.17/v0.21.18/v0.21.19/v0.21.20（增强的引擎配置文件分支）
    * 窗口
        * 修复：修复每次居中窗口时都会出现 24 字节的内存泄漏的问题
    * 调试
        * 修改：内存使用量调试窗口补充当前进程缺页中断数量、工作集大小等指标
        * 修改：改进内存使用量调试窗口显示内容的方式，划分为多个可折叠的栏目
    * 引擎基础
        * 修改：配置文件 `config.json` 支持更丰富的配置，用于替代 `launch` 脚本，请阅读 `doc/specification/引擎配置文件.md` 查看详细内容
* LuaSTG Sub v0.21.16
    * 网络
        * 新增：实验性 HTTP 客户端库 `http` 和类 `http.Request`
    * Lua 拓展库
        * 修改：`luasocket` 网络库改为可选功能，默认情况下禁用
* LuaSTG Sub v0.21.15
    * 窗口
        * 修复：`lstg.Window:getClientAreaSize` 无法读取当前窗口尺寸的问题
* LuaSTG Sub v0.21.14
    * 音频
        * 修复：音效在调用 `lstg.PlaySound` 之后，`lstg.GetSoundState` 读取音效状态只能得到 `"playing"`
    * 窗口
        * 修复：引擎启动时窗口会极其短暂地出现在显示器左上角
* LuaSTG Sub v0.21.13 (第二代游戏循环更新顺序分支)
    * 游戏对象：
        * 修复：超级暂停计时器的更新时机，从 `lstg.ObjFrame` 移动到 `lstg.AfterFrame` 中（仅影响“第二代游戏循环更新顺序”）
    * 音频：
        * 修改：`lstg.PlaySound` 的音量参数改为可选，不填写音量时，默认为 1.0
* LuaSTG Sub v0.21.12 (第二代游戏循环更新顺序分支)
    * 游戏对象：
        * 新增：为 `lstg.BoundCheck` 添加新重载函数，可以接收一个版本号 2，用于切换到“第二代游戏循环更新顺序”，作为“第二代游戏循环更新顺序”的一部分，该重载版本默认采用先完成检测，后执行 `lstg.Del` 的逻辑
        * 废弃：在“第二代游戏循环更新顺序”中，`lstg.UpdateXY` 已经失去作用，目前保留用于兼容旧版本 `THlib`，未来该方法将被彻底移除
* LuaSTG Sub v0.21.11 (第二代游戏循环更新顺序分支)
    * 游戏对象：
        * 新增：为 `lstg.ObjFrame` 添加新重载函数，可以接收一个版本号 2，用于切换到“第二代游戏循环更新顺序”，作为“第二代游戏循环更新顺序”的一部分，该重载版本默认采用先执行 `frame` 回调函数，后更新运动的逻辑，并将 `navi` 开启后自动更新 `rot` 的逻辑从旧版本的 `lstg.UpdateXY` 移动到了粒子系统更新前
        * 新增：为 `lstg.AfterFrame` 添加新的重载函数，可以接收一个版本号 2，用于切换到“第二代游戏循环更新顺序”，作为“第二代游戏循环更新顺序”的一部分，该重载版本将 `dx`、`dy` 的更新从旧版本的 `lstg.UpdateXY` 移动到了计时器更新前
* LuaSTG Sub v0.21.10 (第二代游戏循环更新顺序分支)
    * 游戏对象：
        * 新增：为 `lstg.CollisionCheck` 添加新的重载函数，可以接收一个二维数组参数，用于一次性完成所有碰撞组对检测，作为“第二代游戏循环更新顺序”的一部分，该重载版本默认采用先完成检测，后执行 `colli` 回调函数的逻辑
        * 修复：曲线激光的 `BoundCheck` 方法出现意外的精度损失的问题
    * 文件系统：
        * 更正：读取文件时，如果文件路径存在大小写不一致的部分，错误消息显示“存在大小写一致的部分”，已更正为“存在大小写不一致的部分”
* LuaSTG Sub v0.21.9
    * 窗口
        * 修复：窗口创建时，显示位置偏下的问题
* LuaSTG Sub v0.21.8 (窗口与显示分支)
    * 窗口
        * 新增：平台拓展方法 `lstg.Window.Windows11Extension:setTitleBarAutoHidePreference`，用于在 Windows 11 上开启自动隐藏标题栏功能
        * 修改：自动隐藏标题栏功能默认关闭（由沉浸式窗口标题栏分支引入），如有需要请主动开启
* LuaSTG Sub v0.21.7 (窗口与显示分支)
    * 图形/渲染
        * 新增：交换链对象 `lstg.SwapChain`，用于控制渲染分辨率、缩放方式、垂直同步等
* LuaSTG Sub v0.21.6 (窗口与显示分支)
    * 窗口
        * 修复：启用自动隐藏窗口标题栏功能时，拖动窗口后标题栏可能无法自动隐藏
        * 新增：显示对象 `lstg.Display`，用于枚举显示、获取显示信息
        * 新增：窗口对象 `lstg.Window`，用于控制窗口行为
        * 移除：实验性API `lstg.ListMonitor`，相关功能已转移到 `lstg.Display`
        * 移除：实验性API `lstg.SetWindowCornerPreference`，相关功能已转移到 `lstg.Window`
* LuaSTG Sub v0.21.5 (沉浸式窗口标题栏分支)
    * 窗口
        * 移除：自动隐藏窗口标题栏功能不再支持 Windows 10
        * 修复：启用自动隐藏窗口标题栏功能时，若窗口尺寸过小，会导致程序崩溃
* LuaSTG Sub v0.21.4 (沉浸式窗口标题栏分支)
    * 窗口
        * 修复：自动隐藏窗口标题栏功能在桌面合成引擎模式下出现渲染错误
    * Lua 虚拟机
        * 修改：同步 Lua 5.4.7 的 `utf8` 库逻辑
        * 修复：`utf8.char` 方法无法转换得到正确的字符串
* LuaSTG Sub v0.21.3 (沉浸式窗口标题栏分支)
    * 窗口
        * 新增：在 Windows 10 1809+、Windows 11 中自动隐藏窗口标题栏
* LuaSTG Sub v0.21.2
    * 图形/渲染
        * 修复：gltf 2.0 模型加载时未通过引擎文件系统加载，导致从自定义搜索路径、压缩包加载失败
* LuaSTG Sub v0.21.1
    * 图形/渲染
        * 修改：带有半透明纹理的 gltf 2.0 模型在渲染时使用纱窗（screen door）效果模拟半透明
        * 修复：渲染具有顶点色的 gltf 2.0 模型时，顶点色错误
* LuaSTG Sub v0.21.0
    * 引擎基础
        * 修改：默认编译为 x86 64位版本，32位版本仍然可以编译，但将在未来停止支持
    * 图形/渲染
        * 修复：部分 gltf 2.0 模型渲染时顶点、纹理等坐标完全错误的问题
        * 修复：alpha+bal 渲染模式失效的问题

* LuaSTG Sub v0.20.16
    * 音频
        * 修复：修复音频文件成功加载但仍然报告加载失败的问题
* LuaSTG Sub v0.20.15
    * 引擎基础
        * 更新：更新 `cjson` 库，修复已知高危漏洞 
    * 窗口
        * 修复：修复在多个显示器的设备上，窗口会出现在意料之外的位置的问题
* LuaSTG Sub v0.20.14
    * 引擎基础
        * 修复：修复 `lstg.DoFile` 在协同程序上执行时可能出现会意料之外的问题，例如出错时报错信息格式不正确、执行成功时返回值不正确等
        * 修改：现在 `lstg.DoFile` 的行为与 `dofile` 一致，且不再有“严禁在协同程序中调用”的限制
    * Lua 虚拟机
        * 修复：修复标准库 `io` 库和 `os` 库部分函数可能存在的内存泄漏
* LuaSTG Sub v0.20.13
    * 引擎基础
        * 新增：`config.json` 新增配置项 `persistent_log_file_max_count` 用于限制持久日志文件最大数量，避免日志文件越来越多
    * 窗口
        * 修改：不再默认追踪抢占引擎窗口焦点的窗口，该功能涉及个人隐私，应该交给用户手动开启
        * 新增：`config.json` 新增配置项 `debug_track_window_focus` 用于开启追踪抢占引擎窗口焦点的窗口的调试功能，可以抓出导致独占全屏失效、按键输入没反应的流氓应用
    * 游戏对象
        * 移除：移除多个游戏对象上粒子系统相关 API 的无用警告信息，避免日志文件被写入大量无用内容，涉及的 API：
            * `lstg.ParticleStop`
            * `lstg.ParticleFire`
            * `lstg.ParticleGetn`
            * `lstg.ParticleGetEmission`
            * `lstg.ParticleSetEmission`
* LuaSTG Sub v0.20.12
    * 引擎基础
        * 修复：修复引擎关闭时可能会访问到空指针导致引擎崩溃，且没有报错弹窗或日志记录
        * 修复：修复通过 `error()`、`error(nil)` 或类似方式抛出内容为 `nil` 的错误时导致引擎崩溃，且没有报错弹窗或日志记录
* LuaSTG Sub v0.20.11
    * 引擎基础
        * 新增：新增配置项用于禁止程序多开，用于防止出现文件读写冲突导致存档等文件无法写入或者因为同时写入而损坏，具体使用方法请阅读 `doc/specification/引擎配置文件.md`
    * 游戏对象
        * 新增：曲线激光对象新增 `CollisionCheckWithWidth` 方法，用于增强 `CollisionCheckWidth` 的功能
        * 废弃：曲线激光对象 `CollisionCheckWidth` 方法进入废弃状态，未来的版本更新中将会移除
    * 文档
        * 修正：修正 API 文档中曲线激光对象 `CollisionCheckWidth` 方法的参数顺序
* LuaSTG Sub v0.20.10
    * 资源管理
        * 新增：初步添加实验性 API 集，资源管理器 API，资源集、资源对象化，具体请看 doc/experiment 文件夹
    * 图形/渲染
        * 修改：恢复 `lstg.PostEffect` LuaSTG Plus 版的函数签名，LuaSTG Sub 版的（脑残）函数签名已废弃
    * Lua 拓展库
        * 新增：为 `random` 库中的随机数发生器添加 `clone` 、 `serialize` 、 `deserialize` 方法，用于复制、持久化
        * 移除：移除 `random` 库中的 `sfc` 和 `jsf` 家族随机数发生器
* LuaSTG Sub v0.20.9
    * 引擎基础
        * 更新：`dear-imgui` 更新到 1.89.6
    * 图形/渲染
        * 修复：修复了 `lstg.LoadTTF` 加载矢量字体时，如果加载失败不会返回 `false` 的问题
        * 修改：改进了对现代交换链模型、桌面合成引擎功能是否可用的判断方式，现在会通过 `D3DKMT` 系列 API 获取更多设备功能信息并进行判断，以进一步提升设备兼容性
* LuaSTG Sub v0.20.8
    * 引擎基础
        * 新增：如果系统支持（Windows 10 1803+），将启用新的稳定帧率控制器，替代原有的限速帧率控制器（仅限制帧率，不进行补偿），新的帧率控制器的目标是补偿每帧的时间误差，尽可能让画面显示更稳定，减少画面抖动
    * 图形/渲染
        * 修复：有时候切换到别的窗口再回来，会出现掉帧、延迟升高等问题，现在理论上修复了（微软你就说这个是不是没修完的 bug 吧，老版本 Windows 10 就有的 bug，虽然说是在某个补丁中修复了，但是看起来 bug 还是能通过特殊方式触发）（微软：我修复了大部分帧延迟的问题，但是我保留了一部分，我觉得保留一部分特性才知道你用的是 Windows 10+）
* LuaSTG Sub v0.20.7
    * 资源管理
        * 修改：使用性能、质量更高的 `xxhash` 库来计算资源名称 hash 值
    * 游戏对象
        * 移除：移除 LuaSTG Ex Plus 为游戏对象添加的 `pause` 和 `rmove` 属性，这两个属性使用量及其稀少，但却容易误用并导致难以察觉的问题
    * Lua 拓展库
        * 修复：修复 `lfs` 文件系统库中有一个未捕获的异常的问题
* LuaSTG Sub v0.20.6
    * 引擎基础
        * 更新：更新 `luajit` 库，现在 `luajit` 支持捕获 `__gc` 元方法的错误
    * 图形/渲染
        * 修改：现在程序启动时如果检测不到可用的显卡硬件，将会弹窗报错并显示明确的原因
        * 修改：现在允许使用软件显示适配器（在没有显卡硬件的情况下）运行 LuaSTG Sub，需要指定 `--allow-soft-adapter` 命令行参数，需要注意的是软件显示适配器性能不足以流畅运行程序，仅用于开发、测试、临时或应急情况下使用
        * 修改：当窗口和画布的宽度或高度相等，且窗口尺寸大于或等于画布尺寸时，使用速度更快的直接复制而不是常规渲染来显示画面
    * Lua 拓展库
        * 新增：新增 `luasocket` 网络库，需要注意的是仅包含 `socket.core` 模块，lua 脚本部分请从 `luasocket` 官方仓库获取
* LuaSTG Sub v0.20.5
    * 平台
        * 修改：现在要求 Windows 7 操作系统必须安装 `KB2533623` 补丁（或者后继替代该补丁的其他补丁）
    * 图形/渲染
        * 修改：内置的基本渲染器所使用的着色器预编译为字节码，不再在运行时动态编译
        * 修改：着色器编译库 `d3dcompiler_47.dll` 已改为按需加载，如果开发者没有用到模型渲染功能或者后处理特效功能，那么将不会加载
    * 音频
        * 修复：切换音频设备后，背景音乐不会继续播放
    * Lua 虚拟机
        * 修复：由于（未知的）历史原因，之前仅提供 `?.lua` 搜索路径，未默认提供 `?/init.lua` 搜索路径，现在已补上缺失的 `?/init.lua` 搜索路径
    * 游戏对象
        * 修复：修复了 `dx`、`dy` 的值在游戏对象创建后第 1 帧可能计算不正确的未定义行为，触发的原因是第 0 帧生成时记录 `lastx`、`lasty` 在 `lstg.New` 函数调用返回前，如果随后修改了游戏对象的 `x`、`y` 坐标，将导致记录的 `lastx`、`lasty` 并非期望的值
        * 修复：修复了 `lstg.BoxCheck` 不包含边界的问题，并去除了意味不明的单参数重载
* LuaSTG Sub v0.20.4
    * 音频
        * 修改：现在音频系统支持从错误中恢复，或者在无音频设备的计算机上以空白模式继续运行
* LuaSTG Sub v0.20.3
    * 引擎基础
        * 修改：现在可以通过配置文件来指定 `log` 日志文件的储存位置，详情请参考 `doc/specification/引擎配置文件.md` 文档
    * 数学
        * 修改：随机数发生器 `lstg.Rand:Int` 和 `lstg.Rand:Float` 方法不再要求第二个参数大于或等于第一个参数
* LuaSTG Sub v0.20.2
    * 窗口
        * 修复：修复了按住 `Alt` 键的同时切换到别的窗口后会导致引擎一直以为 `Alt` 键处于按下状态
    * 图形/渲染
        * 修复：修复了 `lstg.Render`、`lstg.RenderRect`、`lstg.Render4V`、`lstg.RenderAnimation` 在找不到资源时不抛出错误的问题
* LuaSTG Sub v0.20.1
    * 引擎基础
        * 移除：不再使用 `fancylib` 库
