//
// Created by School on 2025/3/12.
//

#ifndef CELESTEPET_GAME_WINDOW_H
#define CELESTEPET_GAME_WINDOW_H

#include <chrono>

#include "CelestePetConsts.h"

#ifdef _WIN32
#include "input.h"
#include "data_type.h"
#endif//_WIN32

namespace Game {
	class Window {
	public:
		explicit Window(int minFps = 5);
		
		void gameLoop();
		float getDeltaTime();
		
		[[nodiscard]] int getMinFps() const;
		
		[[nodiscard]] bool is_running() const;
		void stop_running();
		
		Input* getInput();
		
		void set_focused(bool val);
		[[nodiscard]] bool is_focused() const;
		
		[[nodiscard]] bool is_button_pressed(int buttonIndex) const;
		[[nodiscard]] bool is_button_triggered(int buttonIndex) const;
		[[nodiscard]] bool is_button_held(int buttonIndex) const;
		[[nodiscard]] bool is_button_released(int buttonIndex) const;

		[[nodiscard]] bool is_lmb_pressed() const;
		[[nodiscard]] bool is_lmb_triggered() const;
		[[nodiscard]] bool is_lmb_held() const;
		[[nodiscard]] bool is_lmb_released() const;

		[[nodiscard]] bool is_rmb_pressed() const;
		[[nodiscard]] bool is_rmb_triggered() const;
		[[nodiscard]] bool is_rmb_held() const;
		[[nodiscard]] bool is_rmb_released() const;

		[[nodiscard]] std::vector<int> get_buttons_pressed() const;
		[[nodiscard]] std::vector<int> get_buttons_triggered() const;
		[[nodiscard]] std::vector<int> get_buttons_held() const;
		[[nodiscard]] std::vector<int> get_buttons_released() const;
	private:
		bool running = true;
		int minFps;
		HWND m_hwnd;
		Input input;
		std::chrono::high_resolution_clock::time_point lastFrameTime;
		Rect2<int> screenRect;

		static LRESULT CALLBACK staticWindowProc(HWND p_hwnd, unsigned int msg, WPARAM wp, LPARAM lp);
		LRESULT windowProc(unsigned int msg, WPARAM wp, LPARAM lp);
	};
}

#endif//CELESTEPET_GAME_WINDOW_H
