//
// Created by School on 2025/3/12.
//

#ifndef CELESTEPET_GAME_WINDOW_H
#define CELESTEPET_GAME_WINDOW_H

#include <chrono>

#include "CelestePetConsts.h"

#ifdef _WIN32
#include <Windows.h>

#include "windows/data_type.h"
#include "windows/input.h"
#endif//_WIN32

#ifdef RENDER_VULKAN
#include <vulkan/vulkan.h>
#endif

namespace Madline {
	class Window {
	public:
		explicit Window(int minFps = 5);
		~Window();
		
		void gameLoop();
		float getDeltaTime();
		
		[[nodiscard]] int getMinFps() const;
		[[nodiscard]] Rect2<int> getScreenRect() const;
		[[nodiscard]] bool isRunning() const;
		void stopRunning();
		
		Input* getInput();
		
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
		
		void test();

#ifdef RENDER_VULKAN
		void getVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) const;
#endif
	private:
		Madline::Rect2<int> screenRect;
		bool running = true;
		int minFps;
		HWND mHwnd;
		Input input;
		std::chrono::high_resolution_clock::time_point lastFrameTime;

		static LRESULT CALLBACK staticWindowProc(HWND pHwnd, unsigned int msg, WPARAM wp, LPARAM lp);
		LRESULT windowProc(unsigned int msg, WPARAM wp, LPARAM lp);
	};
}

#endif//CELESTEPET_GAME_WINDOW_H
