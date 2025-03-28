//
// Created by School on 2025/3/12.
//

#ifndef MADLINEENGINE_GAME_WINDOW_H
#define MADLINEENGINE_GAME_WINDOW_H

#include <chrono>

#include "CelestePetConsts.h"

#include <Windows.h>
#include <imgui_impl_win32.h>

#include "winapi/data_type.h"
#include "winapi/input.h"

#ifdef RENDER_VULKAN
#include <vulkan/vulkan.h>
#endif

#ifdef _WIN64
#include <Windows.h>
#include <unordered_map>

#endif

#define WM_MENUOPEN (WM_APP + 1)
#define ID_EXIT 40004

#define MAIN_WINDOW "mainWnd"
#define INPUT_WINDOW "inputWnd"

namespace Madline {
	class Window {
	public:
		explicit Window(int minFps = 5);
		~Window();
		
		Window(const Window&) = delete;
		Window &operator=(const Window&) = delete;
		
		void gameLoop();
		float getDeltaTime();
		
		[[nodiscard]] HWND getHwnd() const;
		[[nodiscard]] int getMinFps() const;
		[[nodiscard]] Rect2<int> getWindowRect() const;
		[[nodiscard]] bool isRunning() const;
		void stopRunning();
		
		Input* getInput();
		
		void getRect() const;
		
		void setFocused(bool val);
		[[nodiscard]] bool isFocused() const;
		
		[[nodiscard]] bool isButtonPressed(int buttonIndex) const;
		[[nodiscard]] bool isButtonTriggered(int buttonIndex) const;
		[[nodiscard]] bool isButtonHeld(int buttonIndex) const;
		[[nodiscard]] bool isButtonReleased(int buttonIndex) const;

		[[nodiscard]] bool isLmbPressed() const;
		[[nodiscard]] bool isLmbTriggered() const;
		[[nodiscard]] bool isLmbHeld() const;
		[[nodiscard]] bool isLmbReleased() const;

		[[nodiscard]] bool isRmbPressed() const;
		[[nodiscard]] bool isRmbTriggered() const;
		[[nodiscard]] bool isRmbHeld() const;
		[[nodiscard]] bool isRmbReleased() const;

		[[nodiscard]] std::vector<int> getButtonsPressed() const;
		[[nodiscard]] std::vector<int> getButtonsTriggered() const;
		[[nodiscard]] std::vector<int> getButtonsHeld() const;
		[[nodiscard]] std::vector<int> getButtonsReleased() const;
		
		#ifdef RENDER_VULKAN
			std::unordered_map<std::string, VkSurfaceKHR> getVulkanSurfaces(VkInstance instance) const;
		#endif
	private:
		Madline::Rect2<int> screenRect;
		bool running = true;
		int minFps;
		HWND mHwnd;
		HWND inputHwnd;
		Input input;
		std::chrono::high_resolution_clock::time_point lastFrameTime;
		
		void initWindow();
		static WINBOOL addTrayIcon(HWND hwnd);
		
		LRESULT CALLBACK inputWndProc(unsigned int msg, WPARAM wp, LPARAM lp);
		LRESULT CALLBACK mainWndProc(unsigned int msg, WPARAM wp, LPARAM lp);
		
		static LRESULT CALLBACK staticMainWndProc(HWND pHwnd, unsigned int msg, WPARAM wp, LPARAM lp);
		static LRESULT CALLBACK staticInputWndProc(HWND hwnd, unsigned int msg, WPARAM wp, LPARAM lp);
		
		void showContextMenu(POINT pt);
		
		static BOOL CALLBACK enumWindowsProcDesktopWallpaper(HWND hwnd, LPARAM lParam);
		static HWND getDesktopWallpaper();
		static BOOL CALLBACK enumWindowsProcTopOfWallpaper(HWND hwnd, LPARAM lParam);
        static HWND getTopOfWallpaper();
	};
}

#endif//MADLINEENGINE_GAME_WINDOW_H
