using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using LuaSTG.Core;

namespace LuaSTG
{
    public class LuaSTGAppFactory : ILuaSTGAppFactory
    {
        public ILuaSTGApp Create()
        {
            return new LuaSTGApp();
        }
    }
}
