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
    public unsafe abstract class GameObjectBase
    {
        public enum GameObjectStatus : byte
        {
            /// <summary>
            /// Available state
            /// </summary>
            Free = 0,

            /// <summary>
            /// Normal state, active in object pool.
            /// </summary>
            Active = 1,

            /// <summary>
            /// End of life cycle.
            /// </summary>
            Dead = 2,

            /// <summary>
            /// End of life cycle.
            /// </summary>
            Killed = 4,
        }

        internal static GameObjectBase?[] _id2Obj = new GameObjectBase[65536];

        internal IntPtr _nativePtr;
        private GameObject* _nativePtrConverted;

        internal GameObjectBase()
        {
            _nativePtr = GameObject_New();
            _nativePtrConverted = (GameObject*)_nativePtr.ToPointer();
            _id2Obj[GameObject_GetID(_nativePtr)] = this;
        }

        internal GameObjectBase(IntPtr nativePtr)
        {
            _nativePtr = nativePtr;
            _nativePtrConverted = (GameObject*)_nativePtr.ToPointer();
            _id2Obj[GameObject_GetID(_nativePtr)] = this;
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
            _nativePtrConverted = null;
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

        #region properties

        public GameObjectStatus Status
        {
            get => _nativePtrConverted->status;
        }

        public double X
        {
            get => _nativePtrConverted->x;
            set => _nativePtrConverted->x = value;
        }

        public double Y
        {
            get => _nativePtrConverted->y;
            set => _nativePtrConverted->y = value;
        }

        public double LastX
        {
            get => _nativePtrConverted->lastx;
        }

        public double LastY
        {
            get => _nativePtrConverted->lasty;
        }

        public double DX
        {
            get => _nativePtrConverted->dx;
        }

        public double DY
        {
            get => _nativePtrConverted->dy;
        }

        public double VX
        {
            get => _nativePtrConverted->vx;
            set => _nativePtrConverted->vx = value;
        }

        public double VY
        {
            get => _nativePtrConverted->vy;
            set => _nativePtrConverted->vy = value;
        }

        public double AX
        {
            get => _nativePtrConverted->ax;
            set => _nativePtrConverted->ax = value;
        }

        public double AY
        {
            get => _nativePtrConverted->ay;
            set => _nativePtrConverted->ay = value;
        }

        public double MaxVX
        {
            get => _nativePtrConverted->maxvx;
            set => _nativePtrConverted->maxvx = value;
        }

        public double MaxVY
        {
            get => _nativePtrConverted->maxvy;
            set => _nativePtrConverted->maxvy = value;
        }

        public double MaxV
        {
            get => _nativePtrConverted->maxv;
            set => _nativePtrConverted->maxv = value;
        }

        public long Group
        {
            get => _nativePtrConverted->group;
            // TODO: API
            //set => ((GameObject*)_nativePtr.ToPointer())->group = value;
        }

        public bool Bound
        {
            get => _nativePtrConverted->Bound;
            set => _nativePtrConverted->Bound = value;
        }

        public bool Colli
        {
            get => _nativePtrConverted->Colli;
            set => _nativePtrConverted->Colli = value;
        }

        public bool Rect
        {
            get => _nativePtrConverted->Rect;
            set => _nativePtrConverted->Rect = value;
        }

        public double A
        {
            get => _nativePtrConverted->a;
            set => _nativePtrConverted->a = value;
        }

        public double B
        {
            get => _nativePtrConverted->b;
            set => _nativePtrConverted->b = value;
        }

        public double Layer
        {
            get => _nativePtrConverted->layer;
            // TODO: API
            //set => _nativePtrConverted->layer = value;
        }

        public double HScale
        {
            get => _nativePtrConverted->hscale;
            set => _nativePtrConverted->hscale = value;
        }

        public double VScale
        {
            get => _nativePtrConverted->vscale;
            set => _nativePtrConverted->vscale = value;
        }

        public double Rot
        {
            get => _nativePtrConverted->rot;
            set => _nativePtrConverted->rot = value;
        }

        public double Omega
        {
            get => _nativePtrConverted->omega;
            set => _nativePtrConverted->omega = value;
        }

        public bool Hide
        {
            get => _nativePtrConverted->Hide;
            set => _nativePtrConverted->Hide = value;
        }

        public bool Navi
        {
            get => _nativePtrConverted->Navi;
            set => _nativePtrConverted->Navi = value;
        }


        public long Timer
        {
            get => _nativePtrConverted->timer;
            set => _nativePtrConverted->timer = value;
        }

        [StructLayout(LayoutKind.Explicit, Size = 320)]
        private struct GameObject
        {
            [FieldOffset(0)] public IntPtr pUpdatePrev;
            [FieldOffset(8)] public IntPtr pUpdateNext;
            [FieldOffset(16)] public IntPtr pColliPrev;
            [FieldOffset(24)] public IntPtr pColliNext;

            [FieldOffset(32)] public ulong uid;
            [FieldOffset(40)] public ulong id;

            [FieldOffset(48)] public double lastx;
            [FieldOffset(56)] public double lasty;
            [FieldOffset(64)] public double x;
            [FieldOffset(72)] public double y;
            [FieldOffset(80)] public double dx;
            [FieldOffset(88)] public double dy;
            [FieldOffset(96)] public double vx;
            [FieldOffset(104)] public double vy;
            [FieldOffset(112)] public double ax;
            [FieldOffset(120)] public double ay;
            [FieldOffset(128)] public double maxvx;
            [FieldOffset(136)] public double maxvy;
            [FieldOffset(144)] public double maxv;
            [FieldOffset(152)] public double ag;

            [FieldOffset(160)] public long group;

            [FieldOffset(168)] public double a;
            [FieldOffset(176)] public double b;
            [FieldOffset(184)] public double col_r;
            [FieldOffset(192)] public double layer;
            [FieldOffset(200)] public double nextlayer;
            [FieldOffset(208)] public double hscale;
            [FieldOffset(216)] public double vscale;
            [FieldOffset(224)] public double rot;
            [FieldOffset(232)] public double omega;
            [FieldOffset(240)] public double ani_timer;

            [FieldOffset(248)] public IntPtr res;
            [FieldOffset(256)] public IntPtr ps;

            [FieldOffset(264)] public long timer;

            [FieldOffset(272)] public uint vertexcolor;

            [FieldOffset(276)] public BlendMode blendmode;
            //[FieldOffset(277)] public GameObjectFeatures features;
            [FieldOffset(278)] public GameObjectStatus status;

            // 位域标志位集合
            [FieldOffset(279)] public byte flags;

            // 位域访问属性
            public bool Bound
            {
                readonly get => (flags & (1 << 0)) != 0;
                set
                {
                    if (value)
                        flags |= (1 << 0);
                    else
                        flags &= unchecked((byte)~(1 << 0));
                }
            }

            public bool Colli
            {
                readonly get => (flags & (1 << 1)) != 0;
                set
                {
                    if (value)
                        flags |= (1 << 1);
                    else
                        flags &= unchecked((byte)~(1 << 1));
                }
            }

            public bool Rect
            {
                readonly get => (flags & (1 << 2)) != 0;
                set
                {
                    if (value)
                        flags |= (1 << 2);
                    else
                        flags &= unchecked((byte)~(1 << 2));
                }
            }

            public bool Hide
            {
                readonly get => (flags & (1 << 3)) != 0;
                set
                {
                    if (value)
                        flags |= (1 << 3);
                    else
                        flags &= unchecked((byte)~(1 << 3));
                }
            }

            public bool Navi
            {
                readonly get => (flags & (1 << 4)) != 0;
                set
                {
                    if (value)
                        flags |= (1 << 4);
                    else
                        flags &= unchecked((byte)~(1 << 4));
                }
            }

            public bool IgnoreSuperpause
            {
                readonly get => (flags & (1 << 5)) != 0;
                set
                {
                    if (value)
                        flags |= (1 << 5);
                    else
                        flags &= unchecked((byte)~(1 << 5));
                }
            }

            public bool TouchLastXY
            {
                readonly get => (flags & (1 << 6)) != 0;
                set
                {
                    if (value)
                        flags |= (1 << 6);
                    else
                        flags &= unchecked((byte)~(1 << 6));
                }
            }
        }
        #endregion
    }

    public static unsafe partial class LuaSTGAPI
    {
        // TODO: pass non-zero arg for callback optimization
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static IntPtr GameObject_New() => api.gameObject_New(0);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static int GameObject_GetID(IntPtr p) => api.gameObject_GetID(p);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static void GameObject_DefaultRenderFunc(IntPtr nativePtr) => api.gameObject_DefaultRenderFunc(nativePtr);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void Del(GameObjectBase gameObject, bool killMode = false)
        {
            api.Del(gameObject._nativePtr, (byte)(killMode ? 1 : 0));
        }

        public static IEnumerable<GameObjectBase> ObjList(long groupID)
        {
            for (var id = FirstObject(groupID); id >= 0; id = NextObject(groupID, id))
            {
                yield return GameObjectBase._id2Obj[id]!;
            }
        }

        private static int FirstObject(long groupID) => api.FirstObject(groupID);
        private static int NextObject(long groupID, int id) => api.NextObject(groupID, id);

        [UnmanagedCallersOnly]
        private static void CreateLuaGameObject(IntPtr p)
        {
            _ = new LuaGameObject(p);
        }

        [UnmanagedCallersOnly]
        private static void DetachGameObject(ulong id)
        {
            GameObjectBase._id2Obj[id]!.Detach();
            GameObjectBase._id2Obj[id] = null;
        }

        [UnmanagedCallersOnly]
        private static void CallOnFrame(ulong id)
        {
            GameObjectBase._id2Obj[id]!.OnFrame();
        }

        [UnmanagedCallersOnly]
        private static void CallOnRender(ulong id)
        {
            GameObjectBase._id2Obj[id]!.OnRender();
        }

        [UnmanagedCallersOnly]
        private static void CallOnDestroy(ulong id, int reason)
        {
            GameObjectBase._id2Obj[id]!.OnDestroy(new((DestroyEventType)reason));
        }

        [UnmanagedCallersOnly]
        private static void CallOnColli(ulong id, ulong otherID)
        {
            var self = GameObjectBase._id2Obj[id]!;
            self.OnColli(new(self, GameObjectBase._id2Obj[otherID]!));
        }
    }
}
