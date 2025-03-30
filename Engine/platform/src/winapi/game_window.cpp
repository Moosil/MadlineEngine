//
// Created by School on 2025/3/12.
//

#include <format>

#include "MadlineEngineDefines.h"

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
	Window::Window(int minFps): minFps(minFps), screenRect(Rect2<int>()) {
		std::printf("Started Creating Window\n");
	
		initWindow();
		
		createdWindows.push_back(this);
		
		SetHooks();
	
		lastFrameTime = std::chrono::high_resolution_clock::now();
	
		std::printf("Finished Creating Window\n");
	}
	
	void Window::initWindow() {
		std::printf("Started Creating Window\n");
	
		DEVMODEA devMode = {};
		devMode.dmSize = sizeof(DEVMODEA);
		EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
		screenRect = Rect2<int>(static_cast<int>(devMode.dmPelsWidth), static_cast<int>(devMode.dmPelsHeight));
	
		WNDCLASS wpWndCls{};
		if (!GetClassInfo(GetModuleHandle(nullptr), WALLPAPER_CLASS_NAME, &wpWndCls)) {
			wpWndCls.hCursor       = LoadCursor(nullptr, IDC_ARROW);
			wpWndCls.hInstance     = GetModuleHandle(nullptr);
			wpWndCls.lpszClassName = WALLPAPER_CLASS_NAME;
			wpWndCls.style         = CS_DBLCLKS;
			wpWndCls.lpfnWndProc   = &staticMainWndProc;
			wpWndCls.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		
			assert(RegisterClass(&wpWndCls));
		}
		mHwnd = CreateWindowEx(
		        WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_NOACTIVATE,
		        wpWndCls.lpszClassName,
		        WINDOW_NAME,
		        WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS,
		        0,
		        0,
		        screenRect.getWidth(),
		        screenRect.getHeight(),
		        nullptr, nullptr, GetModuleHandle(nullptr), this
		);
	
		SetLayeredWindowAttributes(mHwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
	
		HWND wallpaperHwnd = getDesktopWallpaper();
	
		if (SetParent(mHwnd, wallpaperHwnd) == nullptr)
			throw std::runtime_error("Cannot find wallpaper handle");
	
		SetWindowPos(mHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW);

		SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, nullptr, SPIF_UPDATEINIFILE);
	}

	Window::~Window() {
		DestroyWindow(mHwnd);
		mHwnd = nullptr;
		RemoveHooks();
	}

	float Window::getDeltaTime() {
		auto now = std::chrono::high_resolution_clock::now();
	
		long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now - lastFrameTime).count();
		float rez = (float) microseconds / 1000000.0f;
		lastFrameTime = std::chrono::high_resolution_clock::now();
		return rez;
	}

	void Window::gameLoop() {
		MSG msg = {};
		while (PeekMessage(&msg, mHwnd, 0, 0, PM_REMOVE) > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			std::printf("main received %u\n", msg.message);
		}
	
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(mHwnd, &point);
		input.cursorPos = static_cast<Vec2i>(point);
	}

	LRESULT CALLBACK Window::MouseProc(int nCode, WPARAM wp, LPARAM lp) {
		if (nCode != HC_ACTION)	{
			bool mButtonDown = (wp == WM_LBUTTONDOWN || wp == WM_RBUTTONDOWN);
			
			if (mButtonDown || wp == WM_LBUTTONUP || wp == WM_RBUTTONUP) {
				Button::MouseButton mButton = (wp == WM_LBUTTONDOWN || wp == WM_LBUTTONUP) ? Button::LMB : Button::RMB;
				for (auto currWnd : Window::createdWindows) {
					currWnd->input.setMouseButtonState(mButton, mButtonDown);
				}
			}
		}
		return CallNextHookEx(nullptr, nCode, wp, lp);
	}
	
	LRESULT CALLBACK Window::KeyboardProc(int nCode, WPARAM wp, LPARAM lp) {
		if (nCode == HC_ACTION) {
			bool keyDown = (wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN);
			
			if (keyDown || wp == WM_KEYUP || wp == WM_SYSKEYUP) {
				auto* keyInfo = (KBDLLHOOKSTRUCT*)lp;
				
				bool pressed = keyDown;

				
				for (int i = Button::A; i != Button::BUTTONS_COUNT; i++) {
					if (keyInfo->vkCode == Button::buttonValues[i]) {
						for (auto currWnd: Window::createdWindows) {
							auto bID = static_cast<Button::ButtonID>(i);
							currWnd->input.setKeyState(bID, pressed);
							if (keyInfo->flags & LLKHF_ALTDOWN) {
								currWnd->input.setKeyModifiers(bID, Button::Modifier::ALT, Button::Modifier::ALT);
							}
						}
					}
				}
			}
		}
		return CallNextHookEx(nullptr, nCode, wp, lp);
	}

	LRESULT CALLBACK Window::mainWndProc(unsigned int msg, WPARAM wp, LPARAM lp) {
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
			
			case WM_COMMAND: {
				// User selected the quit button
				switch (LOWORD(wp)) {
					case ID_EXIT: {
						delete (this);
						break;
					}
				}
			}
			
			case WM_MENUOPEN: {
				// User opened the menu
				if (LOWORD(lp) == WM_CONTEXTMENU)
					showContextMenu({LOWORD(wp), HIWORD(wp)});
				break;
			}
	
			case WM_WINDOWPOSCHANGING: {
				auto *pos = (WINDOWPOS *) lp;

				if (pos->x == -32000) {
					// Set the flags to prevent this and "survive" to the desktop toggle
					pos->flags |= SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE;
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

	HWND Window::getHwnd() const {
		return mHwnd;
	}

	int Window::getMinFps() const {
		return minFps;
	}

	Rect2<int> Window::getWindowRect() const {
		return screenRect;
	}

	bool Window::isRunning() const {
		return running;
	}
	void Window::stopRunning() {
		running = false;
	}

	Input *Window::getInput() {
		return &input;
	}

	void Window::setFocused(bool val) {
		input.focused = val;
	}

	bool Window::isFocused() const {
		return input.focused;
	}

	bool Window::isButtonJustPressed(int buttonIndex) const {
		return input.keyBoard[buttonIndex].justPressed;
	}

	bool Window::isButtonTriggered(int buttonIndex) const {
		return input.keyBoard[buttonIndex].triggered;
	}

	bool Window::isButtonHeld(int buttonIndex) const {
		return input.keyBoard[buttonIndex].held;
	}

	bool Window::isButtonJustReleased(int buttonIndex) const {
		return input.keyBoard[buttonIndex].justReleased;
	}

	bool Window::isLmbJustPressed() const {
		return input.lmb.justPressed;
	}

	bool Window::isLmbTriggered() const {
		return input.lmb.justPressed;
	}

	bool Window::isLmbHeld() const {
		return input.lmb.justPressed;
	}

	bool Window::isLmbJustReleased() const {
		return input.lmb.justPressed;
	}

	bool Window::isRmbJustPressed() const {
		return input.rmb.justPressed;
	}

	bool Window::isRmbTriggered() const {
		return input.rmb.justPressed;
	}

	bool Window::isRmbHeld() const {
		return input.rmb.justPressed;
	}

	bool Window::isRmbJustReleased() const {
		return input.rmb.justPressed;
	}

	std::vector<int> Window::getButtonsPressed() const {
		std::vector<int> pressed;
		for (int i = 0; i < Button::BUTTONS_COUNT; i++) {
			if (input.keyBoard[i].justPressed) {
				pressed.push_back(i);
			}
		}
		return pressed;
	}

	std::vector<int> Window::getButtonsTriggered() const {
		std::vector<int> triggered;
		for (int i = 0; i < Button::BUTTONS_COUNT; i++) {
			if (input.keyBoard[i].triggered) {
				triggered.push_back(i);
			}
		}
		return triggered;
	}

	std::vector<int> Window::getButtonsHeld() const {
		std::vector<int> held;
		for (int i = 0; i < Button::BUTTONS_COUNT; i++) {
			if (input.keyBoard[i].held) {
				held.push_back(i);
			}
		}
		return held;
	}

	std::vector<int> Window::getButtonsReleased() const {
		std::vector<int> released;
		for (int i = 0; i < Button::BUTTONS_COUNT; i++) {
			if (input.keyBoard[i].justReleased) {
				released.push_back(i);
			}
		}
		return released;
	}

#ifdef RENDER_VULKAN
	std::unordered_map<std::string, SurfaceInfo> Window::getVulkanSurfaces(VkInstance instance) const {
		std::unordered_map<std::string, SurfaceInfo> surfaces{};
		surfaces[MAIN_WINDOW] = SurfaceInfo(mHwnd);
		//surfaces[INPUT_WINDOW] = SurfaceInfo(inputHwnd);
	
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	
		surfaceCreateInfo.hwnd = surfaces[MAIN_WINDOW].handle;
		VK_CHECK(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surfaces[MAIN_WINDOW].surface));
	
		return surfaces;
	}
#endif//RENDER_VULKAN

	BOOL Window::addTrayIcon(HWND hwnd)
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

	void Window::showContextMenu(POINT pt) {
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

	LRESULT CALLBACK Window::staticMainWndProc(HWND pHwnd, unsigned int msg, WPARAM wp, LPARAM lp) {
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
			return self->mainWndProc(msg, wp, lp);

		return DefWindowProc(pHwnd, msg, wp, lp);
	}

	BOOL CALLBACK Window::enumWindowsProcDesktopWallpaper(HWND hwnd, LPARAM lParam) {
		HWND p = FindWindowEx(hwnd, nullptr, "SHELLDLL_DefView", nullptr);
		HWND* ret = reinterpret_cast<HWND*>(lParam);
	
		if (p != nullptr) {
			*ret = FindWindowEx(nullptr, hwnd, "WorkerW", nullptr);
		}
		return true;
	}

	HWND Window::getDesktopWallpaper() {
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

	BOOL CALLBACK Window::enumWindowsProcTopOfWallpaper(HWND hwnd, LPARAM lParam) {
		HWND defView = FindWindowEx(hwnd, nullptr, "SHELLDLL_DefView", nullptr);
		HWND* ret = reinterpret_cast<HWND*>(lParam);
	
		if (defView != nullptr) {
			HWND folderView = FindWindowEx(defView, nullptr, "SysListView32", nullptr);
			if (folderView != nullptr)
				*ret = folderView;
		}
		return true;
	}

	HWND Window::getTopOfWallpaper() {
		HWND topHwnd = nullptr;
		EnumWindows(enumWindowsProcTopOfWallpaper, reinterpret_cast<LPARAM>(&topHwnd));
	
		if (topHwnd == nullptr)
			throw std::runtime_error("Could not find top of wallpaper");
	
		std::cout << "Top of wallpaper handle: " << topHwnd << std::endl;
	
		return topHwnd;
	}
}



