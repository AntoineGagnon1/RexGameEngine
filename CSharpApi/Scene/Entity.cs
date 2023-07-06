using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace RexEngine
{
    public class Entity
    {
        public GUID Guid { get; private set; }

        public bool IsAlive => IsEntityAlive(Guid);
        public string Name => GetEntityName(Guid);

        public Transform Transform => GetComponent<Transform>();

        public bool HasComponent<T>() where T : ScriptComponent => !(EntityGetComponent(Guid, typeof(T).Name) is null);
        public T GetComponent<T>() where T : ScriptComponent => EntityGetComponent(Guid, typeof(T).Name) as T;
        // Will return null if the component was not added
        public T AddComponent<T>() where T : ScriptComponent => EntityAddComponent(Guid, typeof(T).Name) as T;
        public bool RemoveComponent<T>() where T : ScriptComponent => EntityRemoveComponent(Guid, typeof(T).Name);

        internal Entity(GUID guid)
        {
            Guid = guid;
        }

        public static implicit operator bool(Entity self)
        {
            return !(self is null) && self.IsAlive;
        }
        // == with null will check if the entity is alive
        public static bool operator ==(Entity lhs, Entity rhs)
        {
            if (lhs is null && rhs is null)
                return true;
            else if (lhs is null)
                return !rhs.IsAlive;
            else if (rhs is null)
                return !lhs.IsAlive;
            else
                return lhs.Guid == rhs.Guid;
        }
        public static bool operator !=(Entity lhs, Entity rhs) => !(lhs == rhs);
        public override bool Equals(object obj) => obj is Entity entity && this == entity;
        public override int GetHashCode() => Guid.GetHashCode();

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern private bool IsEntityAlive(GUID guid);

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern private string GetEntityName(GUID guid);

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern private ScriptComponent EntityGetComponent(GUID guid, string typeName);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern private ScriptComponent EntityAddComponent(GUID guid, string typeName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern private bool EntityRemoveComponent(GUID guid, string typeName);
    }
}
