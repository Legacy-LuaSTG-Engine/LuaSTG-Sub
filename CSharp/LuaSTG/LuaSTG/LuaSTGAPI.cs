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
        public static delegate* unmanaged<void> log;

        public delegate void GameCallback();

        [UnmanagedCallersOnly]
        public static void StartUp(InitPayload initPayload)
        {
            log = initPayload.log;

            Console.WriteLine("From CLR");

            Log();
        }

        public static void GameInit()
        {

        }

        public static void Log() => log();
    }
}
