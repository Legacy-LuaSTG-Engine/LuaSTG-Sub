using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    /// <summary>
    /// Represents a factory creating a LuaSTG application.
    /// </summary>
    /// <remarks>
    /// After finished loading assemblies, launching procedure will try to find 
    /// a type with full name "LuaSTG.LuaSTGAppFactory".
    /// if the type is found, it will try to create an object with the parameterless constructor
    /// and invoke <see cref="GetApplication"/> once to obtain an application.
    /// The application object obtained will be a singleton.
    /// </remarks>
    public interface ILuaSTGAppFactory
    {
        /// <summary>
        /// Get an application instance.
        /// </summary>
        /// <returns>An application instance if success, otherwise null.</returns>
        ILuaSTGApp? GetApplication();
    }
}
