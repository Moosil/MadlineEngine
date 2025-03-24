//
// Created by School on 2025/3/12.
//

#include <format>

#include <imgui.h>
#include <imgui_impl_glfw.h>

#include "CelestePetConsts.h"

#include "glfw/game_window.h"

#ifdef RENDER_VULKAN
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

#include "vulkan/vk_types.h"
#endif

#ifdef _WIN64
#define GLFW_EXPOSE_NATIVE_WIN32
#include <dwmapi.h>
#endif

#include <GLFW/glfw3native.h>

namespace Madline {
	WNDPROC originalWindowProc;
}


Madline::Window::Window(int minFps): minFps(minFps), screenRect(Rect2<int>()) { // NOLINT(*-pro-type-member-init)
	std::printf("Started Creating Window\n");
	
	initWindow();
	
	lastFrameTime = std::chrono::high_resolution_clock::now();
	
	std::printf("Finished Creating Window\n");
}

void Madline::Window::initWindow() {
	glfwInit();
	
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
//	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
	glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
	
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	
	screenRect = Rect2<int>(mode->width, mode->height);

	mWindow = glfwCreateWindow(mode->width, mode->height, GAME_NAME, monitor, nullptr);
	
	windowHwnd = glfwGetWin32Window(mWindow);
	
	long long exStyles = GetWindowLongPtr(windowHwnd, GWL_EXSTYLE);
	
	exStyles &= ~WS_EX_ACCEPTFILES;
//	exStyles &= ~WS_EX_APPWINDOW;
	exStyles &= ~WS_EX_TOPMOST;
//	exStyles &= ~WS_EX_WINDOWEDGE;
	
//	exStyles |= WS_EX_NOACTIVATE;
//	exStyles |= WS_EX_TOOLWINDOW;
	
	SetWindowLongPtr(windowHwnd, GWL_EXSTYLE, exStyles);
	
	originalWindowProc = reinterpret_cast<WNDPROC>(GetClassLongPtr(windowHwnd, GCLP_WNDPROC));
	
	SetWindowLongPtr(windowHwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(windowProc));
	
	MARGINS margins = { -1 };
	DwmExtendFrameIntoClientArea(windowHwnd, &margins); // Make the window background transparent
	SetLayeredWindowAttributes(windowHwnd, RGB(0, 0, 0), 0, LWA_COLORKEY); // Transparent color key
	
	HWND progmanHwnd = FindWindow("progman", nullptr);
	
	
	if (progmanHwnd != nullptr) {
		HWND defView = FindWindowEx(progmanHwnd, nullptr, "SHELLDLL_DefView", nullptr);
		
		if (defView == nullptr) {
			HWND desktopHwnd = GetDesktopWindow();
			HWND workerW{};
			do {
				workerW = FindWindowEx(desktopHwnd, workerW, "WorkerW", nullptr);
				defView = FindWindowEx(workerW, nullptr, "SHELLDLL_DefView", nullptr);
			} while (!defView && workerW);
		}
		
		if (defView != nullptr) {
			HWND sysListView32 = FindWindowEx(defView, nullptr, "SysListView32", nullptr);
			SetParent(windowHwnd, sysListView32);
			RECT rect{};
			GetWindowRect(sysListView32, &rect);
			SetWindowPos(windowHwnd, HWND_BOTTOM, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
		} else
			throw std::runtime_error("Cannot find defview");
	} else
		throw std::runtime_error("Cannot find program manager");
}

Madline::Window::~Window() {
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

float Madline::Window::getDeltaTime() {
	auto now = std::chrono::high_resolution_clock::now();
	
	long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now - lastFrameTime).count();
	float rez = (float) microseconds / 1000000.0f;
	lastFrameTime = std::chrono::high_resolution_clock::now();
	return rez;
}

void Madline::Window::gameLoop() {
	glfwPollEvents();
	if (glfwWindowShouldClose(mWindow)) {
		running = false;
	}
	
//	POINT point;
//	GetCursorPos(&point);
//	ScreenToClient(mHwnd, &point);
//	input.cursorPos = Madline::vec2iFromPoint(point);
}

#ifdef _WIN32
	LRESULT CALLBACK Madline::Window::windowProc(HWND hwnd, unsigned int msg, WPARAM wp, LPARAM lp) {
		LRESULT rez = 0;
	
		switch (msg) {
		    case WM_SIZE: {
			    // Handle size changes if needed
			    break;
		    }
//		    case WM_ERASEBKGND: {
//			    return 1; // Prevent background erasing (for transparency)
//			}
//			case WM_WINDOWPOSCHANGING: {
//				auto *pos = (WINDOWPOS*) lp;
//
//				if (pos->x == -32000) {
//					// Set the flags to prevent this and "survive" to the desktop toggle
//					pos->flags |= SWP_NOMOVE | SWP_NOSIZE;
//				}
//
//				pos->hwndInsertAfter = HWND_BOTTOM;
//				break;
//			}
	
//			case WM_NCHITTEST: {
//				rez = HTNOWHERE;
//				break;
//			}
			
			default: {
				rez = CallWindowProc(originalWindowProc, hwnd, msg, wp, lp);
				break;
			}
		}
		
		return rez;
	}
#endif

void Madline::Window::getRect() const {
	RECT rect{};
	GetWindowRect(windowHwnd, &rect);
	std::printf("%ld %ld\n", rect.right-rect.left, rect.bottom-rect.top);
}

GLFWwindow* Madline::Window::getWindow() const {
	return mWindow;
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