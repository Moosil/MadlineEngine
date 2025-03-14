//
// Created by School on 2025/3/12.
//

#ifndef CELESTEPET_GAME_WINDOW_H
#define CELESTEPET_GAME_WINDOW_H

#include <Windows.h>
#include <cassert>
#include <exception>
#include <iostream>
#include <vector>
#include <input.h>

namespace Game {
	class Window {
	public:
		bool running = true;

		Input input;

		int minFps;

		Rect2<int> screenRect;

		explicit Window(int minFps = 5);

		HWND getHwnd();

		void set_focused(bool val);

		[[nodiscard]] bool is_focused() const;

		bool is_button_pressed(int buttonIndex);

		bool is_button_triggered(int buttonIndex);

		bool is_button_held(int buttonIndex);

		bool is_button_released(int buttonIndex);

		[[nodiscard]] bool is_lmb_pressed() const;

		[[nodiscard]] bool is_lmb_triggered() const;

		[[nodiscard]] bool is_lmb_held() const;

		[[nodiscard]] bool is_lmb_released() const;

		[[nodiscard]] bool is_rmb_pressed() const;

		[[nodiscard]] bool is_rmb_triggered() const;

		[[nodiscard]] bool is_rmb_held() const;

		[[nodiscard]] bool is_rmb_released() const;

		std::vector<int> get_buttons_pressed();

		std::vector<int> get_buttons_triggered();

		std::vector<int> get_buttons_held();

		std::vector<int> get_buttons_released();

	private:
		HWND m_hwnd;

		static LRESULT CALLBACK staticWindowProc(HWND p_hwnd, unsigned int msg, WPARAM wp, LPARAM lp);

		LRESULT windowProc(unsigned int msg, WPARAM wp, LPARAM lp);
	};
}

#endif//CELESTEPET_GAME_WINDOW_H
