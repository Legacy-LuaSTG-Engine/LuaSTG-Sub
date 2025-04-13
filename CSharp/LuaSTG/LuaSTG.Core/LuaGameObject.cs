using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    /// <summary>
    /// Gameobject instances created by lua script. This object should not be instantiated by user.
    /// </summary>
    public sealed class LuaGameObject : GameObjectBase
    {
        internal LuaGameObject(IntPtr nativePtr) : base(nativePtr)
        {

        }

        public override sealed void OnFrame()
        {
        }

        public override void OnRender()
        {
        }

        public override sealed void OnDestroy(DestroyEventArgs args)
        {
        }

        public override sealed void OnColli(Collision collision)
        {
        }
    }
}
