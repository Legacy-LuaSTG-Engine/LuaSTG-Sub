using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    public interface ILuaSTGAppFactory
    {
        ILuaSTGApp Create();
    }
}
