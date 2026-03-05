---@meta lstg.Video

---@class lstg.Video
---视频对象，用于播放视频文件
---Video object for playing video files
local Video = {}

--------------------------------------------------------------------------------
--- 静态方法 / Static Methods
--------------------------------------------------------------------------------

---创建视频对象
---Create a video object from file
---
---# 示例 / Example
---```lua
---local video = lstg.Video.create('test_video.mp4')
---local video_with_options = lstg.Video.create('test_video.mp4', {
---    video_stream = 0,
---    width = 1280,
---    height = 720,
---    premultiplied_alpha = false,
---    looping = true,
---    loop_end = 10.0,
---    loop_duration = 5.0
---})
---```
---@param path string 视频文件路径 / Video file path
---@param options? lstg.VideoOpenOptions 视频打开选项 / Video open options
---@return lstg.Video video 视频对象 / Video object
function Video.create(path, options) end

--------------------------------------------------------------------------------
--- 视频信息 / Video Information
--------------------------------------------------------------------------------

---获取视频宽度（像素）
---Get video width in pixels
---@return number width 视频宽度 / Video width
function Video:getWidth() end

---获取视频高度（像素）
---Get video height in pixels
---@return number height 视频高度 / Video height
function Video:getHeight() end

---获取视频总时长（秒）
---Get video duration in seconds
---@return number duration 视频时长 / Video duration
function Video:getDuration() end

---获取当前播放时间（秒）
---Get current playback time in seconds
---@return number time 当前时间 / Current time
function Video:getCurrentTime() end

---获取视频帧率（FPS）
---Get video frame rate (frames per second)
---@return number fps 帧率 / Frame rate
function Video:getFPS() end

---获取是否循环播放
---Get whether the video is looping
---@return boolean looping 是否循环 / Is looping
function Video:isLooping() end

---获取视频纹理对象
---Get the texture object of the video
---
---# 说明 / Description
---返回的纹理对象可以用于创建精灵或直接渲染
---The returned texture can be used to create sprites or render directly
---
---# 示例 / Example
---```lua
---local video = lstg.Video.create('test_video.mp4')
---local texture = video:getTexture()
---local sprite = lstg.Sprite.create(texture, 0, 0, video:getWidth(), video:getHeight())
---```
---@return lstg.Texture2D texture 纹理对象 / Texture object
function Video:getTexture() end

--------------------------------------------------------------------------------
--- 播放控制 / Playback Control
--------------------------------------------------------------------------------

---跳转到指定时间
---Seek to specified time
---
---# 说明 / Description
---跳转到视频的指定时间点，会解码到该时间点的帧
---Seeks to the specified time in the video and decodes to that frame
---@param time number 目标时间（秒）/ Target time in seconds
---@return boolean success 是否成功 / Success status
function Video:seek(time) end

---更新视频到指定时间
---Update video to specified time
---
---# 说明 / Description
---将视频更新到指定的绝对时间，通常配合 StopWatch 使用
---Updates the video to the specified absolute time, typically used with StopWatch
---
---# 示例 / Example
---```lua
---local video = lstg.Video.create('test_video.mp4')
---local clock = lstg.StopWatch()
---
---function RenderFunc()
---    video:update(clock:GetElapsed())
---    -- 渲染视频 / Render video
---end
---```
---@param time number 目标时间（秒）/ Target time in seconds
---@return boolean success 是否成功 / Success status
function Video:update(time) end

---设置是否循环播放
---Set whether to loop playback
---
---# 示例 / Example
---```lua
---video:setLooping(true)  -- 启用循环 / Enable looping
---video:setLooping(false) -- 禁用循环 / Disable looping
---```
---@param loop boolean 是否循环 / Whether to loop
function Video:setLooping(loop) end

---设置循环区间
---Set loop range
---
---# 说明 / Description
---设置循环区间为 [loop_end - loop_duration, loop_end)
---当播放时间超过 loop_end 时，会跳转到 loop_end - loop_duration
---Sets the loop range to [loop_end - loop_duration, loop_end)
---When playback time exceeds loop_end, it jumps to loop_end - loop_duration
---
---# 示例 / Example
---```lua
---video:setLoopRange(10.0, 5.0)  -- 循环区间 [5.0, 10.0) / Loop range [5.0, 10.0)
---```
---@param loop_end number 循环结束时间（秒）/ Loop end time in seconds
---@param loop_duration number 循环时长（秒）/ Loop duration in seconds
function Video:setLoopRange(loop_end, loop_duration) end

