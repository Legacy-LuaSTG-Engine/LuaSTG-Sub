local DIR = "../../LuaSTG/LuaSTG/LuaBinding/generated"

local property_map = require("property_map")

local GameObjectMember = property_map.Configuration()
    :setNamespace("LuaSTG")
    :setClassName("GameObjectMember")
    :setFunctionName("MapGameObjectMember")
    -- basic
    :addClassMember("STATUS", nil, "status")
    :addClassMember("CLASS" , nil, "class" )
    -- user
    :addClassMember("TIMER", nil, "timer")
    -- transform
    :addClassMember("X"     , nil, "x"     )
    :addClassMember("Y"     , nil, "y"     )
    :addClassMember("ROT"   , nil, "rot"   )
    :addClassMember("HSCALE", nil, "hscale")
    :addClassMember("VSCALE", nil, "vscale")
    -- movement
    :addClassMember("DX"    , nil, "dx"   )
    :addClassMember("DY"    , nil, "dy"   )
    :addClassMember("OMIGA" , nil, "omiga")
    :addClassMember("OMEGA" , nil, "omega")
    :addClassMember("AX"    , nil, "ax"   )
    :addClassMember("AY"    , nil, "ay"   )
    :addClassMember("AG"    , nil, "ag"   )
    :addClassMember("VX"    , nil, "vx"   )
    :addClassMember("VY"    , nil, "vy"   )
    :addClassMember("MAXV"  , nil, "maxv" )
    :addClassMember("MAXVX" , nil, "maxvx")
    :addClassMember("MAXVY" , nil, "maxvy")
    :addClassMember("NAVI"  , nil, "navi" )
    -- render
    :addClassMember("LAYER" , nil, "layer" )
    :addClassMember("HIDE"  , nil, "hide"  )
    :addClassMember("IMG"   , nil, "img"   )
    :addClassMember("RES_RC", nil, "rc"    )
    :addClassMember("ANI"   , nil, "ani"   )
    :addClassMember("_BLEND", nil, "_blend")
    :addClassMember("_COLOR", nil, "_color")
    :addClassMember("_A"    , nil, "_a"    )
    :addClassMember("_R"    , nil, "_r"    )
    :addClassMember("_G"    , nil, "_g"    )
    :addClassMember("_B"    , nil, "_b"    )
    -- collision
    :addClassMember("BOUND"   , nil, "bound"   )
    :addClassMember("GROUP"   , nil, "group"   )
    :addClassMember("COLLI"   , nil, "colli"   )
    :addClassMember("A"       , nil, "a"       )
    :addClassMember("B"       , nil, "b"       )
    :addClassMember("RECT"    , nil, "rect"    )
    :addClassMember("COLLIDER", nil, "collider") -- TODO: remove it
    -- TODO: deprecate ex+ properties
    :addClassMember("VANGLE"          , nil, "_angle" )
    :addClassMember("VSPEED"          , nil, "_speed" )
    :addClassMember("PAUSE"           , nil, "pause"  ) -- TODO: remove it
    :addClassMember("IGNORESUPERPAUSE", nil, "nopause")
    :addClassMember("RESOLVEMOVE"     , nil, "rmove"  )
    :addClassMember("WORLD"           , nil, "world"  )

property_map.build(GameObjectMember, nil, DIR)

local BlendModeX = property_map.Configuration()
    :setNamespace("LuaSTG")
    :setClassName("BlendModeX")
    :setFunctionName("MapBlendModeX")
    -- plus
    :addClassMember("MulAlpha"  ,  1, "mul+alpha" )
    :addClassMember("MulAdd"    ,  2, "mul+add"   )
    :addClassMember("MulRev"    ,  3, "mul+rev"   )
    :addClassMember("MulSub"    ,  4, "mul+sub"   )
    :addClassMember("AddAlpha"  ,  5, "add+alpha" )
    :addClassMember("AddAdd"    ,  6, "add+add"   )
    :addClassMember("AddRev"    ,  7, "add+rev"   )
    :addClassMember("AddSub"    ,  8, "add+sub"   )
    -- ex plus
    :addClassMember("AlphaBal"  ,  9, "alpha+bal" )
    -- sub
    :addClassMember("MulMin"    , 10, "mul+min"   )
    :addClassMember("MulMax"    , 11, "mul+max"   )
    :addClassMember("MulMutiply", 12, "mul+mul"   )
    :addClassMember("MulScreen" , 13, "mul+screen")
    :addClassMember("AddMin"    , 14, "add+min"   )
    :addClassMember("AddMax"    , 15, "add+max"   )
    :addClassMember("AddMutiply", 16, "add+mul"   )
    :addClassMember("AddScreen" , 17, "add+screen")
    :addClassMember("One"       , 18, "one"       )

property_map.build(BlendModeX, nil, DIR)

local ColorMember = property_map.Configuration()
    :setNamespace("LuaSTG")
    :setClassName("ColorMember")
    :setFunctionName("MapColorMember")
    -- basic
    :addClassMember("m_a"   , nil, "a"   )
    :addClassMember("m_r"   , nil, "r"   )
    :addClassMember("m_g"   , nil, "g"   )
    :addClassMember("m_b"   , nil, "b"   )
    :addClassMember("m_argb", nil, "argb")
    :addClassMember("f_ARGB", nil, "ARGB")
    -- ext
    :addClassMember("m_h"   , nil, "h"   ) -- TODO: remove it
    :addClassMember("m_s"   , nil, "s"   ) -- TODO: remove it
    :addClassMember("m_v"   , nil, "v"   ) -- TODO: remove it
    :addClassMember("f_AHSV", nil, "AHSV") -- TODO: remove it

property_map.build(ColorMember, nil, DIR)
