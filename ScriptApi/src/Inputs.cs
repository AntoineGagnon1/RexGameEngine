using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RexEngine
{
    // TODO : Change this once loading inputs from file is implemented
    public static class Inputs
    {
        private static Dictionary<string, Action> m_actions = new();

        public static Action GetAction(string name)
        {
            if (m_actions.ContainsKey(name))
                return m_actions[name];
            else
            {
                Core.LogError($"No Action named {name} found !");
                return new Action(false, false, false, 0.0f);
            }
        }

        // name is a utf-8 string
        // bools : bit 0 = IsDown, bit 1 = IsJustDown, bit 2 = IsJustUp
        [UnmanagedCallersOnly]
        public static void SetActionData(IntPtr name, byte bools, float value)
        {
            string? nameStr = Marshal.PtrToStringUTF8(name);

            if(nameStr != null)
            {
                bool down = ((bools >> 0) & 1) > 0;
                bool justDown = ((bools >> 1) & 1) > 0;
                bool justUp = ((bools >> 2) & 1) > 0;

                Action action = new Action(down, justDown, justUp, value);
                m_actions[nameStr] = action;

            }
        }
    }

    public class Action
    {
        public bool IsDown { get; private set; }
        public bool IsJustDown { get; private set; }
        public bool IsJustUp { get; private set; }

        public float Value { get; private set; }

        internal Action(bool down, bool justDown, bool justUp, float value)
        {
            IsDown = down;
            IsJustDown = justDown;
            IsJustUp = justUp;
            Value = value;
        }
    }
}
