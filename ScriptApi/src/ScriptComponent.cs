using Newtonsoft.Json;
using RexEngine;
using System;
using System.Collections.Generic;
using System.Data.Common;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace RexEngine
{
    // All user scripts need to implement this to work with the ECS
    public class ScriptComponent
    {
        public GUID Guid { get; private set; }



        public virtual void OnUpdate() { }
    }

    public static unsafe class ScriptComponentManager
    {
        // This will be used to return the result of SaveComponentsToString
        public static delegate* unmanaged[Stdcall]<string, void> SaveComponentsCallback;

        [UnmanagedCallersOnly]
        public static unsafe void SaveComponentsToString(GUID* guids, Int32 count)
        {
            List<ScriptComponent> toSerialize = new List<ScriptComponent>();
            toSerialize.Capacity = count;
            for(int i = 0; i < count; i++)
            {
                var guid = guids[i];
                if(ScriptComponents.TryGetValue(guid, out ScriptComponent? value))
                    toSerialize.Add(value);
            }

            SaveComponentsCallback(JsonConvert.SerializeObject(toSerialize, Formatting.Indented));
        }

        // Data is the serialized string
        [UnmanagedCallersOnly]
        public static void LoadComponentsFromString(IntPtr dataStr)
        {
            string? data = Marshal.PtrToStringUTF8(dataStr);

            if (data == null)
            {
                Core.LogError("Failed to read the script component serialization string !");
                return;
            }

            List<ScriptComponent>? components = JsonConvert.DeserializeObject<List<ScriptComponent>>(data);

            if (components == null)
            {
                Core.LogError("Failed to Deserialize script components : " + data);
            }
            else
            {
                foreach (var c in components)
                    ScriptComponents[c.Guid] = c;
            }
        }

        [UnmanagedCallersOnly]
        public static unsafe void RemoveComponents(GUID* guids, Int32 count)
        {
            for (int i = 0; i < count; i++)
            {
                ScriptComponents.Remove(guids[i]);
            }
        }

        private static Dictionary<GUID, ScriptComponent> ScriptComponents = new Dictionary<GUID, ScriptComponent>();
    }
}
