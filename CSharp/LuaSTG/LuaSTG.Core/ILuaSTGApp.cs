using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    public interface ILuaSTGApp
    {
        void GameInit();
        bool FrameFunc();
        void RenderFunc();
        void GameExit();
        void FocusGainFunc();
        void FocusLoseFunc();
    }
}
