#pragma once

#include "Input.h"

namespace RexEngine
{
	enum class KeyCode
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
	};

	class KeyboardInput final : public Input
	{
	public:
		KeyboardInput(KeyCode positive, KeyCode negative = KeyCode::None);

		void PollInputs() override;

	private:
		KeyCode m_positive;
		KeyCode m_negative;
	};
}