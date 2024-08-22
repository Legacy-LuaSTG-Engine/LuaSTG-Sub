
local findtable = require("findtable")
local E = findtable.makeEntry

---@type findtable.config
local lstg_GameObject = {
    cpp_namespace = { "LuaSTG" },
    func_name = "MapGameObjectMember",
    enum_name = "GameObjectMember",
    enum_entry = {
        -- basic
        E("status", "STATUS"),
        E("class" , "CLASS" ),
        -- user
        E("timer" , "TIMER" ),
        -- transform
        E("x"     , "X"     ),
        E("y"     , "Y"     ),
        E("rot"   , "ROT"   ),
        E("hscale", "HSCALE"),
        E("vscale", "VSCALE"),
        -- movement
        E("dx"    , "DX"    ),
        E("dy"    , "DY"    ),
        E("omiga" , "OMIGA" ),
	    E("omega" , "OMEGA" ),
        E("ax"    , "AX"    ),
        E("ay"    , "AY"    ),
        E("ag"    , "AG"    ),
        E("vx"    , "VX"    ),
        E("vy"    , "VY"    ),
        E("maxv"  , "MAXV"  ),
        E("maxvx" , "MAXVX" ),
        E("maxvy" , "MAXVY" ),
        E("navi"  , "NAVI"  ),
        -- render
        E("layer" , "LAYER" ),
        E("hide"  , "HIDE"  ),
        E("img"   , "IMG"   ),
        E("rc"    , "RES_RC"),
        E("ani"   , "ANI"   ),
        E("_blend", "_BLEND"),
        E("_color", "_COLOR"),
        E("_a"    , "_A"    ),
        E("_r"    , "_R"    ),
        E("_g"    , "_G"    ),
        E("_b"    , "_B"    ),
        -- collision
        E("bound" , "BOUND" ),
        E("group" , "GROUP" ),
        E("colli" , "COLLI" ),
        E("a"     , "A"     ),
        E("b"     , "B"     ),
        E("rect"  , "RECT"  ),
        E("collider", "COLLIDER"), -- TODO: remove it
        -- TODO: fuck ex+
        E("_angle", "VANGLE"  ),
        E("_speed", "VSPEED"  ),
        E("pause" , "PAUSE"  ),
        E("nopause", "IGNORESUPERPAUSE"  ),
        E("rmove" , "RESOLVEMOVE"  ),
        E("world" , "WORLD"  ),
    },
}

---@type findtable.config
local lstg_BlendMode = {
    cpp_namespace = { "LuaSTG" },
    func_name = "MapBlendModeX",
    enum_name = "BlendModeX",
    enum_entry = {
        -- plus
        E("mul+alpha" , "MulAlpha"  ,  1),
        E("mul+add"   , "MulAdd"    ,  2),
        E("mul+rev"   , "MulRev"    ,  3),
        E("mul+sub"   , "MulSub"    ,  4),
        E("add+alpha" , "AddAlpha"  ,  5),
        E("add+add"   , "AddAdd"    ,  6),
        E("add+rev"   , "AddRev"    ,  7),
        E("add+sub"   , "AddSub"    ,  8),
        -- ex plus
        E("alpha+bal" , "AlphaBal"  ,  9),
        -- sub
        E("mul+min"   , "MulMin"    , 10),
        E("mul+max"   , "MulMax"    , 11),
        E("mul+mul"   , "MulMutiply", 12),
        E("mul+screen", "MulScreen" , 13),
        E("add+min"   , "AddMin"    , 14),
        E("add+max"   , "AddMax"    , 15),
        E("add+mul"   , "AddMutiply", 16),
        E("add+screen", "AddScreen" , 17),
        E("one"       , "One"       , 18),
    },
}

---@type findtable.config
local lstg_Color = {
    cpp_namespace = { "LuaSTG" },
    func_name = "MapColorMember",
    enum_name = "ColorMember",
    enum_entry = {
        -- basic
        E("a"   , "m_a"   ),
        E("r"   , "m_r"   ),
        E("g"   , "m_g"   ),
        E("b"   , "m_b"   ),
        E("argb", "m_argb"),
        E("ARGB", "f_ARGB"),
        -- ext
        E("h"   , "m_h"   ), -- TODO: remove it
        E("s"   , "m_s"   ), -- TODO: remove it
        E("v"   , "m_v"   ), -- TODO: remove it
        E("AHSV", "f_AHSV"), -- TODO: remove it
    },
}

findtable.makeSource("lua_luastg_hash", {
    lstg_GameObject,
    lstg_BlendMode,
    lstg_Color,
})
