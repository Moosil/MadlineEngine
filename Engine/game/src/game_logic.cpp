//
// Created by School on 2025/3/11.
//
#include "game_logic.h"

struct GameData {
	int frameCounter = 0;
	float timeCounter = 0;
};
static GameData data;

bool Madline::EngineDebug::initGameplay() {
	data = {};
	printf("Madline initialised in debug mode\n");
	return true;
}

// when implementing your own process function, make sure to remove the Window& parameter
// and instead pass in each part of window, for example the input, seperately to reduce the scope
bool Madline::EngineDebug::process(float delta, Window& gameWindow) {
	data.frameCounter += 1;
	data.timeCounter += delta;
	if (data.timeCounter >= 1.0) {
		data.timeCounter -= 1.0;
		std::printf("FPS: %i ~ Debug mode\n", data.frameCounter);
		data.frameCounter = 0;
	}
	
	if (gameWindow.isButtonJustPressed(Button::A))
		std::printf("Hello world\n");
	
	if (gameWindow.isLmbJustPressed())
		std::printf("LMB\n");
	
	return true;
}

void Madline::EngineDebug::onCloseGame() {
	printf("Madline closed in debug mode");
}