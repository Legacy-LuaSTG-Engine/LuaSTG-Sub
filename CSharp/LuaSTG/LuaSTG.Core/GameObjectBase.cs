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

    /// <summary>
    /// Base class for any objects created by game engine.
    /// </summary>
    public abstract class GameObjectBase
    {
        internal static Dictionary<ulong, GameObjectBase> _id2Obj = new(65536);

        private IntPtr _nativePtr;

        internal GameObjectBase()
        {
            _nativePtr = GameObject_New();
            _id2Obj.Add(GameObject_GetID(_nativePtr), this);
        }

        internal GameObjectBase(IntPtr nativePtr)
        {
            _nativePtr = nativePtr;
            _id2Obj.Add(GameObject_GetID(_nativePtr), this);
        }

        /// <summary>
        /// Check whether the current gameobject is active in object pool.
        /// </summary>
        /// <returns><see cref="true"/> for active, otherwise <see cref="false"/>.</returns>
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

        /// <summary>
        /// When override in child class, called each frame when this gameobject is active.
        /// </summary>
        public abstract void OnFrame();

        /// <summary>
        /// When override in child class, called when this gameobject is set to be deactivated.
        /// </summary>
        /// <param name="args">Args depends on the call site.</param>
        public abstract void OnDestroy(DestroyEventArgs args);

        /// <summary>
        /// When override in child class, called when a collision detected.
        /// </summary>
        /// <param name="collision">Info of the collision.</param>
        public abstract void OnColli(Collision collision);

        /// <summary>
        /// When override in child class, called when this gameobject is being rendered.
        /// </summary>
        public virtual void OnRender()
        {
            DefaultRenderFunc();
        }

        /// <summary>
        /// Call default render func for current game object.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        protected void DefaultRenderFunc()
        {
            ThrowIfInvalid();
            GameObject_DefaultRenderFunc(_nativePtr);
        }

        /// <summary>
        /// Throw an exception if the current gameobject is active in object pool.
        /// </summary>
        /// <exception cref="InvalidOperationException">Exception thrown if not in valid state.</exception>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        protected void ThrowIfInvalid()
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
            GameObjectBase._id2Obj[id].OnDestroy(new((DestroyEventType)reason));
        }

        [UnmanagedCallersOnly]
        private static void CallOnColli(ulong id, ulong otherID)
        {
            var self = GameObjectBase._id2Obj[id];
            self.OnColli(new(self, GameObjectBase._id2Obj[otherID]));
        }
    }
}
