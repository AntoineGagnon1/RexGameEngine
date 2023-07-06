﻿using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RexEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct GUID : IEquatable<GUID>
    {
        private UInt64 dataHigh;
        private UInt64 dataLow;


        public GUID(UInt64 low, UInt64 high)
        {
            dataLow = low;
            dataHigh = high;
        }


        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern public GUID Generate();

        public override string ToString()
        {
            return GuidToString(this);
        }

        public override bool Equals(object obj) => obj is GUID other && this.Equals(other);
        public bool Equals(GUID p) => dataHigh == p.dataHigh && dataLow == p.dataLow;
        public override int GetHashCode() => (dataHigh, dataLow).GetHashCode();

        public static bool operator ==(GUID lhs, GUID rhs) => lhs.Equals(rhs);
        public static bool operator !=(GUID lhs, GUID rhs) => !(lhs == rhs);


        public readonly static GUID Empty = new GUID(0, 0);

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern public string GuidToString(GUID guid);
    }

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

    public static class Directions
    {
        public static Vector3 Up = new Vector3(0, 1, 0);
        public static Vector3 Down = new Vector3(0, -1, 0);

        public static Vector3 Right = new Vector3(1, 0, 0);
        public static Vector3 Left = new Vector3(-1, 0, 0);

        public static Vector3 Forward = new Vector3(0, 0, 1);
        public static Vector3 Backward = new Vector3(0, 0, -1);
    }
}