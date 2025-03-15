//
// Created by School on 2025/3/11.
//

#ifndef CELESTEPET_GAME_LOGIC_H
#define CELESTEPET_GAME_LOGIC_H

#include <iostream>

#ifdef _WIN32
#include "windows/game_window.h"
#endif//_WIN32

namespace Game {
	bool initGameplay();

	bool process(float delta, Window gameWindow);

	void onCloseGame();
}

#endif//CELESTEPET_GAME_LOGIC_H
