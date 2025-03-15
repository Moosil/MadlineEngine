//
// Created by School on 2025/3/12.
//

#include "windows/game_window.h"

Game::Window::Window(int minFps): minFps(minFps), screenRect(Rect2<int>()) {
	WNDCLASS wc = {sizeof(WNDCLASS)};
	const std::string className = CLASS_NAME;
	
	if (!GetClassInfo(GetModuleHandle(nullptr), className.c_str(), &wc)) {
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hInstance = GetModuleHandle(nullptr);
		wc.lpszClassName = className.c_str();
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = &staticWindowProc;

		assert(RegisterClass(&wc));
	}

	DEVMODEA devMode = {};
	devMode.dmSize = sizeof(DEVMODEA);
	
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
	
    m_hwnd = CreateWindowEx(
        WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        wc.lpszClassName,
        WINDOW_NAME,
        WS_POPUP | WS_VISIBLE,
        0,
        0,
        static_cast<int>(devMode.dmPelsWidth),
        static_cast<int>(devMode.dmPelsHeight),
        nullptr, nullptr, GetModuleHandle(nullptr), this
    );
	
	assert(m_hwnd);
	
	lastFrameTime = std::chrono::high_resolution_clock::now();
}

LRESULT CALLBACK Game::Window::staticWindowProc(HWND p_hwnd, unsigned int msg, WPARAM wp, LPARAM lp) {
	Window* self;
	
	if (msg == WM_NCCREATE)	{
		auto *cs = (CREATESTRUCT*) lp;
		self = static_cast<Window*>(cs->lpCreateParams);
		self->m_hwnd = p_hwnd;
		SetLastError(0);
		if (SetWindowLongPtr(p_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self)) == 0)
		{
			if (GetLastError() != 0)
				return false;
		}
	}
	else
	{
		self = reinterpret_cast<Window*>(GetWindowLongPtr(p_hwnd, GWLP_USERDATA));
	}

	if (self)
		return self->windowProc(msg, wp, lp);

	return DefWindowProc(p_hwnd, msg, wp, lp);
}

LRESULT Game::Window::windowProc(unsigned int msg, WPARAM wp, LPARAM lp) {
	LRESULT rez = 0;
	
	bool pressed = false;

	switch (msg) {

		case WM_CLOSE: {
			running = false;
			break;
		}

		case WM_LBUTTONDOWN: {
			processEventButton(input.lmb, true);
			break;
		}

		case WM_LBUTTONUP: {
			processEventButton(input.lmb, false);
			break;
		}

		case WM_RBUTTONDOWN: {
			processEventButton(input.rmb, true);
			break;
		}

		case WM_RBUTTONUP: {
			processEventButton(input.rmb, false);
			break;
		}

		case WM_SYSKEYDOWN: case WM_KEYDOWN:
			pressed = true;
		case WM_SYSKEYUP: case WM_KEYUP: {
			bool altPressed = lp & (1 << 29);

			for (int i = 0; i < Button::BUTTONS_COUNT; i++) {
				if (wp == Button::buttonValues[i]) {
					processEventButton(input.keyBoard[i], pressed);
					input.keyBoard[i].altPressed = altPressed;
				}
			}

			// so alt + f4 works
			rez = DefWindowProc(m_hwnd, msg, wp, lp);
		} break;

		case WM_SETFOCUS: {
			set_focused(true);
			break;
		}

		case WM_KILLFOCUS: {
			set_focused(false);
			break;
		}
		
		case WM_WINDOWPOSCHANGING: {
			auto *pos = (WINDOWPOS *) lp;

			if (pos->x == -32000) {
				// Set the flags to prevent this and "survive" to the desktop toggle
				pos->flags |= SWP_NOMOVE | SWP_NOSIZE;
			}

			pos->hwndInsertAfter = HWND_BOTTOM;
			break;
		}

		case WM_NCHITTEST: {
			rez = HTNOWHERE;
			break;
		}
		
        default: {
			rez = DefWindowProc(m_hwnd, msg, wp, lp);
			break;
		}
	}
	return rez;
}

float Game::Window::getDeltaTime() {
	auto now = std::chrono::high_resolution_clock::now();
	
	long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now - lastFrameTime).count();
	float rez = (float) microseconds / 1000000.0f;
	lastFrameTime = std::chrono::high_resolution_clock::now();
	return rez;
}

void Game::Window::gameLoop() {
	MSG msg = {};
	while (PeekMessage(&msg, m_hwnd, 0, 0, PM_REMOVE) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(m_hwnd, &point);
	input.cursorPos = Game::vec2iFromPoint(point);

	RECT rect = {};
	GetWindowRect(m_hwnd, &rect);
	screenRect = rect;
}


int Game::Window::getMinFps() const {
	return minFps;
}

bool Game::Window::is_running() const {
	return running;
}
void Game::Window::stop_running() {
	running = false;
}

Game::Input *Game::Window::getInput() {
	return &input;
}

void Game::Window::set_focused(bool val) {
	input.focused = val;
}

bool Game::Window::is_focused() const {
	return input.focused;
}

bool Game::Window::is_button_pressed(int buttonIndex) const {
	return input.keyBoard[buttonIndex].pressed;
}

bool Game::Window::is_button_triggered(int buttonIndex) const {
	return input.keyBoard[buttonIndex].triggered;
}

bool Game::Window::is_button_held(int buttonIndex) const {
	return input.keyBoard[buttonIndex].held;
}

bool Game::Window::is_button_released(int buttonIndex) const {
	return input.keyBoard[buttonIndex].released;
}

bool  Game::Window::is_lmb_pressed() const {
	return input.lmb.pressed;
}

bool  Game::Window::is_lmb_triggered() const {
	return input.lmb.pressed;
}

bool  Game::Window::is_lmb_held() const {
	return input.lmb.pressed;
}

bool  Game::Window::is_lmb_released() const {
	return input.lmb.pressed;
}

bool  Game::Window::is_rmb_pressed() const {
	return input.rmb.pressed;
}

bool  Game::Window::is_rmb_triggered() const {
	return input.rmb.pressed;
}

bool  Game::Window::is_rmb_held() const {
	return input.rmb.pressed;
}

bool  Game::Window::is_rmb_released() const {
	return input.rmb.pressed;
}

std::vector<int> Game::Window::get_buttons_pressed() const {
	std::vector<int> pressed;
	for (int i = 0; i < Button::BUTTONS_COUNT; i++) {
		if (input.keyBoard[i].pressed) {
			pressed.push_back(i);
		}
	}
	return pressed;
}

std::vector<int> Game::Window::get_buttons_triggered() const {
	std::vector<int> triggered;
	for (int i = 0; i < Button::BUTTONS_COUNT; i++) {
		if (input.keyBoard[i].triggered) {
			triggered.push_back(i);
		}
	}
	return triggered;
}

std::vector<int> Game::Window::get_buttons_held() const {
	std::vector<int> held;
	for (int i = 0; i < Button::BUTTONS_COUNT; i++) {
		if (input.keyBoard[i].held) {
			held.push_back(i);
		}
	}
	return held;
}

std::vector<int> Game::Window::get_buttons_released() const {
	std::vector<int> released;
	for (int i = 0; i < Button::BUTTONS_COUNT; i++) {
		if (input.keyBoard[i].released) {
			released.push_back(i);
		}
	}
	return released;
}