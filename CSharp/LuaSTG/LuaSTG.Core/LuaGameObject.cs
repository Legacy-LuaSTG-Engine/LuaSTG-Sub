using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
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

        public override sealed void OnDestroy(DeleteEventArgs args)
        {
        }

        public override sealed void OnColli(Collision collision)
        {
        }
    }
}
