--------------------------------------------------------------------------------
--- 视频播放示例（现代对象化 API）
--- Video playback example (Modern object-oriented API)
--------------------------------------------------------------------------------

-- 创建视频对象
local video = lstg.Video.create('test_video.mp4', {
    looping = true,
})

-- 创建计时器
local video_clock = lstg.StopWatch()

-- 获取视频纹理并创建精灵
local video_texture = video:getTexture()
local video_sprite = lstg.Sprite.create(video_texture, 0, 0, 
    video:getWidth(), video:getHeight())

-- 创建精灵渲染器
local sprite_renderer = lstg.SpriteRenderer.create(video_sprite)

-- 在 RenderFunc 中渲染
function RenderFunc()
    -- 更新视频到当前时间
    video:update(video_clock:GetElapsed())
    
    -- 渲染视频
    sprite_renderer:setPosition(100, 100)
    sprite_renderer:draw()
end

