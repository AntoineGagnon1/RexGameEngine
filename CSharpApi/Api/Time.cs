using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace RexEngine
{
    public static class Time
    {
        // In seconds
        public static float DeltaTime => GetDeltaTime();
        // Time since the start of the app, in seconds
        public static double CurrentTime => GetCurrentTime();

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float GetDeltaTime();
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern double GetCurrentTime();
    }
}
