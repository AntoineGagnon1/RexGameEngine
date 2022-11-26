using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace RexEngine
{
    public class TransformComponent : IComponent
    {
        public static int GetTypeId() => 0;

        public Vector3 Position { get; set; }
        public Vector3 Scale { get; set; }
        public Quaternion Rotation { get; set; }

        public Entity? Parent { get; set; }
    }
}
