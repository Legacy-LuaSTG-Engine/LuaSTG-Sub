using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG
{
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct InitPayload
    {
        public delegate* unmanaged<void> log;
    }
}
