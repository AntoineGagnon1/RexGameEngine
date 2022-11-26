using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

using RexEngine;

namespace RexEditor
{
    public static class Class1
    {
        [UnmanagedCallersOnly]
        public static void Test(GUID entity)
        {
            Entity e = new(entity);
            Core.LogInfo(e.HasComponent<TransformComponent>().ToString());
        }
    }
}
