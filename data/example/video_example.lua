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
-- 2. VideoSeek(name, time) -- 控制时间，但是不更新画面，适合进度条等场景，完成后需要调用 VideoUpdate 来刷新画面
-- 3. VideoUpdate(name, absolute_time) -- 根据绝对时间更新视频画面，适合与 StopWatch 结合使用，确保视频播放与游戏时间同步
-- 4. VideoGetInfo(name) -- 获取视频信息，如帧率、总时长等，返回一个表格
