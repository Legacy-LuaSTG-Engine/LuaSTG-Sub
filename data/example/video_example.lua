-- 视频播放示例
-- 使用 StopWatch 提供绝对时间，手动调用 VideoUpdate 更新画面

-- 在资源池中加载视频
-- 参数：名称，路径
LoadVideo('video1', 'test_video.mp4')

-- 创建计时器并设置循环播放
local video_clock = lstg.StopWatch()
VideoSetLooping('video1', true)

-- 像普通纹理一样创建精灵
LoadImage('video_sprite', 'video1', 0, 0, 640, 480)

-- 在 RenderFunc 中渲染
function RenderFunc()
    -- 以秒表绝对时间驱动视频更新
    VideoUpdate('video1', video_clock:GetElapsed())
    Render('video_sprite', 100, 100)
end

-- 可用控制：
-- 1. VideoSetLooping(name, bool) -- 设置是否循环播放
-- 2. VideoSeek(name, time) -- 控制时间，不更新画面，适合进度条；完成后需 VideoUpdate 刷新
-- 3. VideoUpdate(name, absolute_time) -- 按绝对时间更新画面；传入比上次小的时间可倒放/拖拽到该时刻
-- 4. VideoGetInfo(name) -- 获取视频信息（时长、当前时间、尺寸等）
