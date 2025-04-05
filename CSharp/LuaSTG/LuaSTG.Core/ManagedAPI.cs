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

        public delegate* unmanaged<ulong, void> DetachGameObject;
        public delegate* unmanaged<IntPtr, void> CreateLuaGameObject;
        public delegate* unmanaged<ulong, void> CallOnFrame;
        public delegate* unmanaged<ulong, void> CallOnRender;
        public delegate* unmanaged<ulong, int, void> CallOnDestroy;
        public delegate* unmanaged<ulong, ulong, void> CallOnColli;
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

            managedAPI->CreateLuaGameObject = &CreateLuaGameObject;
            managedAPI->DetachGameObject = &DetachGameObject;
            managedAPI->CallOnFrame = &CallOnFrame;
            managedAPI->CallOnRender = &CallOnRender;
            managedAPI->CallOnDestroy = &CallOnDestroy;
            managedAPI->CallOnColli = &CallOnColli;
        }
    }
}
