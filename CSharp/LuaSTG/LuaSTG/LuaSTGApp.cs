using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using LuaSTG.Core;

using static LuaSTG.Core.LuaSTGAPI;

namespace LuaSTG
{
    public class LuaSTGApp : ILuaSTGApp
    {
        public void GameInit()
        {
        }

        public bool FrameFunc()
        {
            return false;
        }

        public void RenderFunc()
        {
        }

        public void GameExit()
        {
        }

        public void FocusGainFunc()
        {
        }

        public void FocusLoseFunc()
        {
        }
    }
}
