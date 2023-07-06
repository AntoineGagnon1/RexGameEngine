using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RexEngine
{
    public class Transform : ScriptComponent
    {
        public int Value = 4;
    }

    public class Test : ScriptComponent
    {
        void OnStart()
        {
            Log.Info("Start");
            Inputs.AddAction("Forward").AddKeyboardBinding(KeyCode.W, KeyCode.S);
        }

        void OnUpdate()
        {
            Log.Info($"{Inputs.GetAction("Forward").IsDown}");
            //var e = new Entity(GUID.Generate());
            //if (!Parent.HasComponent<Test2>())
            //{
            //    Log.Info($"{Parent.AddComponent<Test2>().Value}");
            //    Parent.RemoveComponent<Test2>();
            //}
        }

        void OnDestroy()
        {
            Log.Info("Destroy");
        }
    }
}
