using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace RexEngine
{
    public class Transform : ScriptComponent
    {
        public Vector3 Position 
        {
            get => GetPos(Owner.Guid);
            set => SetPos(Owner.Guid, value);
        }
        public Vector3 GlobalPosition => GetGlobalPos(Owner.Guid);

        public Vector3 Scale
        {
            get => GetScale(Owner.Guid);
            set => SetScale(Owner.Guid, value);
        }

        public Quaternion Rotation
        {
            get => GetRotation(Owner.Guid);
            set => SetRotation(Owner.Guid, value);
        }
        public Quaternion GlobalRotation => GetGlobalRotation(Owner.Guid);

        public Entity Parent
        {
            get => new Entity(GetParent(Owner.Guid));
            set => SetParent(Owner.Guid, value is null ? GUID.Empty : value.Guid);
        }

        public Vector3 Forward => Vector3.Transform(Directions.Forward, Rotation);
        public Vector3 GlobalForward => Vector3.Transform(Directions.Forward, GlobalRotation);
        public Vector3 Right => Vector3.Transform(Directions.Up, Rotation);
        public Vector3 GlobalRight => Vector3.Transform(Directions.Up, GlobalRotation);
        public Vector3 Up => Vector3.Transform(Directions.Right, Rotation);
        public Vector3 GlobalUp => Vector3.Transform(Directions.Right, GlobalRotation);

        [MethodImpl(MethodImplOptions.InternalCall)] static extern public Vector3 GetPos(GUID ownerGuid);
        [MethodImpl(MethodImplOptions.InternalCall)] static extern public Vector3 GetGlobalPos(GUID ownerGuid);
        [MethodImpl(MethodImplOptions.InternalCall)] static extern private void SetPos(GUID ownerGuid, Vector3 pos);
        [MethodImpl(MethodImplOptions.InternalCall)] static extern public Vector3 GetScale(GUID ownerGuid);
        [MethodImpl(MethodImplOptions.InternalCall)] static extern private void SetScale(GUID ownerGuid, Vector3 scale);
        [MethodImpl(MethodImplOptions.InternalCall)] static extern public Quaternion GetRotation(GUID ownerGuid);
        [MethodImpl(MethodImplOptions.InternalCall)] static extern public Quaternion GetGlobalRotation(GUID ownerGuid);
        [MethodImpl(MethodImplOptions.InternalCall)] static extern private void SetRotation(GUID ownerGuid, Quaternion rotation);
        
        [MethodImpl(MethodImplOptions.InternalCall)] static extern private GUID GetParent(GUID ownerGuid);
        [MethodImpl(MethodImplOptions.InternalCall)] static extern private void SetParent(GUID ownerGuid, GUID newParent);
    }

    public class Test : ScriptComponent
    {
        void OnStart()
        {
            Log.Info($"{Owner.Transform.Parent.Name}");
            Owner.Transform.Parent = null;
        }

        void OnUpdate()
        {
            Owner.Transform.Position = new Vector3(10, 10, 10);
            Owner.Transform.Rotation *= Quaternion.CreateFromAxisAngle(Vector3.UnitY, 0.45f * Time.DeltaTime);
        }

        void OnDestroy()
        {
            Log.Info("Destroy");
        }
    }
}
