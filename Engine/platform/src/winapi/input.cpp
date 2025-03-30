//
// Created by School on 2025/3/10.
//

#include "winapi/input.h"



namespace Madline {
	void Input::resetInput() {
		lmb = {};
		rmb = {};
	
		std::fill(keyBoard.begin(), keyBoard.end(), Button());
	}
	
	void Input::clearInput() {
		for (int i = 0; i < Button::BUTTONS_COUNT; i++) {
			keyBoard[i].justPressed = false;
			keyBoard[i].justReleased = false;
			keyBoard[i].triggered = false;
			keyBoard[i].altPressed = false;
		}

		lmb.justPressed = false;
		lmb.triggered = false;
		lmb.justReleased = false;
		lmb.altPressed = false;
		rmb.justPressed = false;
		rmb.triggered = false;
		rmb.justReleased = false;
		lmb.altPressed = false;
	}
	
	void Input::setKeyState(Button::ButtonID id, bool newState) {
		setButtonState(keyBoard[id], newState);
	}
	
	void Input::setMouseButtonState(Button::MouseButton mouseButton, bool newState) {
		Button &b = lmb;
		if (mouseButton == Button::RMB) b = rmb;
		setButtonState(b, newState);
	}
	
	void Input::setButtonState(Button &b, bool newState) {
		if (newState) {
			if (!b.held) {
				b.justPressed = true;
				b.triggered = true;
				b.held = true;
			}
			b.triggered = true;
		} else {
			b.justPressed = false;
			b.triggered = false;
			b.held = false;
			b.justReleased = true;
		}
	}
	void Input::setKeyModifiers(Button::ButtonID id, int mods, int states) {
		if (mods & Button::Modifier::ALT) keyBoard[id].altPressed = ((states & Button::Modifier::ALT) != 0);
	}
}