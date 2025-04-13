using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    /// <summary>
    /// Stores a collision info.
    /// </summary>
    /// <param name="Self">The object which has group of first argument in CheckCollision.</param>
    /// <param name="Other">The object which has group of second argument in CheckCollision.</param>
    public record struct Collision(GameObjectBase Self, GameObjectBase Other)
    {
    }
}
