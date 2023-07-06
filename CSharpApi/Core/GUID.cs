using System;
using System.Collections.Generic;
using System.Linq;
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


        readonly static GUID Empty = new GUID(0, 0);

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern public string GuidToString(GUID guid);
    }

}
