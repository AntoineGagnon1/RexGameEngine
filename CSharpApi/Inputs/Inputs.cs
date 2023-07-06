using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RexEngine
{
    public enum KeyCode
    {
        None,
		Space,             
		Apostrophe, /* ' */
		Comma, /* , */
		Minus, /* - */
		Period, /* . */
		Slash, /* / */
		Number0, Number1, Number2, Number3, Number4, Number5, Number6, Number7, Number8, Number9,                 
		SemiColon, /* ; */
		Equal, /* = */
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,                  
		LeftBracket, /* [ */
		BackSlash, /* \ */
		RightBracket, /* ] */
		GraveAccent, /* ` */
		Escape, Enter, Tab, Backspace, Insert, Delete,            
		Right, Left, Down, Up,                 
		PageUp, PageDown,          
		Home,  
		End,                
		CapsLock, ScrollLock, NumLock,           
		PrintScreen,       
		Pause,              
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,
		Keypad0, Keypad1, Keypad2, Keypad3, Keypad4, Keypad5, Keypad6, Keypad7, Keypad8, Keypad9,
		KeypadDecimal, KeypadDivide, KeypadMultiply, KeypadSubtract, KeypadAdd, KeypadEnter, KeypadEqual,          
		LeftShift, LeftControl, LeftAlt, LeftSuper,
		RightShift, RightControl, RightAlt, RightSuper,        
		Menu
    }

    public enum MouseButton { Right, Left, Middle, Back, Front, None }

    public enum MouseInputType { X, Y, DeltaX, DeltaY, RawX, RawY }

    public struct Action
    {
        public string Name { get; private set; }

        public bool IsDown => ActionIsDown(Name);
        public bool IsJustDown => ActionIsJustDown(Name);
        public bool IsJustUp => ActionIsJustUp(Name);
        public float Value => ActionValue(Name);

        public void AddKeyboardBinding(KeyCode positive, KeyCode negative = KeyCode.None) => ActionAddKeyboardBinding(Name, (int)positive, (int)negative);
        public void AddMouseButtonBinding(MouseButton positive, MouseButton negative = MouseButton.None) => ActionAddMouseButtonBinding(Name, (int)positive, (int)negative);
        public void AddMouseBinding(MouseInputType type) => ActionAddMouseBinding(Name, (int)type);

        internal Action(string name)
        {
            Name = name;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern private bool ActionIsDown(string name);
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern private bool ActionIsJustDown(string name);
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern private bool ActionIsJustUp(string name);
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern private float ActionValue(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern private void ActionAddKeyboardBinding(string actionName, int positive, int negative);
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern private void ActionAddMouseButtonBinding(string actionName, int positive, int negative);
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern private void ActionAddMouseBinding(string actionName, int type);
    }

    public static class Inputs
    {
        static public Action AddAction(string name)
        {
            AddActionInternal(name);
            return new Action(name);
        }

        static public Action GetAction(string name) => new Action(name);


        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern private void AddActionInternal(string name);
    }

    public enum CursorMode { Free, Locked }

    public static class Cursor
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern public void SetCursorMode(CursorMode mode);
    }
}
