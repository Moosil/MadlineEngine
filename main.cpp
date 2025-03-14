#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <iostream>
#include <game_logic.h>

void errorCallback(int error, const char* description);

Game::Window gameWindow{};

int main() {
	glfwSetErrorCallback(errorCallback);
	
	if (!Game::initGameplay()) { return -1; }
	
	gameWindow.mainLoop();

	glfwDestroyWindow(gameWindow.m_window);
	glfwTerminate();
	
	Game::onCloseGame();
	return 0;
}

void errorCallback(int error, const char* description) {
	std::cerr << "GLFW error " << error << ": " << description << std::endl;
}