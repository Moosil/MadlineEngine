#include "game_engine.h"

int Madline::startGame(Madline::Engine& gameEngine) {
	Madline::Window gameWindow{};
	
	if (!gameEngine.initGameplay()) {
		gameWindow.stopRunning();
		return -1;
	}
	
	Madline::GraphicsEngine graphicsEngine{ gameWindow };

	while (gameWindow.isRunning()) {
		
		
		gameWindow.gameLoop();
		float dt = gameWindow.getDeltaTime();
		float adt = (dt > 1.f / (float) gameWindow.getMinFps()) ? 1.f / (float) gameWindow.getMinFps() : dt;// augmented delta time

		if (!gameEngine.process(adt, gameWindow)) {
			gameWindow.stopRunning();
		}
		
		graphicsEngine.drawLoop();

		if (!gameWindow.isFocused()) {
			resetInput(gameWindow.getInput());
		}

		processInputAfter(gameWindow.getInput());
	}

	gameEngine.onCloseGame();
	graphicsEngine.cleanup();

	return 0;
}