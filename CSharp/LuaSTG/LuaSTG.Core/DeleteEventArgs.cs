using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    public record struct DeleteEventArgs(DeleteEventType DeleteEventType)
    {
    }
}
