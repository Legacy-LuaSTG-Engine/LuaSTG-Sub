using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    [StructLayout(LayoutKind.Sequential)]
    internal unsafe struct ManagedAPI
    {
        public delegate* unmanaged<void> GameInit;
        public delegate* unmanaged<byte> FrameFunc;
        public delegate* unmanaged<void> RenderFunc;
        public delegate* unmanaged<void> GameExit;
        public delegate* unmanaged<void> FocusLoseFunc;
        public delegate* unmanaged<void> FocusGainFunc;
    }

    public static unsafe partial class LuaSTGAPI
    {
        [UnmanagedCallersOnly]
        internal unsafe static void GameInit()
        {
            app?.GameInit();
        }

        [UnmanagedCallersOnly]
        internal unsafe static byte FrameFunc()
        {
            return (byte)((app?.FrameFunc() ?? false) ? 1 : 0);
        }

        [UnmanagedCallersOnly]
        internal unsafe static void RenderFunc()
        {
            app?.RenderFunc();
        }

        [UnmanagedCallersOnly]
        internal unsafe static void GameExit()
        {
            app?.GameExit();
        }

        [UnmanagedCallersOnly]
        internal unsafe static void FocusGainFunc()
        {
            app?.FocusGainFunc();
        }

        [UnmanagedCallersOnly]
        internal unsafe static void FocusLoseFunc()
        {
            app?.FocusLoseFunc();
        }

        private static void AssignManagedAPI(ManagedAPI* managedAPI)
        {
            managedAPI->GameInit = &GameInit;
            managedAPI->FrameFunc = &FrameFunc;
            managedAPI->RenderFunc = &RenderFunc;
            managedAPI->GameExit = &GameExit;
            managedAPI->FocusGainFunc = &FocusGainFunc;
            managedAPI->FocusLoseFunc = &FocusLoseFunc;
        }
    }
}
