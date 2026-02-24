---@diagnostic disable: missing-return

--------------------------------------------------------------------------------
--- 操作系统坐标系
--- OS coordinate system
--- Windows GDI: https://learn.microsoft.com/en-us/windows/win32/gdi/multiple-display-monitors
--- Windows WDDM CCD: https://learn.microsoft.com/en-us/windows-hardware/drivers/display/connecting-and-configuring-displays



--                            |
--                            |
--                            |
--            ....+-----------+----------------+....
--            :   |           .                |   :
--            :   |           .                |   :
--            :   |         [Display 2]        |   :
--            :   |           .                |   :
--            :   |           .                |   :
-- -----------+---+-----------+----------------+---+------> +X
--            |               |\_ (0, 0)           |
--            |  [Display 3]  |                    |
--            |               |     [Display 1]    |
--            +---------------+      (primary)     |
--            :               |                    |
--            :...............+--------------------+
--                            |
--            ^               |
--            |               |
--      virtual-screen        |
--                            v
--                            
--                           +Y



--    left-top
--        v
--        +-----------------[Display]---------------+-----> +X
--        |           OS Status Bar, etc...         |    ^
--        +-----------------------------------------+    |
--        |                                         |    |
--        |                                         |    |
--        |                                         |    |
--        |                                         |    |
--        |               [Work Area]               | display-height
--        |                                         |    |
--        |                                         |    |
--        |                                         |    |
--        |                                         |    |
--        +-----------------------------------------+    |
--        |             OS TaskBar, etc...          |    v
--        +-----------------------------------------+   ---
--        |
--        |<------------ display-width ------------>|
--        |
--        v
--        
--       +Y



--    left-top
--        v
--        +-----------------[Display]---------------+-----> +X
--        |                                         |    ^
--        | left-top                                |    |
--        |    v                                    |    |
--        |    +------[Window]------+     ---       |    |
--        |    |                    |      ^        |    |
--        |    |                    |      |        |    |
--        |    |                    | window-height | display-height
--        |    |                    |      |        |    |
--        |    |                    |      v        |    |
--        |    +--------------------+     ---       |    |
--        |                                         |    |
--        |    |<-- window-width -->|               |    |
--        |                                         |    v
--        +-----------------------------------------+   ---
--        |
--        |<------------ display-width ------------>|
--        |
--        v
--        
--       +Y



