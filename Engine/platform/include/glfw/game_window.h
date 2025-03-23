//
// Created by School on 2025/3/12.
//

#ifndef MADLINEENGINE_GAME_WINDOW_H
#define MADLINEENGINE_GAME_WINDOW_H

#include <chrono>

#include "CelestePetConsts.h"

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

#include "glfw/data_type.h"
#include "glfw/input.h"

#ifdef RENDER_VULKAN
#include <vulkan/vulkan.h>
#endif

#ifdef _WIN64
#include <Windows.h>
#endif

namespace Madline {
	class Window {
	public:
		explicit Window(int minFps = 5);
		~Window();
		
		Window(const Window&) = delete;
		Window &operator=(const Window&) = delete;
		
		void gameLoop();
		float getDeltaTime();
		
		[[nodiscard]] GLFWwindow* getWindow() const;
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
	private:
		Madline::Rect2<int> screenRect;
		bool running = true;
		int minFps;
		GLFWwindow* mWindow;
		Input input;
		std::chrono::high_resolution_clock::time_point lastFrameTime;

		#ifdef _WIN32
		HWND windowHwnd;
		ULONG_PTR originalWindowProc;
		#endif
		
		void initWindow();
		
		static APIENTRY windowProc(unsigned int msg, WPARAM wp, LPARAM lp);
	};
}

#endif//MADLINEENGINE_GAME_WINDOW_H
