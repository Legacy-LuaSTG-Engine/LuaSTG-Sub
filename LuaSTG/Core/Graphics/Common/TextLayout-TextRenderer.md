ImageAtlas
    getImage
    add
    get
    remove
    clear

SpriteAtlas
    getImageAtlas
    getTexture
    add
    get
    remove
    clear

GlyphCache
    code: integer
    rect: Rect -- rect on image
    uv: Rect -- rect on texture
    pen: Vector2 -- glyph image left-top position
    advance: Vector2 -- pen move to next position

GlyphCacheManager
    setFont(font: Font)
    setDefaultSize(size_in_pixel: number)
    add(code: integer | string, size_in_pixel: number?): GlyphCache?, string
    get(code: integer | string, size_in_pixel: number?): GlyphCache?
    remove(code: integer | string, size_in_pixel: number?): boolean
    clear()
    getSpriteAtlas(): SpriteAtlas
    getSprite(code: integer | string, size_in_pixel: number?): Sprite?

TextLayout
    setText
    setFontFamily
    setFontSize
    setFontWeight
    setFontStyle
    setLayoutSize
    setTextAlignment
    setParagraphAlignment

TextRenderer
    setTextLayout
    setAnchor
    setPosition
    setScale
    setRotation
    setTransform
    setColor
    setLegacyBlendState
