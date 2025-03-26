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
#include <imgui_impl_win32.cpp>
#include <imgui_impl_win32.h>
#include <iostream>

#include "winapi/game_window.h"


namespace Madline {
	WNDPROC workerDefaultWindowProc;
}

Madline::Window::Window(int minFps): minFps(minFps), screenRect(Rect2<int>()) { // NOLINT(*-pro-type-member-init)
	std::printf("Started Creating Window\n");
	
	initWindow();
	
	lastFrameTime = std::chrono::high_resolution_clock::now();
	
	std::printf("Finished Creating Window\n");
}

void Madline::Window::initWindow() {
	std::printf("Started Creating Window\n");
	WNDCLASS wpWndCls = {sizeof(WNDCLASS)};
	WNDCLASS inpWndCls = {sizeof(WNDCLASS)};
	
	if (!GetClassInfo(GetModuleHandle(nullptr), WALLPAPER_CLASS_NAME, &wpWndCls)) {
		wpWndCls.hCursor       = LoadCursor(nullptr, IDC_ARROW);
		wpWndCls.hInstance     = GetModuleHandle(nullptr);
		wpWndCls.lpszClassName = WALLPAPER_CLASS_NAME;
		wpWndCls.style         = CS_DBLCLKS;
		wpWndCls.lpfnWndProc   = &staticMainWindowProc;
		wpWndCls.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		
		assert(RegisterClass(&wpWndCls));
	}
	
	if (!GetClassInfo(GetModuleHandle(nullptr), INPUT_CLASS_NAME, &inpWndCls)) {
		inpWndCls.hCursor       = LoadCursor(nullptr, IDC_ARROW);
		inpWndCls.hInstance     = GetModuleHandle(nullptr);
		inpWndCls.lpszClassName = INPUT_CLASS_NAME;
		inpWndCls.style         = CS_DBLCLKS;
		inpWndCls.lpfnWndProc   = &staticInputWindowProc;
		inpWndCls.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		
		assert(RegisterClass(&inpWndCls));
	}

	DEVMODEA devMode = {};
	devMode.dmSize = sizeof(DEVMODEA);
	
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
	
	screenRect = Rect2<int>(static_cast<int>(devMode.dmPelsWidth), static_cast<int>(devMode.dmPelsHeight));
	
	mHwnd = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        wpWndCls.lpszClassName,
        WINDOW_NAME,
        WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS,
        0,
        0,
        screenRect.getWidth(),
        screenRect.getHeight(),
        nullptr, nullptr, GetModuleHandle(nullptr), this
	);
	
	inputHwnd = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        inpWndCls.lpszClassName,
        WINDOW_NAME,
        WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS,
        0,
        0,
        screenRect.getWidth(),
        screenRect.getHeight(),
        nullptr, nullptr, GetModuleHandle(nullptr), this
	);
	
//	SetLayeredWindowAttributes(mHwnd, RGB(0, 0, 0), 0, LWA_COLORKEY); // Transparent color key

	HWND wallpaperHwnd = getDesktopWallpaper();
	
	HWND topOfWpHwnd = getTopOfWallpaper();
	
	if (SetParent(mHwnd, wallpaperHwnd) == nullptr)
		throw std::runtime_error("Cannot find wallpaper handle");
	
	if (SetParent(inputHwnd, topOfWpHwnd) == nullptr)
		throw std::runtime_error("Cannot find top of wallpaper");
	
	SetWindowPos(mHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(inputHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, nullptr, SPIF_UPDATEINIFILE);
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

//	std::printf("foreground window handle: %p\n", GetForegroundWindow());
}

