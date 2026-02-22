-- 视频播放示例
-- 使用 StopWatch 提供绝对时间，手动调用 VideoUpdate 更新画面

-- 在资源池中加载视频
-- lstg.LoadVideo(name, path) 或 lstg.LoadVideo(name, path, options)
-- options 表可选：video_stream, width, height, premultiplied_alpha, looping, loop_end, loop_duration
lstg.LoadVideo('video1', 'test_video.mp4')
-- 示例：带选项加载（输出尺寸、循环）
-- lstg.LoadVideo('video1', 'test_video.mp4', { width = 1280, height = 720, looping = true })
-- 示例：带循环区间 [end-duration, end)
-- lstg.LoadVideo('video1', 'test_video.mp4', { looping = true, loop_end = 10, loop_duration = 5 })

-- 枚举流（加载后从已打开的视频枚举，用于多轨选择）
-- local video_streams = lstg.VideoGetVideoStreams('video1')
-- local audio_streams = lstg.VideoGetAudioStreams('video1')

-- 创建计时器；循环也可在 LoadVideo 的 options 里用 looping = true / loop_end / loop_duration 设置
local video_clock = lstg.StopWatch()
lstg.VideoSetLooping('video1', true)

-- 像普通纹理一样创建精灵
lstg.LoadImage('video_sprite', 'video1', 0, 0, 640, 480)

-- 在 RenderFunc 中渲染
function RenderFunc()
    lstg.VideoUpdate('video1', video_clock:GetElapsed())
    lstg.Render('video_sprite', 100, 100)
end

-- 可用控制：
-- lstg.VideoSetLooping(name, bool)
-- lstg.VideoSetLoopRange(name, loop_end, loop_duration)  -- 区间 [end-duration, end)，超过 end 减 duration
-- lstg.VideoSeek(name, time)
-- lstg.VideoUpdate(name, absolute_time)
-- lstg.VideoGetInfo(name)  -- duration, time, looping, loop_end, loop_duration, width, height, fps, video_stream
-- lstg.VideoGetVideoStreams(name)   -- 视频流列表 { index, width, height, fps, duration }
-- lstg.VideoGetAudioStreams(name)  -- 音频流列表 { index, channels, sample_rate, duration }
-- lstg.VideoReopen(name [, options])
