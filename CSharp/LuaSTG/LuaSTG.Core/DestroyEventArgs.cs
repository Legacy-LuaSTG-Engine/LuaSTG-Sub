using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    /// <summary>
    /// Stores a destroy event info.
    /// </summary>
    /// <param name="DestroyEventType">Type of the destroy event.</param>
    public record struct DestroyEventArgs(DestroyEventType DestroyEventType)
    {
    }
}
