using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    using static LuaSTGAPI;

    public class GameObject
    {
        internal static Dictionary<ulong, GameObject> _id2Obj = new(65536);

        private IntPtr _nativePtr;

        public GameObject()
        {
            _nativePtr = GameObject_New();
            _id2Obj.Add(GameObject_GetID(_nativePtr), this);
        }

        internal GameObject(IntPtr nativePtr)
        {
            _nativePtr = nativePtr;
            _id2Obj.Add(GameObject_GetID(_nativePtr), this);
        }

        public bool IsValid()
        {
            return _nativePtr != 0;
        }

        internal void Detach()
        {
            _nativePtr = 0;
        }
    }

    public static unsafe partial class LuaSTGAPI
    {
        internal static IntPtr GameObject_New()
        {
            return api.gameObject_New();
        }

        internal static ulong GameObject_GetID(IntPtr p)
        {
            return api.gameObject_GetID(p);
        }

        [UnmanagedCallersOnly]
        private static void CreateLuaGameObject(IntPtr p)
        {
            _ = new GameObject(p);
        }

        [UnmanagedCallersOnly]
        private static void DetachGameObject(ulong id)
        {
            GameObject._id2Obj[id].Detach();
            GameObject._id2Obj.Remove(id);
        }
    }
}
