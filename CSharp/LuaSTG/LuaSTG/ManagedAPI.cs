using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG
{
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct ManagedAPI
    {
        public delegate* unmanaged<void> GameInit;
        public delegate* unmanaged<byte> FrameFunc;
        public delegate* unmanaged<void> RenderFunc;
        public delegate* unmanaged<void> GameExit;
        public delegate* unmanaged<void> FocusLoseFunc;
        public delegate* unmanaged<void> FocusGainFunc;
    }
}
