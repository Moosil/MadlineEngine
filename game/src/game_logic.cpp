//
// Created by School on 2025/3/11.
//
#include "game_logic.h"

struct GameData {
	int frameCounter = 0;
	float timeCounter = 0;
};
static GameData data;

bool Game::initGameplay() {
	data = {};
	return true;
}

bool Game::process(float delta, Window gameWindow) {
	data.frameCounter += 1;
	data.timeCounter += delta;
	if (data.timeCounter >= 0.01) {
		data.timeCounter -= 0.01;
		std::cout << "FPS: " << data.frameCounter * 100 << std::endl;
		data.frameCounter = 0;
	}
	
	return true;
}

void Game::onCloseGame() {

}