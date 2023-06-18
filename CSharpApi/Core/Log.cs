using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace RexEngine
{
    public static class Log
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern public void Info(string message, [CallerLineNumber] int line = 0, [CallerMemberName] string memberName = "", [CallerFilePath] string file = "");

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern public void Warning(string message, [CallerLineNumber] int line = 0, [CallerMemberName] string memberName = "", [CallerFilePath] string file = "");

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern public void Error(string message, [CallerLineNumber] int line = 0, [CallerMemberName] string memberName = "", [CallerFilePath] string file = "");


        [Conditional("DEBUG")]
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern public void Debug(string message, [CallerLineNumber] int line = 0, [CallerMemberName] string memberName = "", [CallerFilePath] string file = "");
    }
}
