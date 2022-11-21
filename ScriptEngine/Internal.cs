using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace ScriptEngine
{
    public static class Internal
    {
        private static CollectibleAssemblyLoadContext context = new CollectibleAssemblyLoadContext();

        private static Type? TypeByName(string name)
        {
            foreach (var a in context.Assemblies) // Only check in the current context
            {
                var t = a.GetType(name);
                if (t != null)
                    return t;
            }
            
            return null;
        }

        [MethodImpl(MethodImplOptions.NoInlining)] // Really dont want to leak context in another stack (and prevent unloading)
        private static void UnloadContext(out WeakReference contextRef)
        {
            contextRef = new WeakReference(context, true);
            context.Unload();
            context = new CollectibleAssemblyLoadContext();
        }

        // Name is a utf-8 string like : "RexEngine.SomeClass.SomeDelegate"
        // Will return 1 if the call was set and 0 otherwise
        [UnmanagedCallersOnly]
        public static byte SetInternalCall(IntPtr name, IntPtr ptr)
        {
            try
            {
                string? nameStr = Marshal.PtrToStringUTF8(name);
                if (nameStr != null)
                {
                    int seperator = nameStr.LastIndexOf('.');
                    string typeName = nameStr.Substring(0, seperator);
                    string delegateName = nameStr.Substring(seperator + 1);

                    var field = TypeByName(typeName)?.GetRuntimeField(delegateName);

                    if (field != null)
                    {
                        field.SetValue(null, Convert.ChangeType(ptr, field.FieldType));
                        return 1;
                    }
                }
            }
            catch
            {
                return 0;
            }

            return 0;
        }

        // typeName and funcName are utf-8 strings
        [UnmanagedCallersOnly]
        public static unsafe IntPtr GetManagedFunction(IntPtr typeName, IntPtr funcName)
        {
            string? typeStr = Marshal.PtrToStringUTF8(typeName);
            string? funcStr = Marshal.PtrToStringUTF8(funcName);

            if (typeStr != null && funcStr != null)
            {
                foreach (var a in context.Assemblies) // Only check in the current context
                {
                    var type = a.GetType(typeStr);
                    var func = type?.GetMethod(funcStr); 
                    if (type != null && func != null)
                    {
                        return func.MethodHandle.GetFunctionPointer();
                    }
                }
            }

            return IntPtr.Zero;
        }

        // path is a utf-8 string
        // Will return 1 if successful, 0 otherwise
        [UnmanagedCallersOnly]
        public static byte LoadAssembly(IntPtr path)
        {
            string? pathStr = Marshal.PtrToStringUTF8(path);

            if(pathStr != null)
            {
                try
                {
                    using (var fs = new FileStream(Path.GetFullPath(pathStr), FileMode.Open, FileAccess.Read))
                    {
                        context.LoadFromStream(fs);
                    }
                    return 1;
                }
                catch
                {
                    return 0;
                }
            }

            return 0;
        }


        // Unload all the loaded assemblies
        /*
         It looks like the assemblies are still loaded in the AppDomain after this for a while,
         This is why the GetManagedFunction method is needed (nethost will try to find the method in 
         the whole AppDomain and might not get the latest assembly)
         */
        [UnmanagedCallersOnly]
        public static void UnloadAssemblies()
        { // From : https://learn.microsoft.com/en-us/dotnet/standard/assembly/unloadability
            UnloadContext(out var oldContext);

            for (int i = 0; oldContext.IsAlive && (i < 10); i++) // Might need more than 10 ?
            {
                GC.Collect();
                GC.WaitForPendingFinalizers();
            }
        }
    }

    public class CollectibleAssemblyLoadContext : AssemblyLoadContext
    {
        public CollectibleAssemblyLoadContext() : base(isCollectible: true)
        { }

        protected override Assembly? Load(AssemblyName name)
        {
            return null;
        }
    }

}
