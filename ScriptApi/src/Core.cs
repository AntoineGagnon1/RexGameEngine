using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net.Http.Headers;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RexEngine
{
    public unsafe class CoreCalls
    {
        public static delegate* unmanaged[Stdcall]<bool, string, void> Assert;
        public static delegate* unmanaged[Stdcall]<string, string, byte, void> Log; // callerInfo, msg, level

        public static delegate* unmanaged[Stdcall]<GUID> GenGuid; // Generate a guid
        public static delegate* unmanaged[Stdcall]<GUID, StringBuilder, void> GuidToString; // Generate a guid

    }

    public unsafe class Core
    {
        public static void Assert(bool condition, string message)
        {
            CoreCalls.Assert(condition, message);
        }

        public static void LogInfo(string message, [CallerFilePath] string? path = null, [CallerMemberName] string? member = null, [CallerLineNumber] int line = 0)
        {
            CoreCalls.Log($"[{Path.GetFileName(path)}:{member}:{line}]", message, 0);
        }
        public static void LogWarning(string message, [CallerFilePath] string? path = null, [CallerMemberName] string? member = null, [CallerLineNumber] int line = 0)
        {
            CoreCalls.Log($"[{Path.GetFileName(path)}:{member}:{line}]", message, 1);
        }
        public static void LogError(string message, [CallerFilePath] string? path = null, [CallerMemberName] string? member = null, [CallerLineNumber] int line = 0)
        {
            CoreCalls.Log($"[{Path.GetFileName(path)}:{member}:{line}]", message, 2);
        }
    }

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

        public unsafe static GUID Generate()
        {
            return CoreCalls.GenGuid();
        }

        public unsafe override string ToString()
        {
            StringBuilder sb = new StringBuilder(36);
            CoreCalls.GuidToString(this, sb);
            return sb.ToString();
        }

        public override bool Equals(object? obj) => obj is GUID other && this.Equals(other);
        public bool Equals(GUID p) => dataHigh == p.dataHigh && dataLow == p.dataLow;
        public override int GetHashCode() => (dataHigh, dataLow).GetHashCode();

        public static bool operator ==(GUID lhs, GUID rhs) => lhs.Equals(rhs);
        public static bool operator !=(GUID lhs, GUID rhs) => !(lhs == rhs);


        readonly static GUID Empty = new GUID(0,0);
    }

    public static class Time
    {
        public static float DeltaTime { get; private set; }

        [UnmanagedCallersOnly]
        public static void SetDeltaTime(float deltaTime)
        {
            DeltaTime = deltaTime;
        }
    }
}