-- 操作系统提供了各式各样的窗口装饰，如标题栏、边框……
-- OS provides Window Decorations: TitleBar, Borders, etc...
--
-- (0, 0) +-----------------[Window]----------------+-----> +X
--        |              TitleBar, etc... [-][#][X] |    ^
--        |+---------------------------------------+|    |
--        ||                                       ||    |
--        ||                                       ||    |
--        ||                                       ||    |
--        ||                                       ||    |
--        ||             [Client Area]             || window-height
--        ||                                       ||    |
--        ||                                       ||    |
--        ||                                       ||    |
--        ||                                       ||    |
--        ||                                       ||    |
--        ||_______________________________________/|    v
--        +-----------------Borders-----------------+   ---
--        |
--        |<------------- window-width ------------>|
--        |
--        v
--        
--       +Y

--------------------------------------------------------------------------------
--- 类
--- Class

---@class lstg.Display.Size
local _ = {
    width = 0,
    height = 0,
}

---@class lstg.Display.Point
local _ = {
    x = 0,
    y = 0,
}

---@class lstg.Display.Rect
local _ = {
    left = 0,
    top = 0,
    right = 0,
    bottom = 0,
}

--- “显示”并非对应于物理显示器  
--- 桌面操作系统在应用层和硬件之间提供了一层抽象，使复杂的需求变为可能  
--- 在操作系统的设置中，可以设置多个显示，
--- 每个显示可以与物理显示器一一对应，
--- 也可以将一个显示复制显示到多个物理显示器，
--- 还可以完全不输出到物理显示器，而是通过网络传输画面到其他设备或者录制到储存器上  
--- 显示的逻辑位置关系称为“显示布局”，它不一定和物理显示器的实际摆放方式有关  
--- 这些操作由合成器完成，比如 Windows 桌面窗口管理器（Desktop Window Manager）、
--- Linux Wayland Compositor、Android SurfaceFlinger  
---   
--- Display does not correspond to the physical monitor  
--- Desktop operating systems provide a layer of abstraction between the
--- application layer and the hardware, enabling complex requirements  
--- In the OS settings, users can set multiple displays,
--- each display can correspond to the physical monitor one-by-one,
--- can copy and present one display to multiple physical monitors, 
--- can transfer the display to other devices through the network
--- or record to the storage without outputting to the physical monitor at all  
--- The logical position of displays is called a Display Layout and
--- is not necessarily related to how physical monitors are actually placed  
--- These operations are done by Compositor, such as
--- Windows Desktop Window Manager, Linux Wayland Compositor,
--- Android SurfaceFlinger
---@class lstg.Display
local M = {}

--------------------------------------------------------------------------------
--- 显示信息
--- Display info

---@return string
function M:getFriendlyName()
end

--- 获取显示尺寸（像素）  
--- Get display size (in pixels)  
---@return lstg.Display.Size size
function M:getSize()
end

--- 获取显示位置（像素），即左上角的坐标  
--- 只有一个物理显示器的设备，唯一的显示位置一般位于原点 (0, 0)  
--- 具有多个物理显示器的设备，每个显示的位置由显示布局决定  
--- Get display position (in pixels), i.e. the coordinates of left-top  
--- For devices with only one physical monitor,
--- the only one display position is typically origin (0, 0)  
--- For devices with multiple physical monitors, position of
--- each display is determined by the Display Layout
---@return lstg.Display.Point point
function M:getPosition()
end

--- 获取显示矩形（像素）  
--- Get display rect (in pixels)  
---@return lstg.Display.Rect rect
function M:getRect()
end

--- 获取显示工作区尺寸（像素）  
--- 工作区不包含操作系统提供的界面，如状态栏、任务栏等  
--- Get display work-area size (in pixels)  
--- Work-area does not contain shells provided by the OS,
--- such as the StatusBar, TaskBar, etc...
---@return lstg.Display.Size size
function M:getWorkAreaSize()
end

---@see lstg.Display.getPosition
---@see lstg.Display.getWorkAreaSize
---@return lstg.Display.Point point
function M:getWorkAreaPosition()
end

---@see lstg.Display.getRect
---@see lstg.Display.getWorkAreaSize
---@return lstg.Display.Rect rect
function M:getWorkAreaRect()
end

--- 判断当前显示是否为主显示  
--- 主显示的位置一般位于虚拟屏幕的原点，即 (0, 0)  
--- Determines whether the current display is the primary  
--- The position of the primary display is generally at
--- the origin of the Virtual Screen, i.e. (0, 0) 
---@return boolean primary
function M:isPrimary()
end

--- 获取显示缩放，初始值为 1.0（100%）  
--- 用户可能在操作系统设置中设置了更高的缩放值，且可能为每个显示设置了不同的缩放值    
--- 更高的缩放值可以使窗口、控件、文本等内容以更大的尺寸显示，以适配高像素密度显示器  
--- Get display scale, initial value is 1.0 (100%)  
--- Users may set a higher value in the OS Settings,
--- and may set a different value for each display  
--- Higher values allow windows, controls, text, and more
--- to be displayed at a larger size to accommodate high-pixel density monitors
---@return number scale
function M:getDisplayScale()
end

--------------------------------------------------------------------------------
--- 查询
--- Query

--- 获取所有显示  
--- 返回的对象**不应该缓存**  
--- 用户插拔物理显示器、修改显示布局、
--- 修改显示模式、切换应用程序全屏等，都会导致 `lstg.Display` 对象失效  
--- Get all displays  
--- The returned objects **should not be cached**  
--- users plugging and unplugging physical monitors,
--- modifying Display Layout,
--- modifying Display Modes,
--- switching applications fullscreen mode, etc...,
--- will invalidate `lstg.Display` object  
---@return lstg.Display[]
function M.getAll()
end

---@see lstg.Display.getAll
---@see lstg.Display.isPrimary
---@return lstg.Display monitor
function M.getPrimary()
end

--- 查找离指定窗口最近的显示  
--- 如果查找失败，默认返回主显示  
--- Find the display closest to the specified window  
--- If it fails, returns the primary display by default  
---   
---@see lstg.Display.getAll
---@see lstg.Display.getPrimary
---@param window lstg.Window
---@return lstg.Display monitor
function M.getNearestFromWindow(window)
end

return M
