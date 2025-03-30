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
        public static void Log(LogLevel level, string message)
        {
            IntPtr unmanagedString = Marshal.StringToHGlobalAnsi(message);
            api.log((int) level, unmanagedString);
            Marshal.FreeHGlobal(unmanagedString);
        }

        public static void BeginScene() => api.beginScene();

        public static void EndScene() => api.endScene();

        public static void RenderClear(byte a, byte r, byte g, byte b) => api.renderClear(a, r, g, b);

        private static UnmanagedAPI api;

        private static void SaveUnmanagedAPI(UnmanagedAPI* unmanagedAPI)
        {
            api = *unmanagedAPI;
        }
    }
}
