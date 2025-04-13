using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    /// <summary>
    /// Type of the destroy event.
    /// </summary>
    public enum DestroyEventType : int
    {
        /// <summary>
        /// Destroy by out-of-bounds.
        /// </summary>
        Bound = 0,
        /// <summary>
        /// Destroy by Del method.
        /// </summary>
        Del = 1,
        /// <summary>
        /// Destroy by Kill method.
        /// </summary>
        Kill = 2,
    }
}