LRESULT CALLBACK Madline::Window::inputWindowProc(unsigned int msg, WPARAM wp, LPARAM lp) {
	LRESULT rez = 0;
	
	bool pressed = false;
	
	switch (msg) {
		case WM_CLOSE: {
			running = false;
			break;
		}
		
		case WM_DESTROY: {
			PostQuitMessage(0);
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
	 
		case WM_SETFOCUS: {
			setFocused(true);
			break;
		}
		
		case WM_KILLFOCUS: {
			setFocused(false);
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
		
		case WM_COMMAND: {
			// User selected the quit button
			switch (LOWORD(wp)) {
				case ID_EXIT: {
					delete (this);
					break;
				}
			}
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
		
		case WM_MENUOPEN: {
			// User opened the menu
			if (LOWORD(lp) == WM_CONTEXTMENU)
				showContextMenu({LOWORD(wp), HIWORD(wp)});
			break;
		}
		
		default: {
			rez = DefWindowProc(mHwnd, msg, wp, lp);
			break;
		}
	}
	
	return rez;
}

LRESULT CALLBACK Madline::Window::mainWindowProc(unsigned int msg, WPARAM wp, LPARAM lp) {
	LRESULT rez = 0;
    
	switch (msg) {
	    case WM_CLOSE: {
		    running = false;
		    break;
	    }
		
		case WM_DESTROY: {
			PostQuitMessage(0);
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
	surfaceCreateInfo.hwnd = mHwnd;
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	
	VK_CHECK(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, surface));
}
#endif//RENDER_VULKAN

BOOL Madline::Window::addTrayIcon(HWND hwnd)
{
	NOTIFYICONDATA nid   = { sizeof(nid) };
	nid.hWnd             = hwnd;
	nid.uID              = 1;
	nid.uFlags           = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP;
	nid.uCallbackMessage = WM_MENUOPEN;
	nid.hIcon            = LoadIcon(GetModuleHandle(nullptr), IDI_INFORMATION);
	lstrcpy(nid.szTip, "My Tray Icon");
	
	// Spawns the icon
	Shell_NotifyIcon(NIM_ADD, &nid);

	nid.uVersion = NOTIFYICON_VERSION_4;
	return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

void Madline::Window::showContextMenu(POINT pt) {
	HMENU hMenu = LoadMenu(GetModuleHandle(nullptr), IDI_INFORMATION);
	if (hMenu == nullptr) return;
	
	HMENU hSubMenu = GetSubMenu(hMenu, 0);
	if (hSubMenu == nullptr) {
		DestroyMenu(hMenu);
		return;
	}

	// The window must be in the foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
	SetForegroundWindow(mHwnd);

	// Drop alignment
	const int uFlags = TPM_RIGHTBUTTON | (GetSystemMetrics(SM_MENUDROPALIGNMENT) ? TPM_RIGHTALIGN : TPM_LEFTALIGN);

	TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, mHwnd, nullptr);

	DestroyMenu(hMenu);
}

LRESULT CALLBACK Madline::Window::staticMainWindowProc(HWND pHwnd, unsigned int msg, WPARAM wp, LPARAM lp) {
	if (ImGui_ImplWin32_WndProcHandler(pHwnd, msg, wp, lp))
		return true;
	
	Window* self;
	
	if (msg == WM_NCCREATE)	{
		if (addTrayIcon(pHwnd) == 0) {
			DestroyWindow(pHwnd);
			throw std::runtime_error("Cannot create system tray icon");
		}
		
		auto *cs = (CREATESTRUCT*) lp;
		self = static_cast<Window*>(cs->lpCreateParams);
		self->mHwnd = pHwnd;
		SetLastError(0);
		if (SetWindowLongPtr(pHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self)) == 0) {
			if (GetLastError() != 0)
				return false;
		}
	} else
		self = reinterpret_cast<Window*>(GetWindowLongPtr(pHwnd, GWLP_USERDATA));
	
	if (self)
		return self->mainWindowProc(msg, wp, lp);

	return DefWindowProc(pHwnd, msg, wp, lp);
}

BOOL CALLBACK Madline::Window::enumWindowsProcDesktopWallpaper(HWND hwnd, LPARAM lParam) {
	HWND p = FindWindowEx(hwnd, nullptr, "SHELLDLL_DefView", nullptr);
	HWND* ret = reinterpret_cast<HWND*>(lParam);
	
	if (p != nullptr) {
		*ret = FindWindowEx(nullptr, hwnd, "WorkerW", nullptr);
	}
	return true;
}

HWND Madline::Window::getDesktopWallpaper() {
	HWND progman = FindWindow("Progman", nullptr);
	
	SendMessageTimeout(
        progman,
        0x052C,
        0xD,
        0x1,
        SMTO_NORMAL,
        1000,
        nullptr
	);

	HWND wallpaperHwnd = nullptr;
	EnumWindows(enumWindowsProcDesktopWallpaper, reinterpret_cast<LPARAM>(&wallpaperHwnd));
	
	if (wallpaperHwnd == nullptr) {
		wallpaperHwnd = FindWindowEx(progman, nullptr, "WorkerW", nullptr);
	}
	
	std::cout << "Wallpaper handle: " << wallpaperHwnd << std::endl;
	
	if (wallpaperHwnd == nullptr) {
		throw std::runtime_error("Could not find wallpaper");
	}
	
	return wallpaperHwnd;
}

BOOL CALLBACK Madline::Window::enumWindowsProcTopOfWallpaper(HWND hwnd, LPARAM lParam) {
	HWND defView = FindWindowEx(hwnd, nullptr, "SHELLDLL_DefView", nullptr);
	HWND* ret = reinterpret_cast<HWND*>(lParam);
	
	if (defView != nullptr) {
		HWND folderView = FindWindowEx(defView, nullptr, "SysListView32", nullptr);
		if (folderView != nullptr)
			*ret = folderView;
	}
	return true;
}

HWND Madline::Window::getTopOfWallpaper() {
	HWND topHwnd = nullptr;
	EnumWindows(enumWindowsProcTopOfWallpaper, reinterpret_cast<LPARAM>(&topHwnd));
	
	if (topHwnd == nullptr) {
		throw std::runtime_error("Could not find top of wallpaper");
	}
	
	std::cout << "Top of wallpaper handle: " << topHwnd << std::endl;
	
	return topHwnd;
}

