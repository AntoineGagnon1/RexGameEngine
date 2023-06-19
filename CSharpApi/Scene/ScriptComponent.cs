using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace RexEngine
{
    public class ScriptComponent
    {
    }

    [Serializable]
    public class Test1 : ScriptComponent 
    {
        [ShowInEditor] int testInt;
        [ShowInEditor] int testInt2;
    }
    [Serializable]
    public class Test2 : Test1 
    {
        [ShowInEditor] int test3;
    }
}
