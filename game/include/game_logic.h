//
// Created by School on 2025/3/11.
//

#ifndef CELESTEPET_GAME_LOGIC_H
#define CELESTEPET_GAME_LOGIC_H

#include <input.h>
#include <game_window.h>

namespace Game {
	bool initGameplay();

	bool process(float delta);

	void onCloseGame();
}

#endif//CELESTEPET_GAME_LOGIC_H
