//
// Created by School on 2025/3/12.
//

#include <format>

#include <imgui.h>
#include <imgui_impl_win32.h>

#include "CelestePetConsts.h"

#include "glfw/game_window.h"

#ifdef RENDER_VULKAN
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "vulkan/vk_types.h"
#endif


Madline::Window::Window(int minFps): minFps(minFps), screenRect(Rect2<int>()) {
	std::printf("Started Creating Window\n");
	glfwInit();
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	
	window = glfwCreateWindow(mode->width, mode->height, GAME_NAME, monitor, nullptr);
	
	lastFrameTime = std::chrono::high_resolution_clock::now();
	
	screenRect = Rect2<int>(mode->width, mode->height);
	
	std::printf("Finished Creating Window\n");
}

Madline::Window::~Window() {
    DestroyWindow(mHwnd);
	mHwnd = nullptr;
}

LRESULT CALLBACK Madline::Window::staticWindowProc(HWND pHwnd, unsigned int msg, WPARAM wp, LPARAM lp) {
	Window* self;
	
	if (msg == WM_NCCREATE)	{
		auto *cs = (CREATESTRUCT*) lp;
		self = static_cast<Window*>(cs->lpCreateParams);
		self->mHwnd = pHwnd;
		SetLastError(0);
		if (SetWindowLongPtr(pHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self)) == 0)
		{
			if (GetLastError() != 0)
				return false;
		}
	}
	else
	{
		self = reinterpret_cast<Window*>(GetWindowLongPtr(pHwnd, GWLP_USERDATA));
	}

	if (self)
		return self->windowProc(msg, wp, lp);

	return DefWindowProc(pHwnd, msg, wp, lp);
}

LRESULT Madline::Window::windowProc(unsigned int msg, WPARAM wp, LPARAM lp) {
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
			rez = DefWindowProc(mHwnd, msg, wp, lp);
		} break;

		case WM_SETFOCUS: {
			setFocused(true);
			break;
		}

		case WM_KILLFOCUS: {
			setFocused(false);
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
			rez = DefWindowProc(mHwnd, msg, wp, lp);
			break;
		}
	}
	
	return rez;
}

float Madline::Window::getDeltaTime() {
	auto now = std::chrono::high_resolution_clock::now();
	
	long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now - lastFrameTime).count();
	float rez = (float) microseconds / 1000000.0f;
	lastFrameTime = std::chrono::high_resolution_clock::now();
	return rez;
}

void Madline::Window::gameLoop() {
	MSG msg = {};
	while (PeekMessage(&msg, mHwnd, 0, 0, PM_REMOVE) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(mHwnd, &point);
	input.cursorPos = Madline::vec2iFromPoint(point);

	RECT rect = {};
	GetWindowRect(mHwnd, &rect);
	screenRect = static_cast<Rect2<int>>(rect);
}

HWND Madline::Window::getHwnd() const {
	return mHwnd;
}

int Madline::Window::getMinFps() const {
	return minFps;
}

Madline::Rect2<int> Madline::Window::getScreenRect() const {
	return screenRect;
}

bool Madline::Window::isRunning() const {
	return running;
}
void Madline::Window::stopRunning() {
	running = false;
}

Madline::Input *Madline::Window::getInput() {
	return &input;
}

void Madline::Window::setFocused(bool val) {
	input.focused = val;
}

bool Madline::Window::isFocused() const {
	return input.focused;
}

bool Madline::Window::isButtonPressed(int buttonIndex) const {
	return input.keyBoard[buttonIndex].pressed;
}

bool Madline::Window::isButtonTriggered(int buttonIndex) const {
	return input.keyBoard[buttonIndex].triggered;
}

bool Madline::Window::isButtonHeld(int buttonIndex) const {
	return input.keyBoard[buttonIndex].held;
}

bool Madline::Window::isButtonReleased(int buttonIndex) const {
	return input.keyBoard[buttonIndex].released;
}

bool Madline::Window::isLmbPressed() const {
	return input.lmb.pressed;
}

bool Madline::Window::isLmbTriggered() const {
	return input.lmb.pressed;
}

bool Madline::Window::isLmbHeld() const {
	return input.lmb.pressed;
}

bool Madline::Window::isLmbReleased() const {
	return input.lmb.pressed;
}

bool Madline::Window::isRmbPressed() const {
	return input.rmb.pressed;
}

bool Madline::Window::isRmbTriggered() const {
	return input.rmb.pressed;
}

bool Madline::Window::isRmbHeld() const {
	return input.rmb.pressed;
}

bool Madline::Window::isRmbReleased() const {
	return input.rmb.pressed;
}

std::vector<int> Madline::Window::getButtonsPressed() const {
	std::vector<int> pressed;
	for (int i = 0; i < Button::BUTTONS_COUNT; i++) {
		if (input.keyBoard[i].pressed) {
			pressed.push_back(i);
		}
	}
	return pressed;
}

std::vector<int> Madline::Window::getButtonsTriggered() const {
	std::vector<int> triggered;
	for (int i = 0; i < Button::BUTTONS_COUNT; i++) {
		if (input.keyBoard[i].triggered) {
			triggered.push_back(i);
		}
	}
	return triggered;
}

std::vector<int> Madline::Window::getButtonsHeld() const {
	std::vector<int> held;
	for (int i = 0; i < Button::BUTTONS_COUNT; i++) {
		if (input.keyBoard[i].held) {
			held.push_back(i);
		}
	}
	return held;
}

std::vector<int> Madline::Window::getButtonsReleased() const {
	std::vector<int> released;
	for (int i = 0; i < Button::BUTTONS_COUNT; i++) {
		if (input.keyBoard[i].released) {
			released.push_back(i);
		}
	}
	return released;
}

#ifdef RENDER_VULKAN
void Madline::Window::getVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) const {
	
	
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hwnd = mHwnd;
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	
	VK_CHECK(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, surface));
}
void Madline::Window::test() {
	SetLayeredWindowAttributes(mHwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
}
#endif//RENDER_VULKAN