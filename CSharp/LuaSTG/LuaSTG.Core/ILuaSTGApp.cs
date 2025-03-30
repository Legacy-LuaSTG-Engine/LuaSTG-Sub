using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    /// <summary>
    /// Represents a LuaSTG application.
    /// </summary>
    public interface ILuaSTGApp
    {
        /// <summary>
        /// Method called at the startup of the application.
        /// </summary>
        void GameInit();
        /// <summary>
        /// Method called each frame.
        /// </summary>
        /// <returns> 
        /// <see cref="true"/> if app has finished its execution.
        /// </returns>
        bool FrameFunc();
        /// <summary>
        /// Method called after frame for rendering.
        /// </summary>
        void RenderFunc();
        /// <summary>
        /// Method called when exiting application.
        /// </summary>
        void GameExit();
        /// <summary>
        /// Method called when application gain focus.
        /// </summary>
        void FocusGainFunc();
        /// <summary>
        /// Method called when application lose focus.
        /// </summary>
        void FocusLoseFunc();
    }
}
