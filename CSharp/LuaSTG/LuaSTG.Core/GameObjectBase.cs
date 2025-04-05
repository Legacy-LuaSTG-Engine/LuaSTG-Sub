using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    using static LuaSTGAPI;

    public abstract class GameObjectBase
    {
        internal static Dictionary<ulong, GameObjectBase> _id2Obj = new(65536);

        private IntPtr _nativePtr;
        private ulong _id;

        internal GameObjectBase()
        {
            _nativePtr = GameObject_New();
            _id = GameObject_GetID(_nativePtr);
            _id2Obj.Add(_id, this);
        }

        internal GameObjectBase(IntPtr nativePtr)
        {
            _nativePtr = nativePtr;
            _id = GameObject_GetID(_nativePtr);
            _id2Obj.Add(_id, this);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool IsValid()
        {
            return _nativePtr != 0;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal void Detach()
        {
            _nativePtr = 0;
        }

        public abstract void OnFrame();
        public abstract void OnDestroy(DeleteEventArgs args);
        public abstract void OnColli(Collision other);
        public virtual void OnRender()
        {
            DefaultRenderFunc();
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        protected void DefaultRenderFunc()
        {
            ThrowIfInvalid();
            GameObject_DefaultRenderFunc(_nativePtr);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void ThrowIfInvalid()
        {
            if (!IsValid()) throw new InvalidOperationException("gameobject has been disposed.");
        }
    }

    public static unsafe partial class LuaSTGAPI
    {
        // TODO: pass non-zero arg for callback optimization
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static IntPtr GameObject_New() => api.gameObject_New(0);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static ulong GameObject_GetID(IntPtr p) => api.gameObject_GetID(p);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static void GameObject_DefaultRenderFunc(IntPtr nativePtr) => api.gameObject_DefaultRenderFunc(nativePtr);

        [UnmanagedCallersOnly]
        private static void CreateLuaGameObject(IntPtr p)
        {
            _ = new LuaGameObject(p);
        }

        [UnmanagedCallersOnly]
        private static void DetachGameObject(ulong id)
        {
            GameObjectBase._id2Obj[id].Detach();
            GameObjectBase._id2Obj.Remove(id);
        }

        [UnmanagedCallersOnly]
        private static void CallOnFrame(ulong id)
        {
            GameObjectBase._id2Obj[id].OnFrame();
        }

        [UnmanagedCallersOnly]
        private static void CallOnRender(ulong id)
        {
            GameObjectBase._id2Obj[id].OnRender();
        }

        [UnmanagedCallersOnly]
        private static void CallOnDestroy(ulong id, int reason)
        {
            GameObjectBase._id2Obj[id].OnDestroy(new((DeleteEventType)reason));
        }

        [UnmanagedCallersOnly]
        private static void CallOnColli(ulong id, ulong otherID)
        {
            GameObjectBase._id2Obj[id].OnColli(new(GameObjectBase._id2Obj[otherID]));
        }
    }
}
