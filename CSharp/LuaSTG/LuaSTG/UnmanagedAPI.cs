using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG
{
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct UnmanagedAPI
    {
        public delegate* unmanaged<void> beginScene;
        public delegate* unmanaged<void> endScene;
        public delegate* unmanaged<byte, byte, byte, byte, void> renderClear;
    }
}
