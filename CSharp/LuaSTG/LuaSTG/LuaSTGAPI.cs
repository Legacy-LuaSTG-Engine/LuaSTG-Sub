using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG
{
    public static unsafe class LuaSTGAPI
    {
        private static UnmanagedAPI api;

        [UnmanagedCallersOnly]
        public static int StartUp(UnmanagedAPI* unmanagedAPI, ManagedAPI* managedAPI)
        {
            try
            {
                api = *unmanagedAPI;

                managedAPI->GameInit = &GameInit;
                managedAPI->FrameFunc = &FrameFunc;
                managedAPI->RenderFunc = &RenderFunc;
                managedAPI->GameExit = &GameExit;
                managedAPI->FocusGainFunc = &FocusGainFunc;
                managedAPI->FocusLoseFunc = &FocusLoseFunc;

                return 0;
            }
            catch (Exception)
            {
                return 1;
            }
        }

        [UnmanagedCallersOnly]
        public unsafe static void GameInit()
        {

        }

        static long time = 0;
        private const double PI_3_2 = Math.PI / 3 * 2;
        private const double DEG2RAD = Math.PI / 180;

        [UnmanagedCallersOnly]
        public unsafe static byte FrameFunc()
        {
            time++;
            return 0;
        }

        [UnmanagedCallersOnly]
        public unsafe static void RenderFunc()
        {
            //var rad = time * DEG2RAD;
            //BeginScene();
            //RenderClear(255,
            //    Convert.ToByte(127.0f + 127.0f * Math.Cos(rad)),
            //    Convert.ToByte(127.0f + 127.0f * Math.Cos(rad + PI_3_2)),
            //    Convert.ToByte(127.0f + 127.0f * Math.Cos(rad + PI_3_2 * 2)));
            //EndScene();
        }

        [UnmanagedCallersOnly]
        public unsafe static void GameExit()
        {

        }

        [UnmanagedCallersOnly]
        public unsafe static void FocusGainFunc()
        {

        }

        [UnmanagedCallersOnly]
        public unsafe static void FocusLoseFunc()
        {

        }

        public static void BeginScene() => api.beginScene();
        public static void EndScene() => api.endScene();
        public static void RenderClear(byte a, byte r, byte g, byte b) => api.renderClear(a, r, g, b);
    }
}
