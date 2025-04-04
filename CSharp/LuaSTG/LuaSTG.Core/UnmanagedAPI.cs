using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    [StructLayout(LayoutKind.Sequential)]
    internal unsafe struct UnmanagedAPI
    {
        public delegate* unmanaged<int, IntPtr, void> log;
        public delegate* unmanaged<void> beginScene;
        public delegate* unmanaged<void> endScene;
        public delegate* unmanaged<byte, byte, byte, byte, void> renderClear;
    }

    public static unsafe partial class LuaSTGAPI
    {
        /// <summary>
        /// Print a log to LuaSTG Engine.
        /// </summary>
        /// <param name="level">Logging level.</param>
        /// <param name="message">Logging message.</param>
        public static void Log(LogLevel level, string message)
        {
            IntPtr unmanagedString = Marshal.StringToHGlobalAnsi(message);
            api.log((int)level, unmanagedString);
            Marshal.FreeHGlobal(unmanagedString);
        }

        /// <summary>
        /// Notify engine to start rendering.
        /// </summary>
        public static void BeginScene() => api.beginScene();

        /// <summary>
        /// Notify engine to finish rendering.
        /// </summary>
        public static void EndScene() => api.endScene();

        /// <summary>
        /// Clear the current render target by the color provided.
        /// </summary>
        /// <param name="a">Alpha value.</param>
        /// <param name="r">Red value.</param>
        /// <param name="g">Green value.</param>
        /// <param name="b">Blue value.</param>
        public static void RenderClear(byte a, byte r, byte g, byte b) => api.renderClear(a, r, g, b);

        private static UnmanagedAPI api;

        private static void SaveUnmanagedAPI(UnmanagedAPI* unmanagedAPI)
        {
            api = *unmanagedAPI;
        }
    }
}
