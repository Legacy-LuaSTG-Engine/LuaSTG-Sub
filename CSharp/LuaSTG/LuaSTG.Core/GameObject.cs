using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    /// <summary>
    /// Base class for any objects created in CLR.
    /// </summary>
    public class GameObject : GameObjectBase
    {
        public GameObject() : base()
        {

        }

        public override void OnFrame()
        {
        }

        public override void OnColli(Collision collision)
        {
        }

        public override void OnDestroy(DestroyEventArgs args)
        {
        }
    }
}
