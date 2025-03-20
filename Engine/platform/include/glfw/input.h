//
// Created by School on 2025/3/10.
//

#ifndef CELESTEPET_INPUT_H
#define CELESTEPET_INPUT_H

#include <array>

#include <glm.hpp>

#ifdef _WIN32
#include <Windows.h>
#endif//_WIN32

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
			Shift,
			BackSpace, //next line
			PlusEqual,
			PeriodRightArrow,
			MinusUnderscore,
			CommaLeftArrow,
			SemiColon,
			QuestionBackSlash,
			Tilde,
			Quotes,
			Slash,
			SquareBracketsOpen,
			SquareBracketsClose,
			BUTTONS_COUNT, //
		};
		static constexpr int buttonValues[BUTTONS_COUNT] =
	    {
	        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
	        'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	        VK_SPACE, VK_RETURN, VK_ESCAPE, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_SHIFT,
	        VK_BACK, VK_OEM_PLUS, VK_OEM_PERIOD, VK_OEM_MINUS, VK_OEM_COMMA, VK_OEM_1, VK_OEM_2, VK_OEM_3,
	        VK_OEM_7, VK_OEM_5, VK_OEM_4, VK_OEM_6,
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

#endif//CELESTEPET_INPUT_H
