using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace RexEngine
{
    public class ScriptComponent
    {
        public Entity Parent { get; set; } = null;

        
        // Child classes can have :
        // void OnUpdate()  // Called every frame
        // void OnStart()   // Called when the script is created
        // Void OnDestroy() // Called when the script is destroyed 

        // Will be called from c++
        private void SetParent(GUID parent)
        {
            Parent = new Entity(parent);
        }
    }

    public class Transform : ScriptComponent
    {
        public int Value = 4;
    }

    public class Test2 : ScriptComponent
    {
        public int Value = 14;
    }

    public class Test : ScriptComponent
    {
        void OnStart()
        {
            Log.Info("Start");
        }

        void OnUpdate()
        {
            var e = new Entity(GUID.Generate());
            if (!Parent.HasComponent<Test2>())
            {
                Log.Info($"{Parent.AddComponent<Test2>().Value}");
                Parent.RemoveComponent<Test2>();
            }
        }

        void OnDestroy()
        {
            Log.Info("Destroy");
        }
    }
}
