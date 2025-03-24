//
// Created by School on 2025/3/10.
//

#ifndef MADLINEENGINE_INPUT_H
#define MADLINEENGINE_INPUT_H

#include <array>

#include <glm.hpp>

#include <Windows.h>

namespace Madline {
	struct Button
	{
		unsigned char pressed = 0;
		unsigned char triggered = 0;
		unsigned char held = 0;
		unsigned char released = 0;
		unsigned char altPressed = 0;
		enum {
			A = 0,
			B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
			Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
			Space,
			Enter,
			Escape,
			Up,
			Down,
			Left,
			Right,
			LShift,
			RShift,
			LControl,
			RControl,
			LAlt,
			RAlt,
			LSuper,
			RSuper,
			BackSpace,
			Tab,
			Insert,
			Delete,
			CapsLock,
			ScrollLock,
			NumLock,
			PageUp,
			PageDown,
			Home,
			End,
			Menu,
			PrintScreen,
			Pause,
			F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17,
			F18, F19, F20, F21, F22, F23, F24, F25,
			KeyPad0, KeyPad1, KeyPad2, KeyPad3, KeyPad, KeyPad5, KeyPad6, KeyPad7, KeyPad8, KeyPad9,
			KeyPadDecimal,
			KeyPadDivide,
			KeyPadMultiply,
			KeyPadSubtract,
			KeyPadAdd,
			//KeyPadEnter, not a thing key 0x1E
			KeyPadEqual,
			Apostrophe,
			Comma,
			Plus,
			Minus,
			Period,
			Slash,
			Semicolon,
			LSquareBracket,
			RSquareBracket,
			Backslash,
			GraveAccent,
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCInconsistentNamingInspection"
			BUTTONS_COUNT, //
#pragma clang diagnostic pop
		};
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCInconsistentNamingInspection"
		static constexpr int buttonValues[BUTTONS_COUNT] =
	        {
	            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	            'I', 'J', 'K', 'L', 'M', 'N', 'O',
	            'P', 'Q', 'R', 'S', 'T', 'U', 'V',
	            'W', 'X', 'Y', 'Z', '0', '1', '2',
	            '3', '4', '5', '6', '7', '8', '9',
	            VK_SPACE, VK_RETURN, VK_ESCAPE, VK_UP, VK_DOWN, VK_LEFT,
	            VK_RIGHT, VK_LSHIFT, VK_RSHIFT, VK_LCONTROL, VK_RCONTROL,
                VK_LMENU, VK_RMENU, VK_LWIN, VK_RWIN, VK_BACK,
	            VK_TAB, VK_INSERT, VK_DELETE, VK_CAPITAL, VK_SCROLL,
                VK_NUMLOCK, VK_PRIOR, VK_NEXT, VK_HOME, VK_END, VK_MENU,
                VK_SNAPSHOT, VK_PAUSE, VK_F1, VK_F2, VK_F3, VK_F4,
	            VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11,
	            VK_F12, VK_F13, VK_F14, VK_F15, VK_F16, VK_F17, VK_F18,
	            VK_F19, VK_F20, VK_F21, VK_F22, VK_F2, VK_F24,
	            VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4, VK_NUMPAD5,
	            VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9, VK_DECIMAL, VK_DIVIDE,
	            VK_MULTIPLY, VK_SUBTRACT, VK_ADD,
                VK_OEM_7, VK_OEM_COMMA, VK_OEM_PLUS, VK_OEM_MINUS, VK_OEM_PERIOD, VK_OEM_2,
                VK_OEM_1, VK_OEM_4, VK_OEM_6, VK_OEM_5, VK_OEM_3
		};
#pragma clang diagnostic pop
	};
	
	struct Input {
		std::array<Button, Button::BUTTONS_COUNT> keyBoard;
		
		glm::vec<2, int> cursorPos = glm::vec<2, int>(0,0);
		
		Button lmb = {};
		Button rmb = {};
		
		bool focused = false;
	};
	void resetInput(Input *input);
	
	void processInputAfter(Input *input);
	
	void processEventButton(Button &b, bool newState);
}

#endif//MADLINEENGINE_INPUT_H
