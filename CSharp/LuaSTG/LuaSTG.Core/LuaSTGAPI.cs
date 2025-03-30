using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Text;
using System.Threading.Tasks;

namespace LuaSTG.Core
{
    public static unsafe partial class LuaSTGAPI
    {
        private static ILuaSTGApp? app;

        [UnmanagedCallersOnly]
        private static int StartUp(UnmanagedAPI* unmanagedAPI, ManagedAPI* managedAPI)
        {
            try
            {
                SaveUnmanagedAPI(unmanagedAPI);
                AssignManagedAPI(managedAPI);
                LoadAppAssembly();

                return 0;
            }
            catch (Exception)
            {
                return 1;
            }
        }

        private static void LoadAppAssembly()
        {
            var currAssembly = typeof(LuaSTGAPI).Assembly;
            var dir = Path.GetDirectoryName(currAssembly.Location);
            if (dir == null) return;
            Assembly mainAssembly = AssemblyLoadContext.GetLoadContext(currAssembly)
                ?.LoadFromAssemblyPath(Path.Combine(dir, "LuaSTG.dll")) ?? currAssembly;

            LoadDependencyRecursively(mainAssembly);

            foreach (var asm in AppDomain.CurrentDomain.GetAssemblies())
            {
                var type = asm.GetType("LuaSTG.LuaSTGAppFactory");
                if (type != null)
                {
                    app = (Activator.CreateInstance(type) as ILuaSTGAppFactory)?.Create();
                    break;
                }
            }
        }

        private static void LoadDependencyRecursively(Assembly assembly)
        {
            var resolver = new AssemblyDependencyResolver(assembly.Location);
            foreach (var an in assembly.GetReferencedAssemblies())
            {
                if (an.Name?.StartsWith("LuaSTG") ?? false)
                {
                    Assembly loaded;
                    var path = resolver.ResolveAssemblyToPath(an);
                    if (path != null)
                    {
                        loaded = Assembly.LoadFrom(path);
                    }
                    else
                    {
                        loaded = Assembly.Load(an);
                    }
                    LoadDependencyRecursively(loaded);
                }
            }
        }
    }
}
