//
// Created by School on 2025/3/12.
//

#include <format>

#include "CelestePetConsts.h"

#ifdef RENDER_VULKAN
#include <VkBootstrap.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include "vulkan/vk_types.h"
#endif

#ifdef _WIN64
#include <dwmapi.h>
#endif

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_win32.cpp>

#include "winapi/game_window.h"

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
	std::printf("Started Creating Window\n");
	WNDCLASS wc = {sizeof(WNDCLASS)};
	const std::string className = CLASS_NAME;
	
	if (!GetClassInfo(GetModuleHandle(nullptr), className.c_str(), &wc)) {
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hInstance = GetModuleHandle(nullptr);
		wc.lpszClassName = className.c_str();
		wc.style = CS_DBLCLKS;
		wc.lpfnWndProc = &staticWindowProc;
		wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		
		assert(RegisterClass(&wc));
	}

	DEVMODEA devMode = {};
	devMode.dmSize = sizeof(DEVMODEA);
	
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
	
	screenRect = Rect2<int>(static_cast<int>(devMode.dmPelsWidth), static_cast<int>(devMode.dmPelsHeight));
	
	mHwnd = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        wc.lpszClassName,
        WINDOW_NAME,
        WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS,
        0,
        0,
        screenRect.getWidth(),
        screenRect.getHeight(),
        nullptr, nullptr, GetModuleHandle(nullptr), this
	);
	
//	SetLayeredWindowAttributes(mHwnd, RGB(0, 0, 0), 0, LWA_COLORKEY); // Transparent color key
	
//	HWND wallpaper = getDesktopWallpaper();
//
//	HWND defView = FindWindowEx(wallpaper, nullptr, "SHELLDLL_DefView", nullptr);
//	if (defView != nullptr) {
//		HWND sysListView32 = FindWindowEx(defView, nullptr, "SysListView32", nullptr);
//		SetParent(mHwnd, sysListView32);
//		SetWindowPos(mHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
//	} else
//		throw std::runtime_error("Cannot find defview");
}

Madline::Window::~Window() {
	DestroyWindow(mHwnd);
	mHwnd = nullptr;
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
	input.cursorPos = static_cast<Madline::Vec2i>(point);
	
//	RECT rect = {};
//	GetWindowRect(mHwnd, &rect);
//	screenRect = static_cast<Rect2<int>>(rect);
}

LRESULT CALLBACK Madline::Window::windowProc(unsigned int msg, WPARAM wp, LPARAM lp) {
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
		   
	    case WM_SIZE: {
		    // Handle size changes if needed
		    break;
	    }
	    
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

//	    case WM_NCHITTEST: {
//		    rez = HTNOWHERE;
//		    break;
//	    }
	
	    default: {
		    rez = DefWindowProc(mHwnd, msg, wp, lp);
		    break;
	    }
	}
	
	return rez;
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
	surfaceCreateInfo.hwnd = getDesktopWallpaper();
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	
	VK_CHECK(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, surface));
}
#endif//RENDER_VULKAN

LRESULT CALLBACK Madline::Window::staticWindowProc(HWND pHwnd, unsigned int msg, WPARAM wp, LPARAM lp) {
	if (ImGui_ImplWin32_WndProcHandler(pHwnd, msg, wp, lp))
		return true;
	
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

HWND Madline::Window::getDesktopWallpaper() {
	HWND progman = FindWindow("Progman", nullptr);

	ULONG_PTR result = 0;
	SendMessageTimeout(
        progman,
        0x052C,
        NULL,
        NULL,
        SMTO_NORMAL,
        1000,
        &result
	);

	HWND defView = nullptr;

	EnumWindows([](HWND topHandle, LPARAM tmp) -> BOOL {
			HWND p = FindWindowEx(topHandle,
				nullptr,
				"SHELLDLL_DefView",
				nullptr
			);
		    if (p != nullptr)
			    *((HWND *) tmp) = FindWindowEx(
		            nullptr,
		            topHandle,
		            "WorkerW",
		            nullptr
				);


	        return true;
        },
        (LPARAM)(&defView)
    );
	
	return defView;
//	HWND progmanHwnd = FindWindow("Progman", nullptr);
//
//	if (progmanHwnd != nullptr) {
//		HWND defView = FindWindowEx(progmanHwnd, nullptr, "SHELLDLL_DefView", nullptr);
//
//		if (defView == nullptr) {
//			HWND desktopHwnd = GetDesktopWindow();
//			HWND workerW{};
//			unsigned int counter = 0;
//			do {
//				workerW = FindWindowEx(desktopHwnd, workerW, "WorkerW", nullptr);
//				defView = FindWindowEx(workerW, nullptr, "SHELLDLL_DefView", nullptr);
//				counter++;
//			} while (!defView && workerW && counter < 10000);
//			if (counter >= 10000)
//				throw std::runtime_error("Cannot find program manager (WorkerW and SHELLDLL_DefView)");
//			else
//				return workerW;
//		} else
//			return progmanHwnd;
//
//		// for putting window on top of desktop completely
////		if (defView != nullptr) {
////			HWND sysListView32 = FindWindowEx(defView, nullptr, "SysListView32", nullptr);
////		} else
////			throw std::runtime_error("Cannot find defview");
//	} else
//		throw std::runtime_error("Cannot find program manager (progman)");
}