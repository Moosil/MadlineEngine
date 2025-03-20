//
// Created by School on 2025/3/10.
//

#ifndef MADLINEENGINE_INPUT_H
#define MADLINEENGINE_INPUT_H

#include <array>

#include <glm.hpp>

#include <GLFW/glfw3.h>

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
			KeyPadEnter,
			KeyPadEqual,
			Apostrophe,
			Comma,
			Minus,
			Period,
			Slash,
			Semicolon,
			Equals,
			LSquareBracket,
			RSquareBracket,
			Backslash,
			GraveAccent,
			World1,
			World2,
			BUTTONS_COUNT, //
		};
		static constexpr int buttonValues[BUTTONS_COUNT] =
	        {
	            GLFW_KEY_A, GLFW_KEY_B, GLFW_KEY_C, GLFW_KEY_D, GLFW_KEY_E, GLFW_KEY_F, GLFW_KEY_G, GLFW_KEY_H,
	            GLFW_KEY_I, GLFW_KEY_J, GLFW_KEY_K, GLFW_KEY_L, GLFW_KEY_M, GLFW_KEY_N, GLFW_KEY_O,
	            GLFW_KEY_P, GLFW_KEY_Q, GLFW_KEY_R, GLFW_KEY_S, GLFW_KEY_T, GLFW_KEY_U, GLFW_KEY_V,
	            GLFW_KEY_W, GLFW_KEY_X, GLFW_KEY_Y, GLFW_KEY_Z, GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2,
	            GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9,
	            GLFW_KEY_SPACE, GLFW_KEY_ENTER, GLFW_KEY_ESCAPE, GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT,
	            GLFW_KEY_RIGHT, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_RIGHT_SHIFT, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_RIGHT_CONTROL,
	            GLFW_KEY_LEFT_ALT, GLFW_KEY_RIGHT_ALT, GLFW_KEY_LEFT_SUPER, GLFW_KEY_RIGHT_SUPER, GLFW_KEY_BACKSPACE,
	            GLFW_KEY_TAB, GLFW_KEY_INSERT, GLFW_KEY_DELETE, GLFW_KEY_CAPS_LOCK, GLFW_KEY_SCROLL_LOCK,
	            GLFW_KEY_NUM_LOCK, GLFW_KEY_PAGE_UP, GLFW_KEY_PAGE_DOWN, GLFW_KEY_HOME, GLFW_KEY_END, GLFW_KEY_MENU,
	            GLFW_KEY_PRINT_SCREEN, GLFW_KEY_PAUSE, GLFW_KEY_F1, GLFW_KEY_F2, GLFW_KEY_F3, GLFW_KEY_F4,
	            GLFW_KEY_F5, GLFW_KEY_F6, GLFW_KEY_F7, GLFW_KEY_F8, GLFW_KEY_F9, GLFW_KEY_F10, GLFW_KEY_F11,
	            GLFW_KEY_F12, GLFW_KEY_F13, GLFW_KEY_F14, GLFW_KEY_F15, GLFW_KEY_F16, GLFW_KEY_F17, GLFW_KEY_F18,
	            GLFW_KEY_F19, GLFW_KEY_F20, GLFW_KEY_F21, GLFW_KEY_F22, GLFW_KEY_F2, GLFW_KEY_F24, GLFW_KEY_F25,
	            GLFW_KEY_KeyPad_0, GLFW_KEY_KeyPad_1, GLFW_KEY_KeyPad_2, GLFW_KEY_KeyPad_3, GLFW_KEY_KeyPad_4, GLFW_KEY_KeyPad_5,
	            GLFW_KEY_KeyPad_6, GLFW_KEY_KeyPad_7, GLFW_KEY_KeyPad_8, GLFW_KEY_KeyPad_9, GLFW_KEY_KeyPad_DECIMAL, GLFW_KEY_KeyPad_DIVIDE,
	            GLFW_KEY_KeyPad_MULTIPLY, GLFW_KEY_KeyPad_SUBTRACT, GLFW_KEY_KeyPad_ADD, GLFW_KEY_KeyPad_ENTER,
	            GLFW_KEY_APOSTROPHE, GLFW_KEY_COMMA, GLFW_KEY_MINUS, GLFW_KEY_PERIOD, GLFW_KEY_SLASH, GLFW_KEY_SEMICOLON,
	            GLFW_KEY_EQUAL, GLFW_KEY_LEFT_BRACKET, GLFW_KEY_RIGHT_BRACKET, GLFW_KEY_BACKSLASH, GLFW_KEY_GRAVE_ACCENT,
	            GLFW_KEY_WORLD_1, GLFW_KEY_WORLD_2
		};
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
