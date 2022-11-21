using System;
using System.Collections.Generic;
using System.IO.Pipes;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RexEngine
{

    public unsafe static class Test
    {
        public static delegate* unmanaged[Stdcall]<int, GUID, void> TestFuncCpp;
        
        [UnmanagedCallersOnly]
        public static void TestFunc()
        {
            //Core.Assert(false, "Test assert");
            Core.LogInfo(GUID.Generate().ToString());
            Core.LogWarning(Time.DeltaTime.ToString());
            Core.LogError("Hello error");
            TestFuncCpp(15, GUID.Generate());
            Console.WriteLine($"Hello from test5 {((IntPtr)TestFuncCpp).ToString()}");
        }
    }
}
