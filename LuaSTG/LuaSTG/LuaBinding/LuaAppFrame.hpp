#pragma once

namespace luastg
{
    namespace LuaEngine
    {
        constexpr auto G_CALLBACK_EngineInit    = "GameInit";
        constexpr auto G_CALLBACK_EngineStop    = "GameExit";
        constexpr auto G_CALLBACK_EngineUpdate  = "FrameFunc";
        constexpr auto G_CALLBACK_EngineDraw    = "RenderFunc";
        constexpr auto G_CALLBACK_EngineEvent   = "EventFunc";
        constexpr auto G_CALLBACK_FocusLoseFunc = "FocusLoseFunc";
        constexpr auto G_CALLBACK_FocusGainFunc = "FocusGainFunc";
        
        enum class EngineEvent
        {
            Idle,
            WindowActive, // handle(event:number, active:boolean)
            WindowResize, // handle(event:number, width:number, height:number)
        };
    };
};