---获取循环区间
---Get loop range
---
---# 示例 / Example
---```lua
---local loop_end, loop_duration = video:getLoopRange()
---print(string.format("Loop range: [%.2f, %.2f)", loop_end - loop_duration, loop_end))
---```
---@return number loop_end 循环结束时间（秒）/ Loop end time in seconds
---@return number loop_duration 循环时长（秒）/ Loop duration in seconds
function Video:getLoopRange() end

--------------------------------------------------------------------------------
--- 流信息 / Stream Information
--------------------------------------------------------------------------------

---获取视频流列表
---Get list of video streams
---
---# 说明 / Description
---返回视频文件中所有视频流的信息，用于多轨视频的流选择
---Returns information about all video streams in the file, used for multi-track video stream selection
---
---# 示例 / Example
---```lua
---local streams = video:getVideoStreams()
---for i, stream in ipairs(streams) do
---    print(string.format("Stream %d: %dx%d, %.2f fps, %.2f sec",
---        stream.index, stream.width, stream.height, stream.fps, stream.duration))
---end
---```
---@return lstg.VideoStreamInfo[] streams 视频流信息数组 / Array of video stream info
function Video:getVideoStreams() end

---获取音频流列表
---Get list of audio streams
---
---# 说明 / Description
---返回视频文件中所有音频流的信息
---Returns information about all audio streams in the file
---
---# 示例 / Example
---```lua
---local streams = video:getAudioStreams()
---for i, stream in ipairs(streams) do
---    print(string.format("Stream %d: %d channels, %d Hz, %.2f sec",
---        stream.index, stream.channels, stream.sample_rate, stream.duration))
---end
---```
---@return lstg.AudioStreamInfo[] streams 音频流信息数组 / Array of audio stream info
function Video:getAudioStreams() end

---获取当前视频流索引
---Get current video stream index
---@return number index 视频流索引 / Video stream index
function Video:getVideoStreamIndex() end

---重新打开视频
---Reopen the video with new options
---
---# 说明 / Description
---使用新的选项重新打开视频，可用于切换视频流或修改输出尺寸
---Reopens the video with new options, can be used to switch video streams or change output size
---
---# 示例 / Example
---```lua
---video:reopen({ video_stream = 1, width = 1920, height = 1080 })
---```
---@param options? lstg.VideoOpenOptions 新的打开选项 / New open options
---@return boolean success 是否成功 / Success status
function Video:reopen(options) end

--------------------------------------------------------------------------------
--- 类型定义 / Type Definitions
--------------------------------------------------------------------------------

---@class lstg.VideoOpenOptions
---视频打开选项
---Video open options
---@field video_stream? number 视频流索引，默认自动选择 / Video stream index, auto-select by default
---@field width? number 输出宽度（像素），0 表示使用原始尺寸 / Output width in pixels, 0 for original size
---@field height? number 输出高度（像素），0 表示使用原始尺寸 / Output height in pixels, 0 for original size
---@field premultiplied_alpha? boolean 是否使用预乘 Alpha，默认 false / Use premultiplied alpha, default false
---@field looping? boolean 是否循环播放，默认 false / Enable looping, default false
---@field loop_end? number 循环结束时间（秒），默认 0 / Loop end time in seconds, default 0
---@field loop_duration? number 循环时长（秒），默认 0 / Loop duration in seconds, default 0

---@class lstg.VideoInfo
---视频信息
---Video information
---@field duration number 视频总时长（秒）/ Total duration in seconds
---@field time number 当前播放时间（秒）/ Current playback time in seconds
---@field looping boolean 是否循环播放 / Whether looping
---@field loop_end number 循环结束时间（秒）/ Loop end time in seconds
---@field loop_duration number 循环时长（秒）/ Loop duration in seconds
---@field width number 视频宽度（像素）/ Video width in pixels
---@field height number 视频高度（像素）/ Video height in pixels
---@field video_stream number 当前视频流索引 / Current video stream index
---@field frame_interval number 帧间隔（秒）/ Frame interval in seconds
---@field fps number 帧率 / Frame rate

---@class lstg.VideoStreamInfo
---视频流信息
---Video stream information
---@field index number 流索引 / Stream index
---@field width number 视频宽度（像素）/ Video width in pixels
---@field height number 视频高度（像素）/ Video height in pixels
---@field fps number 帧率 / Frame rate
---@field duration number 时长（秒）/ Duration in seconds

---@class lstg.AudioStreamInfo
---音频流信息
---Audio stream information
---@field index number 流索引 / Stream index
---@field channels number 声道数 / Number of channels
---@field sample_rate number 采样率（Hz）/ Sample rate in Hz
---@field duration number 时长（秒）/ Duration in seconds

return Video
