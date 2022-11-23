using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RexEngine
{
    public unsafe static class SceneCalls
    {
        public static delegate* unmanaged[Stdcall]<GUID, byte> IsEntityValid;
    }

    public class Entity : IEquatable<Entity>
    {
        public GUID Guid { get; private set; }

        public Entity(GUID guid)
        {
            Guid = guid;
        }

        public override bool Equals(object? obj) => obj is Entity other && this.Equals(other);
        public bool Equals(Entity? other)
        {
            if(ReferenceEquals(other, null)) // Special case, check if the object is valid
            {
                unsafe
                {
                    return SceneCalls.IsEntityValid(Guid) == 0; // return true if not valid
                }
            }

            return Guid == other.Guid;
        }
        public override int GetHashCode() => Guid.GetHashCode();

        public static bool operator ==(Entity? lhs, Entity? rhs)
        {
            if (ReferenceEquals(lhs, null))
            {
                if (ReferenceEquals(rhs, null))
                    return true; // Both null

                return false;
            }

            return lhs.Equals(rhs);
        }
        public static bool operator !=(Entity? lhs, Entity? rhs) => !(lhs == rhs);
    }
}
